#include <string.h>
#include <errno.h>

#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"

#include "buses.h"
#include "error.h"
#include "ezo.h"

static const char *TAG = "ezo";

#define LOG(args...) ESP_LOGD(args)

esp_err_t ezo_read_version(ezo_sensor_t *sensor) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->cmd_device_info, NULL));
    if (sensor->status != EZO_SENSOR_RESPONSE_SUCCESS) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    // ?I,XX,m.m
    if (sensor->bytes_read < 3) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    char *type = NULL; // Will be dynamically allocated by "%m".
    int ret = sscanf(sensor->buf, "?I,%m[^,],%ms", &type, &sensor->version);
    if (ret != 2) {
        ESP_LOGW(TAG, "[0x%.2x] Unexpected response: %d '%s'", ret, sensor->address, sensor->buf);
        return ESP_ERR_INVALID_RESPONSE;
    }
    if (strcmp(type, sensor->type) != 0) {
        ESP_LOGW(TAG, "[0x%.2x] Unexpected probe type. Expected: %s got: %s", sensor->address, sensor->type, type);
        free(type);
        return ESP_ERR_INVALID_RESPONSE;
    }
    free(type);
    return ESP_OK;
}

esp_err_t ezo_init(ezo_sensor_t *sensor) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);

    // Allow the device to sleep a little bit just in case we were in the middle of a read operation before the reset.
    // If we don't, then sometimes we read the probe value instead of what was requested.
    vTaskDelay(pdMS_TO_TICKS(sensor->cmd_read.delay_ms));

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

esp_err_t ezo_send_command(ezo_sensor_t *sensor, const ezo_cmd_t cmd, const char *args) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    LOG(TAG, "[0x%.2x] send_command '%s' and wait %dms", sensor->address, cmd.cmd, cmd.delay_ms);

    sensor->status = EZO_SENSOR_RESPONSE_UNKNOWN;
    sensor->bytes_read = 0;
    strlcpy(sensor->buf, cmd.cmd, EZO_MAX_BUFFER_LEN);
    if (args != NULL) {
        strlcat(sensor->buf, args, EZO_MAX_BUFFER_LEN);
    }

    // Write I2C address and send command.
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(handle));
    ESP_ERROR_CHECK(i2c_master_write_byte(handle, (sensor->address << 1) | I2C_MASTER_WRITE, I2C_WRITE_ACK_CHECK));
    ESP_ERROR_CHECK(i2c_master_write(handle, (uint8_t *) sensor->buf, strlen(sensor->buf), I2C_WRITE_ACK_CHECK));
    ESP_ERROR_CHECK(i2c_master_stop(handle));
    ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, handle, pdMS_TO_TICKS(I2C_TIMEOUT_MS)));
    i2c_cmd_link_delete(handle);

    memset(sensor->buf, 0, EZO_MAX_BUFFER_LEN);
    if (cmd.delay_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(cmd.delay_ms));
    }
    if (!cmd.has_read) {
        sensor->status = EZO_SENSOR_RESPONSE_SUCCESS;
        sensor->bytes_read = 0;
        return ESP_OK;
    }
    for (int retry = 0; retry < EZO_MAX_RETRIES; retry++) {
        sensor->status = EZO_SENSOR_RESPONSE_UNKNOWN;
        sensor->bytes_read = 0;
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

esp_err_t ezo_read_command(ezo_sensor_t *sensor, float *value) {
    ARG_CHECK(sensor != NULL, ERR_PARAM_NULL);
    ARG_CHECK(value != NULL, ERR_PARAM_NULL);

    ESP_ERROR_CHECK(ezo_send_command(sensor, sensor->cmd_read, NULL));
    if (sensor->status != EZO_SENSOR_RESPONSE_SUCCESS || sensor->bytes_read < 1) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    errno = 0; // To distinguish success/failure after call.
    *value = strtof(sensor->buf, NULL);
    if (errno != 0) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    LOG(TAG, "[0x%.2x] read value: %f", sensor->address, *value);
    return ESP_OK;
}
