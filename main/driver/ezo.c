#include <string.h>

#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"

#include "error.h"
#include "ezo.h"

static const char *TAG = "ezo";

esp_err_t ezo_init(ezo_sensor_t *sensor) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    memset(sensor->type, 0, sizeof(sensor->type));
    memset(sensor->version, 0, sizeof(sensor->version));
    sensor->lock = xSemaphoreCreateMutex();

    // Allow the device to sleep a little bit just in case we were in the middle of a read operation before the reset.
    // If we don't, then sometimes we read the probe value instead of what was requested.
    vTaskDelay(pdMS_TO_TICKS(sensor->delay_read_ms));

    // Read type and version information.
    ESP_ERROR_CHECK(ezo_device_info(sensor));
    ESP_LOGI(TAG, "[0x%.2x] Found EZO-%s module (v%s) with probe %s", sensor->address, sensor->type, sensor->version,
             sensor->probe);

    return ESP_OK;
}

esp_err_t ezo_free(ezo_sensor_t *sensor) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    memset(sensor->type, 0, sizeof(sensor->type));
    memset(sensor->version, 0, sizeof(sensor->version));
    return ESP_OK;
}
