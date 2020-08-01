#include <stdio.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "cJSON.h"

#include "esp_err.h"

#include "config.h"
#include "context.h"
#include "error.h"
#include "storage.h"
#include "tuya.h"
#include "tuya_io.h"
#include "utils.h"

typedef enum {
    TUYA_OP_CONFIG = 1,
    TUYA_OP_SET = 2,
} tuya_op_type_t;

typedef struct {
    tuya_op_type_t type;
    union {
        struct {
            const Hydroponics__Config *config;
        } config;
        struct {
            const Hydroponics__Output output;
            bool value;
        } set;
    };
} tuya_op_t;

static const char *TAG = "tuya_io";
//                                               dev_id          dev_id                    timestamp      id   value
static const char PAYLOAD_CONTROL[] = "{\"devId\":\"%s\",\"gwId\":\"%s\",\"uid\":\"\",\"t\":%lu,\"dps\":{\"%d\":%s}}";
static QueueHandle_t queue;
static const Hydroponics__Config *config;
static const tuya_connection_t UDP = {
        .type = TUYA_CONNECTION_UDP,
        .ip = "0.0.0.0",
        .timeout = {.tv_sec = 5},
};
static tuya_connection_t LOCAL = {
        .type = TUYA_CONNECTION_TCP,
        .ip = "10.0.0.9",
        .key = "cb94da2311895bbc",
        .timeout = {.tv_sec = 5},
};

static void tuya_config_callback(const Hydroponics__Config *new_config) {
    ESP_LOGI(TAG, "Applying config...");
    if (config == new_config) {
        return;
    }
    const tuya_op_t cmd = {.type = TUYA_OP_CONFIG, .config = {.config = new_config}};
    xQueueSend(queue, &cmd, portMAX_DELAY);
}

static esp_err_t tuya_io_udp_listen(void) {
    tuya_msg_t rx = {0};
    cJSON *root = NULL;
    char *key = NULL;

    esp_err_t ret = tuya_recv(&UDP, &rx);
    FAIL_IF(ret != ESP_OK, "Unable to listen for UDP, error: %d", ret);

    root = cJSON_Parse((const char *) rx.payload.data);
    FAIL_IF(root == NULL, "Unable to parse the UDP Json payload, error: %s", cJSON_GetErrorPtr());

    cJSON *elem = cJSON_GetObjectItem(root, "ip");
    FAIL_IF(elem == NULL, "Unable to parse the UDP Json payload, missing \"ip\"");
    strlcpy(LOCAL.ip, cJSON_GetStringValue(elem), sizeof(LOCAL.ip));

    elem = cJSON_GetObjectItem(root, "gwId");
    const char *gwid = cJSON_GetStringValue(elem);
    FAIL_IF(elem == NULL, "Unable to parse the UDP Json payload, missing \"ip\"");
    ESP_LOGI(TAG, "Found tuya device %s at %s", gwid, LOCAL.ip);

    char storage_key[128] = {0};
    snprintf(storage_key, sizeof(storage_key) - 1, "tuya_key_%s", gwid);
    size_t len = 0;
    ret = storage_get_string(storage_key, &key, &len);
    FAIL_IF(ret != ESP_OK, "Unable to fetch '%s' from storage, error: %d", storage_key, ret);
    FAIL_IF(len != sizeof(LOCAL.key), "'%s' should be exactly %zu chars", storage_key, sizeof(LOCAL.key));
    strlcpy((char *) LOCAL.key, key, sizeof(LOCAL.key));

    ESP_ERROR_CHECK(tuya_free(&rx));
    cJSON_Delete(root);
    SAFE_FREE(key);
    return ESP_OK;

    fail:
    ESP_ERROR_CHECK(tuya_free(&rx));
    if (root != NULL) {
        cJSON_Delete(root);
    }
    SAFE_FREE(key);
    return ESP_FAIL;
}

static const Hydroponics__HardwareId *tuya_io_find_hardware_io(Hydroponics__Output output) {
    if (config == NULL) {
        return NULL;
    }
    for (int i = 0; i < config->n_hardware_id; ++i) {
        if (config->hardware_id[i]->output == output) {
            return config->hardware_id[i];
        }
    }
    return NULL;
}

