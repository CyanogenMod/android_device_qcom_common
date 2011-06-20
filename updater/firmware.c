/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bootloader.h"
#include "common.h"
#include "firmware.h"
#include "mtdutils/mtdutils.h"

#include <errno.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/mount.h>

/* Bootloader / Recovery Flow
 *
 * On every boot, the bootloader will read the bootloader_message
 * from flash and check the command field.  The bootloader should
 * deal with the command field not having a 0 terminator correctly
 * (so as to not crash if the block is invalid or corrupt).
 *
 * The bootloader will have to publish the partition that contains
 * the bootloader_message to the linux kernel so it can update it.
 *
 * if command == "boot-recovery" -> boot recovery.img
 * else if command == "update-radio" -> update radio image (below)
 * else -> boot boot.img (normal boot)
 *
 * Radio Update Flow
 * 1. the bootloader will attempt to load and validate the header
 * 2. if the header is invalid, status="invalid-update", goto #8
 * 3. display the busy image on-screen
 * 4. if the update image is invalid, status="invalid-radio-image", goto #8
 * 5. attempt to update the firmware (depending on the command)
 * 6. if successful, status="okay", goto #8
 * 7. if failed, and the old image can still boot, status="failed-update"
 * 8. write the bootloader_message, leaving the recovery field
 *    unchanged, updating status, and setting command to
 *    "boot-recovery"
 * 9. reboot
 *
 * The bootloader will not modify or erase the cache partition.
 * It is recovery's responsibility to clean up the mess afterwards.
 */

#undef LOGE
#define LOGE(...) fprintf(stderr, "E:" __VA_ARGS__)

static int num_volumes = 0;
static Volume* device_volumes = NULL;

int install_firmware_update(const char *update_type,
                            const char *update_data,
                            size_t update_length,
                            const char *log_filename) {
    if (update_data == NULL || update_length == 0) return 0;

    mtd_scan_partitions();

    /* We destroy the cache partition to pass the update image to the
     * bootloader, so all we can really do afterwards is wipe cache and reboot.
     * Set up this instruction now, in case we're interrupted while writing.
     */

    struct bootloader_message boot;
    memset(&boot, 0, sizeof(boot));
    strlcpy(boot.command, "boot-recovery", sizeof(boot.command));
    strlcpy(boot.recovery, "recovery\n--wipe_cache\n", sizeof(boot.command));
    if (set_bootloader_message(&boot)) return -1;

    if (write_update_for_bootloader(
            update_data, update_length,
            log_filename)) {
        LOGE("Can't write %s image\n(%s)\n", update_type, strerror(errno));
        return -1;
    }

    /* The update image is fully written, so now we can instruct the bootloader
     * to install it.  (After doing so, it will come back here, and we will
     * wipe the cache and reboot into the system.)
     */
    snprintf(boot.command, sizeof(boot.command), "update-%s", update_type);
    if (set_bootloader_message(&boot)) {
        return -1;
    }

    reboot(RB_AUTOBOOT);

    LOGE("Can't reboot\n");
    return -1;
}

static void load_volume_table() {
    int alloc = 2;
    device_volumes = malloc(alloc * sizeof(Volume));

    // Insert an entry for /tmp, which is the ramdisk and is always mounted.
    device_volumes[0].mount_point = "/tmp";
    device_volumes[0].fs_type = "ramdisk";
    device_volumes[0].device = NULL;
    device_volumes[0].device2 = NULL;
    num_volumes = 1;

    FILE* fstab;
    fstab = fopen("/etc/recovery_mmc.fstab", "r");

    if (fstab == NULL) {
        LOGE("failed to open /etc/recovery.fstab (%s)\n", strerror(errno));
        return;
    }

    char buffer[1024];
    int i;
    while (fgets(buffer, sizeof(buffer)-1, fstab)) {
        for (i = 0; buffer[i] && isspace(buffer[i]); ++i);
        if (buffer[i] == '\0' || buffer[i] == '#') continue;

        char* original = strdup(buffer);

        char* mount_point = strtok(buffer+i, " \t\n");
        char* fs_type = strtok(NULL, " \t\n");
        char* device = strtok(NULL, " \t\n");
        // lines may optionally have a second device, to use if
        // mounting the first one fails.
        char* device2 = strtok(NULL, " \t\n");

        if (mount_point && fs_type && device) {
            while (num_volumes >= alloc) {
                alloc *= 2;
                device_volumes = realloc(device_volumes, alloc*sizeof(Volume));
            }
            device_volumes[num_volumes].mount_point = strdup(mount_point);
            device_volumes[num_volumes].fs_type = strdup(fs_type);
            device_volumes[num_volumes].device = strdup(device);
            device_volumes[num_volumes].device2 =
                device2 ? strdup(device2) : NULL;
            ++num_volumes;
        } else {
            LOGE("skipping malformed recovery.fstab line: %s\n", original);
        }
        free(original);
    }

    fclose(fstab);
}

static Volume* volume_for_path(const char* path) {
    int i;
    for (i = 0; i < num_volumes; ++i) {
        Volume* v = device_volumes+i;
        int len = strlen(v->mount_point);
        if (strncmp(path, v->mount_point, len) == 0 &&
            (path[len] == '\0' || path[len] == '/')) {
            return v;
        }
    }
    return NULL;
}

int start_firmware_update(char *update_type, char *part_type)
{
    int result;
    struct bootloader_message boot;

    memset(&boot, 0, sizeof(boot));

    if(!strcmp(part_type, "mtd"))
    {
        mtd_scan_partitions();

        strlcpy(boot.recovery, "recovery\n--radio_status\n", sizeof(boot.command));
        snprintf(boot.command, sizeof(boot.command), "update-%s", update_type);
        if (set_bootloader_message(&boot)) {
            return -1;
        }
    }
    else if(!strcmp(part_type, "emmc"))
    {
        Volume *v = NULL;

        load_volume_table();

        v = volume_for_path("/sys_boot");
        if (strcmp(v->fs_type, "vfat"))
        {
            LOGE("Error in fs_type for sys_boot partition\n");
            return -1;
        }

        mkdir("/sys_boot", 777);

        /* Try mounting device first */
        result = mount(v->device, v->mount_point, v->fs_type,
                       MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");
        if(result)
        {
            /* Try mounting device2 next */
            result = mount(v->device2, v->mount_point, v->fs_type,
                           MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");
        }
        if(result == 0)
        {
             /* Creating cookie file for radio update */
             FILE *fp = fopen("/sys_boot/upcookie.txt", "w");
             fclose(fp);

             /* Unmount the sdcard now */
             if(umount(v->mount_point))
             {
                 LOGE("Error in unmounting  %s\n",v->mount_point);
                 return -1;
             }
             else
                 LOGI("Created cookie file for eMMC radio update\n");
        }
        else
        {
             LOGE("Error in mounting  %s\n",v->mount_point);
             return -1;
        }
        strlcpy(boot.recovery, "recovery\n--radio_status\n", sizeof(boot.command));
        snprintf(boot.command, sizeof(boot.command), "update-%s", update_type);
        v = volume_for_path("/misc");
        if (set_bootloader_message_emmc(&boot, v)) {
            return -1;
        }
    }
    else
    {
        LOGE("Error in part_type %s\n",part_type);
        return -1;
    }

    sync();
    reboot(RB_AUTOBOOT);

    // Can't reboot?  WTF?
    LOGE("Can't reboot\n");
    return -1;
}
