set(exclude_srcs)
set(requires)

if (CONFIG_ESP_SENSOR_SIMULATE)
    list(APPEND exclude_srcs
            "driver/ezo_hw.c"
            "sensors/humidity_pressure_hw.c"
            "sensors/temperature_hw.c")
else ()
    list(APPEND exclude_srcs
            "driver/ezo_sim.c"
            "sensors/humidity_pressure_sim.c"
            "sensors/temperature_sim.c")
endif ()

if (CONFIG_ESP_LCD_ST7796S)
    list(APPEND requires "hydroponics-lcd-dev-st7796s")
elseif (CONFIG_ESP_LCD_RM68090)
    list(APPEND requires "hydroponics-lcd-dev-rm68090")
endif ()

idf_component_register(
        SRC_DIRS "." "console" "display" "display/screens" "driver" "driver/lcd" "filter" "network" "sensors" "tasks"
        EXCLUDE_SRCS "${exclude_srcs}"
        INCLUDE_DIRS "." "console" "display" "display/screens" "driver" "driver/lcd" "filter" "network" "sensors" "tasks"
        EMBED_FILES "../firmware/private/ec_private.pem" "embed/hydroponics_logo.bin"
        REQUIRES
        # Own components.
        "hydroponics-context" "hydroponics-error" "hydroponics-utils"
        "hydroponics-lcd" "hydroponics-lcd-ucglib"
        "esp-tuya"
        "button"
        "${requires}"
        # External components.
        "bme280" "esp32-ads1115" "esp-ccronexpr" "esp-google-iot" "esp32-ds18b20" "esp32-owb" "protos" "u8g2"
        "esp-timespec"
        # ESP-IDF components.
        "console" "json" "nvs_flash"
)
