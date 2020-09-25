#ifndef HYDROPONICS_NETWORK_MQTT_H
#define HYDROPONICS_NETWORK_MQTT_H

#include "esp_err.h"

#include "context.h"

typedef esp_err_t (*mqtt_handle_config_t)(context_t *context, const uint8_t *config, size_t size);

typedef esp_err_t (*mqtt_handle_command_t)(context_t *context, const uint8_t *command, size_t size);

typedef esp_err_t (*mqtt_handle_publish_telemetry_t)(context_t *context);

typedef struct {
    const mqtt_handle_config_t handle_config;
    const mqtt_handle_command_t handle_command;
    const mqtt_handle_publish_telemetry_t handle_publish_telemetry;
} mqtt_config_t;

esp_err_t mqtt_init(context_t *context, const mqtt_config_t *config);

esp_err_t mqtt_publish_event(uint8_t *data, size_t size);

esp_err_t mqtt_publish_state(uint8_t *data, size_t size);

#endif //HYDROPONICS_NETWORK_MQTT_H
