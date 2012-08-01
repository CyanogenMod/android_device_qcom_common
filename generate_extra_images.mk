# This makefile is used to generate extra images for QCOM targets
# persist & NAND images required for different QCOM targets.

TARGET_OUT_PERSIST := $(PRODUCT_OUT)/persist

INTERNAL_PERSISTIMAGE_FILES := \
	$(filter $(TARGET_OUT_PERSIST)/%,$(ALL_DEFAULT_INSTALLED_MODULES))

INSTALLED_PERSISTIMAGE_TARGET := $(PRODUCT_OUT)/persist.img

define build-persistimage-target
    $(call pretty,"Target persist fs image: $(INSTALLED_PERSISTIMAGE_TARGET)")
    @mkdir -p $(TARGET_OUT_PERSIST)
    $(hide) $(MKEXTUSERIMG) -s $(TARGET_OUT_PERSIST) $@ ext4 persist $(BOARD_PERSISTIMAGE_PARTITION_SIZE)
    $(hide) chmod a+r $@
    $(hide) $(call assert-max-image-size,$@,$(BOARD_PERSISTIMAGE_PARTITION_SIZE),yaffs)
endef

$(INSTALLED_PERSISTIMAGE_TARGET): $(MKEXTUSERIMG) $(MAKE_EXT4FS) $(INTERNAL_PERSISTIMAGE_FILES)
	$(build-persistimage-target)

ALL_DEFAULT_INSTALLED_MODULES += $(INSTALLED_PERSISTIMAGE_TARGET)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(INSTALLED_PERSISTIMAGE_TARGET)

ifeq ($(call is-board-platform-in-list,msm7627a msm7630_surf),true)

2K_NAND_OUT := $(PRODUCT_OUT)/2k_nand_images
4K_NAND_OUT := $(PRODUCT_OUT)/4k_nand_images
BCHECC_OUT := $(PRODUCT_OUT)/bchecc_images

INSTALLED_2K_BOOTIMAGE_TARGET := $(2K_NAND_OUT)/boot.img
INSTALLED_2K_SYSTEMIMAGE_TARGET := $(2K_NAND_OUT)/system.img
INSTALLED_2K_USERDATAIMAGE_TARGET := $(2K_NAND_OUT)/userdata.img
INSTALLED_2K_PERSISTIMAGE_TARGET := $(2K_NAND_OUT)/persist.img
INSTALLED_2K_RECOVERYIMAGE_TARGET := $(2K_NAND_OUT)/recovery.img

INSTALLED_4K_BOOTIMAGE_TARGET := $(4K_NAND_OUT)/boot.img
INSTALLED_4K_SYSTEMIMAGE_TARGET := $(4K_NAND_OUT)/system.img
INSTALLED_4K_USERDATAIMAGE_TARGET := $(4K_NAND_OUT)/userdata.img
INSTALLED_4K_PERSISTIMAGE_TARGET := $(4K_NAND_OUT)/persist.img
INSTALLED_4K_RECOVERYIMAGE_TARGET := $(4K_NAND_OUT)/recovery.img

INSTALLED_BCHECC_BOOTIMAGE_TARGET := $(BCHECC_OUT)/boot.img
INSTALLED_BCHECC_SYSTEMIMAGE_TARGET := $(BCHECC_OUT)/system.img
INSTALLED_BCHECC_USERDATAIMAGE_TARGET := $(BCHECC_OUT)/userdata.img
INSTALLED_BCHECC_PERSISTIMAGE_TARGET := $(BCHECC_OUT)/persist.img
INSTALLED_BCHECC_RECOVERYIMAGE_TARGET := $(BCHECC_OUT)/recovery.img

# These variables are required to make sure that the required
# files/targets are available before generating NAND images.
# As these are not available while parsing this makefile,
# defining here. These variables will be overwritten by
# Build System again.
INSTALLED_RAMDISK_TARGET := $(PRODUCT_OUT)/ramdisk.img
INSTALLED_SYSTEMIMAGE := $(PRODUCT_OUT)/system.img
INSTALLED_USERDATAIMAGE_TARGET := $(PRODUCT_OUT)/userdata.img
INSTALLED_RECOVERYIMAGE_TARGET := $(PRODUCT_OUT)/recovery.img
recovery_ramdisk := $(PRODUCT_OUT)/ramdisk-recovery.img
recovery_nand_fstab := $(TARGET_DEVICE_DIR)/recovery_nand.fstab

NAND_BOOTIMAGE_ARGS := \
	--kernel $(INSTALLED_KERNEL_TARGET) \
	--ramdisk $(INSTALLED_RAMDISK_TARGET) \
	--cmdline "$(BOARD_KERNEL_CMDLINE)" \
	--base $(BOARD_KERNEL_BASE)

NAND_RECOVERYIMAGE_ARGS := \
	--kernel $(INSTALLED_KERNEL_TARGET) \
	--ramdisk $(recovery_ramdisk) \
	--cmdline "$(BOARD_KERNEL_CMDLINE)" \
	--base $(BOARD_KERNEL_BASE)

