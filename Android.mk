MEDIA_PATH := $(call my-dir)

include $(all-subdir-makefiles)

ifeq ($(call is-board-platform-in-list,msm8960 msm8660 msm7627a msm7630_surf msm7630_fusion),true)

LOCAL_PATH := $(MEDIA_PATH)

########################
include $(CLEAR_VARS)

ifeq ($(call is-board-platform,msm7627a),true)
   LOCAL_SRC_FILES := media/media_profiles_7627a.xml
else ifeq ($(call is-board-platform-in-list,msm8960 msm8660 msm7630_surf msm7630_fusion),true)
   LOCAL_SRC_FILES := media/media_profiles.xml
endif

LOCAL_MODULE := media_profiles.xml

LOCAL_MODULE_TAGS := optional

# This will install the file in /system/etc
#
LOCAL_MODULE_CLASS := ETC

include $(BUILD_PREBUILT)
endif

