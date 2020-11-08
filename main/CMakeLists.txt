set(exclude_srcs)

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

idf_component_register(
        SRC_DIRS "." "console" "display" "display/screens" "driver" "driver/lcd" "filter" "network" "sensors" "tasks"
        EXCLUDE_SRCS "${exclude_srcs}"
        INCLUDE_DIRS "." "console" "display" "display/screens" "driver" "driver/lcd" "filter" "network" "sensors" "tasks"
        EMBED_FILES "../firmware/private/ec_private.pem" "embed/hydroponics_logo.bin"
        REQUIRES
        # Own components.
        "hydroponics-context" "hydroponics-error" "hydroponics-lcd"
        "hydroponics-lcd-ucglib"
        "hydroponics-lcd-dev-st7796s"
        "hydroponics-utils"
        "esp-tuya"
        "button"
        # External components.
        "bme280" "esp32-ads1115" "esp-ccronexpr" "esp-google-iot" "esp32-ds18b20" "esp32-owb" "protos" "u8g2"
        "esp-timespec"
        # ESP-IDF components.
        "console" "json" "nvs_flash"
)
