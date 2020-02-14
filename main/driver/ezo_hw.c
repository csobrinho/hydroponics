#include <string.h>
#include <stdarg.h>

#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"

#include "buses.h"
#include "error.h"
#include "ezo.h"

#define LOG(args...) ESP_LOGD(args)
static const char *TAG = "ezo";

esp_err_t ezo_send_command(ezo_sensor_t *sensor, uint16_t delay_ms, const char *cmd_fmt, ...) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    ARG_CHECK(delay_ms > 0, ERR_PARAM_LE_ZERO);
    ARG_CHECK(cmd_fmt != NULL, ERR_PARAM_NULL);
    LOG(TAG, "[0x%.2x] send_command '%s' and wait %dms", sensor->address, cmd_fmt, delay_ms);

    sensor->status = EZO_SENSOR_RESPONSE_UNKNOWN;
    sensor->bytes_read = 0;
    va_list va;
    va_start(va, cmd_fmt);
    vsnprintf(sensor->buf, EZO_MAX_BUFFER_LEN, cmd_fmt, va);
    va_end(va);

    // Write I2C address and send command.
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(handle));
    ESP_ERROR_CHECK(i2c_master_write_byte(handle, (sensor->address << 1) | I2C_MASTER_WRITE, I2C_WRITE_ACK_CHECK));
    ESP_ERROR_CHECK(i2c_master_write(handle, (uint8_t *) sensor->buf, strlen(sensor->buf), I2C_WRITE_ACK_CHECK));
    ESP_ERROR_CHECK(i2c_master_stop(handle));

    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, handle, pdMS_TO_TICKS(I2C_TIMEOUT_MS)));
    i2c_cmd_link_delete(handle);

    if (delay_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
    for (int retry = 0; retry < EZO_MAX_RETRIES; retry++) {
        sensor->status = EZO_SENSOR_RESPONSE_UNKNOWN;
        memset(sensor->buf, 0, EZO_MAX_BUFFER_LEN);

        handle = i2c_cmd_link_create();
        i2c_master_start(handle);
        i2c_master_write_byte(handle, (sensor->address << 1) | I2C_MASTER_READ, I2C_WRITE_ACK_CHECK);
        i2c_master_read_byte(handle, (uint8_t *) &sensor->status, I2C_MASTER_ACK);
        i2c_master_read(handle, (uint8_t *) sensor->buf, EZO_MAX_BUFFER_LEN - 1, I2C_MASTER_LAST_NACK);
        i2c_master_stop(handle);
        esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, handle, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
        i2c_cmd_link_delete(handle);
        LOG(TAG, "[0x%.2x] send_command err: 0x%02x status: %d", sensor->address, err, sensor->status);

        if (sensor->status == EZO_SENSOR_RESPONSE_PROCESSING) {
            LOG(TAG, "[0x%.2x] send_command (retrying)", sensor->address);
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }
        if (err == ESP_FAIL
            || err == ESP_ERR_TIMEOUT
            || sensor->status == EZO_SENSOR_RESPONSE_UNKNOWN
            || sensor->status == EZO_SENSOR_RESPONSE_NO_DATA
            || sensor->status == EZO_SENSOR_RESPONSE_SYNTAX_ERROR) {
            break;
        }
        if (err != ESP_OK || sensor->status != EZO_SENSOR_RESPONSE_SUCCESS) {
            ESP_LOGE(TAG, "[0x%.2x] send_command unexpected state", sensor->address);
            break;
        }
        sensor->bytes_read = strlen(sensor->buf);
        LOG(TAG, "[0x%.2x] read: '%s'", sensor->address, sensor->buf);
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t ezo_parse_response(ezo_sensor_t *sensor, uint8_t fields, const char *response_fmt, ...) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    LOG(TAG, "[0x%.2x] read_response '%s'", sensor->address, response_fmt);

    if (response_fmt == NULL && fields == 0 && sensor->bytes_read != 0) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    if (sensor->bytes_read <= 0) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    // Make sure the buffer ends with a \0 to avoid vsscanf scanning our whole memory.
    sensor->buf[EZO_MAX_BUFFER_LEN - 1] = '\0';
    va_list va;
    va_start(va, response_fmt);
    int scanned_fields = vsscanf(sensor->buf, response_fmt, va);
    va_end(va);

    if (scanned_fields != fields) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}

esp_err_t ezo_read(ezo_sensor_t *sensor, float *value) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->delay_read_ms, "R"));
    ESP_ERROR_CHECK(ezo_parse_response(sensor, 1, "%f", value));
    xSemaphoreGive(sensor->lock);

    return ESP_OK;
}

