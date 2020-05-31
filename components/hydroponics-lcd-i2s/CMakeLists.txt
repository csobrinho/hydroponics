idf_component_register(
        SRC_DIRS "."
        INCLUDE_DIRS "."
        REQUIRES "hydroponics-error" "hydroponics-lcd" "hydroponics-utils" "lcd_common"
        REQUIRED_IDF_TARGETS "esp32"
)
