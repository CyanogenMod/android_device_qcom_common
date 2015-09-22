/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 * Copyright (c) 2014, The CyanogenMod Project
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
 *     * Neither the name of The Linux Foundation nor the names of its
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
#define LOG_NIDEBUG 0

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>

#define LOG_TAG "QCOM PowerHAL"
#include <utils/Log.h>
#include <hardware/hardware.h>
#include <hardware/power.h>

#include "utils.h"
#include "metadata-defs.h"
#include "hint-data.h"
#include "performance.h"
#include "power-common.h"

#define MIN_FREQ_CPU0_DISP_OFF 400000
#define MIN_FREQ_CPU0_DISP_ON  960000

char scaling_min_freq[4][80] ={
    "sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq",
    "sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq",
    "sys/devices/system/cpu/cpu2/cpufreq/scaling_min_freq",
    "sys/devices/system/cpu/cpu3/cpufreq/scaling_min_freq"
};

static int is_8916 = -1;

static int display_hint_sent;
int display_boost;
static int saved_interactive_mode = -1;
static int slack_node_rw_failed = 0;

enum {
    PROFILE_POWER_SAVE = 0,
    PROFILE_BALANCED,
    PROFILE_HIGH_PERFORMANCE
};

static int current_power_profile = PROFILE_BALANCED;

static int is_target_8916() /* Returns value=8916 if target is 8916 else value 0 */
{
    int fd;
    char buf[10] = {0};

    if (is_8916 >= 0)
        return is_8916;

    fd = open("/sys/devices/soc0/soc_id", O_RDONLY);
    if (fd >= 0) {
        if (read(fd, buf, sizeof(buf) - 1) == -1) {
            ALOGW("Unable to read soc_id");
            is_8916 = 0;
        } else {
            int soc_id = atoi(buf);
            if (soc_id == 206 || (soc_id >= 247 && soc_id <= 250))  {
            is_8916 = 8916; /* Above SOCID for 8916 */
            }
        }
    }
    close(fd);
    return is_8916;
}

static int profile_high_performance_8916[3] = {
    0x1C00, 0x0901, CPU0_MIN_FREQ_TURBO_MAX,
};

static int profile_high_performance_8939[11] = {
    SCHED_BOOST_ON, 0x1C00, 0x0901,
    CPU0_MIN_FREQ_TURBO_MAX, CPU1_MIN_FREQ_TURBO_MAX,
    CPU2_MIN_FREQ_TURBO_MAX, CPU3_MIN_FREQ_TURBO_MAX,
    CPU4_MIN_FREQ_TURBO_MAX, CPU5_MIN_FREQ_TURBO_MAX,
    CPU6_MIN_FREQ_TURBO_MAX, CPU7_MIN_FREQ_TURBO_MAX,
};

static int profile_power_save_8916[1] = {
    CPU0_MAX_FREQ_NONTURBO_MAX,
};

static int profile_power_save_8939[5] = {
    CPUS_ONLINE_MAX_LIMIT_2,
    CPU0_MAX_FREQ_NONTURBO_MAX, CPU1_MAX_FREQ_NONTURBO_MAX,
    CPU2_MAX_FREQ_NONTURBO_MAX, CPU3_MAX_FREQ_NONTURBO_MAX,
};

static void set_power_profile(int profile) {

    if (profile == current_power_profile)
        return;

    ALOGV("%s: profile=%d", __func__, profile);

    if (current_power_profile != PROFILE_BALANCED) {
        undo_hint_action(DEFAULT_PROFILE_HINT_ID);
        ALOGV("%s: hint undone", __func__);
    }

    if (profile == PROFILE_HIGH_PERFORMANCE) {
        int *resource_values = is_target_8916() ?
            profile_high_performance_8916 : profile_high_performance_8939;

        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        ALOGD("%s: set performance mode", __func__);

    } else if (profile == PROFILE_POWER_SAVE) {
        int* resource_values = is_target_8916() ?
            profile_power_save_8916 : profile_power_save_8939;

        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        ALOGD("%s: set powersave", __func__);
    }

    current_power_profile = profile;
}

