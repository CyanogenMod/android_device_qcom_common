# Board platforms lists to be used for
# TARGET_BOARD_PLATFORM specific featurization
QCOM_BOARD_PLATFORMS := msm7627_surf
QCOM_BOARD_PLATFORMS += msm7627_6x
QCOM_BOARD_PLATFORMS += msm7627a
QCOM_BOARD_PLATFORMS += msm7630_surf
QCOM_BOARD_PLATFORMS += msm7630_fusion
QCOM_BOARD_PLATFORMS += msm8660
QCOM_BOARD_PLATFORMS += msm8960
QCOM_BOARD_PLATFORMS += msm8974
QCOM_BOARD_PLATFORMS += msm8610
QCOM_BOARD_PLATFORMS += msm8226
QCOM_BOARD_PLATFORMS += apq8084
QCOM_BOARD_PLATFORMS += mpq8092

MSM7K_BOARD_PLATFORMS := msm7630_surf
MSM7K_BOARD_PLATFORMS += msm7630_fusion
MSM7K_BOARD_PLATFORMS += msm7627_surf
MSM7K_BOARD_PLATFORMS += msm7627_6x
MSM7K_BOARD_PLATFORMS += msm7627a
MSM7K_BOARD_PLATFORMS += msm7k

QSD8K_BOARD_PLATFORMS := qsd8k

TARGET_USE_VENDOR_CAMERA_EXT := true

# Below projects/packages with LOCAL_MODULEs will be used by
# PRODUCT_PACKAGES to build LOCAL_MODULEs that are tagged with
# optional tag, which will not be available on target unless
# explicitly list here. Where project corresponds to the vars here
# in CAPs.

#ALSA
ALSA_HARDWARE := alsa.msm8960
ALSA_HARDWARE += alsa.msm8974
ALSA_HARDWARE += alsa.msm8226
ALSA_HARDWARE += alsa.msm8610
ALSA_HARDWARE += alsa.apq8084

ALSA_UCM := snd_soc_msm
ALSA_UCM += snd_soc_msm_2x
ALSA_UCM += snd_soc_msm_2x_mpq
ALSA_UCM += snd_soc_msm_2x_Fusion3
ALSA_UCM += snd_soc_msm_Sitar
ALSA_UCM += snd_soc_msm_auxpcm
ALSA_UCM += snd_soc_msm_2x_auxpcm
ALSA_UCM += snd_soc_msm_2x_mpq_auxpcm
ALSA_UCM += snd_soc_msm_2x_Fusion3_auxpcm
ALSA_UCM += snd_soc_msm_Sitar_auxpcm
ALSA_UCM += snd_soc_msm_Taiko
ALSA_UCM += snd_soc_msm_Taiko_CDP
ALSA_UCM += snd_soc_msm_Taiko_Fluid
ALSA_UCM += snd_soc_msm_Taiko_liquid
ALSA_UCM += snd_soc_apq_Taiko_DB
ALSA_UCM += snd_soc_msm_I2SFusion
ALSA_UCM += snd_soc_msm_Tapan
ALSA_UCM += snd_soc_msm_TapanLite
ALSA_UCM += snd_soc_msm_Tapan_SKUF
ALSA_UCM += snd_soc_msm_TapanLite_SKUF
ALSA_UCM += snd_soc_msm_8x10_wcd
ALSA_UCM += snd_soc_msm_8x10_wcd_skuab
ALSA_UCM += snd_soc_msm_8x10_wcd_skuaa
ALSA_UCM += snd_soc_msm_samarium_Tapan

#ANGLE
ANGLE := libangle

