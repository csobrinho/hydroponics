#include <string.h>

#include "esp_log.h"

#include "ds18b20.h"

#include "buses.h"
#include "temperature.h"

#define DS18B20_RESOLUTION DS18B20_RESOLUTION_12_BIT

static const char *TAG = "temperature";

#define OWB_STATUS_CHECK(x, reason) do {            \
        owb_status __err_rc = (x);                  \
        if (__err_rc != OWB_STATUS_OK) {            \
            return reason;                          \
        }                                           \
    } while(0)

esp_err_t temperature_hal_init(temperature_t *dev) {
    dev->owb = owb_rmt_initialize(&dev->rmt_driver_info, ONE_WRITE_GPIO, RMT_CHANNEL_1, RMT_CHANNEL_0);
    OWB_STATUS_CHECK(owb_use_crc(dev->owb, true), ESP_ERR_NOT_SUPPORTED);  // enable CRC check for ROM code.

    // Find all connected devices
    ESP_LOGI(TAG, "1-Wire scanner:");
    OneWireBus_ROMCode device_rom_codes[OWB_MAX_DEVICES] = {0};
    OneWireBus_SearchState search_state = {0};
    bool found = false;
    OWB_STATUS_CHECK(owb_search_first(dev->owb, &search_state, &found), ESP_ERR_NOT_FOUND);
    while (found) {
        char rom_code_s[17];
        owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
        ESP_LOGI(TAG, "  %d : %s", dev->num_devices, rom_code_s);
        device_rom_codes[dev->num_devices] = search_state.rom_code;
        dev->num_devices++;
        owb_search_next(dev->owb, &search_state, &found);
    }
    ESP_LOGI(TAG, "Found %d sensor%s", dev->num_devices, dev->num_devices == 1 ? "" : "s");

    // Create DS18B20 devices on the 1-Wire bus.
    for (int i = 0; i < dev->num_devices; ++i) {
        DS18B20_Info *ds18b20_info = ds18b20_malloc();               // heap allocation.
        dev->devices[i] = ds18b20_info;

        if (dev->num_devices == 1) {
            ESP_LOGI(TAG, "Single device optimizations enabled\n");
            ds18b20_init_solo(ds18b20_info, dev->owb);                    // only one device on bus.
        } else {
            ds18b20_init(ds18b20_info, dev->owb, device_rom_codes[i]);    // associate with bus and device.
        }
        ds18b20_use_crc(ds18b20_info, true);                         // enable CRC check for temperature readings.
        ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION);
    }
    return ESP_OK;
}

void temperature_hal_read(temperature_t *dev) {
    if (dev->num_devices <= 0) {
        return;
    }
    ds18b20_convert_all(dev->owb);
    // All devices use the same resolution so use the first device to determine the delay
    ds18b20_wait_for_conversion(dev->devices[0]);

    // Read the results immediately after conversion otherwise it may fail.
    memset(dev->readings, 0, sizeof(dev->readings));
    memset(dev->errors, 0, sizeof(dev->errors));

    for (int i = 0; i < dev->num_devices; i++) {
        dev->errors[i] = ds18b20_read_temp(dev->devices[i], &dev->readings[i]);
    }
}
