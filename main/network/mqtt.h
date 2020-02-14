#ifndef HYDROPONICS_NETWORK_MQTT_H
#define HYDROPONICS_NETWORK_MQTT_H

#include "esp_err.h"

#include "context.h"

typedef esp_err_t (*mqtt_handle_config_t)(context_t *context, const char *config);

typedef esp_err_t (*mqtt_handle_publish_t)(context_t *context, char **message);

typedef esp_err_t (*mqtt_handle_command_t)(context_t *context, const char *command);

typedef struct {
    const mqtt_handle_config_t handle_config;
    const mqtt_handle_publish_t handle_publish;
    const mqtt_handle_command_t handle_command;
} mqtt_config_t;

esp_err_t mqtt_init(context_t *context, const mqtt_config_t *config);

#endif //HYDROPONICS_NETWORK_MQTT_H
