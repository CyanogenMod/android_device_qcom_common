/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * *    * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>

#define LOG_TAG "QCOM PowerHAL"

#include <utils/Log.h>
#include <cutils/properties.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

#include "metadata-defs.h"

#define NODE_MAX (64)

#define SCALING_GOVERNOR_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
#define ONDEMAND_PATH "/sys/devices/system/cpu/cpufreq/ondemand/"
#define ONDEMAND_IO_BUSY_PATH "/sys/devices/system/cpu/cpufreq/ondemand/io_is_busy"
#define ONDEMAND_SAMPLING_DOWN_PATH "/sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor"
#define DCVS_CPU0_SLACK_MAX_NODE "/sys/module/msm_dcvs/cores/cpu0/slack_time_max_us"
#define DCVS_CPU0_SLACK_MIN_NODE "/sys/module/msm_dcvs/cores/cpu0/slack_time_min_us"
#define MPDECISION_SLACK_MAX_NODE "/sys/module/msm_mpdecision/slack_time_max_us"
#define MPDECISION_SLACK_MIN_NODE "/sys/module/msm_mpdecision/slack_time_min_us"

static int (*perf_vote_turnoff_ondemand_io_busy)(int vote);
static int perf_vote_ondemand_io_busy_unavailable;
static int (*perf_vote_lower_ondemand_sdf)(int vote);
static int perf_vote_ondemand_sdf_unavailable;
static void *qcopt_handle;
static int qcopt_handle_unavailable;
static int saved_ondemand_sampling_down_factor = 4;
static int saved_ondemand_io_is_busy_status = 1;
static int saved_dcvs_cpu0_slack_max = -1;
static int saved_dcvs_cpu0_slack_min = -1;
static int saved_mpdecision_slack_max = -1;
static int saved_mpdecision_slack_min = -1;
static int saved_interactive_mode = -1;
static int slack_node_rw_failed = 0;

static void *get_qcopt_handle()
{
    if (qcopt_handle_unavailable) {
        return NULL;
    }

    if (!qcopt_handle) {
        char qcopt_lib_path[PATH_MAX] = {0};
        dlerror();

        if (property_get("ro.vendor.extension_library", qcopt_lib_path,
                    NULL) != 0) {
            if((qcopt_handle = dlopen(qcopt_lib_path, RTLD_NOW)) == NULL) {
                qcopt_handle_unavailable = 1;
                ALOGE("Unable to open %s: %s\n", qcopt_lib_path,
                        dlerror());
            }
        } else {
            qcopt_handle_unavailable = 1;
            ALOGE("Property ro.vendor.extension_library does not exist.");
        }
    }

    return qcopt_handle;
}

static int sysfs_read(char *path, char *s, int num_bytes)
{
    char buf[80];
    int count;
    int ret = 0;
    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);

        return -1;
    }

    if ((count = read(fd, s, num_bytes - 1)) < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);

        ret = -1;
    } else {
        s[count] = '\0';
    }

    close(fd);

    return ret;
}

static int sysfs_write(char *path, char *s)
{
    char buf[80];
    int len;
    int ret = 0;
    int fd = open(path, O_WRONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return -1 ;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);

        ret = -1;
    }

    close(fd);

    return ret;
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

void power_init(struct power_module *module)
{
    ALOGI("QCOM power HAL initing.");
}

static int get_scaling_governor(char governor[], int size) {
    if (sysfs_read(SCALING_GOVERNOR_PATH, governor,
                size) == -1) {
        // Can't obtain the scaling governor. Return.
        return -1;
    } else {
        // Strip newline at the end.
        int len = strlen(governor);

        len--;

        while (len >= 0 && (governor[len] == '\n' || governor[len] == '\r'))
            governor[len--] = '\0';
    }

    return 0;
}

