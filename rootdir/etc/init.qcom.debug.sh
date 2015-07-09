#!/system/bin/sh
# Copyright (c) 2014, The Linux Foundation. All rights reserved.
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

# function to enable ftrace events to CoreSight STM
enable_stm_events()
{
    # bail out if its perf config
    if [ ! -d /sys/module/msm_rtb ]
    then
        return
    fi
    # bail out if coresight isn't present
    if [ ! -d /sys/bus/coresight ]
    then
        return
    fi
    # bail out if ftrace events aren't present
    if [ ! -d /sys/kernel/debug/tracing/events ]
    then
        return
    fi

    echo 1 > /sys/bus/coresight/devices/coresight-tmc-etr/curr_sink
    echo 1 > /sys/bus/coresight/devices/coresight-stm/enable
    echo 1 > /sys/kernel/debug/tracing/tracing_on
    # schedular
    echo 1 > /sys/kernel/debug/tracing/events/sched/sched_cpu_hotplug/enable
    echo 1 > /sys/kernel/debug/tracing/events/sched/sched_cpu_load/enable
    echo 1 > /sys/kernel/debug/tracing/events/sched/sched_enq_deq_task/enable
    echo 1 > /sys/kernel/debug/tracing/events/sched/sched_load_balance/enable
    echo 1 > /sys/kernel/debug/tracing/events/sched/sched_migrate_task/enable
    echo 1 > /sys/kernel/debug/tracing/events/sched/sched_switch/enable
    echo 1 > /sys/kernel/debug/tracing/events/sched/sched_task_load/enable
    echo 1 > /sys/kernel/debug/tracing/events/sched/sched_update_history/enable
    echo 1 > /sys/kernel/debug/tracing/events/sched/sched_update_task_ravg/enable
    echo 1 > /sys/kernel/debug/tracing/events/sched/sched_wakeup/enable
    echo 1 > /sys/kernel/debug/tracing/events/sched/sched_wakeup_new/enable
    # sound
    echo 1 > /sys/kernel/debug/tracing/events/asoc/snd_soc_reg_read/enable
    echo 1 > /sys/kernel/debug/tracing/events/asoc/snd_soc_reg_write/enable
    # mdp
    echo 1 > /sys/kernel/debug/tracing/events/mdss/mdp_video_underrun_done/enable
    # video
    echo 1 > /sys/kernel/debug/tracing/events/msm_vidc/enable
    # clock
    echo 1 > /sys/kernel/debug/tracing/events/power/clock_set_rate/enable
    # regulator
    echo 1 > /sys/kernel/debug/tracing/events/regulator/enable
    # power
    echo 1 > /sys/kernel/debug/tracing/events/msm_low_power/enable
    #thermal
    echo 1 > /sys/kernel/debug/tracing/events/thermal/thermal_pre_core_offline/enable
    echo 1 > /sys/kernel/debug/tracing/events/thermal/thermal_post_core_offline/enable
    echo 1 > /sys/kernel/debug/tracing/events/thermal/thermal_pre_core_online/enable
    echo 1 > /sys/kernel/debug/tracing/events/thermal/thermal_post_core_online/enable
    echo 1 > /sys/kernel/debug/tracing/events/thermal/thermal_pre_frequency_mit/enable
    echo 1 > /sys/kernel/debug/tracing/events/thermal/thermal_post_frequency_mit/enable
}

# Function MSM8996 DCC configuration
enable_msm8996_dcc_config()
{
    echo  0 > /sys/bus/platform/devices/4b3000.dcc/enable
    echo cap > /sys/bus/platform/devices/4b3000.dcc/func_type
    echo sram > /sys/bus/platform/devices/4b3000.dcc/data_sink
    echo  1 > /sys/bus/platform/devices/4b3000.dcc/config_reset

    #SPM Registers
    # CPU0
    echo  0x998000C > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x9980030 > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x998003C > /sys/bus/platform/devices/4b3000.dcc/config
    # CPU1
    echo  0x999000C > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x9990030 > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x999003C > /sys/bus/platform/devices/4b3000.dcc/config
    # CPU2
    echo  0x99B000C > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x99B0030 > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x99B003C > /sys/bus/platform/devices/4b3000.dcc/config
    # CPU3
    echo  0x99C000C > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x99C0030 > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x99C003C > /sys/bus/platform/devices/4b3000.dcc/config
    # PWRL2
    echo  0x99A000C > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x99A0030 > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x99A003C > /sys/bus/platform/devices/4b3000.dcc/config
    # PERFL2
    echo  0x99D000C > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x99D0030 > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x99D003C > /sys/bus/platform/devices/4b3000.dcc/config
    # L3
    echo  0x9A0000C > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x9A00030 > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x9A0003C > /sys/bus/platform/devices/4b3000.dcc/config
    # CBF
    echo  0x9A1000C > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x9A10030 > /sys/bus/platform/devices/4b3000.dcc/config
    echo  0x9A1003C > /sys/bus/platform/devices/4b3000.dcc/config
    # PWR L2 HW-FLUSH
    echo  0x99A1060 > /sys/bus/platform/devices/4b3000.dcc/config
    # PERF L2 HW-FLUSH
    echo  0x99D1060 > /sys/bus/platform/devices/4b3000.dcc/config
    # APCS_APC0_SLEEP_EN_VOTE
    echo  0x99A2030 > /sys/bus/platform/devices/4b3000.dcc/config
    # APCS_APCC_SW_EN_VOTE
    echo  0x99E0020 > /sys/bus/platform/devices/4b3000.dcc/config

    echo  1 > /sys/bus/platform/devices/4b3000.dcc/enable
}

# Function DCC configuration
enable_dcc_config()
{
    target=`getprop ro.board.platform`

    case "$target" in
        "msm8996")
            echo "Enabling DCC config."
            enable_msm8996_dcc_config
            ;;
    esac
}

coresight_config=`getprop ro.dbg.coresight.config`

enable_dcc_config
case "$coresight_config" in
    "stm_events")
        echo "Enabling STM events."
        enable_stm_events
        ;;
    *)
        echo "Skipping coresight configuration."
        exit
        ;;
esac
