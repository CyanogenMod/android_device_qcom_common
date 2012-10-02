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

#define LOG_NIDEBUG 0

#include <dlfcn.h>
#include <fcntl.h>
#include <errno.h>

#include "utils.h"
#include "list.h"
#include "hint-data.h"

#define LOG_TAG "QCOM PowerHAL"
#include <utils/Log.h>

static int qcopt_handle_unavailable;
static void *qcopt_handle;
static int (*perf_vote_turnoff_ondemand_io_busy)(int vote);
static int perf_vote_ondemand_io_busy_unavailable;
static int (*perf_vote_lower_ondemand_sdf)(int vote);
static int perf_vote_ondemand_sdf_unavailable;
static int (*perf_lock_acq)(unsigned long handle, int duration,
    int list[], int numArgs);
static int perf_lock_acq_unavailable;
static int (*perf_lock_rel)(unsigned long handle);
static int perf_lock_rel_unavailable;
static struct list_node active_hint_list_head;

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

int sysfs_read(char *path, char *s, int num_bytes)
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

int sysfs_write(char *path, char *s)
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

int get_scaling_governor(char governor[], int size)
{
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

void perform_hint_action(int hint_id, int resource_values[], int num_resources)
{
    void *handle;

    if ((handle = get_qcopt_handle())) {
        if (!perf_lock_acq_unavailable && !perf_lock_rel_unavailable) {
            perf_lock_acq = dlsym(handle, "perf_lock_acq");

            if (perf_lock_acq) {
                /* Acquire an indefinite lock for the requested resources. */
                int lock_handle = perf_lock_acq(0, 0, resource_values,
                    num_resources);

                if (lock_handle == -1) {
                    ALOGE("Failed to acquire lock.");
                } else {
                    /* Add this handle to our internal hint-list. */
                    struct hint_data *new_hint =
                        (struct hint_data *)malloc(sizeof(struct hint_data));

                    if (new_hint) {
                        if (!active_hint_list_head.compare) {
                            active_hint_list_head.compare =
                                (int (*)(void *, void *))hint_compare;
                            active_hint_list_head.dump = (void (*)(void *))hint_dump;
                        }

                        new_hint->hint_id = hint_id;
                        new_hint->perflock_handle = lock_handle;

                        if (add_list_node(&active_hint_list_head, new_hint) == NULL) {
                            free(new_hint);
                            /* Can't keep track of this lock. Release it. */
                            perf_lock_rel(lock_handle);
                            ALOGE("Failed to process hint.");
                        }
                    } else {
                        /* Can't keep track of this lock. Release it. */
                        perf_lock_rel(lock_handle);
                        ALOGE("Failed to process hint.");
                    }
                }
            } else {
                perf_lock_acq_unavailable = 1;
                ALOGE("Can't commit hint action. Lock acquire function is not available.");
            }
        }
    }
}

void undo_hint_action(int hint_id)
{
    void *handle;

    if ((handle = get_qcopt_handle())) {
        if (!perf_lock_acq_unavailable && !perf_lock_rel_unavailable) {
            perf_lock_rel = dlsym(handle, "perf_lock_rel");

            if (perf_lock_rel) {
                /* Get hint-data associated with this hint-id */
                struct list_node *found_node;
                struct hint_data temp_hint_data = {
                    .hint_id = hint_id
                };

                if ((found_node = find_node(&active_hint_list_head,
                    &temp_hint_data)) != NULL) {
                    /* Release this lock. */
                    struct hint_data *found_hint_data =
                        (struct hint_data *)(found_node->data);

                    if (found_hint_data &&
                        (perf_lock_rel(found_hint_data->perflock_handle) == -1)) {
                        ALOGE("Perflock release failed.");
                    } else {
                        ALOGI("Perflock released.");
                    }

                    if (found_node->data) {
                        /* We can free the hint-data for this node. */
                        free(found_node->data);
                    }

                    remove_list_node(&active_hint_list_head, found_node);
                } else {
                    ALOGE("Invalid hint ID.");
                }
            } else {
                perf_lock_rel_unavailable = 1;
                ALOGE("Can't undo hint action. Lock release function is not available.");
            }
        }
    }
}

void vote_ondemand_io_busy_off()
{
    void *handle;

    if ((handle = get_qcopt_handle())) {
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
    }
}

void unvote_ondemand_io_busy_off()
{
    void *handle;

    if ((handle = get_qcopt_handle())) {
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
    }
}

void vote_ondemand_sdf_low()
{
    void *handle;

    if ((handle = get_qcopt_handle())) {
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
}

void unvote_ondemand_sdf_low() {
    void *handle;

    if ((handle = get_qcopt_handle())) {
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
