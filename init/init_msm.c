/*
   Copyright (c) 2013, The Linux Foundation. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vendor_init.h"
#include "property_service.h"
#include "log.h"
#include "util.h"

#include "init_msm.h"

#define SOC_ID_PATH1     "/sys/devices/soc0/soc_id"
#define SOC_ID_PATH2     "/sys/devices/system/soc/soc0/id"
#define SOC_VER_PATH1    "/sys/devices/soc0/platform_version"
#define SOC_VER_PATH2    "/sys/devices/system/soc/soc0/platform_version"
#define BOARD_TYPE_PATH1 "/sys/devices/soc0/hw_platform"
#define BOARD_TYPE_PATH2 "/sys/devices/system/soc/soc0/hw_platform"

#define BUF_SIZE         64
#define STRCONV_(x)      #x
#define STRCONV(x)       "%" STRCONV_(x) "s"

static unsigned long msm_id;
static unsigned long msm_ver;
static char board_type[BUF_SIZE];
static char tmp[BUF_SIZE];

__attribute__ ((weak))
void init_msm_properties(unsigned long soc, unsigned long socrev, char *board)
{
    UNUSED(soc);
    UNUSED(socrev);
    UNUSED(board);
}

static int read_file2(const char *fname, char *data, int max_size)
{
    int fd, rc;

    if (max_size < 1)
        return 0;

    fd = open(fname, O_RDONLY);
    if (fd < 0) {
        ERROR("failed to open '%s'\n", fname);
        return 0;
    }

    rc = read(fd, data, max_size - 1);
    if ((rc > 0) && (rc < max_size))
        data[rc] = '\0';
    else
        data[0] = '\0';
    close(fd);

    return 1;
}

void vendor_load_properties()
{
    int rc;

    /* Collect MSM info */
    rc = read_file2(SOC_ID_PATH1, tmp, sizeof(tmp));
    if (!rc) {
        rc = read_file2(SOC_ID_PATH2, tmp, sizeof(tmp));
    }
    if (rc) {
        msm_id = strtoul(tmp, NULL, 0);
    }
    rc = read_file2(SOC_VER_PATH1, tmp, sizeof(tmp));
    if (!rc) {
        rc = read_file2(SOC_VER_PATH2, tmp, sizeof(tmp));
    }
    if (rc) {
        msm_ver = strtoul(tmp, NULL, 0);
    }
    rc = read_file2(BOARD_TYPE_PATH1, tmp, sizeof(tmp));
    if (!rc) {
        rc = read_file2(BOARD_TYPE_PATH2, tmp, sizeof(tmp));
    }
    if (rc) {
        sscanf(tmp, STRCONV(BUF_SIZE), board_type);
    }
    if (!msm_id) {
        /* abort */
        ERROR("MSM SOC detection failed, skipping MSM initialization\n");
        return;
    }

    ERROR("Detected MSM SOC ID=%lu SOC VER=%lu BOARD TYPE=%s\n",
          msm_id, msm_ver, board_type);

    /* Define MSM family properties */
    init_msm_properties(msm_id, msm_ver, board_type);
}