INTERNAL_4K_BOOTIMAGE_ARGS := $(NAND_BOOTIMAGE_ARGS)
INTERNAL_4K_BOOTIMAGE_ARGS += --pagesize $(BOARD_KERNEL_PAGESIZE)

INTERNAL_2K_BOOTIMAGE_ARGS := $(NAND_BOOTIMAGE_ARGS)
INTERNAL_2K_BOOTIMAGE_ARGS += --pagesize $(BOARD_KERNEL_2KPAGESIZE)

INTERNAL_4K_MKYAFFS2_FLAGS := -c $(BOARD_KERNEL_PAGESIZE)
INTERNAL_4K_MKYAFFS2_FLAGS += -s $(BOARD_KERNEL_SPARESIZE)

INTERNAL_2K_MKYAFFS2_FLAGS := -c $(BOARD_KERNEL_2KPAGESIZE)
INTERNAL_2K_MKYAFFS2_FLAGS += -s $(BOARD_KERNEL_2KSPARESIZE)

INTERNAL_BCHECC_MKYAFFS2_FLAGS := -c $(BOARD_KERNEL_PAGESIZE)
INTERNAL_BCHECC_MKYAFFS2_FLAGS += -s $(BOARD_KERNEL_BCHECC_SPARESIZE)

INTERNAL_4K_RECOVERYIMAGE_ARGS := $(NAND_RECOVERYIMAGE_ARGS)
INTERNAL_4K_RECOVERYIMAGE_ARGS += --pagesize $(BOARD_KERNEL_PAGESIZE)

INTERNAL_2K_RECOVERYIMAGE_ARGS := $(NAND_RECOVERYIMAGE_ARGS)
INTERNAL_2K_RECOVERYIMAGE_ARGS += --pagesize $(BOARD_KERNEL_2KPAGESIZE)

# Generate boot image for NAND
define build-nand-bootimage
  @echo "target NAND boot image: $(3)"
  $(hide) mkdir -p $(1)
  $(hide) $(MKBOOTIMG) $(2) --output $(3)
  $(hide) $(call assert-max-image-size,$@,$(BOARD_BOOTIMAGE_PARTITION_SIZE),raw)
endef

# Generate system image for NAND
define build-nand-systemimage
  @echo "target NAND system image: $(3)"
  $(hide) mkdir -p $(1)
  $(hide) $(MKYAFFS2) -f $(2) $(TARGET_OUT) $(3)
  $(hide) chmod a+r $(3)
  $(hide) $(call assert-max-image-size,$@,$(BOARD_SYSTEMIMAGE_PARTITION_SIZE),yaffs)
endef

# Generate userdata image for NAND
define build-nand-userdataimage
  @echo "target NAND userdata image: $(3)"
  $(hide) mkdir -p $(1)
  $(hide) $(MKYAFFS2) -f $(2) $(TARGET_OUT_DATA) $(3)
  $(hide) chmod a+r $(3)
  $(hide) $(call assert-max-image-size,$@,$(BOARD_USERDATAIMAGE_PARTITION_SIZE),yaffs)
endef

# Generate persist image for NAND
define build-nand-persistimage
  @echo "target NAND persist image: $(3)"
  $(hide) mkdir -p $(1)
  $(hide) $(MKYAFFS2) -f $(2) $(TARGET_OUT_PERSIST) $(3)
  $(hide) chmod a+r $(3)
  $(hide) $(call assert-max-image-size,$@,$(BOARD_PERSISTIMAGE_PARTITION_SIZE),yaffs)
endef

$(INSTALLED_4K_BOOTIMAGE_TARGET): $(MKBOOTIMG) $(INSTALLED_KERNEL_TARGET) $(INSTALLED_RAMDISK_TARGET)
	$(hide) $(call build-nand-bootimage,$(4K_NAND_OUT),$(INTERNAL_4K_BOOTIMAGE_ARGS),$(INSTALLED_4K_BOOTIMAGE_TARGET))
ifeq ($(call is-board-platform,msm7627a),true)
	$(hide) $(call build-nand-bootimage,$(2K_NAND_OUT),$(INTERNAL_2K_BOOTIMAGE_ARGS),$(INSTALLED_2K_BOOTIMAGE_TARGET))
	$(hide) $(call build-nand-bootimage,$(BCHECC_OUT),$(INTERNAL_4K_BOOTIMAGE_ARGS),$(INSTALLED_BCHECC_BOOTIMAGE_TARGET))
endif # is-board-platform

$(INSTALLED_4K_SYSTEMIMAGE_TARGET): $(MKYAFFS2) $(INSTALLED_SYSTEMIMAGE)
	$(hide) $(call build-nand-systemimage,$(4K_NAND_OUT),$(INTERNAL_4K_MKYAFFS2_FLAGS),$(INSTALLED_4K_SYSTEMIMAGE_TARGET))
