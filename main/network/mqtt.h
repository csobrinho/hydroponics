#ifndef HYDROPONICS_MQTT_H
#define HYDROPONICS_MQTT_H

#include "esp_err.h"

#include "context.h"

typedef void (*mqtt_handle_config_t)(context_t *context, char *config);

typedef void (*mqtt_handle_publish_t)(context_t *context, char **message);

typedef void (*mqtt_handle_command_t)(context_t *context, char *command);

typedef struct {
    const mqtt_handle_config_t handle_config;
    const mqtt_handle_publish_t handle_publish;
    const mqtt_handle_command_t handle_command;
} mqtt_config_t;

esp_err_t mqtt_init(context_t *context, const mqtt_config_t *config);

#endif //HYDROPONICS_MQTT_H
