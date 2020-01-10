set(exclude_srcs)

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
        SRC_DIRS "." "console" "display" "driver" "network" "sensors"
        EXCLUDE_SRCS "${exclude_srcs}"
        INCLUDE_DIRS "." "console" "display" "driver" "network" "sensors"
        EMBED_FILES "embed/hydroponics_logo.bin")
