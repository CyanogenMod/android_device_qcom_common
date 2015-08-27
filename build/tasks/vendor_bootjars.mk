# This makefile is used to include
# extra product boot jars for SDK

ifneq ($(call is-vendor-board-platform,QCOM),true)

#add extra jars here
PRODUCT_BOOT_JARS += tcmiface

# Preloading QPerformance jar to ensure faster perflocks in Boost Framework
PRODUCT_BOOT_JARS += QPerformance

#call dex_preopt.mk for extra jars
include $(BUILD_SYSTEM)/dex_preopt.mk

endif
