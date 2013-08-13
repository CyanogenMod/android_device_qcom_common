/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
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

#include <errno.h>
#include <fcntl.h>
#include <syslog.h>
#include <linux/random.h>
#include <linux/qrng.h>
#include <linux/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#ifdef ANDROID_CHANGES
#include <android/log.h>
#endif

#define RANDOM_DEVICE_HW    "/dev/msm-rng"

#define APP_VERSION "1.0"
#define APP_NAME    "qrngp"

/* Logging information */
enum log_level {
	DEBUG = 0,
	INFO = 1,
	WARNING = 2,
	ERROR = 3,
	FATAL = 4,
	LOG_MAX = 4,
};

/* Logging function for outputing to stderr or log */
void log_print(int level, char *format, ...)
{
	if (level >= 0 && level <= LOG_MAX) {
#ifdef ANDROID_CHANGES
		static int levels[5] = {
			ANDROID_LOG_DEBUG, ANDROID_LOG_INFO, ANDROID_LOG_WARN,
			ANDROID_LOG_ERROR, ANDROID_LOG_FATAL
		};
		va_list ap;
		va_start(ap, format);
		__android_log_vprint(levels[level], APP_NAME, format, ap);
		va_end(ap);
#else
		static char *levels = "DIWEF";
		va_list ap;
		fprintf(stderr, "%c: ", levels[level]);
		va_start(ap, format);
		vfprintf(stderr, format, ap);
		va_end(ap);
		fputc('\n', stderr);
#endif
	}
}

/* The beginning of everything */
int main(int argc, char **argv)
{
	int random_hw_fd = 0;/* input file descriptor             */
	int ret;

	log_print(INFO, "Entering qrngp");
	/* open hardware random device */
	random_hw_fd = open(RANDOM_DEVICE_HW, O_RDONLY);
	log_print(INFO,"msm_rng fd = %d\n",random_hw_fd);
	if(random_hw_fd < 0) {
		log_print(ERROR, "Invalid fd, device open failed\n");
		return -ENODEV;
	}
	ret = ioctl(random_hw_fd,QRNG_IOCTL_RESET_BUS_BANDWIDTH);
	if (ret)
		log_print(ERROR, "Failed to reset BUS BANDWIDTH, errno = %s", strerror(errno));
	else
		log_print(INFO, "bus_bandwidth_reset success");

	close(random_hw_fd);
	return ret;
}