static void process_video_encode_hint(void *metadata)
{
    void *handle;
    char governor[80];
    struct video_encode_metadata_t video_encode_metadata;

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");

        return;
    }

    /* Initialize encode metadata struct fields. */
    memset(&video_encode_metadata, 0, sizeof(video_encode_metadata));
    video_encode_metadata.state = -1;

    if (metadata) {
        if (parse_video_metadata((char *)metadata, &video_encode_metadata) ==
            -1) {
            ALOGE("Error occurred while parsing metadata.");
            return;
        }
    } else {
        return;
    }

    if ((handle = get_qcopt_handle())) {
        if (video_encode_metadata.state == 1) {
            if ((strlen(governor) == strlen("ondemand")) &&
                    (strncmp(governor, "ondemand", strlen("ondemand")) == 0)) {
                if (!perf_vote_ondemand_io_busy_unavailable) {
                    perf_vote_turnoff_ondemand_io_busy = dlsym(handle,
                            "perf_vote_turnoff_ondemand_io_busy");

                    if (perf_vote_turnoff_ondemand_io_busy) {
                        /* Vote to turn io_is_busy off */
                        perf_vote_turnoff_ondemand_io_busy(1);
                    } else {
                        perf_vote_ondemand_io_busy_unavailable = 1;
                        ALOGE("Can't set io_busy_status.");
                    }
                }

                if (!perf_vote_ondemand_sdf_unavailable) {
                    perf_vote_lower_ondemand_sdf = dlsym(handle,
                            "perf_vote_lower_ondemand_sdf");

                    if (perf_vote_lower_ondemand_sdf) {
                        perf_vote_lower_ondemand_sdf(1);
                    } else {
                        perf_vote_ondemand_sdf_unavailable = 1;
                        ALOGE("Can't set sampling_down_factor.");
                    }
                }
            }
        } else if (video_encode_metadata.state == 0) {
            if ((strlen(governor) == strlen("ondemand")) &&
                    (strncmp(governor, "ondemand", strlen("ondemand")) == 0)) {
                if (!perf_vote_ondemand_io_busy_unavailable) {
                    perf_vote_turnoff_ondemand_io_busy = dlsym(handle,
                            "perf_vote_turnoff_ondemand_io_busy");

                    if (perf_vote_turnoff_ondemand_io_busy) {
                        /* Remove vote to turn io_busy off. */
                        perf_vote_turnoff_ondemand_io_busy(0);
                    } else {
                        perf_vote_ondemand_io_busy_unavailable = 1;
                        ALOGE("Can't set io_busy_status.");
                    }
                }

                if (!perf_vote_ondemand_sdf_unavailable) {
                    perf_vote_lower_ondemand_sdf = dlsym(handle,
                            "perf_vote_lower_ondemand_sdf");

                    if (perf_vote_lower_ondemand_sdf) {
                        /* Remove vote to lower sampling down factor. */
                        perf_vote_lower_ondemand_sdf(0);
                    } else {
                        perf_vote_ondemand_sdf_unavailable = 1;
                        ALOGE("Can't set sampling_down_factor.");
                    }
                }
            }
        }
    }
}

int __attribute__ ((weak)) power_hint_override(struct power_module *module, power_hint_t hint,
        void *data)
{
    return -1;
}

static void power_hint(struct power_module *module, power_hint_t hint,
        void *data)
{
    /* Check if this hint has been overridden. */
    if (power_hint_override(module, hint, data) == 0) {
        /* The power_hint has been handled. We can skip the rest. */
        return;
    }

    switch(hint) {
        case POWER_HINT_VSYNC:
        break;
        case POWER_HINT_INTERACTION:
        break;
        case POWER_HINT_VIDEO_ENCODE:
            process_video_encode_hint(data);
        break;
    }
}

