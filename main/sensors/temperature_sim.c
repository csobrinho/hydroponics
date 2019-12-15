#include "esp_log.h"

#include "simulation.h"
#include "temperature.h"

static const char *TAG = "temperature";

esp_err_t temperature_hal_init(temperature_t *dev) {
    dev->num_devices = 1;
    ESP_LOGI(TAG, "Found %d sensor%s", dev->num_devices, dev->num_devices == 1 ? "" : "s");
    return ESP_OK;
}

void temperature_hal_read(temperature_t *dev) {
    dev->readings[0] = WITH_THRESHOLD(19, 0.1);
}