AUDIO_HARDWARE := audio.primary.mpq8064
AUDIO_HARDWARE += audio.primary.apq8084
AUDIO_HARDWARE += audio.primary.msm8960
AUDIO_HARDWARE += audio.primary.msm8974
AUDIO_HARDWARE += audio.primary.msm8226
AUDIO_HARDWARE += audio.primary.msm8660
AUDIO_HARDWARE += audio.primary.msm8610
#AUDIO_HARDWARE += audio.primary.msm7627_surf
AUDIO_HARDWARE += audio.primary.msm7627a
AUDIO_HARDWARE += audio.primary.msm7630_surf
AUDIO_HARDWARE += audio.primary.msm7630_fusion
#AUDIO_HARDWARE += audio.primary.default
AUDIO_HARDWARE += audio.a2dp.default
AUDIO_HARDWARE += audio.usb.default
#
AUDIO_POLICY := audio_policy.mpq8064
AUDIO_POLICY += audio_policy.apq8084
AUDIO_POLICY += audio_policy.msm8960
AUDIO_POLICY += audio_policy.msm8974
AUDIO_POLICY += audio_policy.msm8226
AUDIO_POLICY += audio_policy.msm8660
AUDIO_POLICY += audio_policy.msm8610
#AUDIO_POLICY += audio_policy.msm7627_surf
AUDIO_POLICY += audio_policy.msm7627a
AUDIO_POLICY += audio_policy.msm7630_surf
AUDIO_POLICY += audio_policy.msm7630_fusion
#AUDIO_POLICY += audio_policy.default
AUDIO_POLICY += audio_policy.conf
AUDIO_POLICY += audio_policy_8064.conf

#tinyalsa test apps
TINY_ALSA_TEST_APPS := tinyplay
TINY_ALSA_TEST_APPS += tinycap
TINY_ALSA_TEST_APPS += tinymix
TINY_ALSA_TEST_APPS += tinypcminfo
TINY_ALSA_TEST_APPS += cplay

#AMPLOADER
AMPLOADER := amploader

#APPS
APPS := QualcommSoftAP
APPS += TSCalibration

#BRCTL
BRCTL := brctl
BRTCL += libbridge

#BSON
BSON := libbson

#BT
BT := javax.btobex
BT += libattrib_static
BT += hcidump.sh
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/qcom/common

#C2DColorConvert
C2DCC := libc2dcolorconvert

#CIMAX
CIMAX := libcimax_spi

#CONNECTIVITY
CONNECTIVITY := libcnefeatureconfig
CONNECTIVITY += services-ext

#CURL
CURL := libcurl
CURL += curl

#DASH
DASH := libdashplayer
DASH += qcmediaplayer

#DATA_OS
DATA_OS := librmnetctl
DATA_OS += rmnetcli

#E2FSPROGS
E2FSPROGS := e2fsck

#EBTABLES
EBTABLES := ebtables
EBTABLES += ethertypes
EBTABLES += libebtc

#FASTPOWERON
FASTPOWERON := FastBoot

#FM
FM := qcom.fmradio
FM += libqcomfm_jni

#GPS
GPS_HARDWARE := gps.conf
GPS_HARDWARE += gps.default
GPS_HARDWARE += gps.mahimahi
GPS_HARDWARE += libloc_adapter
GPS_HARDWARE += libgps.utils
GPS_HARDWARE += libloc_eng
GPS_HARDWARE += libloc_api_v02

#HDMID
HDMID := hdmid

#HOSTAPD
HOSTAPD := hostapd
HOSTAPD += hostapd_cli
HOSTAPD += nt_password_hash
HOSTAPD += hlr_auc_gw
HOSTAPD += test-milenage
HOSTAPD += hostapd.conf
HOSTAPD += hostapd_default.conf
HOSTAPD += hostapd.deny
HOSTAPD += hostapd.accept

#I420COLORCONVERT
I420CC := libI420colorconvert

