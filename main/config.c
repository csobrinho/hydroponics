#include "freertos/FreeRTOS.h"

#include "esp_err.h"
#include "esp_log.h"

#include "config.h"
#include "context.h"
#include "storage.h"

#define CONFIG_KEY_DEVICE_ID "device_id"
#define CONFIG_KEY_SSID      "ssid"
#define CONFIG_KEY_PASSWORD  "password"

static const char *TAG = "config";

esp_err_t config_init(context_t *context) {
    char *device_id = "";
    char *ssid = "";
    char *password = "";
    storage_get_string(CONFIG_KEY_DEVICE_ID, &device_id, NULL);
    storage_get_string(CONFIG_KEY_SSID, &ssid, NULL);
    storage_get_string(CONFIG_KEY_PASSWORD, &password, NULL);

    ESP_ERROR_CHECK(context_set_config(context, device_id, ssid, password));
    ESP_LOGI(TAG, "Config loaded");
    ESP_LOGI(TAG, "  device_id: %s", device_id);
    ESP_LOGI(TAG, "  ssid:      %s", ssid);

    return ESP_OK;
}