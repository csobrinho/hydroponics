#include "freertos/FreeRTOS.h"

#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "context.h"
#include "storage.h"

static nvs_handle_t handle;

esp_err_t storage_init(context_t *context) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased. Retry nvs_flash_init.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &handle));
    return ESP_OK;
}

esp_err_t storage_get_string(const char *key, char **buf, size_t *length) {
    esp_err_t err = nvs_get_str(handle, key, NULL, length);
    switch (err) {
        case ESP_OK:
            *buf = malloc(*length);
            if (*buf == NULL) {
                return ESP_ERR_NO_MEM;
            }
            return nvs_get_str(handle, key, *buf, length);
        case ESP_ERR_NVS_NOT_FOUND:
            return ESP_OK;
        default:
            return err;
    }
}

esp_err_t storage_set_string(const char *key, char *buf) {
    esp_err_t err = nvs_set_str(handle, key, buf);
    return err == ESP_OK ? nvs_commit(handle) : err;
}