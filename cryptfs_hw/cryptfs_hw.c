/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cryptfs_hw.h>
#include <stdlib.h>
#include <string.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include "cutils/log.h"
#include "cutils/properties.h"
#include "cutils/android_reboot.h"
#include "keymaster_common.h"
#include "hardware.h"


// When device comes up or when user tries to change the password, user can
// try wrong password upto a certain number of times. If user enters wrong
// password further, HW would wipe all disk encryption related crypto data
// and would return an error ERR_MAX_PASSWORD_ATTEMPTS to VOLD. VOLD would
// wipe userdata partition once this error is received.
#define ERR_MAX_PASSWORD_ATTEMPTS -10
#define QSEECOM_DISK_ENCRYPTION 1
#define QSEECOM_UFS_ICE_DISK_ENCRYPTION 3
#define QSEECOM_SDCC_ICE_DISK_ENCRYPTION 4
#define MAX_PASSWORD_LEN 32
#define QTI_ICE_STORAGE_UFS 1
#define QTI_ICE_STORAGE_SDCC 2

/* Operations that be performed on HW based device encryption key */
#define SET_HW_DISK_ENC_KEY 1
#define UPDATE_HW_DISK_ENC_KEY 2
#define MAX_DEVICE_ID_LENGTH 4 /* 4 = 3 (MAX_SOC_ID_LENGTH) + 1 */

static unsigned int cpu_id[] = {
	239, /* MSM8939 SOC ID */
};

#define KEYMASTER_PARTITION_NAME "/dev/block/bootdevice/by-name/keymaster"

static int loaded_library = 0;
static int (*qseecom_create_key)(int, void*);
static int (*qseecom_update_key)(int, void*, void*);
static int (*qseecom_wipe_key)(int);

static int map_usage(int usage)
{
    int storage_type = is_ice_enabled();
    if (usage == QSEECOM_DISK_ENCRYPTION) {
        if (storage_type == QTI_ICE_STORAGE_UFS) {
            return QSEECOM_UFS_ICE_DISK_ENCRYPTION;
        }
        else if (storage_type == QTI_ICE_STORAGE_SDCC) {
            return QSEECOM_SDCC_ICE_DISK_ENCRYPTION ;
        }
    }
    return usage;
}

static unsigned char* get_tmp_passwd(const char* passwd)
{
    int passwd_len = 0;
    unsigned char * tmp_passwd = NULL;
    if(passwd) {
        tmp_passwd = (unsigned char*)malloc(MAX_PASSWORD_LEN);
        if(tmp_passwd) {
            memset(tmp_passwd, 0, MAX_PASSWORD_LEN);
            passwd_len = (strlen(passwd) > MAX_PASSWORD_LEN) ? MAX_PASSWORD_LEN : strlen(passwd);
            memcpy(tmp_passwd, passwd, passwd_len);
        } else {
            SLOGE("%s: Failed to allocate memory for tmp passwd \n", __func__);
        }
    } else {
        SLOGE("%s: Passed argument is NULL \n", __func__);
    }
    return tmp_passwd;
}

static void wipe_userdata()
{
    mkdir("/cache/recovery", 0700);
    int fd = open("/cache/recovery/command", O_RDWR|O_CREAT|O_TRUNC|O_NOFOLLOW, 0600);
    if (fd >= 0) {
        write(fd, "--wipe_data", strlen("--wipe_data") + 1);
        close(fd);
    } else {
        SLOGE("could not open /cache/recovery/command\n");
    }
    android_reboot(ANDROID_RB_RESTART2, 0, "recovery");
}

static int load_qseecom_library()
{
    const char *error = NULL;
    if (loaded_library)
        return loaded_library;

#ifdef __LP64__
    void * handle = dlopen("/vendor/lib64/libQSEEComAPI.so", RTLD_NOW);
#else
    void * handle = dlopen("/vendor/lib/libQSEEComAPI.so", RTLD_NOW);
#endif
    if(handle) {
        dlerror(); /* Clear any existing error */
        *(void **) (&qseecom_create_key) = dlsym(handle,"QSEECom_create_key");

        if((error = dlerror()) == NULL) {
            SLOGD("Success loading QSEECom_create_key \n");
            *(void **) (&qseecom_update_key) = dlsym(handle,"QSEECom_update_key_user_info");
            if ((error = dlerror()) == NULL) {
                SLOGD("Success loading QSEECom_update_key_user_info\n");
                *(void **) (&qseecom_wipe_key) = dlsym(handle,"QSEECom_wipe_key");
                if ((error = dlerror()) == NULL) {
                    loaded_library = 1;
                    SLOGD("Success loading QSEECom_wipe_key \n");
                }
                else
                    SLOGE("Error %s loading symbols for QSEECom APIs \n", error);
            }
            else
                SLOGE("Error %s loading symbols for QSEECom APIs \n", error);
        }
    } else {
        SLOGE("Could not load libQSEEComAPI.so \n");
    }

    if(error)
        dlclose(handle);

    return loaded_library;
}

