ifeq ($(TARGET_RECOVERY_UI_LIB),librecovery_ui_qcom)
ifneq ($(TARGET_SIMULATOR),true)
ifeq ($(TARGET_ARCH),arm)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := librecovery_ui_qcom

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += bootable/recovery

LOCAL_SRC_FILES += qcom_recovery_ui.c

LOCAL_STATIC_LIBRARIES += libext4_utils libz
LOCAL_STATIC_LIBRARIES += libminzip libunz libmtdutils libmincrypt
LOCAL_STATIC_LIBRARIES += libminui libpixelflinger_static libpng libcutils
LOCAL_STATIC_LIBRARIES += libstdc++ libc

include $(BUILD_STATIC_LIBRARY)
endif   # TARGET_ARCH == arm
endif   # !TARGET_SIMULATOR
endif