static esp_err_t tuya_io_control(int sequence, const Hydroponics__HardwareId *hid, bool value) {
    ARG_CHECK(hid != NULL, ERR_PARAM_NULL);
    ARG_CHECK(hid->dev_id != NULL, ERR_PARAM_NULL);
    uint8_t payload[256] = {0};
    int payload_len = snprintf((char *) payload, sizeof(payload) - 1, PAYLOAD_CONTROL, hid->dev_id, hid->dev_id,
                               time(NULL), hid->dps_id, value ? "true" : "false");
    ARG_CHECK(payload_len > 0, "tuya_control payload, error: %d", payload_len);

    tuya_msg_t rx = {0};
    // TODO: LOCAL -> Get the right connection.
    ESP_LOGI(TAG, "Setting tuya {\"%d\":%s}", hid->dps_id, value ? "true" : "false");
    esp_err_t ret = tuya_send(&LOCAL, sequence, TUYA_COMMAND_CONTROL, payload, payload_len, &rx);
    FAIL_IF(ret != ESP_OK, "tuya_send, error: %d", ret);
    FAIL_IF(sequence != rx.sequence, "tuya_send sequence mismatch, %d != %d", sequence, rx.sequence);
    ESP_ERROR_CHECK(tuya_free(&rx));
    return ESP_OK;

    fail:
    ESP_ERROR_CHECK(tuya_free(&rx));
    return ESP_FAIL;
}

static void tuya_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    // Wait until the config is ready.
    xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_CONFIG, pdFALSE, pdTRUE, portMAX_DELAY);
    ESP_ERROR_CHECK(context_get_config(context, &config));

    int sequence = 0;
    bool reload_udp = true;
    while (true) {
        // Wait until the network and time are set.
        xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_NETWORK | CONTEXT_EVENT_TIME, pdFALSE, pdTRUE,
                            portMAX_DELAY);
        if (reload_udp) {
            esp_err_t ret = tuya_io_udp_listen();
            if (ret == ESP_OK) {
                reload_udp = false;
            } else {
                vTaskDelay(pdMS_TO_TICKS(20000));
                continue;
            }
        }
        tuya_op_t op = {0};
        if (xQueueReceive(queue, &op, portMAX_DELAY) == pdTRUE) {
            switch (op.type) {
                case TUYA_OP_CONFIG: { // Re-load config.
                    if (config != NULL) {
                        hydroponics__config__free_unpacked((Hydroponics__Config *) config, NULL);
                    }
                    config = op.config.config;
                    // TODO(sobrinho): Apply a default state to all ports?
                    break;
                }
                case TUYA_OP_SET: { // Change the status of a Tuya IO.
                    const Hydroponics__HardwareId *hid = tuya_io_find_hardware_io(op.set.output);
                    FAIL_IF(hid == NULL, "Could not find the HardwareId for output %d", op.set.output);
                    esp_err_t ret = tuya_io_control(sequence++, hid, op.set.value);
                    FAIL_IF(ret != ESP_OK, "Could not set tuya {\"%d\":%s}, error: %d", hid->dps_id,
                            op.set.value ? "true" : "false", ret);
                    break;
                    fail:
                    reload_udp = true;
                    break;
                }
                default:
                    ESP_LOGE(TAG, "Unsupported op.type: %d", op.type);
                    break;
            }
        }
    }
}

esp_err_t tuya_io_init(context_t *context) {
    queue = xQueueCreate(10, sizeof(tuya_command_t));
    CHECK_NO_MEM(queue);

    config_register(tuya_config_callback);
    xTaskCreatePinnedToCore(tuya_task, "tuya_io", 4096, context, tskIDLE_PRIORITY + 10, NULL, tskNO_AFFINITY);
    return ESP_OK;
}

esp_err_t tuya_io_set(Hydroponics__Output output, bool value) {
    const tuya_op_t cmd = {.type = TUYA_OP_SET, .set = {.output = output, .value = value}};
    return xQueueSend(queue, &cmd, portMAX_DELAY) == pdPASS ? ESP_OK : ESP_FAIL;
}
