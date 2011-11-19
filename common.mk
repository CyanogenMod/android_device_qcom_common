# Board platforms lists to be used for
# TARGET_BOARD_PLATFORM specific featurization
QCOM_BOARD_PLATFORMS := msm7627_surf
QCOM_BOARD_PLATFORMS += msm7627_6x
QCOM_BOARD_PLATFORMS += msm7627a
QCOM_BOARD_PLATFORMS += msm7630_surf
QCOM_BOARD_PLATFORMS += msm7630_fusion
QCOM_BOARD_PLATFORMS += msm8660
QCOM_BOARD_PLATFORMS += msm8960

MSM7K_BOARD_PLATFORMS := msm7630_surf
MSM7K_BOARD_PLATFORMS += msm7630_fusion
MSM7K_BOARD_PLATFORMS += msm7627_surf
MSM7K_BOARD_PLATFORMS += msm7627_6x
MSM7K_BOARD_PLATFORMS += msm7627a
MSM7K_BOARD_PLATFORMS += msm7k

QSD8K_BOARD_PLATFORMS := qsd8k


# Below projects/packages with LOCAL_MODULEs will be used by
# PRODUCT_PACKAGES to build LOCAL_MODULEs that are tagged with
# optional tag, which will not be available on target unless
# explicitly list here. Where project corresponds to the vars here
# in CAPs.

#ALSA
ALSA_HARDWARE := alsa.msm8960

AUDIO_HARDWARE := audio.primary.msm8960
AUDIO_HARDWARE += audio.primary.msm8660
AUDIO_HARDWARE += audio.primary.msm7627a
AUDIO_HARDWARE += audio.primary.msm7630_surf
AUDIO_HARDWARE += audio.primary.default

AUDIO_POLICY := audio_policy.msm8960
AUDIO_POLICY += audio_policy.msm8660
AUDIO_POLICY += audio_policy.msm7627a
AUDIO_POLICY += audio_policy.msm7630_surf
AUDIO_POLICY += audio_policy.default

#AMPLOADER
AMPLOADER := amploader

#APPS
APPS := QualcommSoftAP
APPS += TSCalibration

#BSON
BSON := libbson

#BT
BT := libattrib_static

#CONNECTIVITY
CONNECTIVITY := cnd
CONNECTIVITY += librefcne

#CURL
CURL := libcurl
CURL += curl

#E2FSPROGS
E2FSPROGS := e2fsck

#GPS
GPS_HARDWARE := gps.default
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

#INIT
INIT := init.qcom.composition_type.sh
INIT += init.qcom.mdm_links.sh
INIT += init.qcom.modem_links.sh
INIT += init.target.rc
INIT += init.qcom.bt.sh
INIT += init.qcom.coex.sh
INIT += init.qcom.fm.sh
INIT += init.qcom.post_boot.sh
INIT += init.qcom.rc
INIT += init.qcom.sdio.sh
INIT += init.qcom.sh
INIT += init.qcom.wifi.sh
INIT += vold.fstab

#IPROUTE2
IPROUTE2 := ip
IPROUTE2 += libiprouteutil

#IPTABLES
IPTABLES := libiptc
IPTABLES += libext
IPTABLES += iptables

#KERNEL_TESTS
KERNEL_TESTS := mm-audio-native-test
KERNEL_TESTS += HiFi
KERNEL_TESTS += FM_Digital_Radio
KERNEL_TESTS += HiFi_Rec
KERNEL_TESTS += Voice_Call_IP
KERNEL_TESTS += Voice_Call
KERNEL_TESTS += FM_REC
KERNEL_TESTS += HiFi_Low_Power
KERNEL_TESTS += FM_A2DP_REC
KERNEL_TESTS += snd_soc_msm
KERNEL_TESTS += libalsa-intf

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
KEYPAD += ffa-keypad.kl
KEYPAD += fluid-keypad.kl
KEYPAD += keypad_8960.kl
KEYPAD += msm8960-snd-card_Button_Jack.kl
KEYPAD += msm_tma300_ts.kl
KEYPAD += surf_keypad.kl

#LIBCAMERA
LIBCAMERA := libcamera
LIBCAMERA += camera.msm8960
LIBCAMERA += camera.msm8660
LIBCAMERA += camera.msm7630_surf
LIBCAMERA += camera.msm7627a

#LIBCOPYBIT
LIBCOPYBIT := copybit.msm8660
LIBCOPYBIT += copybit.msm8960
LIBCOPYBIT += copybit.msm7k
LIBCOPYBIT += copybit.qsd8k
LIBCOPYBIT += copybit.msm7630_surf
LIBCOPYBIT += copybit.msm7630_fusion
LIBCOPYBIT += copybit.msm7627_surf
LIBCOPYBIT += copybit.msm7627_6x
LIBCOPYBIT += copybit.msm7627a

#LIBGRALLOC
LIBGRALLOC := gralloc.default
LIBGRALLOC += gralloc.msm8660
LIBGRALLOC += gralloc.msm8960
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
LIBLIGHTS += lights.msm7k
LIBLIGHTS += lights.msm7630_surf
LIBLIGHTS += lights.msm7630_fusion
LIBLIGHTS += lights.msm7627_surf
LIBLIGHTS += lights.msm7627_6x
LIBLIGHTS += lights.msm7627a

#LIBOPENCOREHW
LIBOPENCOREHW := libopencorehw

