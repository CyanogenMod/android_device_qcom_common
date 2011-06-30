/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "edify/expr.h"
#include "firmware.h"

Value* UpdateFn(const char* name, State* state, int argc, Expr* argv[]) {
    if (argc != 1) {
        return ErrorAbort(state, "%s() expects 6 args, got %d", name, argc);
    }

    char* type = strrchr(name, '_');
    if (type == NULL || *(type+1) == '\0') {
        return ErrorAbort(state, "%s() couldn't get type from function name",
                          name);
    }
    ++type;

    Value* image;

    if (ReadValueArgs(state, argv, 1, &image) <0) {
        return NULL;
    }

    if (image->type != VAL_BLOB) {
        printf("image argument is not blob (is type %d)\n", image->type);
        goto done;
    }

    install_firmware_update(type, image->data, image->size, "/tmp/recovery.log");
    printf("%s: install_firmware_update returned!\n", name);

  done:
    FreeValue(image);
    // install_firmware_update should reboot.  If it returns, it failed.
    return StringValue(strdup(""));
}

Value* SetRadioFn(const char* name, State* state, int argc, Expr* argv[]) {
    char *part_type;

    if (argc != 1) {
        return ErrorAbort(state, "%s() expects arg, got %d", name, argc);
    }

    char* type = strrchr(name, '_');
    if (type == NULL || *(type+1) == '\0') {
        return ErrorAbort(state, "%s() couldn't get type from function name",
                          name);
    }
    ++type;

    if (ReadArgs(state, argv, 1, &part_type) <0) {
        return NULL;
    }

    start_firmware_update(type,part_type);

    return StringValue(strdup(""));
}

void Register_librecovery_updater_qcom() {
    fprintf(stderr, "installing QCOM updater extensions\n");

    RegisterFunction("qcom.install_radio", UpdateFn);
    RegisterFunction("qcom.set_radio", SetRadioFn);
}
