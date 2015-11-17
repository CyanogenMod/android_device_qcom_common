/*
 *Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are
 *met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 *WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 *ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/klog.h>
#include <batteryservice/BatteryService.h>
#include <cutils/android_reboot.h>
#include <healthd.h>
#include "minui/minui.h"

#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))

#define HVDCP_CHARGER           "USB_HVDCP"
#define HVDCP_BLINK_TYPE        2

#define RED_LED_PATH            "/sys/class/leds/red/brightness"
#define GREEN_LED_PATH          "/sys/class/leds/green/brightness"
#define BLUE_LED_PATH           "/sys/class/leds/blue/brightness"
#define RED_LED_BLINK_PATH      "/sys/class/leds/red/blink"
#define GREEN_LED_BLINK_PATH    "/sys/class/leds/green/blink"
#define BACKLIGHT_PATH          "/sys/class/leds/lcd-backlight/brightness"

#define CHARGING_ENABLED_PATH   "/sys/class/power_supply/battery/charging_enabled"
#define CHARGER_TYPE_PATH       "/sys/class/power_supply/usb/type"

#define LOGE(x...) do { KLOG_ERROR("charger", x); } while (0)
#define LOGW(x...) do { KLOG_WARNING("charger", x); } while (0)
#define LOGV(x...) do { KLOG_DEBUG("charger", x); } while (0)

enum {
    RED_LED = 0x01 << 0,
    GREEN_LED = 0x01 << 1,
    BLUE_LED = 0x01 << 2,
};

struct led_ctl {
    int color;
    const char *path;
};

struct led_ctl leds[3] =
    {{RED_LED, RED_LED_PATH},
    {GREEN_LED, GREEN_LED_PATH},
    {BLUE_LED, BLUE_LED_PATH}};

#define HVDCP_COLOR_MAP         (RED_LED | GREEN_LED)

struct soc_led_color_mapping {
    int soc;
    int color;
};

struct soc_led_color_mapping soc_leds[3] = {
    {15, RED_LED},
    {90, RED_LED | GREEN_LED},
    {100, GREEN_LED},
};

static int write_file_int(char const* path, int value)
{
    int fd;
    char buffer[20];
    int rc = -1, bytes;

    fd = open(path, O_WRONLY);
    if (fd >= 0) {
        bytes = snprintf(buffer, sizeof(buffer), "%d\n", value);
        rc = write(fd, buffer, bytes);
        close(fd);
    }

    return rc > 0 ? 0 : -1;
}

static int set_tricolor_led(int on, int color)
{
    int fd, i;
    char buffer[10];

    for (i = 0; i < (int)ARRAY_SIZE(leds); i++) {
        if ((color & leds[i].color) && (access(leds[i].path, R_OK | W_OK) == 0)) {
            fd = open(leds[i].path, O_RDWR);
            if (fd < 0) {
                LOGE("Could not open red led node\n");
                goto cleanup;
            }
            if (on)
                snprintf(buffer, sizeof(int), "%d\n", 255);
            else
                snprintf(buffer, sizeof(int), "%d\n", 0);

            if (write(fd, buffer, strlen(buffer)) < 0)
                LOGE("Could not write to led node\n");
cleanup:
            if (fd >= 0)
                close(fd);
        }
    }

    return 0;
}

static bool is_hvdcp_inserted()
{
    bool hvdcp = false;
    char buff[12] = "\0";
    int fd, cnt;

    fd = open(CHARGER_TYPE_PATH, O_RDONLY);
    if (fd >= 0) {
        cnt = read(fd, buff, sizeof(buff));
        if (cnt > 0 && !strncmp(buff, HVDCP_CHARGER, 9))
            hvdcp = true;
        close(fd);
    }

    return hvdcp;
}

static int leds_blink_for_hvdcp_allow(void)
{
    int rc = 0, bytes;
    int red_blink_fd = -1, green_blink_fd = -1, type_fd = -1;
    char buf[20];

    green_blink_fd = open(GREEN_LED_BLINK_PATH, O_RDWR);
    red_blink_fd = open(RED_LED_BLINK_PATH, O_RDWR);
    if (red_blink_fd < 0 && green_blink_fd < 0) {
        LOGE("Could not open red && green led blink node\n");
    } else {
        type_fd = open(CHARGER_TYPE_PATH, O_RDONLY);
        if (type_fd < 0) {
            LOGE("Could not open USB type node\n");
            close(red_blink_fd);
            close(green_blink_fd);
            return rc;
        } else {
            close(type_fd);
            if (red_blink_fd > 0) {
                rc |= RED_LED;
                bytes = snprintf(buf, sizeof(buf), "%d\n", 0);
                if (write(red_blink_fd, buf, bytes) < 0) {
                    LOGE("Fail to write: %s\n", RED_LED_BLINK_PATH);
                    rc = 0;
                }
                close(red_blink_fd);
            }
            if (green_blink_fd > 0) {
                rc |= GREEN_LED;
                bytes = snprintf(buf, sizeof(buf), "%d\n", 0);
                if (write(green_blink_fd, buf, bytes) < 0) {
                    LOGE("Fail to write: %s\n", GREEN_LED_BLINK_PATH);
                    rc = 0;
                }
                close(green_blink_fd);
            }
        }
    }

    return rc;
}

#define STR_LEN 8
void healthd_board_mode_charger_draw_battery(
                struct android::BatteryProperties *batt_prop)
{
    char cap_str[STR_LEN];
    int x, y;
    int str_len_px;
    static int char_height = -1, char_width = -1;

    if (char_height == -1 && char_width == -1)
        gr_font_size(&char_width, &char_height);
    snprintf(cap_str, (STR_LEN - 1), "%d%%", batt_prop->batteryLevel);
    str_len_px = gr_measure(cap_str);
    x = (gr_fb_width() - str_len_px) / 2;
    y = (gr_fb_height() + char_height) / 2;
    gr_color(0xa4, 0xc6, 0x39, 255);
    gr_text(x, y, cap_str, 0);
}

void healthd_board_mode_charger_battery_update(
                struct android::BatteryProperties *batt_prop)
{
    static int blink_for_hvdcp = -1;
    static int old_color = 0;
    int i, color, soc, rc;
    bool blink = false;

    if (blink_for_hvdcp == -1)
        blink_for_hvdcp = leds_blink_for_hvdcp_allow();

    if ((blink_for_hvdcp > 0) && is_hvdcp_inserted())
        blink = true;

    soc = batt_prop->batteryLevel;

    for (i = 0; i < ((int)ARRAY_SIZE(soc_leds) - 1); i++) {
        if (soc < soc_leds[i].soc)
            break;
    }
    color = soc_leds[i].color;

    if (old_color != color) {
        if ((color == HVDCP_COLOR_MAP) && blink) {
            if (blink_for_hvdcp & RED_LED) {
                rc = write_file_int(RED_LED_BLINK_PATH, HVDCP_BLINK_TYPE);
                if (rc < 0) {
                    LOGE("Fail to write: %s\n", RED_LED_BLINK_PATH);
                    return;
                }
            }
            if (blink_for_hvdcp & GREEN_LED) {
                rc = write_file_int(GREEN_LED_BLINK_PATH, HVDCP_BLINK_TYPE);
                if (rc < 0) {
                    LOGE("Fail to write: %s\n", GREEN_LED_BLINK_PATH);
                    return;
                }
            }
        } else {
                set_tricolor_led(0, old_color);
                set_tricolor_led(1, color);
                old_color = color;
                LOGV("soc = %d, set led color 0x%x\n", soc, soc_leds[i].color);
        }
    }
}

#define BACKLIGHT_ON_LEVEL    100
#define BACKLIGHT_OFF_LEVEL    0
void healthd_board_mode_charger_set_backlight(bool en)
{
    int fd;
    char buffer[10];

    if (access(BACKLIGHT_PATH, R_OK | W_OK) != 0)
    {
        LOGW("Backlight control not support\n");
        return;
    }

    memset(buffer, '\0', sizeof(buffer));
    fd = open(BACKLIGHT_PATH, O_RDWR);
    if (fd < 0) {
        LOGE("Could not open backlight node : %s\n", strerror(errno));
        goto cleanup;
    }
    LOGV("set backlight status to %d\n", en);
    if (en)
        snprintf(buffer, sizeof(buffer), "%d\n", BACKLIGHT_ON_LEVEL);
    else
        snprintf(buffer, sizeof(buffer), "%d\n", BACKLIGHT_OFF_LEVEL);

    if (write(fd, buffer,strlen(buffer)) < 0) {
        LOGE("Could not write to backlight node : %s\n", strerror(errno));
        goto cleanup;
    }
cleanup:
    if (fd >= 0)
        close(fd);
}

void healthd_board_mode_charger_init()
{
    int ret;
    char buff[8] = "\0";
    int charging_enabled = 0;
    int fd;

    /* check the charging is enabled or not */
    fd = open(CHARGING_ENABLED_PATH, O_RDONLY);
    if (fd < 0)
        return;
    ret = read(fd, buff, sizeof(buff));
    close(fd);
    if (ret > 0 && sscanf(buff, "%d\n", &charging_enabled)) {
        /* if charging is disabled, reboot and exit power off charging */
        if (charging_enabled)
            return;
        LOGW("android charging is disabled, exit!\n");
        android_reboot(ANDROID_RB_RESTART, 0, 0);
    }
}

void healthd_board_init(struct healthd_config*)
{
    // use defaults
}

int healthd_board_battery_update(struct android::BatteryProperties*)
{
    // return 0 to log periodic polled battery status to kernel log
    return 1;
}
