#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"

#include "buses.h"
#include "context.h"
#include "error.h"
#include "temperature.h"

#define MAX_DEVICES        2
#define DS18B20_RESOLUTION DS18B20_RESOLUTION_12_BIT

static const char *TAG = "temperature";

static owb_rmt_driver_info rmt_driver_info;
static OneWireBus *owb;
static DS18B20_Info *devices[MAX_DEVICES] = {0};
static int num_devices;

#define OWB_STATUS_CHECK(x, reason) do {            \
        owb_status __err_rc = (x);                  \
        if (__err_rc != OWB_STATUS_OK) {            \
            return reason;                          \
        }                                           \
    } while(0)

static void temperature_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    int errors_count[MAX_DEVICES] = {0};
    int sample_count = 0;

    while (1) {
        TickType_t last_wake_time = xTaskGetTickCount();

        ds18b20_convert_all(owb);
        // All devices use the same resolution so use the first device to determine the delay
        ds18b20_wait_for_conversion(devices[0]);

        // Read the results immediately after conversion otherwise it may fail.
        float readings[MAX_DEVICES] = {0};
        DS18B20_ERROR errors[MAX_DEVICES] = {0};

        for (int i = 0; i < num_devices; i++) {
            errors[i] = ds18b20_read_temp(devices[i], &readings[i]);
            context->sensors.temp.water = readings[i];
        }
        ESP_LOGI(TAG, "Temperature readings (degrees C): sample %d", ++sample_count);
        for (int i = 0; i < num_devices; i++) {
            if (errors[i] != DS18B20_OK) {
                ++errors_count[i];
            }
            ESP_LOGI(TAG, "  %d: %.1f    %d errors", i, readings[i], errors_count[i]);
        }
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_TEMPERATURE_MS));
    }
}

esp_err_t temperature_init(context_t *context) {
    // Setup the GPIOs.
    gpio_set_direction(ONE_WRITE_GPIO, GPIO_MODE_INPUT_OUTPUT);

    owb = owb_rmt_initialize(&rmt_driver_info, ONE_WRITE_GPIO, RMT_CHANNEL_1, RMT_CHANNEL_0);
    OWB_STATUS_CHECK(owb_use_crc(owb, true), ESP_ERR_NOT_SUPPORTED);  // enable CRC check for ROM code.

    // Find all connected devices
    ESP_LOGI(TAG, "1-Wire scanner:");
    OneWireBus_ROMCode device_rom_codes[MAX_DEVICES] = {0};
    OneWireBus_SearchState search_state = {0};
    bool found = false;
    OWB_STATUS_CHECK(owb_search_first(owb, &search_state, &found), ESP_ERR_NOT_FOUND);
    while (found) {
        char rom_code_s[17];
        owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
        ESP_LOGI(TAG, "  %d : %s", num_devices, rom_code_s);
        device_rom_codes[num_devices] = search_state.rom_code;
        ++num_devices;
        owb_search_next(owb, &search_state, &found);
    }
    ESP_LOGI(TAG, "Found %d sensor%s", num_devices, num_devices == 1 ? "" : "s");

    // Create DS18B20 devices on the 1-Wire bus.
    for (int i = 0; i < num_devices; ++i) {
        DS18B20_Info *ds18b20_info = ds18b20_malloc();               // heap allocation.
        devices[i] = ds18b20_info;

        if (num_devices == 1) {
            ESP_LOGI(TAG, "Single device optimizations enabled\n");
            ds18b20_init_solo(ds18b20_info, owb);                    // only one device on bus.
        } else {
            ds18b20_init(ds18b20_info, owb, device_rom_codes[i]);    // associate with bus and device.
        }
        ds18b20_use_crc(ds18b20_info, true);                         // enable CRC check for temperature readings.
        ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION);
    }

    xTaskCreatePinnedToCore(temperature_task, "temperature", 4096, context, 15, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
