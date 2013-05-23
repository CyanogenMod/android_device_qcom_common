#!/system/bin/sh
# Copyright (c) 2013, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of The Linux Foundation nor
#       the names of its contributors may be used to endorse or promote
#       products derived from this software without specific prior written
#       permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

ssr_str="$1"
IFS=,
ssr_array=($ssr_str)
declare -i ssr_status=0

for num in "${!ssr_array[@]}"
do
    case "${ssr_array[$num]}" in
        "subsys0")
            ssr_status=$ssr_status+1
        ;;
        "subsys1")
            ssr_status=$ssr_status+2
        ;;
        "subsys2")
            ssr_status=$ssr_status+4
        ;;
        "subsys3")
            ssr_status=$ssr_status+8
        ;;
        "3")
            ssr_status=$ssr_status+16
        ;;
        "riva")
            ssr_status=$ssr_status+32
        ;;
    esac
done

if [ $((ssr_status & 1)) == 1 ]; then
    echo "related" > /sys/bus/msm_subsys/devices/subsys0/restart_level
else
    echo "system" > /sys/bus/msm_subsys/devices/subsys0/restart_level
fi

if [ $((ssr_status & 2)) == 2 ]; then
    echo "related" > /sys/bus/msm_subsys/devices/subsys1/restart_level
else
    echo "system" > /sys/bus/msm_subsys/devices/subsys1/restart_level
fi

if [ $((ssr_status & 4)) == 4 ]; then
    echo "related" > /sys/bus/msm_subsys/devices/subsys2/restart_level
else
    echo "system" > /sys/bus/msm_subsys/devices/subsys2/restart_level
fi

if [ $((ssr_status & 8)) == 8 ]; then
    echo "related" > /sys/bus/msm_subsys/devices/subsys3/restart_level
else
    echo "system" > /sys/bus/msm_subsys/devices/subsys3/restart_level
fi

if [ $((ssr_status & 16)) == 16 ]; then
    echo 3 > /sys/module/subsystem_restart/parameters/restart_level
else
    echo 1 > /sys/module/subsystem_restart/parameters/restart_level
fi

if [ $((ssr_status & 32)) == 32 ]; then
    echo 1 > /sys/module/wcnss_ssr_8960/parameters/enable_riva_ssr
else
    echo 0 > /sys/module/wcnss_ssr_8960/parameters/enable_riva_ssr
fi
