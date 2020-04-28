cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/../esp-iot-solution/components/i2s_devices/lcd_common" "main/config")

# Compile less components.
set(COMPONENTS cxx app bootloader esptool_py main)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hydroponics)