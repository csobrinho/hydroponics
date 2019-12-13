#include <errno.h>
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

const ezo_cmd_t ezo_cmd_read = {.cmd = "R"};
const ezo_cmd_t ezo_cmd_read_temperature = {.cmd = "RT"};
const ezo_cmd_t ezo_cmd_device_info = {.cmd = "I", .cmd_response = "?I,%m[^,],%ms"};
const ezo_cmd_t ezo_cmd_status = {.cmd = "Status", .cmd_response = "?Status,%c,%f"};
const ezo_cmd_t ezo_cmd_export = {.cmd = "Export", .cmd_response = "%d,%d"};
const ezo_cmd_t ezo_cmd_calibration = {.cmd = "Cal,?", .cmd_response = "?Cal,%d"};
const ezo_cmd_t ezo_cmd_calibration_low = {.cmd = "Cal,low"};
const ezo_cmd_t ezo_cmd_calibration_high = {.cmd = "Cal,high"};
const ezo_cmd_t ezo_cmd_calibration_mid = {.cmd = "Cal,mid"};

static esp_err_t ezo_read_version(ezo_sensor_t *sensor) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    ARG_CHECK(sensor->cmd_device_info != NULL, ERR_PARAM_NULL);

    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->cmd_device_info, sensor->delay_ms, NULL));
    if (sensor->status != EZO_SENSOR_RESPONSE_SUCCESS) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    // ?I,XX,m.m
    if (sensor->bytes_read < 3) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    char *type = NULL; // It should be dynamically allocated due to the "%m" inside 'cmd_response'.
    int ret = sscanf(sensor->buf, sensor->cmd_device_info->cmd_response, &type, &sensor->version);
    if (ret != 2) {
        ESP_LOGW(TAG, "[0x%.2x] Unexpected response: %d '%s'", ret, sensor->address, sensor->buf);
        if (type != NULL) {
            free(type);
        }
        return ESP_ERR_INVALID_RESPONSE;
    }
    if (strcmp(type, sensor->type) != 0) {
        ESP_LOGW(TAG, "[0x%.2x] Unexpected probe type. Expected: %s got: %s", sensor->address, sensor->type, type);
        if (type != NULL) {
            free(type);
        }
        return ESP_ERR_INVALID_RESPONSE;
    }
    free(type);
    return ESP_OK;
}

esp_err_t ezo_init(ezo_sensor_t *sensor) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    if (sensor->cmd_read == NULL) {
        sensor->cmd_read = &ezo_cmd_read;
    }
    if (sensor->cmd_read_temperature == NULL) {
        sensor->cmd_read_temperature = &ezo_cmd_read_temperature;
    }
    if (sensor->cmd_device_info == NULL) {
        sensor->cmd_device_info = &ezo_cmd_device_info;
    }
    if (sensor->cmd_status == NULL) {
        sensor->cmd_status = &ezo_cmd_status;
    }
    if (sensor->cmd_export == NULL) {
        sensor->cmd_export = &ezo_cmd_export;
    }
    if (sensor->cmd_calibration == NULL) {
        sensor->cmd_calibration = &ezo_cmd_calibration;
    }

    // Allow the device to sleep a little bit just in case we were in the middle of a read operation before the reset.
    // If we don't, then sometimes we read the probe value instead of what was requested.
    vTaskDelay(pdMS_TO_TICKS(sensor->delay_read_ms));

    // Read version and device information.
    ESP_ERROR_CHECK(ezo_read_version(sensor));
    ESP_LOGI(TAG, "[0x%.2x] Found EZO-%s module (v%s) with probe %s", sensor->address, sensor->type, sensor->version,
             sensor->probe);

    return ESP_OK;
}

esp_err_t ezo_free(ezo_sensor_t *sensor) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    if (sensor->version != NULL) {
        free(sensor->version);
        sensor->version = NULL;
    }
    return ESP_OK;
}

esp_err_t ezo_send_command_float(ezo_sensor_t *sensor, const ezo_cmd_t *cmd, uint16_t delay_ms, float value) {
    return ezo_send_command(sensor, cmd, delay_ms, ",%.2f", value);
}

esp_err_t ezo_send_command(ezo_sensor_t *sensor, const ezo_cmd_t *cmd, uint16_t delay_ms, const char *fmt, ...) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    LOG(TAG, "[0x%.2x] send_command '%s%s' wait %dms", sensor->address, cmd->cmd, fmt != NULL ? fmt : "", delay_ms);

    sensor->status = EZO_SENSOR_RESPONSE_UNKNOWN;
    sensor->bytes_read = 0;
    size_t written = strlcpy(sensor->buf, cmd->cmd, EZO_MAX_BUFFER_LEN);
    if (fmt != NULL) {
        va_list va;
        va_start(va, fmt);
        vsnprintf(&sensor->buf[written], EZO_MAX_BUFFER_LEN - written, fmt, va);
        va_end(va);
    }

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

static esp_err_t ezo_parse_float(ezo_sensor_t *sensor, float *value) {
    if (sensor->status != EZO_SENSOR_RESPONSE_SUCCESS || sensor->bytes_read < 1) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    errno = 0; // To distinguish success/failure after call.
    *value = strtof(sensor->buf, NULL);
    if (errno != 0) {
        LOG(TAG, "[0x%.2x] parse error: '%s'", sensor->address, sensor->buf);
        return ESP_ERR_INVALID_RESPONSE;
    }
    LOG(TAG, "[0x%.2x] parse: %f", sensor->address, *value);
    return ESP_OK;
}

esp_err_t ezo_read_command(ezo_sensor_t *sensor, float *value) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    ARG_CHECK(value != NULL, ERR_PARAM_NULL);

    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->cmd_read, sensor->delay_read_ms, NULL));
    return ezo_parse_float(sensor, value);
}

esp_err_t ezo_read_temperature_command(ezo_sensor_t *sensor, float *value, float temp) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    ARG_CHECK(value != NULL, ERR_PARAM_NULL);

    ESP_ERROR_CHECK(ezo_send_command_float(sensor, sensor->cmd_read_temperature, sensor->delay_read_ms, temp));
    return ezo_parse_float(sensor, value);
}

esp_err_t ezo_export_calibration(ezo_sensor_t *sensor, char **buffer, size_t *size) {
    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->cmd_export, sensor->delay_ms, ",?"));

    int rows;
    int bytes;
    int ret = sscanf(sensor->buf, sensor->cmd_export->cmd_response, &rows, &bytes);
    if (ret != 2) {
        ESP_LOGW(TAG, "[0x%.2x] Unexpected response: %d '%s'", ret, sensor->address, sensor->buf);
        return ESP_ERR_INVALID_RESPONSE;
    }
    size_t max_size = bytes + rows + 1;
    if (*buffer == NULL || *size != max_size) {
        *buffer = calloc(1, bytes + rows);
    }
    *size = 0;
    for (int i = 0; i < rows; ++i) {
        ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->cmd_export, sensor->delay_ms, NULL));
        *size += strlcat(*buffer, sensor->buf, max_size - *size);
        *size += strlcat(*buffer, "\n", max_size - *size);
    }
    return ESP_OK;
}

esp_err_t ezo_calibration_status(ezo_sensor_t *sensor, uint8_t *value) {
    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->cmd_calibration, sensor->delay_ms, NULL));
    int ret = sscanf(sensor->buf, sensor->cmd_export->cmd_response, &value);
    if (ret != 1) {
        ESP_LOGW(TAG, "[0x%.2x] Unexpected response: %d '%s'", ret, sensor->address, sensor->buf);
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}