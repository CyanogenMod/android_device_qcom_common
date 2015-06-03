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

static int display_hint_sent;
int display_boost;

enum {
    PROFILE_POWER_SAVE = 0,
    PROFILE_BALANCED,
    PROFILE_HIGH_PERFORMANCE
};

static int current_power_profile = PROFILE_BALANCED;

static int is_target_8916() /* Returns value=8916 if target is 8916 else value 0 */
{
    int fd;
    int is_target_8916=0;
    char buf[10] = {0};

    fd = open("/sys/devices/soc0/soc_id", O_RDONLY);
    if (fd >= 0) {
        if (read(fd, buf, sizeof(buf) - 1) == -1) {
            ALOGW("Unable to read soc_id");
            is_target_8916 = 0;
        } else {
            int soc_id = atoi(buf);
            if (soc_id == 206 || (soc_id >= 247 && soc_id <= 250))  {
            is_target_8916 = 8916; /* Above SOCID for 8916 */
            }
        }
    }
    close(fd);
    return is_target_8916;
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
        int resource_values[] = { SCHED_BOOST_ON, 0x1C00, 0x0901,
            CPU0_MIN_FREQ_TURBO_MAX, CPU1_MIN_FREQ_TURBO_MAX,
            CPU2_MIN_FREQ_TURBO_MAX, CPU3_MIN_FREQ_TURBO_MAX,
            CPU4_MIN_FREQ_TURBO_MAX, CPU5_MIN_FREQ_TURBO_MAX,
            CPU6_MIN_FREQ_TURBO_MAX, CPU7_MIN_FREQ_TURBO_MAX };
        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        ALOGD("%s: set performance mode", __func__);

    } else if (profile == PROFILE_POWER_SAVE) {
        int resource_values[] = { CPUS_ONLINE_MAX_LIMIT_2,
            CPU0_MAX_FREQ_NONTURBO_MAX, CPU1_MAX_FREQ_NONTURBO_MAX,
            CPU2_MAX_FREQ_NONTURBO_MAX, CPU3_MAX_FREQ_NONTURBO_MAX };
        perform_hint_action(DEFAULT_PROFILE_HINT_ID,
            resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
        ALOGD("%s: set powersave", __func__);
    }

    current_power_profile = profile;
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

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");

        return HINT_HANDLED;
    }

    if (!on) {
        /* Display off. */
        switch(is_target_8916()) {

            case 8916:
                if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                        (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
                    int resource_values[] = {TR_MS_50, THREAD_MIGRATION_SYNC_OFF};

                    if (!display_hint_sent) {
                        perform_hint_action(DISPLAY_STATE_HINT_ID,
                        resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
                        display_hint_sent = 1;
                    }
                } /* Perf time rate set for 8916 target*/
                break ;

            default:
                if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                        (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
                    int resource_values[] = {TR_MS_CPU0_50,TR_MS_CPU4_50, THREAD_MIGRATION_SYNC_OFF};

                    /* Set CPU0 MIN FREQ to 400Mhz avoid extra peak power
                       impact in volume key press  */
                    snprintf(tmp_str, NODE_MAX, "%d", MIN_FREQ_CPU0_DISP_OFF);
                    if (sysfs_write(SCALING_MIN_FREQ, tmp_str) != 0) {
                        ALOGE("Failed to write to %s",SCALING_MIN_FREQ );
                        rc = 1;
                    }

                    if (!display_hint_sent) {
                        perform_hint_action(DISPLAY_STATE_HINT_ID,
                        resource_values, sizeof(resource_values)/sizeof(resource_values[0]));
                        display_hint_sent = 1;
                    } /* Perf time rate set for CORE0,CORE4 8939 target*/
                }/* End of Switch case for 8939 */
                break ;
        }
    } else {
        /* Display on. */
        switch(is_target_8916()){
            case 8916:
                if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                        (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {
                    undo_hint_action(DISPLAY_STATE_HINT_ID);
                    display_hint_sent = 0;
                }
                break;
            default :
                if ((strncmp(governor, INTERACTIVE_GOVERNOR, strlen(INTERACTIVE_GOVERNOR)) == 0) &&
                        (strlen(governor) == strlen(INTERACTIVE_GOVERNOR))) {

                    /* Recovering MIN_FREQ in display ON case */
                    snprintf(tmp_str, NODE_MAX, "%d", MIN_FREQ_CPU0_DISP_ON);
                    if (sysfs_write(SCALING_MIN_FREQ, tmp_str) != 0) {
                        ALOGE("Failed to write to %s",SCALING_MIN_FREQ );
                        rc = 1;
                    }
                    undo_hint_action(DISPLAY_STATE_HINT_ID);
                    display_hint_sent = 0;
                }
                break ;
        } /* End of check condition during the DISPLAY ON case */
    }
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

    if (hint == POWER_HINT_CPU_BOOST) {
        int duration = (hintdata)data / 1000;
        int resources[] = { CPUS_ONLINE_MIN_2, SCHED_BOOST_ON, 0x20B, 0x30B, 0x1C00};

        if (duration > 0)
            interaction(duration, sizeof(resources)/sizeof(resources[0]), resources);

        return HINT_HANDLED;
	}

    if (hint == POWER_HINT_INTERACTION) {
        int resources[] = {0x702, 0x20F, 0x30F};
        int duration = 3000;
        interaction(duration, sizeof(resources)/sizeof(resources[0]), resources);
        return HINT_HANDLED;
    }

    if (hint == POWER_HINT_VIDEO_ENCODE || hint == POWER_HINT_VIDEO_DECODE) {
        // skip this one
        return HINT_HANDLED;
    }

	return HINT_NONE;
}
