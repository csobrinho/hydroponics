#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"

#include "config.h"
#include "context.h"
#include "error.h"
#include "storage.h"
#include "utils.h"

#define CONFIG_KEY_DEVICE_ID  "device_id"
#define CONFIG_KEY_SSID       "wifi_ssid"
#define CONFIG_KEY_PASSWORD   "wifi_password"
#define CONFIG_KEY_IOT_CONFIG "iot_config"

static const char *TAG = "config";
#define OUTPUT_BY_VALUE hydroponics__task__output__descriptor.values
#define OUTPUT_MAX hydroponics__task__output__descriptor.n_values
#define OUTPUT_ACTION_BY_VALUE hydroponics__task__output_action__descriptor.values
#define OUTPUT_ACTION_MAX hydroponics__task__output_action__descriptor.n_values

static esp_err_t config_load_from_storage(context_t *context) {
    uint8_t *data = NULL;
    size_t size = 0;
    ESP_ERROR_CHECK(storage_get_blob(CONFIG_KEY_IOT_CONFIG, &data, &size));
    if (size <= 0 || data == NULL) {
        SAFE_FREE(data);
        return ESP_OK;
    }
    ESP_ERROR_CHECK(config_update(context, data, size));
    SAFE_FREE(data);
    return ESP_OK;
}

static esp_err_t config_save_to_storage(const uint8_t *data, size_t size, bool *updated) {
    if (data == NULL || size == 0) {
        ESP_LOGI(TAG, "Config was deleted.");
        *updated = true;
        ESP_ERROR_CHECK(storage_delete(CONFIG_KEY_IOT_CONFIG));
        return ESP_OK;
    }
    uint8_t *current = NULL;
    size_t current_size = 0;
    ESP_ERROR_CHECK(storage_get_blob(CONFIG_KEY_IOT_CONFIG, &current, &current_size));
    if (current_size == size && memcmp(data, current, size) == 0) {
        // Same config, nothing to do.
        *updated = false;
        return ESP_OK;
    }
    ESP_ERROR_CHECK(storage_set_blob(CONFIG_KEY_IOT_CONFIG, data, size));
    *updated = true;
    return ESP_OK;
}

esp_err_t config_update(context_t *context, const uint8_t *data, size_t size) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    bool updated = false;
    ESP_ERROR_CHECK(config_save_to_storage(data, size, &updated));
    if (data == NULL || size <= 0) {
        ESP_ERROR_CHECK(context_set_config(context, NULL));
        return ESP_OK;
    }
    if (updated || context->config.config == NULL) {
        Hydroponics__Config *config = hydroponics__config__unpack(NULL, size, data);
        ESP_ERROR_CHECK(context_set_config(context, config));
        if (config == NULL) {
            ESP_LOGE(TAG, "Failed to parse the config proto.");
            return ESP_FAIL;
        }
        ESP_ERROR_CHECK(config_dump(config));
    }

    return ESP_OK;
}

esp_err_t config_dump(const Hydroponics__Config *config) {
    if (config == NULL) {
        return ESP_OK;
    }
    char *buf = NULL;
    size_t len = 0;
    FILE *stream = open_memstream(&buf, &len);
    if (stream == NULL) {
        ESP_LOGE(TAG, "Stream == NULL");
        return ESP_FAIL;
    }
    fprintf(stream, "\n");
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
            fprintf(stream, "  target: %.0f  [%.0f, %.0f]\n", config->controller->ec->target,
                    config->controller->ec->min,
                    config->controller->ec->max);
            if (config->controller->ec->pid != NULL) {
                fprintf(stream, "  PID sampling: %d ms\n", config->controller->ec->pid->sampling);
                fprintf(stream, "  PID p: %.2f   i: %.2f   d: %.2f\n", config->controller->ec->pid->p,
                        config->controller->ec->pid->i, config->controller->ec->pid->d);
            }
        } else {
            fprintf(stream, "  none\n");
        }
        fprintf(stream, "Controller PH:\n");
        if (config->controller->ph != NULL) {
            fprintf(stream, "  target: %.2f  [%.2f, %.2f]\n", config->controller->ph->target,
                    config->controller->ph->min,
                    config->controller->ph->max);
            if (config->controller->ph->pid != NULL) {
                fprintf(stream, "  PID sampling: %d ms\n", config->controller->ph->pid->sampling);
                fprintf(stream, "  PID p: %.2f   i: %.2f   d: %.2f\n", config->controller->ph->pid->p,
                        config->controller->ph->pid->i, config->controller->ph->pid->d);
            }
        } else {
            fprintf(stream, "  none\n");
        }
    }
    fprintf(stream, "Tasks:\n");
    if (config->n_task > 0 && config->task != NULL) {
        for (int i = 0; i < config->n_task; ++i) {
            fprintf(stream, "  [%*d] name: %s\n", config->n_task >= 10 ? 2 : 1, i, config->task[i]->name);
            for (int j = 0; j < config->task[i]->n_output; ++j) {
                Hydroponics__Task__Output output = config->task[i]->output[j];
                fprintf(stream, "      output: %s\n", output < OUTPUT_MAX ? OUTPUT_BY_VALUE[output].name : "???");
            }
            for (int j = 0; j < config->task[i]->n_cron; ++j) {
                Hydroponics__Task__Cron *cron = config->task[i]->cron[j];
                Hydroponics__Task__OutputAction action = cron->action;
                for (int k = 0; k < cron->n_expression; ++k) {
                    fprintf(stream, "      cron %s: %s\n",
                            action < OUTPUT_ACTION_MAX ? OUTPUT_ACTION_BY_VALUE[action].name : "???",
                            cron->expression[k]);
                }
            }
        }
    } else {
        fprintf(stream, "  none\n");
    }
    fclose(stream);
    ESP_LOGI(TAG, "%*s", len, buf);
    SAFE_FREE(buf);
    return ESP_OK;
}

esp_err_t config_init(context_t *context) {
    char *device_id = CONFIG_GIOT_DEVICE_ID;
    char *ssid = CONFIG_ESP_WIFI_SSID;
    char *password = CONFIG_ESP_WIFI_PASSWORD;
    ESP_ERROR_CHECK(storage_get_string(CONFIG_KEY_DEVICE_ID, &device_id, NULL));
    ESP_ERROR_CHECK(storage_get_string(CONFIG_KEY_SSID, &ssid, NULL));
    ESP_ERROR_CHECK(storage_get_string(CONFIG_KEY_PASSWORD, &password, NULL));
    ESP_ERROR_CHECK(context_set_base_config(context, device_id, ssid, password));
    ESP_ERROR_CHECK(config_load_from_storage(context));
    ESP_LOGI(TAG, "Config loaded");
    ESP_LOGI(TAG, "  device_id: %s", device_id);
    ESP_LOGI(TAG, "  ssid:      %s", ssid);
    ESP_ERROR_CHECK(config_dump(context->config.config));
    return ESP_OK;
}
