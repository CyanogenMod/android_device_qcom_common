/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <hardware/power.h>
#include "power-feature.h"

#define NODE_MAX 64

extern void set_device_specific_feature(struct power_module *module, feature_t feature, int state);

void set_feature(struct power_module *module, feature_t feature, int state)
{
#ifdef TAP_TO_WAKE_NODE
    char tmp_str[NODE_MAX];
    if (feature == POWER_FEATURE_DOUBLE_TAP_TO_WAKE) {
        snprintf(tmp_str, NODE_MAX, "%d", state);
        sysfs_write(TAP_TO_WAKE_NODE, tmp_str);
        return;
    }
#endif
    set_device_specific_feature(module, feature, state);
}

