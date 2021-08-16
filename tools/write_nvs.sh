#!/usr/bin/env bash
set -o errexit

TARGET=${1:-"Default (ESP32-S2)"}
export IDF_TARGET="$(awk -F'[\(|\)]' '{ gsub("-", ""); print tolower($2) }' <<<$TARGET)"

if [[ "${IDF_TARGET}" == "esp32" ]]; then
  export ESPBAUD=921600
  export ESPPORT="/dev/cu.usbserial-0001"
else
  export ESPBAUD=3000000
  export ESPPORT="/dev/cu.usbserial-1410"
fi

BUILD_DIR="$2"
NAME="$3"
CSV="${BUILD_DIR}/nvs_config_${NAME}.csv"
BIN="${BUILD_DIR}/nvs_config_${NAME}.bin"

source "${IDF_PATH}/export.sh" 2>&1 >/dev/null

cp -f "${ESPPROJECT}/firmware/private/nvs_config.csv" "${CSV}"
echo "iot_config,file,binary,components/protos/${NAME}.pb" >> "${CSV}"

python "${IDF_PATH}/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py" generate "${CSV}" "${BIN}" 0x6000

esptool.py --port "${ESPPORT}" write_flash 0x9000 "${BIN}"