#INIT
INIT := init.qcom.composition_type.sh
INIT += init.target.8x25.sh
INIT += init.qcom.mdm_links.sh
INIT += init.qcom.modem_links.sh
INIT += init.qcom.sensor.sh
INIT += init.target.rc
INIT += init.qcom.bt.sh
INIT += hsic.control.bt.sh
INIT += init.qcom.coex.sh
INIT += init.qcom.fm.sh
INIT += init.qcom.early_boot.sh
INIT += init.qcom.post_boot.sh
INIT += init.qcom.syspart_fixup.sh
INIT += init.qcom.rc
INIT += init.qcom.factory.sh
INIT += init.qcom.sdio.sh
INIT += init.qcom.sh
INIT += init.qcom.class_core.sh
INIT += init.qcom.wifi.sh
INIT += vold.fstab
INIT += init.qcom.ril.path.sh
INIT += init.ril.rc
INIT += init.qcom.ril.sh
INIT += init.qcom.usb.rc
INIT += init.qcom.usb.sh
INIT += usf_post_boot.sh
INIT += init.qcom.efs.sync.sh
INIT += ueventd.qcom.rc
INIT += init.ath3k.bt.sh
INIT += init.qcom.audio.sh
INIT += init.qcom.ssr.sh
INIT += init.mdm.sh

#IPROUTE2
IPROUTE2 := ip
IPROUTE2 += libiprouteutil

#IPTABLES
IPTABLES := libiptc
IPTABLES += libext
IPTABLES += iptables

#KERNEL_TESTS
KERNEL_TESTS := mm-audio-native-test

#KEYPAD
KEYPAD := ffa-keypad_qwerty.kcm
KEYPAD += ffa-keypad_numeric.kcm
KEYPAD += fluid-keypad_qwerty.kcm
KEYPAD += fluid-keypad_numeric.kcm
KEYPAD += surf_keypad_qwerty.kcm
KEYPAD += surf_keypad_numeric.kcm
KEYPAD += surf_keypad.kcm
KEYPAD += 7k_ffa_keypad.kcm
KEYPAD += 7x27a_kp.kcm
KEYPAD += keypad_8960_qwerty.kcm
KEYPAD += 7k_ffa_keypad.kl
KEYPAD += 7k_handset.kl
KEYPAD += 7x27a_kp.kl
KEYPAD += 8660_handset.kl
KEYPAD += atmel_mxt_ts.kl
KEYPAD += synaptics_rmi4_i2c.kl
KEYPAD += cyttsp-i2c.kl
KEYPAD += ft5x06_ts.kl
KEYPAD += ffa-keypad.kl
KEYPAD += fluid-keypad.kl
KEYPAD += gpio-keys.kl
KEYPAD += keypad_8960.kl
KEYPAD += keypad_8960_liquid.kl
KEYPAD += synaptics_rmi4_i2c.kl
KEYPAD += msm_tma300_ts.kl
KEYPAD += philips_remote_ir.kl
KEYPAD += samsung_remote_ir.kl
KEYPAD += surf_keypad.kl
KEYPAD += ue_rf4ce_remote.kl

#KS
KS := ks
KS += qcks
KS += efsks

#LIB_NL
LIB_NL := libnl_2

#LIB_XML2
LIB_XML2 := libxml2

#LIBCAMERA
LIBCAMERA := camera.apq8084
LIBCAMERA += camera.msm8974
LIBCAMERA += camera.msm8226
LIBCAMERA += camera.msm8610
LIBCAMERA += camera.msm8960
LIBCAMERA += camera.msm8660
LIBCAMERA += camera.msm7630_surf
LIBCAMERA += camera.msm7630_fusion
LIBCAMERA += camera.msm7627a
LIBCAMERA += libcamera
LIBCAMERA += libmmcamera_interface
LIBCAMERA += libmmcamera_interface2
LIBCAMERA += libmmjpeg_interface
LIBCAMERA += libqomx_core
LIBCAMERA += mm-qcamera-app

#LIBCOPYBIT
LIBCOPYBIT := copybit.msm8660
LIBCOPYBIT += copybit.msm8960
LIBCOPYBIT += copybit.msm8974
LIBCOPYBIT += copybit.msm8226
LIBCOPYBIT += copybit.msm8610
LIBCOPYBIT += copybit.apq8084
LIBCOPYBIT += copybit.msm7k
LIBCOPYBIT += copybit.qsd8k
LIBCOPYBIT += copybit.msm7630_surf
LIBCOPYBIT += copybit.msm7630_fusion
LIBCOPYBIT += copybit.msm7627_surf
LIBCOPYBIT += copybit.msm7627_6x
LIBCOPYBIT += copybit.msm7627a

