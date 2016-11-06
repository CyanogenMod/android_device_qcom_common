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

static int display_hint_sent;
static int video_encode_hint_sent;
static int current_power_profile = PROFILE_BALANCED;

static void process_video_encode_hint(void *metadata);

extern void interaction(int duration, int num_args, int opt_list[]);

static int profile_high_performance[] = {
    SCHED_BOOST_ON_V3, 0x1,
    ALL_CPUS_PWR_CLPS_DIS_V3, 0x1,
    CPUS_ONLINE_MIN_BIG, 0x2,
    CPUS_ONLINE_MIN_LITTLE, 0x2,
    MIN_FREQ_BIG_CORE_0, 0xFFF,
    MIN_FREQ_LITTLE_CORE_0, 0xFFF,
};

static int profile_power_save[] = {
    CPUS_ONLINE_MAX_LIMIT_BIG, 0x1,
    MAX_FREQ_BIG_CORE_0, 0x3bf,
    MAX_FREQ_LITTLE_CORE_0, 0x300,
};

static int profile_bias_power[] = {
    MAX_FREQ_BIG_CORE_0, 0x4B0,
    MAX_FREQ_LITTLE_CORE_0, 0x300,
};

static int profile_bias_performance[] = {
    CPUS_ONLINE_MAX_LIMIT_BIG, 0x2,
    CPUS_ONLINE_MAX_LIMIT_LITTLE, 0x2,
    MIN_FREQ_BIG_CORE_0, 0x540,
};

int get_number_of_profiles() {
    return 5;
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
        perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_high_performance,
                ARRAY_SIZE(profile_high_performance));
        ALOGD("%s: set performance mode", __func__);

    } else if (profile == PROFILE_POWER_SAVE) {
        perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_power_save,
                ARRAY_SIZE(profile_power_save));
        ALOGD("%s: set powersave", __func__);
    } else if (profile == PROFILE_BIAS_POWER) {
        perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_bias_power,
                ARRAY_SIZE(profile_bias_power));
        ALOGD("%s: Set bias power mode", __func__);

    } else if (profile == PROFILE_BIAS_PERFORMANCE) {
        perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_bias_performance,
                ARRAY_SIZE(profile_bias_performance));
        ALOGD("%s: Set bias perf mode", __func__);
    }

    current_power_profile = profile;
}

int power_hint_override(__unused struct power_module *module, power_hint_t hint,
        void *data)
{
    int duration, duration_hint;
    static struct timespec s_previous_boost_timespec;
    struct timespec cur_boost_timespec;
    long long elapsed_time;
    int resources_launch_boost[] = {
        SCHED_BOOST_ON_V3, 0x1,
        MAX_FREQ_BIG_CORE_0, 0xFFF,
        MAX_FREQ_LITTLE_CORE_0, 0xFFF,
        MIN_FREQ_BIG_CORE_0, 0xFFF,
        MIN_FREQ_LITTLE_CORE_0, 0xFFF,
        ALL_CPUS_PWR_CLPS_DIS_V3, 0x1,
        STOR_CLK_SCALE_DIS, 0x1,
    };

    int resources_cpu_boost[] = {
        SCHED_BOOST_ON_V3, 0x1,
        MIN_FREQ_BIG_CORE_0, 0x3BF,
    };

    int resources_interaction_fling_boost[] = {
        MIN_FREQ_BIG_CORE_0, 0x3BF,
        MIN_FREQ_LITTLE_CORE_0, 0x300,
        SCHED_BOOST_ON_V3, 0x1,
    };

    int resources_interaction_boost[] = {
        MIN_FREQ_BIG_CORE_0, 0x300,
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
        case POWER_HINT_INTERACTION:
            duration = 500;
            duration_hint = 0;

            if (data) {
                duration_hint = *((int *)data);
            }

            duration = duration_hint > 0 ? duration_hint : 500;

            clock_gettime(CLOCK_MONOTONIC, &cur_boost_timespec);
            elapsed_time = calc_timespan_us(s_previous_boost_timespec, cur_boost_timespec);
            if (elapsed_time > 750000)
                elapsed_time = 750000;
            // don't hint if it's been less than 250ms since last boost
            // also detect if we're doing anything resembling a fling
            // support additional boosting in case of flings
            else if (elapsed_time < 250000 && duration <= 750)
                return HINT_HANDLED;

            s_previous_boost_timespec = cur_boost_timespec;

            if (duration >= 1500) {
                interaction(duration, ARRAY_SIZE(resources_interaction_fling_boost),
                        resources_interaction_fling_boost);
            } else {
                interaction(duration, ARRAY_SIZE(resources_interaction_boost),
                        resources_interaction_boost);
            }
            return HINT_HANDLED;
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
        default:
            break;
    }
    return HINT_NONE;
}

int set_interactive_override(__unused struct power_module *module, int on)
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
                                    return;
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
