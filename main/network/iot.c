#include <string.h>

#include "cJSON.h"
#include "protobuf-c/protobuf-c.h"

#include "esp_err.h"
#include "esp_log.h"

#include "commands.pb-c.h"
#include "config.h"
#include "context.h"
#include "error.h"
#include "iot.h"
#include "mqtt.h"
#include "tasks/io.h"

static const char *TAG = "iot";

static esp_err_t iot_handle_command(context_t *context, const uint8_t *command, size_t size) {
    ARG_UNUSED(context);
    if (command == NULL || size == 0) {
        ESP_LOGW(TAG, "Empty command payload.");
        return ESP_OK;
    }
    Hydroponics__Commands *cmds = hydroponics__commands__unpack(NULL, size, command);
    ARG_CHECK(cmds != NULL, "Error unpacking the Commands proto");
    for (int i = 0; i < cmds->n_command; ++i) {
        Hydroponics__Command *cmd = cmds->command[i];
        switch (cmd->command_case) {
            case HYDROPONICS__COMMAND__COMMAND__NOT_SET:
                break;
            case HYDROPONICS__COMMAND__COMMAND_REBOOT: {
                ESP_LOGW(TAG, "System going to reboot in 4s");
                vTaskDelay(pdMS_TO_TICKS(4000));
                esp_restart();
            }
            case HYDROPONICS__COMMAND__COMMAND_SET: {
                for (int idx = 0; idx < cmd->set->n_output; ++idx) {
                    bool state = cmd->set->state == HYDROPONICS__OUTPUT_STATE__ON;
                    ESP_ERROR_CHECK(io_set_level(cmd->set->output[idx], state, 0));
                }
                break;
            }
            case HYDROPONICS__COMMAND__COMMAND_IMPULSE: {
                for (int idx = 0; idx < cmd->impulse->n_output; ++idx) {
                    ESP_ERROR_CHECK(io_set_level(cmd->impulse->output[idx], cmd->impulse->state,
                                                 cmd->impulse->delay_ms));
                }
                break;
            }
            default:
                ESP_LOGW(TAG, "Unknown command: %d", cmd->command_case);
                break;
        }
    }
    hydroponics__commands__free_unpacked(cmds, NULL);
    return ESP_OK;
}

static esp_err_t iot_handle_publish_telemetry(context_t *context, uint8_t **data, size_t *size) {
    ARG_CHECK(data != NULL, ERR_PARAM_NULL);
    ARG_CHECK(size != NULL, ERR_PARAM_NULL);

    *size = 0;
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "sensors.temp.indoor", context->sensors.temp.indoor);
    cJSON_AddNumberToObject(root, "sensors.temp.probe", context->sensors.temp.probe);
    cJSON_AddNumberToObject(root, "sensors.humidity", context->sensors.humidity);
    cJSON_AddNumberToObject(root, "sensors.pressure", context->sensors.pressure);
    if (CONTEXT_VALUE_IS_VALID(context->sensors.ec[CONFIG_TANK_A].value)) {
        cJSON_AddNumberToObject(root, "sensors.eca.value", context->sensors.ec[CONFIG_TANK_A].value);
    }
    if (CONTEXT_VALUE_IS_VALID(context->sensors.ph[CONFIG_TANK_A].value)) {
        cJSON_AddNumberToObject(root, "sensors.pha.value", context->sensors.ph[CONFIG_TANK_A].value);
    }
    if (CONTEXT_VALUE_IS_VALID(context->sensors.ec[CONFIG_TANK_B].value)) {
        cJSON_AddNumberToObject(root, "sensors.ecb.value", context->sensors.ec[CONFIG_TANK_B].value);
    }
    if (CONTEXT_VALUE_IS_VALID(context->sensors.ph[CONFIG_TANK_B].value)) {
        cJSON_AddNumberToObject(root, "sensors.phb.value", context->sensors.ph[CONFIG_TANK_B].value);
    }
    cJSON_AddNumberToObject(root, "sensors.tanka.value", context->sensors.tank[CONFIG_TANK_A].value);

    char *msg = cJSON_Print(root);
    cJSON_Delete(root);
    if (msg == NULL) {
        ESP_LOGE(TAG, "Failed to produce the json telemetry data");
        return ESP_FAIL;
    }
    *data = (uint8_t *) msg;
    *size = strlen(msg);
    return ESP_OK;
}

static esp_err_t iot_handle_publish_state(context_t *context, uint8_t **data, size_t *size) {
    ARG_CHECK(data != NULL, ERR_PARAM_NULL);
    ARG_CHECK(size != NULL, ERR_PARAM_NULL);

    *size = 0;
    char *msg = NULL;
    ESP_ERROR_CHECK(context_get_state_message(context, &msg));
    if (msg != NULL) {
        *data = (uint8_t *) msg;
        *size = strlen(msg);
    }
    return ESP_OK;
}

static mqtt_config_t config = {
        .handle_config = config_update,
        .handle_command = iot_handle_command,
        .handle_publish_telemetry = iot_handle_publish_telemetry,
        .handle_publish_state = iot_handle_publish_state,
};

esp_err_t iot_init(context_t *context) {
    ESP_ERROR_CHECK(mqtt_init(context, &config));
    return ESP_OK;
}
