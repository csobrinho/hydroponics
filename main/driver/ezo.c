#include <string.h>

#include "freertos/FreeRTOS.h"

#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "ezo.h"

#define MAX_SENSORS 5

static const char *TAG = "ezo";
static ezo_sensor_t *sensors[MAX_SENSORS] = {0};
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

static esp_err_t ezo_register(ezo_sensor_t *sensor) {
    portENTER_CRITICAL(&spinlock);
    for (int n = 0; n < MAX_SENSORS; n++) {
        if (sensors[n] == NULL) {
            sensors[n] = sensor;
            portEXIT_CRITICAL(&spinlock);
            return ESP_OK;
        }
    }
    portEXIT_CRITICAL(&spinlock);
    return ESP_ERR_NO_MEM;
}

static void ezo_unregister(ezo_sensor_t *sensor) {
    portENTER_CRITICAL(&spinlock);
    for (int n = 0; n < MAX_SENSORS; n++) {
        if (sensors[n] == sensor) {
            sensors[n] = NULL;
        }
    }
    portEXIT_CRITICAL(&spinlock);
}

esp_err_t ezo_init(ezo_sensor_t *sensor) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    if (sensor->address == EZO_INVALID_ADDRESS) {
        ESP_LOGI(TAG, "[0x%.2x] Not found probe %s with description %s", sensor->address, sensor->probe, sensor->desc);
        return ESP_OK;
    }
    memset(sensor->type, 0, sizeof(sensor->type));
    memset(sensor->version, 0, sizeof(sensor->version));
    sensor->lock = xSemaphoreCreateMutex();

    // Allow the device to sleep a little bit just in case we were in the middle of a read operation before the reset.
    // If we don't, then sometimes we read the probe value instead of what was requested.
    vTaskDelay(pdMS_TO_TICKS(sensor->delay_read_ms));

    // Read type and version information.
    ESP_ERROR_CHECK(ezo_device_info(sensor));
    ESP_LOGI(TAG, "[0x%.2x] Found EZO-%s module (v%s) %s with probe %s", sensor->address, sensor->type, sensor->version,
             sensor->desc, sensor->probe);

    ESP_ERROR_CHECK(ezo_register(sensor));
    return ESP_OK;
}

esp_err_t ezo_free(ezo_sensor_t *sensor) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    memset(sensor->type, 0, sizeof(sensor->type));
    memset(sensor->version, 0, sizeof(sensor->version));
    ezo_unregister(sensor);

    return ESP_OK;
}

ezo_sensor_t *ezo_find(const char *desc) {
    portENTER_CRITICAL(&spinlock);
    for (int n = 0; n < MAX_SENSORS; n++) {
        ezo_sensor_t *sensor = sensors[n];
        if (sensor != NULL && strncasecmp(sensor->desc, desc, sizeof(sensor->desc)) == 0) {
            portEXIT_CRITICAL(&spinlock);
            return sensor;
        }
    }
    portEXIT_CRITICAL(&spinlock);
    return NULL;
}

float ezo_read_and_print(ezo_sensor_t *sensor, float temp, int precision, const char *unit) {
    if (sensor->address == EZO_INVALID_ADDRESS) {
        return CONTEXT_UNKNOWN_VALUE;
    }
    float value = 0.f;
    if (CONTEXT_VALUE_IS_VALID(temp)) {
        ESP_ERROR_CHECK(ezo_read_temperature(sensor, &value, temp));
    } else {
        ESP_ERROR_CHECK(ezo_read(sensor, &value));
    }
    ESP_LOGD(TAG, "%s %.*f %s", sensor->type, precision, value, unit);
    return value;
}
