/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
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

#define LOG_TAG "QTI PowerHAL"
#include <utils/Log.h>
#include <hardware/hardware.h>
#include <hardware/power.h>

#include "utils.h"
#include "metadata-defs.h"
#include "hint-data.h"
#include "performance.h"
#include "power-common.h"

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

static int display_hint_sent;
static int video_encode_hint_sent;
static int current_power_profile = PROFILE_BALANCED;

static void process_video_encode_hint(void *metadata);

extern void interaction(int duration, int num_args, int opt_list[]);

static int profile_high_performance_8952[11] = {
    SCHED_BOOST_ON,
    0x704, 0x4d04, /* Enable all CPUs */
    CPU0_MIN_FREQ_TURBO_MAX, CPU1_MIN_FREQ_TURBO_MAX,
    CPU2_MIN_FREQ_TURBO_MAX, CPU3_MIN_FREQ_TURBO_MAX,
    CPU4_MIN_FREQ_TURBO_MAX, CPU5_MIN_FREQ_TURBO_MAX,
    CPU6_MIN_FREQ_TURBO_MAX, CPU7_MIN_FREQ_TURBO_MAX,
};

static int profile_power_save_8952[5] = {
    0x8fe, 0x3dfd, /* 1 big core, 2 little cores*/
    CPUS_ONLINE_MAX_LIMIT_2,
    CPU0_MAX_FREQ_NONTURBO_MAX, CPU1_MAX_FREQ_NONTURBO_MAX,
    CPU2_MAX_FREQ_NONTURBO_MAX, CPU3_MAX_FREQ_NONTURBO_MAX,
};

int get_number_of_profiles() {
    return 3;
}

static void set_power_profile(int profile) {

    if (profile == current_power_profile)
        return;

    ALOGV("%s: profile=%d", __func__, profile);

    if (current_power_profile != PROFILE_BALANCED) {
        undo_hint_action(DEFAULT_PROFILE_HINT_ID);
        ALOGV("%s: hint undone", __func__);
    }

    if (profile == PROFILE_HIGH_PERFORMANCE) {
        int *resource_values = profile_high_performance_8952;
        perform_hint_action(DEFAULT_PROFILE_HINT_ID, resource_values,
                ARRAY_SIZE(resource_values));
        ALOGD("%s: set performance mode", __func__);

    } else if (profile == PROFILE_POWER_SAVE) {
        int *resource_values = profile_power_save_8952;
        perform_hint_action(DEFAULT_PROFILE_HINT_ID, resource_values,
                ARRAY_SIZE(resource_values));
        ALOGD("%s: set powersave", __func__);
    }

    current_power_profile = profile;
}

int  power_hint_override(struct power_module *module, power_hint_t hint,
        void *data)
{
    int duration;
    int resources_launch_boost[] = {
        SCHED_BOOST_ON,
        0x20f,
        0x101,
        0x3e01,
        0x4001,
        0x4101,
        0x4201,
    };
    int resources_cpu_boost[] = {
        SCHED_BOOST_ON,
        0x20d,
        0x3e01,
        0x101,
    };

    if (hint == POWER_HINT_SET_PROFILE) {
        set_power_profile(*(int32_t *)data);
        return HINT_HANDLED;
    }

    // Skip other hints in custom power modes
    if (current_power_profile != PROFILE_BALANCED) {
        return HINT_HANDLED;
    }

    switch (hint) {
        case POWER_HINT_LAUNCH_BOOST:
            duration = 2000;
            interaction(duration, ARRAY_SIZE(resources_launch_boost),
                    resources_launch_boost);
            return HINT_HANDLED;
        case POWER_HINT_CPU_BOOST:
            duration = *(int32_t *)data / 1000;
            if (duration > 0) {
                interaction(duration, ARRAY_SIZE(resources_cpu_boost),
                        resources_cpu_boost);
            }
            return HINT_HANDLED;
        case POWER_HINT_VIDEO_ENCODE:
            process_video_encode_hint(data);
            return HINT_HANDLED;
    }
    return HINT_NONE;
}

int  set_interactive_override(struct power_module *module, int on)
{
    char governor[80];

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
             if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
               int resource_values[] = {TR_MS_CPU0_50, TR_MS_CPU4_50};

               if (!display_hint_sent) {
                   perform_hint_action(DISPLAY_STATE_HINT_ID,
                   resource_values, ARRAY_SIZE(resource_values));
                  display_hint_sent = 1;
                }
             } /* Perf time rate set for CORE0,CORE4 8952 target*/

    } else {
        /* Display on. */
          if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {

             undo_hint_action(DISPLAY_STATE_HINT_ID);
             display_hint_sent = 0;
          }
   }
    return HINT_HANDLED;
}

/* Video Encode Hint */
static void process_video_encode_hint(void *metadata)
{
    char governor[80];
    struct video_encode_metadata_t video_encode_metadata;

    ALOGI("Got process_video_encode_hint");

    if (get_scaling_governor_check_cores(governor,
        sizeof(governor),CPU0) == -1) {
            if (get_scaling_governor_check_cores(governor,
                sizeof(governor),CPU1) == -1) {
                    if (get_scaling_governor_check_cores(governor,
                        sizeof(governor),CPU2) == -1) {
                            if (get_scaling_governor_check_cores(governor,
                                sizeof(governor),CPU3) == -1) {
                                    ALOGE("Can't obtain scaling governor.");
                                    return HINT_HANDLED;
                            }
                    }
            }
    }

    /* Initialize encode metadata struct fields. */
    memset(&video_encode_metadata, 0, sizeof(struct video_encode_metadata_t));
    video_encode_metadata.state = -1;
    video_encode_metadata.hint_id = DEFAULT_VIDEO_ENCODE_HINT_ID;

    if (metadata) {
        if (parse_video_encode_metadata((char *)metadata,
            &video_encode_metadata) == -1) {
            ALOGE("Error occurred while parsing metadata.");
            return;
        }
    } else {
        return;
    }

    if (video_encode_metadata.state == 1) {
        if ((strncmp(governor, INTERACTIVE_GOVERNOR,
            strlen(INTERACTIVE_GOVERNOR)) == 0) &&
            (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
            int resource_values[] = {TR_MS_CPU0_30, TR_MS_CPU4_30};
            if (!video_encode_hint_sent) {
                perform_hint_action(video_encode_metadata.hint_id,
                resource_values,
                ARRAY_SIZE(resource_values));
                video_encode_hint_sent = 1;
            }
        }
    } else if (video_encode_metadata.state == 0) {
        if ((strncmp(governor, INTERACTIVE_GOVERNOR,
            strlen(INTERACTIVE_GOVERNOR)) == 0) &&
            (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
            undo_hint_action(video_encode_metadata.hint_id);
            video_encode_hint_sent = 0;
            return ;
        }
    }
    return;
}
