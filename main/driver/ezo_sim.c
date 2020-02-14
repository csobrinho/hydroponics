#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "error.h"
#include "ezo.h"
#include "simulation.h"

static const char *TAG = "ezo";

esp_err_t ezo_send_command(ezo_sensor_t *sensor, uint16_t delay_ms, const char *cmd_fmt, ...) {
    ARG_UNUSED(sensor);
    ARG_UNUSED(delay_ms);
    ARG_UNUSED(cmd_fmt);
    return ESP_FAIL;
}

esp_err_t ezo_parse_response(ezo_sensor_t *sensor, uint8_t fields, const char *response_fmt, ...) {
    ARG_UNUSED(sensor);
    ARG_UNUSED(fields);
    ARG_UNUSED(response_fmt);
    return ESP_FAIL;
}

esp_err_t ezo_read(ezo_sensor_t *sensor, float *value) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(sensor->delay_read_ms));
    xSemaphoreGive(sensor->lock);

    *value = WITH_THRESHOLD(sensor->simulate, sensor->threshold);
    return ESP_OK;
}

esp_err_t ezo_read_temperature(ezo_sensor_t *sensor, float *value, float temp) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    ARG_UNUSED(temp);

    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(sensor->delay_read_ms));
    xSemaphoreGive(sensor->lock);

    *value = WITH_THRESHOLD(sensor->simulate, sensor->threshold);
    return ESP_OK;
}

esp_err_t ezo_device_info(ezo_sensor_t *sensor) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    strlcpy(sensor->type, "sim", sizeof(sensor->type));
    strlcpy(sensor->version, "x.xx", sizeof(sensor->version));
    return ESP_OK;
}

esp_err_t ezo_status(ezo_sensor_t *sensor, ezo_status_t *status, float *voltage) {
    ARG_UNUSED(sensor);
    ARG_UNUSED(status);
    ARG_UNUSED(voltage);
    return ESP_FAIL;
}

esp_err_t ezo_export_calibration(ezo_sensor_t *sensor, char **buffer, size_t *size) {
    ARG_UNUSED(sensor);
    ARG_UNUSED(buffer);
    ARG_UNUSED(size);
    return ESP_FAIL;
}

esp_err_t ezo_calibration_mode(ezo_sensor_t *sensor, ezo_calibration_mode_t *mode) {
    ARG_UNUSED(sensor);
    ARG_UNUSED(mode);
    return ESP_FAIL;
}

esp_err_t ezo_calibration_step(ezo_sensor_t *sensor, ezo_calibration_step_t step, float value) {
    ARG_UNUSED(sensor);
    ARG_UNUSED(step);
    ARG_UNUSED(value);
    return ESP_FAIL;
}

esp_err_t ezo_protocol_lock(ezo_sensor_t *sensor, bool lock) {
    ARG_UNUSED(sensor);
    ARG_UNUSED(lock);
    return ESP_FAIL;
}