#!/usr/bin/env bash
set -o errexit

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
"${DIR}/custom_idf.sh" "Default (ESP32-S3)" "${@}"