/*
 * For NON-ICE targets, it would return 0 on success. On ICE based targets,
 * it would return key index in the ICE Key LUT
 */
static int set_key(const char* currentpasswd, const char* passwd, const char* enc_mode, int operation)
{
    int err = -1;
    if (is_hw_disk_encryption(enc_mode) && load_qseecom_library()) {
        unsigned char* tmp_passwd = get_tmp_passwd(passwd);
        unsigned char* tmp_currentpasswd = get_tmp_passwd(currentpasswd);
        if(tmp_passwd) {
            if (operation == UPDATE_HW_DISK_ENC_KEY) {
                if (tmp_currentpasswd)
                   err = qseecom_update_key(map_usage(QSEECOM_DISK_ENCRYPTION), tmp_currentpasswd, tmp_passwd);
            } else if (operation == SET_HW_DISK_ENC_KEY) {
                err = qseecom_create_key(map_usage(QSEECOM_DISK_ENCRYPTION), tmp_passwd);
            }
            if(err < 0) {
                if(ERR_MAX_PASSWORD_ATTEMPTS == err)
                    wipe_userdata();
            }
            free(tmp_passwd);
            free(tmp_currentpasswd);
        }
    }
    return err;
}

int set_hw_device_encryption_key(const char* passwd, const char* enc_mode)
{
    return set_key(NULL, passwd, enc_mode, SET_HW_DISK_ENC_KEY);
}

int update_hw_device_encryption_key(const char* oldpw, const char* newpw, const char* enc_mode)
{
    return set_key(oldpw, newpw, enc_mode, UPDATE_HW_DISK_ENC_KEY);
}

unsigned int is_hw_disk_encryption(const char* encryption_mode)
{
    int ret = 0;
    if(encryption_mode) {
        if (!strcmp(encryption_mode, "aes-xts")) {
            SLOGD("HW based disk encryption is enabled \n");
            ret = 1;
        }
    }
    return ret;
}

int clear_hw_device_encryption_key(void)
{
    if (load_qseecom_library())
        return qseecom_wipe_key(map_usage(QSEECOM_DISK_ENCRYPTION));

    return 0;
}

/*
 * By default HW FDE is enabled, if the execution comes to
 * is_hw_fde_enabled() API then for specific device/soc id,
 * HW FDE is disabled.
 */
#ifdef CONFIG_SWV8_DISK_ENCRYPTION
unsigned int is_hw_fde_enabled(void)
{
    unsigned int device_id = -1;
    unsigned int array_size;
    unsigned int status = 1;
    FILE *fd = NULL;
    unsigned int i;
    int ret = -1;
    char buf[MAX_DEVICE_ID_LENGTH];

    fd = fopen("/sys/devices/soc0/soc_id", "r");
    if (fd) {
        ret = fread(buf, 1, MAX_DEVICE_ID_LENGTH, fd);
        fclose(fd);
    } else {
        fd = fopen("/sys/devices/system/soc/soc0/id", "r");
        if (fd) {
            ret = fread(buf, 1, MAX_DEVICE_ID_LENGTH, fd);
            fclose(fd);
        }
    }

    if (ret > 0) {
        device_id = atoi(buf);
    } else {
        SLOGE("Failed to read device id");
        return status;
    }

    array_size = sizeof(cpu_id) / sizeof(cpu_id[0]);
    for (i = 0; i < array_size; i++) {
        if (device_id == cpu_id[i]) {
            status = 0;
            break;
        }
    }

    return status;
}
#else
unsigned int is_hw_fde_enabled(void)
{
    return 1;
}
#endif

int is_ice_enabled(void)
{
  char prop_storage[PATH_MAX];
  int storage_type = 0;
  int fd;

  if (property_get("ro.boot.bootdevice", prop_storage, "")) {
    if (strstr(prop_storage, "ufs")) {
      /* All UFS based devices has ICE in it. So we dont need
       * to check if corresponding device exists or not
       */
      storage_type = QTI_ICE_STORAGE_UFS;
    } else if (strstr(prop_storage, "sdhc")) {
      if (access("/dev/icesdcc", F_OK) != -1)
        storage_type = QTI_ICE_STORAGE_SDCC;
    }
  }
  return storage_type;
}

static int get_keymaster_version()
{
    int rc = -1;
    const hw_module_t* mod;
    rc = hw_get_module_by_class(KEYSTORE_HARDWARE_MODULE_ID, NULL, &mod);
    if (rc) {
        SLOGE("could not find any keystore module");
        return rc;
    }

    return mod->module_api_version;
}

int should_use_keymaster()
{
    /* HW FDE key would be tied to keymaster only if:
     * New Keymaster is available
     * keymaster partition exists on the device
     */
    int rc = 0;
    if (get_keymaster_version() != KEYMASTER_MODULE_API_VERSION_1_0) {
        SLOGI("Keymaster version is not 1.0");
        return rc;
    }

    if (access(KEYMASTER_PARTITION_NAME, F_OK) == -1) {
        SLOGI("Keymaster partition does not exists");
        return rc;
    }

    return 1;
}
