#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

# Required!
DEVICE=binaries
DEVICE_COMMON=binaries
VENDOR=qcom

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$MY_DIR" ]]; then MY_DIR="$PWD"; fi

CM_ROOT="$MY_DIR"/../../../..

HELPER="$CM_ROOT"/vendor/cm/build/tools/extract_utils.sh
if [ ! -f "$HELPER" ]; then
    echo "Unable to find helper script at $HELPER"
    exit 1
fi
. "$HELPER"

SRC=$1
PLATFORM=$2
SUBSYSTEM=$3

# Initialize the helper for common device
if [ -f "$MY_DIR"/"$SUBSYSTEM"-"$PLATFORM"-32.txt -a -f "$MY_DIR"/"$SUBSYSTEM"-"$PLATFORM"-64.txt ]; then
    setup_vendor "$DEVICE/${PLATFORM}-32/$SUBSYSTEM" "$VENDOR" "$CM_ROOT" true true $SUBSYSTEM
    extract "$MY_DIR"/"$SUBSYSTEM"-"$PLATFORM"-32.txt "$SRC"

    setup_vendor "$DEVICE/${PLATFORM}-64/$SUBSYSTEM" "$VENDOR" "$CM_ROOT" true true $SUBSYSTEM
    extract "$MY_DIR"/"$SUBSYSTEM"-"$PLATFORM"-64.txt "$SRC"
else
    setup_vendor "$DEVICE/$PLATFORM/$SUBSYSTEM" "$VENDOR" "$CM_ROOT" true true $SUBSYSTEM
    extract "$MY_DIR"/"$SUBSYSTEM"-"$PLATFORM".txt "$SRC"
fi

"$MY_DIR"/setup-makefiles.sh
