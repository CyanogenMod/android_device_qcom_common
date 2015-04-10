/*
   Copyright (c) 2015, The Linux Foundation. All rights reserved.

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
#include <fcntl.h>

#include "vendor_init.h"
#include "property_service.h"
#include "log.h"
#include "util.h"

#include "init_msm.h"

#define VIRTUAL_SIZE "/sys/class/graphics/fb0/virtual_size"
#define VERSION "/sys/devices/soc.0/1d00000.qcom,vidc/version"
#define BUF_SIZE 64

int get_version(int *version)
{
    char buf[16];
    int pos = 0, rv = 0, fd = -1;
    long value = 0;
    char *endptr = NULL;
    int bytes_to_read = sizeof(buf);

    fd = open(VERSION, O_RDONLY, 0660);
    if (fd < 0) {
        return -1;
    }

    do {
        pos += rv;
        if ((bytes_to_read - pos) <= 0)
            break;
        rv += read(fd, buf + pos, bytes_to_read - pos);
    } while (rv > 0);

    if (rv < 0) {
        return -1;
    }
    buf[pos - 1] = '\0';
    value = strtol(buf, &endptr, sizeof(long));
    if (endptr == buf) {
        return -1;
    }
    *version = (int)value;

    return 1;
}

void init_msm_properties(unsigned long msm_id, unsigned long msm_ver, char *board_type)
{
    char platform[PROP_VALUE_MAX];
    int rc;
    int version = 0;
    unsigned long virtual_size = 0;
    char str[BUF_SIZE];

    UNUSED(msm_id);
    UNUSED(msm_ver);
    UNUSED(board_type);

    rc = property_get("ro.board.platform", platform);
    if (!rc || !ISMATCH(platform, ANDROID_TARGET)){
        return;
    }

    rc = read_file2(VIRTUAL_SIZE, str, sizeof(str));
    if (rc) {
        virtual_size = strtoul(str, NULL, 0);
    }

    if(virtual_size >= 1080) {
        property_set(PROP_LCDDENSITY, "480");
    } else if (virtual_size >= 720) {
        // For 720x1280 resolution
        property_set(PROP_LCDDENSITY, "320");
    } else if (virtual_size >= 480) {
        // For 480x854 resolution QRD.
        property_set(PROP_LCDDENSITY, "240");
    } else
        property_set(PROP_LCDDENSITY, "320");

    if (msm_id == 266 || msm_id == 278 || msm_id == 277 || msm_id == 274) {
        property_set("media.msm8956hw", "1");
        property_set("media.settings.xml", "/etc/media_profiles_8956.xml");
        get_version(&version);
        if (version == 1)
            property_set("media.msm8956.version", "1");
        else
            property_set("media.msm8956.version", "0");
    }
}
