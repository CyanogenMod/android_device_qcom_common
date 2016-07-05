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

static int display_hint_sent;
static int display_hint2_sent;
static int first_display_off_hint;
extern int display_boost;

static int current_power_profile = PROFILE_BALANCED;

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
        int resource_values[] = { CPUS_ONLINE_MIN_4, 0x0901,
            CPU0_MIN_FREQ_TURBO_MAX, CPU1_MIN_FREQ_TURBO_MAX,
            CPU2_MIN_FREQ_TURBO_MAX, CPU3_MIN_FREQ_TURBO_MAX };
        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, ARRAY_SIZE(resource_values));
        ALOGD("%s: set performance mode", __func__);
    } else if (profile == PROFILE_BIAS_PERFORMANCE) {
        int resource_values[] = {
            CPU0_MIN_FREQ_NONTURBO_MAX + 1, CPU1_MIN_FREQ_NONTURBO_MAX + 1,
            CPU2_MIN_FREQ_NONTURBO_MAX + 1, CPU2_MIN_FREQ_NONTURBO_MAX + 1 };
        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, ARRAY_SIZE(resource_values));
        ALOGD("%s: set bias perf mode", __func__);
    } else if (profile == PROFILE_BIAS_POWER) {
        int resource_values[] = { 0x0A03,
            CPU0_MAX_FREQ_NONTURBO_MAX, CPU1_MAX_FREQ_NONTURBO_MAX,
            CPU1_MAX_FREQ_NONTURBO_MAX, CPU2_MAX_FREQ_NONTURBO_MAX };
        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, ARRAY_SIZE(resource_values));
        ALOGD("%s: set bias power mode", __func__);
    } else if (profile == PROFILE_POWER_SAVE) {
        int resource_values[] = { 0x0A03, CPUS_ONLINE_MAX_LIMIT_2,
            CPU0_MAX_FREQ_NONTURBO_MAX, CPU1_MAX_FREQ_NONTURBO_MAX,
            CPU2_MAX_FREQ_NONTURBO_MAX, CPU3_MAX_FREQ_NONTURBO_MAX };
        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, ARRAY_SIZE(resource_values));
        ALOGD("%s: set powersave", __func__);
    }

    current_power_profile = profile;
}

extern void interaction(int duration, int num_args, int opt_list[]);

int power_hint_override(__attribute__((unused)) struct power_module *module,
        power_hint_t hint, void *data)
{
    if (hint == POWER_HINT_SET_PROFILE) {
        set_power_profile(*(int32_t *)data);
        return HINT_HANDLED;
    }

    // Skip other hints in high/low power modes
    if (current_power_profile == PROFILE_POWER_SAVE ||
            current_power_profile == PROFILE_HIGH_PERFORMANCE) {
        return HINT_HANDLED;
    }

    if (hint == POWER_HINT_LAUNCH_BOOST) {
        int duration = 2000;
        int resources[] = { CPUS_ONLINE_MIN_3,
            CPU0_MIN_FREQ_TURBO_MAX, CPU1_MIN_FREQ_TURBO_MAX,
            CPU2_MIN_FREQ_TURBO_MAX, CPU3_MIN_FREQ_TURBO_MAX };

        interaction(duration, ARRAY_SIZE(resources), resources);

        return HINT_HANDLED;
    }

    if (hint == POWER_HINT_CPU_BOOST) {
        int duration = *(int32_t *)data / 1000;
        int resources[] = { CPUS_ONLINE_MIN_2,
            0x20F, 0x30F, 0x40F, 0x50F };

        if (duration)
            interaction(duration, ARRAY_SIZE(resources), resources);

        return HINT_HANDLED;
    }

    if (hint == POWER_HINT_INTERACTION) {
        int duration = 500, duration_hint = 0;
        static unsigned long long previous_boost_time = 0;

        if (data) {
            duration_hint = *((int *)data);
        }

        duration = duration_hint > 0 ? duration_hint : 500;

        struct timeval cur_boost_timeval = {0, 0};
        gettimeofday(&cur_boost_timeval, NULL);
        unsigned long long cur_boost_time = cur_boost_timeval.tv_sec * 1000000 + cur_boost_timeval.tv_usec;
        double elapsed_time = (double)(cur_boost_time - previous_boost_time);
        if (elapsed_time > 750000)
            elapsed_time = 750000;
        // don't hint if it's been less than 250ms since last boost
        // also detect if we're doing anything resembling a fling
        // support additional boosting in case of flings
        else if (elapsed_time < 250000 && duration <= 750)
            return HINT_HANDLED;

        previous_boost_time = cur_boost_time;

        int resources[] = { (duration >= 2000 ? CPUS_ONLINE_MIN_3 : CPUS_ONLINE_MIN_2),
            0x20F, 0x30F, 0x40F, 0x50F };

        if (duration)
            interaction(duration, ARRAY_SIZE(resources), resources);

        return HINT_HANDLED;
    }


    return HINT_NONE;
}

int set_interactive_override(struct power_module *module __unused, int on)
{
    char governor[80];

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");

        return HINT_NONE;
    }

    if (!on) {
        /* Display off. */
        /*
         * We need to be able to identify the first display off hint
         * and release the current lock holder
         */
        if (display_boost) {
            if (!first_display_off_hint) {
                undo_initial_hint_action();
                first_display_off_hint = 1;
            }
            /* used for all subsequent toggles to the display */
            if (!display_hint2_sent) {
                undo_hint_action(DISPLAY_STATE_HINT_ID_2);
                display_hint2_sent = 1;
            }
        }

        if ((strncmp(governor, ONDEMAND_GOVERNOR, strlen(ONDEMAND_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(ONDEMAND_GOVERNOR))) {
            int resource_values[] = {MS_500, SYNC_FREQ_600, OPTIMAL_FREQ_600, THREAD_MIGRATION_SYNC_OFF};

            if (!display_hint_sent) {
                perform_hint_action(DISPLAY_STATE_HINT_ID,
                        resource_values, ARRAY_SIZE(resource_values));
                display_hint_sent = 1;
            }

            return HINT_HANDLED;
        }
    } else {
        /* Display on */
        if (display_boost && display_hint2_sent) {
            int resource_values2[] = {CPUS_ONLINE_MIN_2};
            perform_hint_action(DISPLAY_STATE_HINT_ID_2,
                    resource_values2, ARRAY_SIZE(resource_values2));
            display_hint2_sent = 0;
        }

        if ((strncmp(governor, ONDEMAND_GOVERNOR, strlen(ONDEMAND_GOVERNOR)) == 0) &&
                (strlen(governor) == strlen(ONDEMAND_GOVERNOR))) {
            undo_hint_action(DISPLAY_STATE_HINT_ID);
            display_hint_sent = 0;

            return HINT_HANDLED;
        }
    }

    return HINT_NONE;
}