#LIBGESTURES
LIBGESTURES := libgestures
LIBGESTURES += gestures.msm8960

#LIBGRALLOC
LIBGRALLOC := gralloc.default
LIBGRALLOC += gralloc.msm8660
LIBGRALLOC += gralloc.msm8960
LIBGRALLOC += gralloc.msm8974
LIBGRALLOC += gralloc.msm8226
LIBGRALLOC += gralloc.msm8610
LIBGRALLOC += gralloc.apq8084
LIBGRALLOC += gralloc.msm7k
LIBGRALLOC += gralloc.msm7630_surf
LIBGRALLOC += gralloc.msm7630_fusion
LIBGRALLOC += gralloc.msm7627_surf
LIBGRALLOC += gralloc.msm7627_6x
LIBGRALLOC += gralloc.msm7627a
LIBGRALLOC += libmemalloc

#LIBLIGHTS
LIBLIGHTS := lights.msm8660
LIBLIGHTS += lights.msm8960
LIBLIGHTS += lights.msm8974
LIBLIGHTS += lights.msm8226
LIBLIGHTS += lights.msm7k
LIBLIGHTS += lights.msm7630_surf
LIBLIGHTS += lights.msm7630_fusion
LIBLIGHTS += lights.msm7627_surf
LIBLIGHTS += lights.msm7627_6x
LIBLIGHTS += lights.msm7627a
LIBLIGHTS += lights.msm8610
LIBLIGHTS += lights.apq8084

#LIBHWCOMPOSER
LIBHWCOMPOSER := hwcomposer.msm8660
LIBHWCOMPOSER += hwcomposer.msm8960
LIBHWCOMPOSER += hwcomposer.msm8974
LIBHWCOMPOSER += hwcomposer.msm8226
LIBHWCOMPOSER += hwcomposer.msm8610
LIBHWCOMPOSER += hwcomposer.apq8084
LIBHWCOMPOSER += hwcomposer.msm7k
LIBHWCOMPOSER += hwcomposer.msm7630_surf
LIBHWCOMPOSER += hwcomposer.msm7630_fusion
LIBHWCOMPOSER += hwcomposer.msm7627_surf
LIBHWCOMPOSER += hwcomposer.msm7627_6x
LIBHWCOMPOSER += hwcomposer.msm7627a

#LIBAUDIOPARAM -- Exposing AudioParameter as dynamic library for SRS TruMedia to work
LIBAUDIOPARAM := libaudioparameter

#LIBAUDIORESAMPLER -- High-quality audio resampler
LIBAUDIORESAMPLER := libaudio-resampler

#LIBOPENCOREHW
LIBOPENCOREHW := libopencorehw

#LIBOVERLAY
LIBOVERLAY := liboverlay
LIBOVERLAY += overlay.default

#LIBGENLOCK
LIBGENLOCK := libgenlock

#LIBPERFLOCK
LIBPERFLOCK := org.codeaurora.Performance

#LIBQCOMUI
LIBQCOMUI := libQcomUI

#LIBQDUTILS
LIBQDUTILS := libqdutils

#LIBQDMETADATA
LIBQDMETADATA := libqdMetaData

#LIBPOWER
LIBPOWER := power.qcom

#LLVM for RenderScript
#use qcom LLVM
$(call inherit-product-if-exists, external/llvm/llvm-select.mk)

#LOC_API
LOC_API := libloc_api-rpc-qc

#MEDIA_PROFILES
MEDIA_PROFILES := media_profiles.xml

#MM_AUDIO
MM_AUDIO := libOmxAacDec
MM_AUDIO += libOmxAacEnc
MM_AUDIO += libOmxAmrEnc
MM_AUDIO += libOmxEvrcEnc
MM_AUDIO += libOmxMp3Dec
MM_AUDIO += libOmxQcelp13Enc
MM_AUDIO += libOmxAc3HwDec

