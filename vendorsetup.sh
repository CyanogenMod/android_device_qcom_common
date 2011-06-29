# This script is invoked during source/envsetup.sh.
# It sets vendor specific environment for build.

if [ -d "vendor/qcom/proprietary/prebuilt_HY11" ]; then
  . vendor/qcom/proprietary/common/build/vendorsetup.sh
elif [ -d "vendor/qcom/proprietary/prebuilt_HY22" ]; then
  . vendor/qcom/proprietary/common/build/vendorsetup.sh
fi
