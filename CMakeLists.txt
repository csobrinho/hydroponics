# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(component_dirs)
if (CONFIG_IDF_TARGET_ESP32)
    # The LCD component doesn't support ESP32-S2 right now so disable it.
    list(APPEND component_dirs "$ENV{IDF_PATH}/../esp-iot-solution/components/i2s_devices/lcd_common")
endif ()
set(EXTRA_COMPONENT_DIRS "${component_dirs}" "main/config")

# Compile less components.
set(COMPONENTS cxx app bootloader esptool_py main)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hydroponics)