static void process_video_decode_hint(void *metadata)
{
    char governor[80];
    struct video_decode_metadata_t video_decode_metadata;

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");

        return;
    }

    if (metadata) {
        ALOGI("Processing video decode hint. Metadata: %s", (char *)metadata);
    }

    /* Initialize encode metadata struct fields. */
    memset(&video_decode_metadata, 0, sizeof(struct video_decode_metadata_t));
    video_decode_metadata.state = -1;
    video_decode_metadata.hint_id = DEFAULT_VIDEO_DECODE_HINT_ID;

    if (metadata) {
        if (parse_video_decode_metadata((char *)metadata, &video_decode_metadata) ==
            -1) {
            ALOGE("Error occurred while parsing metadata.");
            return;
        }
    } else {
        return;
    }

    if (video_decode_metadata.state == 1) {
        if ((strncmp(governor, ONDEMAND_GOVERNOR, strlen(ONDEMAND_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(ONDEMAND_GOVERNOR))) {
            int resource_values[] = {THREAD_MIGRATION_SYNC_OFF};

            perform_hint_action(video_decode_metadata.hint_id,
                    resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        } else if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
            int resource_values[] = {TR_MS_30, HISPEED_LOAD_90, HS_FREQ_1026};

            perform_hint_action(video_decode_metadata.hint_id,
                    resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        }
    } else if (video_decode_metadata.state == 0) {
        if ((strncmp(governor, ONDEMAND_GOVERNOR, strlen(ONDEMAND_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(ONDEMAND_GOVERNOR))) {
        } else if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
            undo_hint_action(video_decode_metadata.hint_id);
        }
    }
}

static void process_video_encode_hint(void *metadata)
{
    char governor[80];
    struct video_encode_metadata_t video_encode_metadata;

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");

        return;
    }

    /* Initialize encode metadata struct fields. */
    memset(&video_encode_metadata, 0, sizeof(struct video_encode_metadata_t));
    video_encode_metadata.state = -1;
    video_encode_metadata.hint_id = DEFAULT_VIDEO_ENCODE_HINT_ID;

    if (metadata) {
        if (parse_video_encode_metadata((char *)metadata, &video_encode_metadata) ==
            -1) {
            ALOGE("Error occurred while parsing metadata.");
            return;
        }
    } else {
        return;
    }

    if (video_encode_metadata.state == 1) {
        if ((strncmp(governor, ONDEMAND_GOVERNOR, strlen(ONDEMAND_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(ONDEMAND_GOVERNOR))) {
            int resource_values[] = {IO_BUSY_OFF, SAMPLING_DOWN_FACTOR_1, THREAD_MIGRATION_SYNC_OFF};

            perform_hint_action(video_encode_metadata.hint_id,
                resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        } else if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
            int resource_values[] = {HS_FREQ_800, 0x1C00};

            perform_hint_action(video_encode_metadata.hint_id,
                    resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        }
    } else if (video_encode_metadata.state == 0) {
        if ((strncmp(governor, ONDEMAND_GOVERNOR, strlen(ONDEMAND_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(ONDEMAND_GOVERNOR))) {
            undo_hint_action(video_encode_metadata.hint_id);
        } else if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
            undo_hint_action(video_encode_metadata.hint_id);
        }
    }
}

extern void interaction(int duration, int num_args, int opt_list[]);

#ifdef __LP64__
typedef int64_t hintdata;
#else
typedef int hintdata;
#endif

int  set_interactive_override(struct power_module *module __unused, int on)
{
    char governor[80];
    char tmp_str[NODE_MAX];
    struct video_encode_metadata_t video_encode_metadata;
    int rc;

    ALOGI("Got set_interactive hint");
    if (get_scaling_governor_check_cores(governor, sizeof(governor),CPU0) == -1) {
        if (get_scaling_governor_check_cores(governor, sizeof(governor),CPU1) == -1) {
            if (get_scaling_governor_check_cores(governor, sizeof(governor),CPU2) == -1) {
                if (get_scaling_governor_check_cores(governor, sizeof(governor),CPU3) == -1) {
                    ALOGE("Can't obtain scaling governor.");
                    return HINT_HANDLED;
                }
            }
        }
    }

    if (!on) {
        /* Display off. */
       switch(is_target_8916()) {

          case 8916:
           {
            if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
               int resource_values[] = {TR_MS_50, THREAD_MIGRATION_SYNC_OFF};

                  if (!display_hint_sent) {
                      perform_hint_action(DISPLAY_STATE_HINT_ID,
                      resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
                      display_hint_sent = 1;
                  }
            } /* Perf time rate set for 8916 target*/
           } /* End of Switch case for 8916 */
            break ;

            default:
            {
             if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
               int resource_values[] = {TR_MS_CPU0_50,TR_MS_CPU4_50, THREAD_MIGRATION_SYNC_OFF};

               /* Set CPU0 MIN FREQ to 400Mhz avoid extra peak power
                  impact in volume key press  */
               snprintf(tmp_str, NODE_MAX, "%d", MIN_FREQ_CPU0_DISP_OFF);
               if (sysfs_write(scaling_min_freq[0], tmp_str) != 0) {
                   if (sysfs_write(scaling_min_freq[1], tmp_str) != 0) {
                       if (sysfs_write(scaling_min_freq[2], tmp_str) != 0) {
                           if (sysfs_write(scaling_min_freq[3], tmp_str) != 0) {
                               if(!slack_node_rw_failed) {
                                  ALOGE("Failed to write to %s",SCALING_MIN_FREQ );
                               }
                                rc = 1;
                           }
                       }
                   }
                }

                  if (!display_hint_sent) {
                      perform_hint_action(DISPLAY_STATE_HINT_ID,
                      resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
                      display_hint_sent = 1;
                  }
             } /* Perf time rate set for CORE0,CORE4 8939 target*/
           }/* End of Switch case for 8939 */
           break ;
          }

    } else {
        /* Display on. */
      switch(is_target_8916()){
         case 8916:
         {
          if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
            undo_hint_action(DISPLAY_STATE_HINT_ID);
            display_hint_sent = 0;
         }
         }
         break ;
         default :
         {

          if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {

              /* Recovering MIN_FREQ in display ON case */
               snprintf(tmp_str, NODE_MAX, "%d", MIN_FREQ_CPU0_DISP_ON);
               if (sysfs_write(scaling_min_freq[0], tmp_str) != 0) {
                   if (sysfs_write(scaling_min_freq[1], tmp_str) != 0) {
                       if (sysfs_write(scaling_min_freq[2], tmp_str) != 0) {
                           if (sysfs_write(scaling_min_freq[3], tmp_str) != 0) {
                               if(!slack_node_rw_failed) {
                                  ALOGE("Failed to write to %s",SCALING_MIN_FREQ );
                               }
                                rc = 1;
                           }
                       }
                   }
                }
             undo_hint_action(DISPLAY_STATE_HINT_ID);
             display_hint_sent = 0;
          }

        }
         break ;
      } /* End of check condition during the DISPLAY ON case */
   }
    saved_interactive_mode = !!on;
    return HINT_HANDLED;
}

int power_hint_override(struct power_module *module __unused, power_hint_t hint, void *data)
{
    if (hint == POWER_HINT_SET_PROFILE) {
        set_power_profile((hintdata)data);
    }

    if (hint == POWER_HINT_LOW_POWER) {
        if (current_power_profile == PROFILE_POWER_SAVE) {
            set_power_profile(PROFILE_BALANCED);
        } else {
            set_power_profile(PROFILE_POWER_SAVE);
        }
    }

    // Skip other hints in custom power modes
    if (current_power_profile != PROFILE_BALANCED) {
        return HINT_HANDLED;
    }

    if (hint == POWER_HINT_LAUNCH_BOOST) {
        int duration = 2000;
        int resources[] = { SCHED_BOOST_ON, 0x20F, 0x101, 0x1C00, 0x3E01, 0x4001, 0x4101, 0x4201 };

        interaction(duration, sizeof(resources)/sizeof(resources[0]), resources);

        return HINT_HANDLED;
	}

    if (hint == POWER_HINT_CPU_BOOST) {
        int duration = (hintdata)data / 1000;
        int resources[] = { SCHED_BOOST_ON, 0x20D, 0x3E01, 0x101 };

        if (duration > 0)
            interaction(duration, sizeof(resources)/sizeof(resources[0]), resources);

        return HINT_HANDLED;
	}

    if (hint == POWER_HINT_VIDEO_ENCODE) {
        process_video_encode_hint(data);
        return HINT_HANDLED;
    }

    if (hint == POWER_HINT_VIDEO_DECODE) {
        process_video_decode_hint(data);
        return HINT_HANDLED;
    }

	return HINT_NONE;
}
