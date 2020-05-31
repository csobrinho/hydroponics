set(requires)
if (IDF_TARGET STREQUAL "esp32")
    list(APPEND requires "hydroponics-lcd-i2s")
else ()
    list(APPEND requires "i2s_parallel" "hydroponics-lcd-i2s-parallel")
endif ()

idf_component_register(
        SRC_DIRS "."
        INCLUDE_DIRS "."
        REQUIRES "hydroponics-error" "hydroponics-utils" "${requires}"
)
