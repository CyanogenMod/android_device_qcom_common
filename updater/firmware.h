/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef _RECOVERY_FIRMWARE_H
#define _RECOVERY_FIRMWARE_H

/* Reboot into the bootloader to install the given update.
 * Returns 0 if no radio image was defined, nonzero on error,
 * doesn't return at all on success...
 */
int install_firmware_update(const char *update_type,
                            const char *update_data,
                            size_t update_length,
                            const char *log_filename);

#endif