ifeq ($(call is-board-platform,msm7627a),true)
	$(hide) $(call build-nand-systemimage,$(2K_NAND_OUT),$(INTERNAL_2K_MKYAFFS2_FLAGS),$(INSTALLED_2K_SYSTEMIMAGE_TARGET))
	$(hide) $(call build-nand-systemimage,$(BCHECC_OUT),$(INTERNAL_BCHECC_MKYAFFS2_FLAGS),$(INSTALLED_BCHECC_SYSTEMIMAGE_TARGET))
endif # is-board-platform

$(INSTALLED_4K_USERDATAIMAGE_TARGET): $(MKYAFFS2) $(INSTALLED_USERDATAIMAGE_TARGET)
	$(hide) $(call build-nand-userdataimage,$(4K_NAND_OUT),$(INTERNAL_4K_MKYAFFS2_FLAGS),$(INSTALLED_4K_USERDATAIMAGE_TARGET))
ifeq ($(call is-board-platform,msm7627a),true)
	$(hide) $(call build-nand-userdataimage,$(2K_NAND_OUT),$(INTERNAL_2K_MKYAFFS2_FLAGS),$(INSTALLED_2K_USERDATAIMAGE_TARGET))
	$(hide) $(call build-nand-userdataimage,$(BCHECC_OUT),$(INTERNAL_BCHECC_MKYAFFS2_FLAGS),$(INSTALLED_BCHECC_USERDATAIMAGE_TARGET))
endif # is-board-platform

$(INSTALLED_4K_PERSISTIMAGE_TARGET): $(MKYAFFS2) $(INSTALLED_PERSISTIMAGE_TARGET)
	$(hide) $(call build-nand-persistimage,$(4K_NAND_OUT),$(INTERNAL_4K_MKYAFFS2_FLAGS),$(INSTALLED_4K_PERSISTIMAGE_TARGET))
ifeq ($(call is-board-platform,msm7627a),true)
	$(hide) $(call build-nand-persistimage,$(2K_NAND_OUT),$(INTERNAL_2K_MKYAFFS2_FLAGS),$(INSTALLED_2K_PERSISTIMAGE_TARGET))
	$(hide) $(call build-nand-persistimage,$(BCHECC_OUT),$(INTERNAL_BCHECC_MKYAFFS2_FLAGS),$(INSTALLED_BCHECC_PERSISTIMAGE_TARGET))
endif # is-board-platform

$(INSTALLED_4K_RECOVERYIMAGE_TARGET): $(MKBOOTIMG) $(INSTALLED_KERNEL_TARGET) $(INSTALLED_RECOVERYIMAGE_TARGET) $(recovery_nand_fstab)
	$(hide) cp -f $(recovery_nand_fstab) $(TARGET_RECOVERY_ROOT_OUT)/etc
	$(MKBOOTFS) $(TARGET_RECOVERY_ROOT_OUT) | $(MINIGZIP) > $(recovery_ramdisk)
	$(hide) $(call build-nand-bootimage,$(4K_NAND_OUT),$(INTERNAL_4K_RECOVERYIMAGE_ARGS),$(INSTALLED_4K_RECOVERYIMAGE_TARGET))
ifeq ($(call is-board-platform,msm7627a),true)
	$(hide) $(call build-nand-bootimage,$(2K_NAND_OUT),$(INTERNAL_2K_RECOVERYIMAGE_ARGS),$(INSTALLED_2K_RECOVERYIMAGE_TARGET))
	$(hide) $(call build-nand-bootimage,$(BCHECC_OUT),$(INTERNAL_4K_RECOVERYIMAGE_ARGS),$(INSTALLED_BCHECC_RECOVERYIMAGE_TARGET))
endif # is-board-platform

ALL_DEFAULT_INSTALLED_MODULES += \
	$(INSTALLED_4K_BOOTIMAGE_TARGET) \
	$(INSTALLED_4K_SYSTEMIMAGE_TARGET) \
	$(INSTALLED_4K_USERDATAIMAGE_TARGET) \
	$(INSTALLED_4K_PERSISTIMAGE_TARGET)

ifneq ($(BUILD_TINY_ANDROID),true)
ALL_DEFAULT_INSTALLED_MODULES += $(INSTALLED_4K_RECOVERYIMAGE_TARGET)
endif # !BUILD_TINY_ANDROID

ALL_MODULES.$(LOCAL_MODULE).INSTALLED += \
	$(INSTALLED_4K_BOOTIMAGE_TARGET) \
	$(INSTALLED_4K_SYSTEMIMAGE_TARGET) \
	$(INSTALLED_4K_USERDATAIMAGE_TARGET) \
	$(INSTALLED_4K_PERSISTIMAGE_TARGET)

ifneq ($(BUILD_TINY_ANDROID),true)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(INSTALLED_4K_RECOVERYIMAGE_TARGET)
endif # !BUILD_TINY_ANDROID

endif # is-board-platform-in-list
