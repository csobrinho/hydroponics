set(exclude_srcs)
set(requires "lcd_common")

if (CONFIG_ESP_SIMULATE_SENSORS)
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
        SRC_DIRS "." "console" "display" "driver" "driver/lcd" "filter" "network" "sensors" "tasks"
        EXCLUDE_SRCS "${exclude_srcs}"
        INCLUDE_DIRS "." "console" "display" "driver" "driver/lcd" "filter" "network" "sensors" "tasks"
        EMBED_FILES "../firmware/private/ec_private.pem" "embed/hydroponics_logo.bin"
        REQUIRES
        # Internal components.
        "protos" "bme280" "esp32-ds18b20" "esp32-owb" "esp32-rotary-encoder" "esp-google-iot" "u8g2" "ucglib" "esp-ccronexpr" "${requires}"
        # External components.
        "console" "json" "nvs_flash")
