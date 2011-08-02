
# board platforms list for msm7k and qsd8k
# to be used for TARGET_BOARD_PLATFORM specific featurization
MSM7K_BOARD_PLATFORMS := msm7630_surf
MSM7K_BOARD_PLATFORMS += msm7630_fusion
MSM7K_BOARD_PLATFORMS += msm7627_surf
MSM7K_BOARD_PLATFORMS += msm7627a
MSM7K_BOARD_PLATFORMS += msm7k

QSD8K_BOARD_PLATFORMS := qsd8k

# Below projects/packages with LOCAL_MODULEs will be used by
# PRODUCT_PACKAGES to build LOCAL_MODULEs that are tagged with
# optional tag, which will not be available on target unless
# explicitly list here. Where project corresponds to the vars here
# in CAPs.

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

#LIBCAMERA
LIBCAMERA := libcamera

#LIBCOPYBIT
LIBCOPYBIT := copybit.msm8660
LIBCOPYBIT := copybit.msm8960
LIBCOPYBIT += copybit.msm7k
LIBCOPYBIT += copybit.qsd8k
LIBCOPYBIT += copybit.msm7630_surf
LIBCOPYBIT += copybit.msm7630_fusion
LIBCOPYBIT += copybit.msm7627_surf
LIBCOPYBIT += copybit.msm7627a

#LIBGRALLOC
LIBGRALLOC := gralloc.default
LIBGRALLOC += gralloc.msm8660
LIBGRALLOC += gralloc.msm8960
LIBGRALLOC += gralloc.msm7k
LIBGRALLOC += gralloc.msm7630_surf
LIBGRALLOC += gralloc.msm7630_fusion
LIBGRALLOC += gralloc.msm7627_surf
LIBGRALLOC += gralloc.msm7627a

#LIBLIGHTS
LIBLIGHTS := lights.msm8660
LIBLIGHTS := lights.msm8960
LIBLIGHTS += lights.msm7k
LIBLIGHTS += lights.msm7630_surf
LIBLIGHTS += lights.msm7630_fusion
LIBLIGHTS += lights.msm7627_surf
LIBLIGHTS += lights.msm7627a

#LIBOPENCOREHW
LIBOPENCOREHW := libopencorehw

#LIBOVERLAY
LIBOVERLAY := liboverlay
LIBOVERLAY += overlay.default

#LOC_API
LOC_API := libloc_api-rpc-qc

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

#PVOMX
PVOMX := libqcomm_omx

#SOFTAP
SOFTAP := libQWiFiSoftApCfg
SOFTAP += libqsap_sdk

#UPDATER
UPDATER := librecovery_ui_qcom
UPDATER += librecovery_updater_qcom

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
    Gallery3D \
    LatinIME \
    Launcher2 \
    Mms \
    Music \
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

PRODUCT_PACKAGES += $(KERNEL_TESTS)
PRODUCT_PACKAGES += $(KEYPAD)
PRODUCT_PACKAGES += $(LIBCAMERA)
PRODUCT_PACKAGES += $(LIBCOPYBIT)
PRODUCT_PACKAGES += $(LIBGRALLOC)
PRODUCT_PACKAGES += $(LIBLIGHTS)
PRODUCT_PACKAGES += $(LIBOPENCOREHW)
PRODUCT_PACKAGES += $(LIBOVERLAY)
PRODUCT_PACKAGES += $(LOC_API)
PRODUCT_PACKAGES += $(MM_AUDIO)
PRODUCT_PACKAGES += $(MM_CORE)
PRODUCT_PACKAGES += $(MM_VIDEO)
PRODUCT_PACKAGES += $(PVOMX)
PRODUCT_PACKAGES += $(SOFTAP)
PRODUCT_PACKAGES += $(UPDATER)

PRODUCT_COPY_FILES := \
    frameworks/base/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/base/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/base/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.telephony.cdma.xml:system/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/base/data/etc/android.hardware.location.xml:system/etc/permissions/android.hardware.location.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.xml:system/etc/permissions/android.hardware.touchscreen.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml \
    frameworks/base/data/etc/android.software.sip.xml:system/etc/permissions/android.software.sip.xml \
    frameworks/base/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/base/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \


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
    ro.vendor.extension_library=/system/lib/libqc-opt.so

PRODUCT_PRIVATE_KEY := device/qcom/common/qcom.key
