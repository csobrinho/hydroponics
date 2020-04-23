#ifndef HYDROPONICS_CONFIG_H
#define HYDROPONICS_CONFIG_H

#include "esp_err.h"

#include "config.pb-c.h"
#include "context.h"

esp_err_t config_init(context_t *context);

esp_err_t config_update(context_t *context, const uint8_t *data, size_t size);

esp_err_t config_dump(const Hydroponics__Config *config);

#endif //HYDROPONICS_CONFIG_H