#MM_CORE
MM_CORE := libmm-omxcore
MM_CORE += libOmxCore

#MM_VIDEO
MM_VIDEO := ast-mm-vdec-omx-test
MM_VIDEO += libdivxdrmdecrypt
MM_VIDEO += liblasic
MM_VIDEO += libOmxVdec
MM_VIDEO += libOmxVdecHevc
MM_VIDEO += libOmxVenc
MM_VIDEO += libOmxVidEnc
MM_VIDEO += libstagefrighthw
MM_VIDEO += mm-vdec-omx-property-mgr
MM_VIDEO += mm-vdec-omx-test
MM_VIDEO += mm-venc-omx-test
MM_VIDEO += mm-venc-omx-test720p
MM_VIDEO += mm-video-driver-test
MM_VIDEO += mm-video-encdrv-test

#OPENCORE
OPENCORE := libomx_aacdec_sharedlibrary
OPENCORE += libomx_amrdec_sharedlibrary
OPENCORE += libomx_amrenc_sharedlibrary
OPENCORE += libomx_avcdec_sharedlibrary
OPENCORE += libomx_m4vdec_sharedlibrary
OPENCORE += libomx_mp3dec_sharedlibrary
OPENCORE += libomx_sharedlibrary
OPENCORE += libopencore_author
OPENCORE += libopencore_common
OPENCORE += libopencore_download
OPENCORE += libopencore_downloadreg
OPENCORE += libopencore_mp4local
OPENCORE += libopencore_mp4localreg
OPENCORE += libopencore_net_support
OPENCORE += libopencore_player
OPENCORE += libopencore_rtsp
OPENCORE += libopencore_rtspreg
OPENCORE += libpvdecoder_gsmamr
OPENCORE += libpvplayer_engine
OPENCORE += libpvamrwbdecoder
OPENCORE += libpvauthorengine
OPENCORE += libomx_amr_component_lib
OPENCORE += pvplayer
OPENCORE += pvplayer_engine_test

#PPP
PPP := ip-up-vpn

#PVOMX
PVOMX := libqcomm_omx
PVOMX += 01_qcomm_omx

#RF4CE
RF4CE := RemoTI_RNP.cfg
RF4CE += rf4ce

#SENSORS_HARDWARE
SENSORS_HARDWARE := sensors.msm7630_surf
SENSORS_HARDWARE += sensors.msm7630_fusion

#SOFTAP
SOFTAP := libQWiFiSoftApCfg
SOFTAP += libqsap_sdk

#STK
STK := Stk

#STM LOG
STMLOG := libstm-log

#TSLIB_EXTERNAL
TSLIB_EXTERNAL := corgi
TSLIB_EXTERNAL += dejitter
TSLIB_EXTERNAL += inputraw
TSLIB_EXTERNAL += linear
TSLIB_EXTERNAL += variance
TSLIB_EXTERNAL += pthres
TSLIB_EXTERNAL += libtslib
TSLIB_EXTERNAL += tsprint
TSLIB_EXTERNAL += tstest
TSLIB_EXTERNAL += tsutils
TSLIB_EXTERNAL += tscalib
TSLIB_EXTERNAL += ts

#QRGND
QRGND := qrngd
QRGND += qrngp
QRGND += qrngtest

#WPA
WPA := wpa_supplicant.conf
WPA += wpa_supplicant_wcn.conf
WPA += wpa_supplicant_ath6kl.conf

#ZLIB
ZLIB := gzip
ZLIB += minigzip
ZLIB += libunz

#Charger
CHARGER := charger
CHARGER += charger_res_images

#VT_JNI
VT_JNI := libvt_jni

#CRDA
CRDA := crda
CRDA += regdbdump
CRDA += regulatory.bin
CRDA += linville.key.pub.pem
CRDA += init.crda.sh

#WLAN
WLAN := prima_wlan.ko

