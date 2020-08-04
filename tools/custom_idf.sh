#!/usr/bin/env bash
set -o errexit

TARGET=${1:-"Default (ESP32-S2)"}
export IDF_TARGET="$(awk -F'[\(|\)]' '{ gsub("-", ""); print tolower($2) }' <<<$TARGET)"

echo "==========================================================================="
echo "                           Building for: ${IDF_TARGET}"
echo "==========================================================================="

source ${IDF_PATH}/export.sh 2>&1 >/dev/null
idf.py -B build/${IDF_TARGET} -G Ninja -DSDKCONFIG=sdkconfig_${IDF_TARGET} "${2:-build}" "${@:3}"