static void set_interactive(struct power_module *module, int on)
{
    char governor[80];
    char tmp_str[NODE_MAX];
    int rc = 0;

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        if (!slack_node_rw_failed) {
            ALOGE("Can't obtain scaling governor.");
            slack_node_rw_failed = 1;
        }

        return;
    }

    if ((strlen(governor) == strlen("msm-dcvs")) &&
            (strncmp(governor, "msm-dcvs", strlen("msm-dcvs")) == 0)) {
        if (on && (saved_interactive_mode == -1 || saved_interactive_mode == 0)) {
            /* Display turned on. Restore if possible. */
            if (saved_dcvs_cpu0_slack_max != -1) {
                snprintf(tmp_str, NODE_MAX, "%d", saved_dcvs_cpu0_slack_max);

                if (sysfs_write(DCVS_CPU0_SLACK_MAX_NODE, tmp_str) != 0) {
                    if (!slack_node_rw_failed) {
                        ALOGE("Failed to write to %s", DCVS_CPU0_SLACK_MAX_NODE);
                    }

                    rc = 1;
                }
            }

            if (saved_dcvs_cpu0_slack_max != -1) {
                snprintf(tmp_str, NODE_MAX, "%d", saved_dcvs_cpu0_slack_min);

                if (sysfs_write(DCVS_CPU0_SLACK_MIN_NODE, tmp_str) != 0) {
                    if (!slack_node_rw_failed) {
                        ALOGE("Failed to write to %s", DCVS_CPU0_SLACK_MIN_NODE);
                    }

                    rc = 1;
                }
            }

            if (saved_mpdecision_slack_max != -1) {
                snprintf(tmp_str, NODE_MAX, "%d", saved_mpdecision_slack_max);

                if (sysfs_write(MPDECISION_SLACK_MAX_NODE, tmp_str) != 0) {
                    if (!slack_node_rw_failed) {
                        ALOGE("Failed to write to %s", MPDECISION_SLACK_MAX_NODE);
                    }

                    rc = 1;
                }
            }

            if (saved_mpdecision_slack_min != -1) {
                snprintf(tmp_str, NODE_MAX, "%d", saved_mpdecision_slack_min);

                if (sysfs_write(MPDECISION_SLACK_MIN_NODE, tmp_str) != 0) {
                    if (!slack_node_rw_failed) {
                        ALOGE("Failed to write to %s", MPDECISION_SLACK_MIN_NODE);
                    }

                    rc = 1;
                }
            }
        } else if (!on && saved_interactive_mode == 1){
            /* Display turned off. */
            if (sysfs_read(DCVS_CPU0_SLACK_MAX_NODE, tmp_str, NODE_MAX - 1)) {
                if (!slack_node_rw_failed) {
                    ALOGE("Failed to read from %s", DCVS_CPU0_SLACK_MAX_NODE);
                }

                rc = 1;
            } else {
                saved_dcvs_cpu0_slack_max = atoi(tmp_str);
            }

            if (sysfs_read(DCVS_CPU0_SLACK_MIN_NODE, tmp_str, NODE_MAX - 1)) {
                if (!slack_node_rw_failed) {
                    ALOGE("Failed to read from %s", DCVS_CPU0_SLACK_MIN_NODE);
                }

                rc = 1;
            } else {
                saved_dcvs_cpu0_slack_min = atoi(tmp_str);
            }

            if (sysfs_read(MPDECISION_SLACK_MAX_NODE, tmp_str, NODE_MAX - 1)) {
                if (!slack_node_rw_failed) {
                    ALOGE("Failed to read from %s", MPDECISION_SLACK_MAX_NODE);
                }

                rc = 1;
            } else {
                saved_mpdecision_slack_max = atoi(tmp_str);
            }

            if (sysfs_read(MPDECISION_SLACK_MIN_NODE, tmp_str, NODE_MAX - 1)) {
                if(!slack_node_rw_failed) {
                    ALOGE("Failed to read from %s", MPDECISION_SLACK_MIN_NODE);
                }

                rc = 1;
            } else {
                saved_mpdecision_slack_min = atoi(tmp_str);
            }

            /* Write new values. */
            if (saved_dcvs_cpu0_slack_max != -1) {
                snprintf(tmp_str, NODE_MAX, "%d", 10 * saved_dcvs_cpu0_slack_max);

                if (sysfs_write(DCVS_CPU0_SLACK_MAX_NODE, tmp_str) != 0) {
                    if (!slack_node_rw_failed) {
                        ALOGE("Failed to write to %s", DCVS_CPU0_SLACK_MAX_NODE);
                    }

                    rc = 1;
                }
            }

            if (saved_dcvs_cpu0_slack_max != -1) {
                snprintf(tmp_str, NODE_MAX, "%d", 10 * saved_dcvs_cpu0_slack_min);

                if (sysfs_write(DCVS_CPU0_SLACK_MIN_NODE, tmp_str) != 0) {
                    if(!slack_node_rw_failed) {
                        ALOGE("Failed to write to %s", DCVS_CPU0_SLACK_MIN_NODE);
                    }

                    rc = 1;
                }
            }

            if (saved_mpdecision_slack_max != -1) {
                snprintf(tmp_str, NODE_MAX, "%d", 10 * saved_mpdecision_slack_max);

                if (sysfs_write(MPDECISION_SLACK_MAX_NODE, tmp_str) != 0) {
                    if(!slack_node_rw_failed) {
                        ALOGE("Failed to write to %s", MPDECISION_SLACK_MAX_NODE);
                    }

                    rc = 1;
                }
            }

            if (saved_mpdecision_slack_min != -1) {
                snprintf(tmp_str, NODE_MAX, "%d", 10 * saved_mpdecision_slack_min);

                if (sysfs_write(MPDECISION_SLACK_MIN_NODE, tmp_str) != 0) {
                    if(!slack_node_rw_failed) {
                        ALOGE("Failed to write to %s", MPDECISION_SLACK_MIN_NODE);
                    }

                    rc = 1;
                }
            }
        }

        slack_node_rw_failed = rc;
    }

    saved_interactive_mode = !!on;
}

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_2,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "QCOM Power HAL",
        .author = "Qualcomm",
        .methods = &power_module_methods,
    },

    .init = power_init,
    .powerHint = power_hint,
    .setInteractive = set_interactive,
};