PRODUCT_PACKAGES := \
    AccountAndSyncSettings \
    DeskClock \
    AlarmProvider \
    Bluetooth \
    BluetoothExt \
    BTTestApp \
    Calculator \
    Calendar \
    Camera \
    CellBroadcastReceiver \
    CertInstaller \
    DrmProvider \
    Email \
    Gallery2 \
    LatinIME \
    Launcher2 \
    Mms \
    Music \
    Phone \
    Provision \
    Protips \
    QuickSearchBox \
    Settings \
    Sync \
    SystemUI \
    Updater \
    CalendarProvider \
    SyncProvider \
    IM \
    VoiceDialer \
    FM \
    FM2 \
    FMRecord \
    VideoEditor

PRODUCT_PACKAGES += $(ALSA_HARDWARE)
PRODUCT_PACKAGES += $(ALSA_UCM)
PRODUCT_PACKAGES += $(ANGLE)
PRODUCT_PACKAGES += $(AUDIO_HARDWARE)
PRODUCT_PACKAGES += $(AUDIO_POLICY)
PRODUCT_PACKAGES += $(TINY_ALSA_TEST_APPS)
PRODUCT_PACKAGES += $(AMPLOADER)
PRODUCT_PACKAGES += $(APPS)
PRODUCT_PACKAGES += $(BRCTL)
PRODUCT_PACKAGES += $(BSON)
PRODUCT_PACKAGES += $(BT)
PRODUCT_PACKAGES += $(C2DCC)
PRODUCT_PACKAGES += $(CIMAX)
PRODUCT_PACKAGES += $(CONNECTIVITY)
PRODUCT_PACKAGES += $(CHARGER)
PRODUCT_PACKAGES += $(CURL)
PRODUCT_PACKAGES += $(DASH)
PRODUCT_PACKAGES += $(DATA_OS)
PRODUCT_PACKAGES += $(E2FSPROGS)
PRODUCT_PACKAGES += $(EBTABLES)
PRODUCT_PACKAGES += $(FASTPOWERON)
PRODUCT_PACKAGES += $(FM)
PRODUCT_PACKAGES += $(GPS_HARDWARE)
PRODUCT_PACKAGES += $(HDMID)
PRODUCT_PACKAGES += $(HOSTAPD)
PRODUCT_PACKAGES += $(I420CC)
PRODUCT_PACKAGES += $(INIT)
PRODUCT_PACKAGES += $(IPROUTE2)
PRODUCT_PACKAGES += $(IPTABLES)
PRODUCT_PACKAGES += $(KERNEL_TESTS)
PRODUCT_PACKAGES += $(KEYPAD)
PRODUCT_PACKAGES += $(KS)
PRODUCT_PACKAGES += $(LIB_NL)
PRODUCT_PACKAGES += $(LIB_XML2)
PRODUCT_PACKAGES += $(LIBCAMERA)
PRODUCT_PACKAGES += $(LIBGESTURES)
PRODUCT_PACKAGES += $(LIBCOPYBIT)
PRODUCT_PACKAGES += $(LIBGRALLOC)
PRODUCT_PACKAGES += $(LIBLIGHTS)
PRODUCT_PACKAGES += $(LIBAUDIOPARAM)
PRODUCT_PACKAGES += $(LIBAUDIORESAMPLER)
PRODUCT_PACKAGES += $(LIBOPENCOREHW)
PRODUCT_PACKAGES += $(LIBOVERLAY)
PRODUCT_PACKAGES += $(LIBHWCOMPOSER)
PRODUCT_PACKAGES += $(LIBGENLOCK)
PRODUCT_PACKAGES += $(LIBPERFLOCK)
PRODUCT_PACKAGES += $(LIBQCOMUI)
PRODUCT_PACKAGES += $(LIBQDUTILS)
PRODUCT_PACKAGES += $(LIBQDMETADATA)
PRODUCT_PACKAGES += $(LIBPOWER)
PRODUCT_PACKAGES += $(LOC_API)
PRODUCT_PACKAGES += $(MEDIA_PROFILES)
PRODUCT_PACKAGES += $(MM_AUDIO)
PRODUCT_PACKAGES += $(MM_CORE)
PRODUCT_PACKAGES += $(MM_VIDEO)
PRODUCT_PACKAGES += $(OPENCORE)
PRODUCT_PACKAGES += $(PPP)
PRODUCT_PACKAGES += $(PVOMX)
PRODUCT_PACKAGES += $(RF4CE)
PRODUCT_PACKAGES += $(SENSORS_HARDWARE)
PRODUCT_PACKAGES += $(SOFTAP)
PRODUCT_PACKAGES += $(STK)
PRODUCT_PACKAGES += $(STMLOG)
PRODUCT_PACKAGES += $(TSLIB_EXTERNAL)
PRODUCT_PACKAGES += $(QRGND)
PRODUCT_PACKAGES += $(UPDATER)
PRODUCT_PACKAGES += $(WPA)
PRODUCT_PACKAGES += $(ZLIB)
PRODUCT_PACKAGES += $(VT_JNI)
PRODUCT_PACKAGES += $(CRDA)
PRODUCT_PACKAGES += $(WLAN)

