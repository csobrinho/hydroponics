#include "freertos/FreeRTOS.h"

#include "cJSON.h"

#include "esp_err.h"
#include "esp_log.h"

#include "context.h"
#include "error.h"
#include "iot.h"
#include "mqtt.h"

static const char *TAG = "iot";

static esp_err_t iot_handle_config(context_t *context, const char *config) {
    ARG_UNUSED(context);
    ARG_UNUSED(config);
    return ESP_OK;
}

static esp_err_t iot_handle_publish(context_t *context, char **message) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "sensors.temp.indoor", context->sensors.temp.indoor);
    cJSON_AddNumberToObject(root, "sensors.temp.probe", context->sensors.temp.probe);
    cJSON_AddNumberToObject(root, "sensors.humidity", context->sensors.humidity);
    cJSON_AddNumberToObject(root, "sensors.pressure", context->sensors.pressure);
    cJSON_AddNumberToObject(root, "sensors.ec.value", context->sensors.ec.value);
    cJSON_AddNumberToObject(root, "sensors.ph.value", context->sensors.ph.value);
    *message = cJSON_Print(root);
    cJSON_Delete(root);
    if (*message == NULL) {
        ESP_LOGE(TAG, "Failed to produce the json telemetry string");
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t iot_handle_command(context_t *context, const char *command) {
    ARG_UNUSED(context);
    ARG_UNUSED(command);
    return ESP_OK;
}

static mqtt_config_t config = {
        .handle_config = iot_handle_config,
        .handle_publish = iot_handle_publish,
        .handle_command = iot_handle_command,
};

esp_err_t iot_init(context_t *context) {
    ESP_ERROR_CHECK(mqtt_init(context, &config));
    return ESP_OK;
}
