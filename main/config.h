#ifndef HYDROPONICS_CONFIG_H
#define HYDROPONICS_CONFIG_H

#include "esp_err.h"

#include "config.pb-c.h"
#include "context.h"

typedef void (*config_callback_t)(const Hydroponics__Config *config);

esp_err_t config_init(context_t *context);

esp_err_t config_update(context_t *context, const uint8_t *data, size_t size);

esp_err_t config_dump(const Hydroponics__Config *config);

esp_err_t config_register(config_callback_t callback);

esp_err_t config_unregister(config_callback_t callback);

#endif //HYDROPONICS_CONFIG_H
