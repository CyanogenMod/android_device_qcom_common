/*
 * Copyright (C) 2010 The Android Open Source Project
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
#include <stdio.h>
#include <string.h>

#include "bootloader.h"
#include "mtdutils/mtdutils.h"

#define LOGE(...) fprintf(stderr, "E:" __VA_ARGS__)

void recover_firmware_update_log() {
    printf("recovering log from before firmware update\n");

    mtd_scan_partitions();
    const MtdPartition *part = mtd_find_partition_by_name(CACHE_NAME);
    if (part == NULL) {
        LOGE("Can't find %s\n", CACHE_NAME);
        return;
    }

    MtdReadContext* read = mtd_read_partition(part);

    size_t erase_size;
    if (mtd_partition_info(part, NULL, &erase_size, NULL) != 0) {
        LOGE("Error reading block size\n(%s)\n", strerror(errno));
        mtd_read_close(read);
        return;
    }

    char* buffer = malloc(erase_size);
    if (mtd_read_data(read, buffer, erase_size) != erase_size) {
        LOGE("Error reading header block\n(%s)\n", strerror(errno));
        mtd_read_close(read);
        free(buffer);
        return;
    }
    if (mtd_read_data(read, buffer, erase_size) != erase_size) {
        LOGE("Error reading log block\n(%s)\n", strerror(errno));
        mtd_read_close(read);
        free(buffer);
        return;
    }
    mtd_read_close(read);

    if (memcmp(buffer, LOG_MAGIC, LOG_MAGIC_SIZE) != 0) {
        printf("no log to recover from cache partition\n");
        free(buffer);
        return;
    }

    size_t log_size = *(size_t *)(buffer + LOG_MAGIC_SIZE);

    printf("\n###\n### START RECOVERED LOG\n###\n\n");
    fwrite(buffer + sizeof(size_t) + LOG_MAGIC_SIZE, 1, log_size, stdout);
    printf("\n\n###\n### END RECOVERED LOG\n###\n\n");

    free(buffer);
}
