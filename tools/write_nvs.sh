#!/usr/bin/env bash
set -o errexit

BUILD_DIR="$1"
NAME="$2"
CSV="${BUILD_DIR}/nvs_config_${NAME}.csv"
BIN="${BUILD_DIR}/nvs_config_${NAME}.bin"

source "${IDF_PATH}/export.sh" 2>&1 >/dev/null

cp -f "${ESPPROJECT}/firmware/private/nvs_config.csv" "${CSV}"
echo "iot_config,file,binary,components/protos/${NAME}.pb" >> "${CSV}"

python "${IDF_PATH}/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py" generate "${CSV}" "${BIN}" 0x6000

esptool.py --port "${ESPPORT}" write_flash 0x9000 "${BIN}"