esp_err_t ezo_read_temperature(ezo_sensor_t *sensor, float *value, float temp) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->delay_read_ms, "RT,%.2f", temp));
    ESP_ERROR_CHECK(ezo_parse_response(sensor, 1, "%f", value));
    xSemaphoreGive(sensor->lock);

    return ESP_OK;
}

esp_err_t ezo_device_info(ezo_sensor_t *sensor) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->delay_ms, "I"));
    ESP_ERROR_CHECK(ezo_parse_response(sensor, 2, "?I,%[^,],%s", sensor->type, sensor->version));
    xSemaphoreGive(sensor->lock);

    return ESP_OK;
}

esp_err_t ezo_status(ezo_sensor_t *sensor, ezo_status_t *status, float *voltage) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->delay_ms, "Status"));
    ESP_ERROR_CHECK(ezo_parse_response(sensor, 2, "?Status,%c,%f", (char *) status, voltage));
    xSemaphoreGive(sensor->lock);

    return ESP_OK;
}

esp_err_t ezo_export_calibration(ezo_sensor_t *sensor, char **buffer, size_t *size) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    int rows;
    int bytes;
    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->delay_ms, "Export,?"));
    ESP_ERROR_CHECK(ezo_parse_response(sensor, 2, "%d,%d", &rows, &bytes));
    size_t max_size = bytes + rows + 1;
    *buffer = calloc(1, bytes + rows);
    *size = 0;
    for (int i = 0; i < rows; ++i) {
        ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->delay_ms, "Export"));
        ESP_ERROR_CHECK(ezo_parse_response(sensor, 1, "%s", &(*buffer)[*size]));
        *size = strnlen(*buffer, max_size);
        *buffer[*size++] = '\n';
    }
    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->delay_ms, "Export"));
    ESP_ERROR_CHECK(ezo_parse_response(sensor, 0, "*DONE"));
    xSemaphoreGive(sensor->lock);

    // Make sure the buffer ends with a \0 to avoid vsscanf scanning our whole memory.
    *buffer[*size] = '\0';
    return *size == max_size ? ESP_OK : ESP_ERR_INVALID_SIZE;
}

esp_err_t ezo_calibration_mode(ezo_sensor_t *sensor, ezo_calibration_mode_t *mode) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->delay_ms, "Cal,?"));
    ESP_ERROR_CHECK(ezo_parse_response(sensor, 1, "?Cal,%d", (int *) mode));
    xSemaphoreGive(sensor->lock);

    return ESP_OK;
}

static const char *EZO_CALIBRATION_STEPS[EZO_CALIBRATION_STEP_MAX] = {
        "Cal,dry",
        "Cal,%.2f",
        "Cal,low,%.2f",
        "Cal,mid,%.2f",
        "Cal,high,%.2f",
        "Cal,clear",
};

esp_err_t ezo_calibration_step(ezo_sensor_t *sensor, ezo_calibration_step_t step, float value) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    ARG_CHECK(step < EZO_CALIBRATION_STEP_MAX, "parameter >= EZO_CALIBRATION_MAX");

    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->delay_calibration_ms, EZO_CALIBRATION_STEPS[step], value));
    ESP_ERROR_CHECK(ezo_parse_response(sensor, 0, NULL));
    xSemaphoreGive(sensor->lock);

    return ESP_OK;
}

esp_err_t ezo_protocol_lock(ezo_sensor_t *sensor, bool lock) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->delay_ms, "Plock,%d", lock ? 1 : 0));
    ESP_ERROR_CHECK(ezo_parse_response(sensor, 0, NULL));
    xSemaphoreGive(sensor->lock);

    return ESP_OK;
}