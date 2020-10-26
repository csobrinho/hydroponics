#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"

#include "esp_err.h"
#include "esp_log.h"

#include "commands.pb-c.h"
#include "config.h"
#include "context.h"
#include "error.h"
#include "iot.h"
#include "mqtt.h"
#include "state.h"
#include "tasks/io.h"
#include "utils.h"

typedef enum {
    OP_TELEMETRY = 0,
    OP_STATE,
    OP_MAX,
} op_type_t;

typedef struct {
    op_type_t type;
    size_t len;
    uint8_t buf[1]; // Reserve one byte but allocate as many bytes as necessary.
} op_t;

#define BUFFER_SIZE 4096
#define BUFFER_WAIT_MS 50
#define PUBLISH_MIN_MS 2000 // 2x the minimum update policy of 1 state/s.

#define ADD_VALUE(type, val) do {       \
    if (CONTEXT_VALUE_IS_VALID(val)) {  \
        types[size] = (type);           \
        values[size++] = (val);         \
    }                                   \
} while(0)

static const char *const TAG = "iot";
static RingbufHandle_t ring = NULL;

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

static esp_err_t iot_handle_publish_telemetry(context_t *context) {
    uint32_t max_types = enum_max(&hydroponics__state_telemetry__type__descriptor);
    Hydroponics__StateTelemetry__Type types[max_types];
    float values[max_types];

    for (int i = 0; i < max_types; ++i) {
        types[i] = HYDROPONICS__STATE_TELEMETRY__TYPE__UNKNOWN;
        values[i] = 0.f;
    }

    int size = 0;
    context_lock(context);
    ADD_VALUE(HYDROPONICS__STATE_TELEMETRY__TYPE__TEMP_INDOOR, context->sensors.temp.indoor);
    ADD_VALUE(HYDROPONICS__STATE_TELEMETRY__TYPE__TEMP_PROBE, context->sensors.temp.probe);
    ADD_VALUE(HYDROPONICS__STATE_TELEMETRY__TYPE__HUMIDITY, context->sensors.humidity);
    ADD_VALUE(HYDROPONICS__STATE_TELEMETRY__TYPE__PRESSURE, context->sensors.pressure);
    ADD_VALUE(HYDROPONICS__STATE_TELEMETRY__TYPE__EC_A, context->sensors.ec[CONFIG_TANK_A].value);
    ADD_VALUE(HYDROPONICS__STATE_TELEMETRY__TYPE__EC_B, context->sensors.ec[CONFIG_TANK_B].value);
    ADD_VALUE(HYDROPONICS__STATE_TELEMETRY__TYPE__PH_A, context->sensors.ph[CONFIG_TANK_A].value);
    ADD_VALUE(HYDROPONICS__STATE_TELEMETRY__TYPE__PH_B, context->sensors.ph[CONFIG_TANK_B].value);
    ADD_VALUE(HYDROPONICS__STATE_TELEMETRY__TYPE__TANK_A, context->sensors.tank[CONFIG_TANK_A].value);
    ADD_VALUE(HYDROPONICS__STATE_TELEMETRY__TYPE__TANK_B, context->sensors.tank[CONFIG_TANK_B].value);
    context_unlock(context);

    ESP_ERROR_CHECK(state_push_telemetry(size, types, values));
    return ESP_OK;
}

static const mqtt_config_t config = {
        .handle_config = config_update,
        .handle_command = iot_handle_command,
        .handle_publish_telemetry = iot_handle_publish_telemetry,
};

void iot_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    TickType_t next = 0;
    while (true) {
        size_t len = 0;
        uint8_t *buf = xRingbufferReceive(ring, &len, portMAX_DELAY);
        if (buf == NULL) {
            continue;
        }
        op_type_t type = buf[0];
        if (type >= OP_MAX) {
            ESP_LOGE(TAG, "Unknown op type: %d", type);
            continue;
        }
        TickType_t now = xTaskGetTickCount();
        if (next >= now) {
            int to_wait_ms = pdTICKS_TO_MS(next - now);
            ESP_LOGI(TAG, "Throttling state publish by %d ms", to_wait_ms);
            safe_delay_ms(to_wait_ms);
        }
        switch (type) {
            case OP_TELEMETRY:
                ESP_ERROR_CHECK(mqtt_publish_event(&buf[1], len - 1));
                break;
            case OP_STATE:
                ESP_ERROR_CHECK(mqtt_publish_state(&buf[1], len - 1));
                break;
            default:
                break;
        }
        next = xTaskGetTickCount() + pdMS_TO_TICKS(PUBLISH_MIN_MS);
        vRingbufferReturnItem(ring, buf);
    }
}

esp_err_t iot_init(context_t *context) {
    ring = xRingbufferCreate(BUFFER_SIZE, RINGBUF_TYPE_NOSPLIT);
    CHECK_NO_MEM(ring);

    xTaskCreatePinnedToCore(iot_task, "iot", 3072, context, tskIDLE_PRIORITY + 5, NULL, tskNO_AFFINITY);
    ESP_ERROR_CHECK(mqtt_init(context, &config));
    return ESP_OK;
}

esp_err_t iot_publish(op_type_t type, Hydroponics__States *states) {
    ARG_CHECK(states != NULL, ERR_PARAM_NULL);

    size_t size = hydroponics__states__get_packed_size(states);
    if (size == 0) {
        ESP_LOGI(TAG, "Unable to publish an empty message.");
        return ESP_OK;
    }
    uint8_t *buf = NULL;
    if (xRingbufferSendAcquire(ring, (void **) &buf, 1 + size, pdMS_TO_TICKS(BUFFER_WAIT_MS)) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to acquire %d bytes from RingBuffer after %d ms", 1 + size, BUFFER_WAIT_MS);
        return ESP_ERR_TIMEOUT;
    }
    CHECK_NO_MEM(buf);
    *buf = (uint8_t) type;
    hydroponics__states__pack(states, &buf[1]);
    if (xRingbufferSendComplete(ring, buf) != pdTRUE) {
        ESP_LOGW(TAG, "Failed to send %d bytes to RingBuffer", size);
        return ESP_FAIL;
    };
    return ESP_OK;
}

esp_err_t iot_publish_telemetry(Hydroponics__States *states) {
    return iot_publish(OP_TELEMETRY, states);
}

esp_err_t iot_publish_state(Hydroponics__States *states) {
    return iot_publish(OP_STATE, states);
}