# Live Wallpapers
PRODUCT_PACKAGES += \
        LiveWallpapers \
        LiveWallpapersPicker \
        VisualizationWallpapers \
        librs_jni

# Filesystem management tools
PRODUCT_PACKAGES += \
    make_ext4fs \
    setup_fs

# Flatland
PRODUCT_PACKAGES += flatland

# MSM updater library
PRODUCT_PACKAGES += librecovery_updater_msm


#intialise PRODUCT_PACKAGES_DEBUG list for debug modules
PRODUCT_PACKAGES_DEBUG :=


PRODUCT_COPY_FILES := \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:system/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml \

# Bluetooth configuration files
PRODUCT_COPY_FILES += \
    system/bluetooth/data/audio.conf:system/etc/bluetooth/audio.conf \
    system/bluetooth/data/auto_pairing.conf:system/etc/bluetooth/auto_pairing.conf \
    system/bluetooth/data/blacklist.conf:system/etc/bluetooth/blacklist.conf \
    system/bluetooth/data/input.conf:system/etc/bluetooth/input.conf \
    system/bluetooth/data/network.conf:system/etc/bluetooth/network.conf \

ifeq ($(BOARD_HAVE_BLUETOOTH_BLUEZ),true)
PRODUCT_COPY_FILES += \
    system/bluetooth/data/stack.conf:system/etc/bluetooth/stack.conf
endif # BOARD_HAVE_BLUETOOTH_BLUEZ

# gps/location secuity configuration file
PRODUCT_COPY_FILES += \
    device/qcom/common/sec_config:system/etc/sec_config

PRODUCT_COPY_FILES += device/qcom/common/media/media_profiles.xml:system/etc/media_profiles.xml \
                      device/qcom/common/media/media_codecs.xml:system/etc/media_codecs.xml

# enable overlays to use our version of
# source/resources etc.
DEVICE_PACKAGE_OVERLAYS += device/qcom/common/overlay

# include additional build utilities
-include device/qcom/common/utils.mk

#Enabling Ring Tones
#include frameworks/base/data/sounds/OriginalAudio.mk

#Enabling video for live effects
-include frameworks/base/data/videos/VideoPackage1.mk

# For PRODUCT_COPY_FILES, the first instance takes precedence.
# Since we want use QC specific files, we should inherit
# device-vendor.mk first to make sure QC specific files gets installed.
$(call inherit-product-if-exists, $(QCPATH)/common/config/device-vendor.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

PRODUCT_BRAND := qcom
PRODUCT_LOCALES := en_US es_US de_DE zh_CN
PRODUCT_LOCALES += hdpi mdpi

PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.extension_library=/vendor/lib/libqc-opt.so \
    persist.radio.apm_sim_not_pwdn=1

PRODUCT_PRIVATE_KEY := device/qcom/common/qcom.key

$(call inherit-product, frameworks/native/build/phone-xhdpi-1024-dalvik-heap.mk)
#$(call inherit-product, frameworks/base/data/fonts/fonts.mk)
#$(call inherit-product, frameworks/base/data/keyboards/keyboards.mk)
