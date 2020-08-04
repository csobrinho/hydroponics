#!/usr/bin/env bash
set -o errexit

source ${IDF_PATH}/export.sh 2>&1 >/dev/null

python ${IDF_PATH}/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py \
  generate \
  ${ESPPROJECT}/firmware/private/nvs_config.csv \
  "$1/hydroponics-nvs.bin" 0x6000

esptool.py --port ${ESPPORT} write_flash 0x9000 "$1/hydroponics-nvs.bin"
