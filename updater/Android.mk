ifneq ($(TARGET_SIMULATOR),true)
ifeq ($(TARGET_ARCH),arm)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# librecovery_ui_qcom is a static library included by all the recovery
# UI libraries for QCOM specific devices.  It knows how to recover the
# log stored in the cache partition when a radio update is done.

LOCAL_SRC_FILES := log_recovery.c
LOCAL_C_INCLUDES += bootable/recovery
LOCAL_MODULE := librecovery_ui_qcom
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

# librecovery_update_qcom is a set of edify extension functions for
# doing radio update on QCOM devices.

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := recovery_updater.c firmware.c bootloader.c
LOCAL_STATIC_LIBRARIES += libmtdutils
LOCAL_C_INCLUDES += bootable/recovery
LOCAL_MODULE := librecovery_updater_qcom
include $(BUILD_STATIC_LIBRARY)

endif   # TARGET_ARCH == arm
endif   # !TARGET_SIMULATOR
