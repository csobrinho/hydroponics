set(exclude_srcs)
set(extra_dirs)

if (CONFIG_IDF_TARGET_ESP32)
    # The LCD component doesn't support ESP32-S2 right now so disable it.
    list(APPEND extra_dirs "display")
endif ()

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
        SRC_DIRS "${extra_dirs}" "." "console" "driver" "filter" "network" "sensors" "tasks"
        EXCLUDE_SRCS "${exclude_srcs}"
        INCLUDE_DIRS "${extra_dirs}" "." "console" "driver" "filter" "network" "sensors" "tasks"
        EMBED_FILES "certs/ec_private.pem" "embed/hydroponics_logo.bin"
        REQUIRES
        # Internal components.
        "bme280" "esp32-ds18b20" "esp32-owb" "esp32-rotary-encoder" "esp-google-iot" "u8g2"
        # External components.
        "config" "console" "json" "nvs_flash")
