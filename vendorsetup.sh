# vendor/qcom/proprietary/prebuilt contains prebuilt binaries for
# a few modules in vendor/qcom/prioprietary. This script is invokde
# during source/envsetup.sh. It copies binaries from vendor/qcom/
# proprietary/prebuilt to croot/out.

if [ ! -d out ]; then
  mkdir out
fi

if [ -d "vendor/qcom/proprietary/prebuilt_HY11" ]; then
  cp -rf vendor/qcom/proprietary/prebuilt_HY11/* out
fi

if [ -d "vendor/qcom/proprietary/prebuilt_HY22" ]; then
  cp -rf vendor/qcom/proprietary/prebuilt_HY22/* out
fi
