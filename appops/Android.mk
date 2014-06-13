define validate-xml-file
	@echo "validating xml: $2"
	@xmllint --noout --schema $1 $2
endef
LOCAL_PATH:= $(call my-dir)
#######################################

include $(CLEAR_VARS)
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE := appops_policy.xml
APPOPS_POLICY_XSD_FILE := $(LOCAL_PATH)/appops_policy.xsd
include $(BUILD_SYSTEM)/base_rules.mk

$(LOCAL_BUILT_MODULE): $(LOCAL_PATH)/appops_policy.xml $(APPOPS_POLICY_XSD_FILE) $(ACP)
	@echo Validating $<
	@xmllint --noout --schema $(APPOPS_POLICY_XSD_FILE) $<
	$(copy-file-to-target)
#######################################

