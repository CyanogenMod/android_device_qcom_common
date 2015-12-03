/*
 * Copyright (C) 2014 The Android Open Source Project
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <cutils/android_reboot.h>
#include <cutils/klog.h>
#include <cutils/misc.h>
#include <cutils/uevent.h>
#include <cutils/properties.h>

#include <pthread.h>
#include <linux/android_alarm.h>
#include <linux/rtc.h>
#include <linux/time.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include "healthd_msm.h"

#define LOGE(x...) do { KLOG_ERROR("charger", x); } while (0)
#define LOGI(x...) do { KLOG_INFO("charger", x); } while (0)
#define LOGV(x...) do { KLOG_DEBUG("charger", x); } while (0)
#define LOGW(x...) do { KLOG_WARNING("charger", x); } while (0)
enum alarm_time_type {
    ALARM_TIME,
    RTC_TIME,
};

/*
 * shouldn't be changed after
 * reading from alarm register
 */
static time_t alm_secs;

static int alarm_get_time(enum alarm_time_type time_type,
                          time_t *secs)
{
    struct tm tm;
    unsigned int cmd;
    int rc, fd = -1;

    if (!secs)
        return -1;

    fd = open("/dev/rtc0", O_RDONLY);
    if (fd < 0) {
        LOGE("Can't open rtc devfs node\n");
        return -1;
    }

    switch (time_type) {
        case ALARM_TIME:
            cmd = RTC_ALM_READ;
            break;
        case RTC_TIME:
            cmd = RTC_RD_TIME;
            break;
        default:
            LOGE("Invalid time type\n");
            goto err;
    }

    rc = ioctl(fd, cmd, &tm);
    if (rc < 0) {
        LOGE("Unable to get time\n");
        goto err;
    }

    *secs = mktime(&tm) + tm.tm_gmtoff;
    if (*secs < 0) {
        LOGE("Invalid seconds = %ld\n", *secs);
        goto err;
    }

    close(fd);
    return 0;

err:
    close(fd);
    return -1;
}

#define ERR_SECS 2
static int alarm_is_alm_expired()
{
    int rc;
    time_t rtc_secs;

    rc = alarm_get_time(RTC_TIME, &rtc_secs);
    if (rc < 0)
        return 0;

    return (alm_secs >= rtc_secs - ERR_SECS &&
            alm_secs <= rtc_secs + ERR_SECS) ? 1 : 0;
}

static int alarm_set_reboot_time_and_wait(time_t secs)
{
    int rc, epollfd, nevents;
    int fd = 0;
    struct timespec ts;
    epoll_event event, events[1];
    struct itimerspec itval;

    epollfd = epoll_create(1);
    if (epollfd < 0) {
        LOGE("epoll_create failed\n");
        goto err;
    }

    fd = timerfd_create(CLOCK_REALTIME_ALARM, 0);
    if (fd < 0) {
        LOGE("timerfd_create failed\n");
        goto err;
    }

    event.events = EPOLLIN | EPOLLWAKEUP;
    rc = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    if (rc < 0) {
        LOGE("epoll_ctl(EPOLL_CTL_ADD) failed \n");
        goto err;
    }

    itval.it_value.tv_sec = secs;
    itval.it_value.tv_nsec = 0;

    itval.it_interval.tv_sec = 0;
    itval.it_interval.tv_nsec = 0;

    rc = timerfd_settime(fd, TFD_TIMER_ABSTIME, &itval, NULL);
    if (rc < 0) {
        LOGE("timerfd_settime failed %d\n",rc);
        goto err;
    }

    do {
         nevents = epoll_wait(epollfd, events, 1, -1);
    } while ((nevents < 0) || !alarm_is_alm_expired());

    if (nevents <= 0) {
        LOGE("Unable to wait on alarm\n");
        goto err;
    }

    close(epollfd);
    close(fd);
    return 0;

err:
    if (epollfd > 0)
        close(epollfd);

    if (fd >= 0)
        close(fd);
    return -1;
}

static void *alarm_thread(void *)
{
    time_t rtc_secs, rb_secs;
    int rc;

    /*
     * to support power off alarm, the time
     * stored in alarm register at latest
     * shutdown time should be some time
     * earlier than the actual alarm time
     * set by user
     */
    rc = alarm_get_time(ALARM_TIME, &alm_secs);
    if (rc < 0 || !alm_secs)
        goto err;

    rc = alarm_set_reboot_time_and_wait(alm_secs);
    if (rc < 0)
        goto err;

    LOGI("Exit from power off charging, reboot the phone!\n");
    android_reboot(ANDROID_RB_RESTART2, 0, (char *)"rtc");

err:
    LOGE("Exit from alarm thread\n");
    return NULL;
}

void power_off_alarm_init(void)
{
    pthread_t tid;
    int rc;
    char value[PROP_VALUE_MAX];

    property_get("ro.bootmode", value, "");
    if (!strcmp("charger", value)) {
        rc = pthread_create(&tid, NULL, alarm_thread, NULL);
        if (rc < 0)
            LOGE("Create alarm thread failed\n");
    }
}
