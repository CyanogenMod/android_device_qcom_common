ifeq ($(BOARD_KERNEL_SEPARATED_DT),true)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifneq ($(BOARD_KERNEL_DT_VERSION),)
DT_VERSION := $(BOARD_KERNEL_DT_VERSION)
else
DT_VERSION := v1
endif

LOCAL_SRC_FILES := \
	dtbtool-$(DT_VERSION).c

LOCAL_CFLAGS += \
	-Wall

LOCAL_MODULE := dtbTool
LOCAL_MODULE_TAGS := optional

include $(BUILD_HOST_EXECUTABLE)
endif
