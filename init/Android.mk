ifeq ($(TARGET_INIT_VENDOR_LIB),libinit_msm)

LOCAL_PATH := $(call my-dir)
LIBINIT_MSM_PATH := $(call my-dir)

LIBINIT_USE_MSM_DEFAULT := $(shell if [ ! -f $(LIBINIT_MSM_PATH)/init_$(TARGET_BOARD_PLATFORM).c ]; then echo true; fi)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := system/core/init
LOCAL_CFLAGS := -Wall -DANDROID_TARGET=\"$(TARGET_BOARD_PLATFORM)\"
LOCAL_SRC_FILES := init_msm.c
ifeq ($(LIBINIT_USE_MSM_DEFAULT),true)
  LOCAL_SRC_FILES += init_msmdefault.c
else
  ifneq ($(TARGET_LIBINIT_DEFINES_FILE),)
    LOCAL_SRC_FILES += ../../../../$(TARGET_LIBINIT_DEFINES_FILE)
  else
    LOCAL_SRC_FILES += init_$(TARGET_BOARD_PLATFORM).c
  endif
endif
LOCAL_MODULE := libinit_msm
include $(BUILD_STATIC_LIBRARY)

endif
