#ifndef HYDROPONICS_DRIVER_STORAGE_H
#define HYDROPONICS_DRIVER_STORAGE_H

#include "esp_err.h"

#include "context.h"

#define STORAGE_KEY_WIFI_SSID     "wifi_ssid"
#define STORAGE_KEY_WIFI_PASSWORD "wifi_password"
#define STORAGE_KEY_CONFIG_HASH   "config_hash"
#define STORAGE_KEY_CONFIG        "config"

esp_err_t storage_init(context_t *context);

esp_err_t storage_get_string(const char *key, char **buf, size_t *length);

esp_err_t storage_set_string(const char *key, const char *buf);

esp_err_t storage_get_blob(const char *key, uint8_t **buf, size_t *length);

esp_err_t storage_set_blob(const char *key, const uint8_t *buf, size_t length);

esp_err_t storage_delete(const char *key);

#endif //HYDROPONICS_DRIVER_STORAGE_H