#LIBOVERLAY
LIBOVERLAY := liboverlay
LIBOVERLAY += overlay.default

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
MM_AUDIO += mm-adec-omxaac-test
MM_AUDIO += mm-adec-omxmp3-test
MM_AUDIO += mm-aenc-omxaac-test
MM_AUDIO += mm-aenc-omxamr-test
MM_AUDIO += mm-aenc-omxevrc-test
MM_AUDIO += mm-aenc-omxqcelp13-test
MM_AUDIO += sw-adec-omxaac-test
MM_AUDIO += sw-adec-omxamr-test
MM_AUDIO += sw-adec-omxamrwb-test
MM_AUDIO += sw-adec-omxmp3-test

#MM_CORE
MM_CORE := libmm-omxcore
MM_CORE += libOmxCore

#MM_VIDEO
MM_VIDEO := ast-mm-vdec-omx-test
MM_VIDEO += libdivxdrmdecrypt
MM_VIDEO += liblasic
MM_VIDEO += libOmxVdec
MM_VIDEO += libOmxVenc
MM_VIDEO += libOmxVidEnc
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

#SENSORS_HARDWARE
SENSORS_HARDWARE := sensors.msm7630_surf
SENSORS_HARDWARE += sensors.msm7630_fusion

#SOFTAP
SOFTAP := libQWiFiSoftApCfg
SOFTAP += libqsap_sdk

#STK
STK := Stk

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
QRGND += qrngtest

#WPA
WPA := wpa_supplicant.conf

#ZLIB
ZLIB := gzip
ZLIB += minigzip
ZLIB += libunz

PRODUCT_PACKAGES := \
    AccountAndSyncSettings \
    DeskClock \
    AlarmProvider \
    Bluetooth \
    Calculator \
    Calendar \
    Camera \
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
    FM

PRODUCT_PACKAGES += $(ALSA_HARDWARE)
PRODUCT_PACKAGES += $(AUDIO_HARDWARE)
PRODUCT_PACKAGES += $(AUDIO_POLICY)
PRODUCT_PACKAGES += $(AMPLOADER)
PRODUCT_PACKAGES += $(APPS)
PRODUCT_PACKAGES += $(BSON)
PRODUCT_PACKAGES += $(BT)
PRODUCT_PACKAGES += $(CONNECTIVITY)
PRODUCT_PACKAGES += $(CURL)
PRODUCT_PACKAGES += $(E2FSPROGS)
PRODUCT_PACKAGES += $(GPS_HARDWARE)
PRODUCT_PACKAGES += $(HDMID)
PRODUCT_PACKAGES += $(HOSTAPD)
PRODUCT_PACKAGES += $(INIT)
PRODUCT_PACKAGES += $(IPROUTE2)
PRODUCT_PACKAGES += $(IPTABLES)
PRODUCT_PACKAGES += $(KERNEL_TESTS)
PRODUCT_PACKAGES += $(KEYPAD)
PRODUCT_PACKAGES += $(LIBCAMERA)
PRODUCT_PACKAGES += $(LIBCOPYBIT)
PRODUCT_PACKAGES += $(LIBGRALLOC)
PRODUCT_PACKAGES += $(LIBLIGHTS)
PRODUCT_PACKAGES += $(LIBOPENCOREHW)
PRODUCT_PACKAGES += $(LIBOVERLAY)
PRODUCT_PACKAGES += $(LOC_API)
PRODUCT_PACKAGES += $(MEDIA_PROFILES)
PRODUCT_PACKAGES += $(MM_AUDIO)
PRODUCT_PACKAGES += $(MM_CORE)
PRODUCT_PACKAGES += $(MM_VIDEO)
PRODUCT_PACKAGES += $(OPENCORE)
PRODUCT_PACKAGES += $(PPP)
PRODUCT_PACKAGES += $(PVOMX)
PRODUCT_PACKAGES += $(SENSORS_HARDWARE)
PRODUCT_PACKAGES += $(SOFTAP)
PRODUCT_PACKAGES += $(STK)
PRODUCT_PACKAGES += $(TSLIB_EXTERNAL)
PRODUCT_PACKAGES += $(QRGND)
PRODUCT_PACKAGES += $(UPDATER)
PRODUCT_PACKAGES += $(WPA)
PRODUCT_PACKAGES += $(ZLIB)


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

PRODUCT_COPY_FILES := \
    frameworks/base/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/base/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.telephony.cdma.xml:system/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml \
    frameworks/base/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/base/data/etc/platform.xml:system/etc/permissions/platform.xml \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/base/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/base/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/base/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/base/data/etc/android.hardware.nfc.xml:system/etc/permissions/android.hardware.nfc.xml \
    frameworks/base/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/base/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \

# include additional build utilities
include build/core/utils.mk

#Enabling Ring Tones
include frameworks/base/data/sounds/OriginalAudio.mk

$(call inherit-product, $(SRC_TARGET_DIR)/product/core.mk)
$(call inherit-product-if-exists, vendor/qcom/proprietary/common/config/device-vendor.mk)

PRODUCT_BRAND := qcom
PRODUCT_LOCALES := en_US es_US de_DE zh_CN
PRODUCT_LOCALES += hdpi mdpi

PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.extension_library=/system/lib/libqc-opt.so \
    hwui.render_dirty_regions=false

PRODUCT_PRIVATE_KEY := device/qcom/common/qcom.key

$(call inherit-product, frameworks/base/build/phone-xhdpi-1024-dalvik-heap.mk)
$(call inherit-product, frameworks/base/data/fonts/fonts.mk)
$(call inherit-product, frameworks/base/data/keyboards/keyboards.mk)
