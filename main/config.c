#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"

#include "config.h"
#include "context.h"
#include "error.h"
#include "storage.h"

#define CONFIG_KEY_DEVICE_ID "device_id"
#define CONFIG_KEY_SSID      "ssid"
#define CONFIG_KEY_PASSWORD  "password"
#define CONFIG_KEY_PROTO     "proto"

static const char *TAG = "config";

static esp_err_t config_load_proto(void) {
    char *proto = NULL;
    size_t proto_len = 0;
    ESP_ERROR_CHECK(storage_get_string(CONFIG_KEY_PROTO, &proto, &proto_len));
    if (proto_len <= 0 || proto == NULL) {
        if (proto != NULL) {
            free(proto);
        }
        return ESP_OK;
    }
    Hydroponics__Config *config;
    ESP_ERROR_CHECK(config_parse((uint8_t *) proto, proto_len, &config));
    free(proto);
    ESP_ERROR_CHECK(config_dump(config));
    ESP_ERROR_CHECK(config_free(config));
    return ESP_OK;
}

esp_err_t config_parse(const uint8_t *data, size_t size, Hydroponics__Config **config) {
    ARG_CHECK(data != NULL, ERR_PARAM_NULL);
    ARG_CHECK(config != NULL, ERR_PARAM_NULL);
    ARG_CHECK(size > 0, ERR_PARAM_LE_ZERO);
    *config = hydroponics__config__unpack(NULL, size, data);
    if (*config == NULL) {
        ESP_LOGE(TAG, "Failed to parse the config proto.");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t config_free(Hydroponics__Config *config) {
    ARG_CHECK(config != NULL, ERR_PARAM_NULL);
    hydroponics__config__free_unpacked(config, NULL);
    return ESP_OK;
}

esp_err_t config_dump(const Hydroponics__Config *config) {
    ARG_CHECK(config != NULL, ERR_PARAM_NULL);

    char *buf = NULL;
    size_t len = 0;
    FILE *stream = open_memstream(&buf, &len);
    if (stream == NULL) {
        ESP_LOGE(TAG, "Stream == NULL");
        return ESP_FAIL;
    }
    fprintf(stream, "Sampling:\n");
    if (config->sampling != NULL) {
        fprintf(stream, "  humidity:      %d ms\n", config->sampling->humidity);
        fprintf(stream, "  temperature:   %d ms\n", config->sampling->temperature);
        fprintf(stream, "  ec_probe:      %d ms\n", config->sampling->ec_probe);
        fprintf(stream, "  ec_probe_temp: %d ms\n", config->sampling->ec_probe_temp);
        fprintf(stream, "  ph_probe:      %d ms\n", config->sampling->ph_probe);
    } else {
        fprintf(stream, "  none\n");
    }
    if (config->controller != NULL) {
        fprintf(stream, "Controller EC:\n");
        if (config->controller->ec != NULL) {
            fprintf(stream, "  target: %.f  [%.f, %.f]\n", config->controller->ec->target, config->controller->ec->min,
                    config->controller->ec->max);
        } else {
            fprintf(stream, "  none\n");
        }
        fprintf(stream, "Controller PH:\n");
        if (config->controller->ph != NULL) {
            fprintf(stream, "  target: %.2f  [%.2f, %.2f]\n", config->controller->ph->target,
                    config->controller->ph->min,
                    config->controller->ph->max);
        } else {
            fprintf(stream, "  none\n");
        }
    }
    fprintf(stream, "Tasks:\n");
    if (config->n_task > 0 && config->task != NULL) {
        for (int i = 0; i < config->n_task; ++i) {
            fprintf(stream, "  [%*d] name: %s\n", config->n_task >= 10 ? 2 : 1, i, config->task[i]->name);
            for (int j = 0; j < config->task[i]->n_output; ++j) {
                fprintf(stream, "      output: %s\n", config->task[i]->output[j]);
            }
            for (int j = 0; j < config->task[i]->n_cron_on; ++j) {
                fprintf(stream, "      cron on: %s\n", config->task[i]->cron_on[j]);
            }
            for (int j = 0; j < config->task[i]->n_cron_off; ++j) {
                fprintf(stream, "      cron off: %s\n", config->task[i]->cron_off[j]);
            }
        }
    } else {
        fprintf(stream, "  none\n");
    }
    fclose(stream);
    ESP_LOGI(TAG, "%*s", len, buf);
    free(buf);
    return ESP_OK;
}

esp_err_t config_init(context_t *context) {
    char *device_id = CONFIG_GIOT_DEVICE_ID;
    char *ssid = CONFIG_ESP_WIFI_SSID;
    char *password = CONFIG_ESP_WIFI_PASSWORD;
    ESP_ERROR_CHECK(storage_get_string(CONFIG_KEY_DEVICE_ID, &device_id, NULL));
    ESP_ERROR_CHECK(storage_get_string(CONFIG_KEY_SSID, &ssid, NULL));
    ESP_ERROR_CHECK(storage_get_string(CONFIG_KEY_PASSWORD, &password, NULL));
    ESP_ERROR_CHECK(config_load_proto());
    ESP_ERROR_CHECK(context_set_config(context, device_id, ssid, password));
    ESP_LOGI(TAG, "Config loaded");
    ESP_LOGI(TAG, "  device_id: %s", device_id);
    ESP_LOGI(TAG, "  ssid:      %s", ssid);
    return ESP_OK;
}
