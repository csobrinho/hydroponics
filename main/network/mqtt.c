#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <iotc.h>
#include <iotc_helpers.h>
#include <iotc_jwt.h>
#include <iotc_types.h>

#include "esp_err.h"
#include "esp_log.h"

#include "context.h"
#include "error.h"
#include "mqtt.h"
#include "utils.h"

#define DEVICE_PATH "projects/%s/locations/%s/registries/%s/devices/%s"
#define SUBSCRIBE_TOPIC_COMMAND "/devices/%s/commands/#"
#define SUBSCRIBE_TOPIC_CONFIG "/devices/%s/config"
#define PUBLISH_TOPIC_EVENT "/devices/%s/events"
#define PUBLISH_TOPIC_STATE "/devices/%s/state"
#define TASK_REPEAT_SEC 30
#define TASK_REPEAT_FOREVER 1

static const char *TAG = "mqtt";

static context_t *context;
static const mqtt_config_t *mqtt_config;

static const iotc_mqtt_qos_t mqtt_qos = IOTC_MQTT_QOS_AT_LEAST_ONCE;
static iotc_timed_task_handle_t delayed_publish_task = IOTC_INVALID_TIMED_TASK_HANDLE;
static iotc_context_handle_t iotc_context = IOTC_INVALID_CONTEXT_HANDLE;
static char *subscribe_topic_command;
static char *subscribe_topic_config;
static char *publish_topic_event;
static char *publish_topic_state;

static char jwt[IOTC_JWT_SIZE] = {0};
static const uint32_t jwt_expiration_sec = 3600 * 24; // 24 hours.
extern const uint8_t EC_PV_KEY_START[] asm("_binary_ec_private_pem_start");

/* Format the key type descriptors so the client understands which type of key is being represented. In this case,
 * a PEM encoded byte array of a ES256 key. */
static const iotc_crypto_key_data_t PRIVATE_KEY_DATA = {
        .crypto_key_signature_algorithm = IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256,
        .crypto_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM,
        .crypto_key_union.key_pem.key = (char *) EC_PV_KEY_START,
};

static void mqtt_dispatch_connected(bool connected) {
    ESP_ERROR_CHECK(context_set_iot_connected(context, connected));
}

esp_err_t mqtt_publish_event(uint8_t *data, size_t size) {
    /* Wait until IoT is connected. */
    xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_IOT, pdFALSE, pdTRUE, portMAX_DELAY);
    iotc_state_t err = iotc_publish_data(iotc_context, publish_topic_event, data, size, mqtt_qos,
            /* callback= */ NULL, /* user_data= */ NULL);
    if (err == IOTC_STATE_OK) {
        return ESP_OK;
    }
    ESP_LOGW(TAG, "Failed to publish event, error: %d", err);
    return ESP_FAIL;
}

esp_err_t mqtt_publish_state(uint8_t *data, size_t size) {
    /* Wait until IoT is connected. */
    xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_IOT, pdFALSE, pdTRUE, portMAX_DELAY);
    iotc_state_t err = iotc_publish_data(iotc_context, publish_topic_state, data, size, mqtt_qos,
            /* callback= */ NULL, /* user_data= */ NULL);
    if (err == IOTC_STATE_OK) {
        return ESP_OK;
    }
    ESP_LOGW(TAG, "Failed to publish state, error: %d", err);
    return ESP_FAIL;
}

static void mqtt_publish_telemetry_event(iotc_context_handle_t context_handle, iotc_timed_task_handle_t timed_task,
                                         void *user_data) {
    ARG_UNUSED(timed_task);
    ARG_UNUSED(user_data);

    /* Publish the telemetry. */
    uint8_t *msg = NULL;
    size_t size = 0;
    ESP_ERROR_CHECK(mqtt_config->handle_publish_telemetry(context, &msg, &size));
    if (msg != NULL) {
        ESP_LOGI(TAG, "Publishing topic: '%s' with message:\n%*s", publish_topic_event, size, msg);
        iotc_publish_data(context_handle, publish_topic_event, msg, size, mqtt_qos, /* callback= */ NULL,
                /* user_data= */ NULL);
        SAFE_FREE(msg);
    }

    /* Now try to publish the state if available. */
    ESP_ERROR_CHECK(mqtt_config->handle_publish_state(context, &msg, &size));
    if (msg != NULL) {
        ESP_LOGI(TAG, "Publishing topic: '%s' with message:\n%*s", publish_topic_state, size, msg);
        iotc_publish_data(context_handle, publish_topic_state, msg, size, mqtt_qos, /* callback= */ NULL,
                /* user_data= */ NULL);
        SAFE_FREE(msg);
    }
}

/* Generate the client authentication JWT, which will serve as the MQTT password. */
static void mqtt_create_jwt_token(void) {
    size_t bytes_written = 0;
    iotc_state_t err = iotc_create_iotcore_jwt(CONFIG_GIOT_PROJECT_ID, jwt_expiration_sec, &PRIVATE_KEY_DATA, jwt,
                                               IOTC_JWT_SIZE, &bytes_written);
    if (err != IOTC_STATE_OK) {
        ESP_LOGE(TAG, "Failed to create a jwt token, error: %d", err);
        ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);
    }
    time_t now;
    time(&now);
    struct tm t = {0};
    localtime_r(&now, &t);
    char buf[64] = {0};
    strftime(buf, sizeof(buf), "%F %R", &t);
    ESP_LOGI(TAG, "Jwt Token created at %s", buf);
}

static void mqtt_subscribe_callback(iotc_context_handle_t in_context_handle, iotc_sub_call_type_t call_type,
                                    const iotc_sub_call_params_t *const params, iotc_state_t state, void *user_data) {
    ARG_UNUSED(in_context_handle);
    ARG_UNUSED(user_data);
    if (params == NULL) {
        return;
    }
    switch (call_type) {
        case IOTC_SUB_CALL_SUBACK: {
            iotc_mqtt_suback_status_t status = params->suback.suback_status;
            const char *ack = status == IOTC_MQTT_QOS_0_GRANTED
                              ? "QOS_0" : status == IOTC_MQTT_QOS_1_GRANTED
                                          ? "QOS_1" : status == IOTC_MQTT_QOS_2_GRANTED
                                                      ? "QOS_2" : status == IOTC_MQTT_SUBACK_FAILED
                                                                  ? "Failed" : "Unknown";
            ESP_LOGI(TAG, "Subscription [state: %2d]: SUBACK %s %s", state, ack, params->suback.topic);
            break;
        }
        case IOTC_SUB_CALL_MESSAGE: {
            ESP_LOGI(TAG, "Subscription [state: %2d]: MSG %s", state, params->message.topic);
            const uint8_t *payload = params->message.temporary_payload_data;
            const size_t payload_size = params->message.temporary_payload_data_length;
            if (payload == NULL) {
                ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
                return;
            }
            if (strcmp(subscribe_topic_config, params->message.topic) == 0) {
                ESP_LOGI(TAG, "Config payload: %d bytes", params->message.temporary_payload_data_length);
                ESP_ERROR_CHECK(mqtt_config->handle_config(context, payload, payload_size));
            } else if (strcmp(subscribe_topic_command, params->message.topic) == 0) {
                ESP_LOGI(TAG, "Message payload: %*s", payload_size, payload);
                ESP_ERROR_CHECK(mqtt_config->handle_command(context, payload, payload_size));
            }
            break;
        }
        default:
            ESP_LOGI(TAG, "Subscription Topic [type: MSG / state: %d]: message %s", state,
                     params->message.topic);
            break;
    }
}

static void mqtt_connection_state_changed(iotc_context_handle_t in_context_handle, void *data, iotc_state_t state) {
    iotc_connection_data_t *conn_data = (iotc_connection_data_t *) data;

    switch (conn_data->connection_state) {
        /* IOTC_CONNECTION_STATE_OPENED means that the connection has been established and the IoTC Client is ready to
         * send/recv messages. */
        case IOTC_CONNECTION_STATE_OPENED:
            ESP_LOGI(TAG, "Connected state: %d", state);

            /* Publish immediately upon connect. 'mqtt_publish_telemetry_event' is defined above and invokes the IoTC
             * API to publish a message. */
            iotc_state_t err = iotc_subscribe(in_context_handle, subscribe_topic_command, mqtt_qos,
                                              &mqtt_subscribe_callback, /* user_data= */ NULL);
            ESP_LOGI(TAG, "Subscribed to topic, error: %d: '%s'", err, subscribe_topic_command);

            err = iotc_subscribe(in_context_handle, subscribe_topic_config, mqtt_qos,
                                 &mqtt_subscribe_callback, /* user_data= */ NULL);
            ESP_LOGI(TAG, "Subscribed to topic, error: %d: '%s'", err, subscribe_topic_config);

            /* Create a timed task to publish every 'x' seconds. */
            delayed_publish_task = iotc_schedule_timed_task(in_context_handle, mqtt_publish_telemetry_event,
                                                            TASK_REPEAT_SEC, TASK_REPEAT_FOREVER, NULL);
            /* Force publish the first telemetry or else it will only send the telemetry in TASK_REPEAT_SEC. */
            mqtt_publish_telemetry_event(in_context_handle, delayed_publish_task, NULL);
            mqtt_dispatch_connected(true);
            break;

            /* IOTC_CONNECTION_STATE_OPEN_FAILED is set when there was a problem when establishing a connection to the
             * server. The reason for the error is contained in the 'state' variable. */
        case IOTC_CONNECTION_STATE_OPEN_FAILED:
            ESP_LOGW(TAG, "Connection error: OPEN_FAILED state: %d", state);
            mqtt_dispatch_connected(false);
            /* Shuts down the event loop and potentially try again. */
            iotc_events_stop();
            break;

            /* IOTC_CONNECTION_STATE_CLOSED is set when the IoTC Client has been disconnected. The disconnection may
             * have been caused by some external issue, or user may have requested a disconnection. In order to
             * distinguish between those two situation it is advised to check the state variable value. If the
             * state == IOTC_STATE_OK then the application has requested a disconnection via 'iotc_shutdown_connection'.
             * If the state != IOTC_STATE_OK then the connection has been closed from one side. */
        case IOTC_CONNECTION_STATE_CLOSED: {
            ESP_LOGW(TAG, "Connection error: CLOSED state: %d", state);
            mqtt_dispatch_connected(false);

            /* When the connection is closed it's better to cancel some of previously registered activities. Using
             * cancel function on handler will remove the handler from the timed queue which prevents the
             * registered handle to be called when there is no connection. */
            if (delayed_publish_task != IOTC_INVALID_TIMED_TASK_HANDLE) {
                iotc_cancel_timed_task(delayed_publish_task);
                delayed_publish_task = IOTC_INVALID_TIMED_TASK_HANDLE;
            }

            if (state == IOTC_STATE_OK) {
                /* The connection has been closed intentionally. Therefore, stop the event processing loop as
                 * there's nothing left to do right now. */
                iotc_events_stop();
                return;
            }
            /* The disconnection was unforeseen. Try to reconnect to the server with previously set configuration and
             * an updated JWT token. */
            mqtt_create_jwt_token();
            iotc_connect(iotc_context, conn_data->username, jwt, conn_data->client_id, conn_data->connection_timeout,
                         conn_data->keepalive_timeout, &mqtt_connection_state_changed);
            break;
        }
        default:
            ESP_LOGW(TAG, "Unsupported connection state: %d", conn_data->connection_state);
            break;
    }
}

static void mqtt_task(void *args) {
    ARG_UNUSED(args);
    while (true) {
        /* Wait until network is connected and time is updated from the network. */
        ESP_LOGI(TAG, "Waiting for network and time to be available.");
        xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_NETWORK | CONTEXT_EVENT_TIME, pdFALSE, pdTRUE,
                            portMAX_DELAY);

        /* Let's wait until the network stabilizes a bit. */
        vTaskDelay(pdMS_TO_TICKS(1000));

        /* Initialize the iotc library and create a context to use to connect to the GCP IoT Core Service. */
        iotc_state_t err = iotc_initialize();
        if (err != IOTC_STATE_OK) {
            ESP_LOGE(TAG, "Failed to initialize, error: %d", err);
            ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);
        }

        /* Create a connection context. A context represents a Connection on a single socket, and can be used to publish
         * and subscribe to numerous topics. */
        iotc_context = iotc_create_context();
        if (iotc_context <= IOTC_INVALID_CONTEXT_HANDLE) {
            ESP_LOGE(TAG, "Failed to create context, error: %d", -iotc_context);
            iotc_context = IOTC_INVALID_CONTEXT_HANDLE;
            ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);
        }

        ESP_LOGI(TAG, "Connecting to Google IoT Core");
        mqtt_create_jwt_token();

        char *device_path = NULL;
        asprintf(&device_path, DEVICE_PATH, CONFIG_GIOT_PROJECT_ID, CONFIG_GIOT_LOCATION, CONFIG_GIOT_REGISTRY_ID,
                 CONFIG_GIOT_DEVICE_ID);
        /* Queue a connection request to be completed asynchronously. The 'mqtt_connection_state_changed' parameter is the
         * name of the callback function after the connection request completes, and its implementation should handle both
         * successful connections and unsuccessful connections as well as disconnections. */
        const uint16_t connection_timeout = 0;
        const uint16_t keepalive_timeout = 20;
        err = iotc_connect(iotc_context, NULL, jwt, device_path, connection_timeout, keepalive_timeout,
                           &mqtt_connection_state_changed);
        if (err != IOTC_STATE_OK) {
            ESP_LOGE(TAG, "iotc_connect returned error: %d", err);
            ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);
        }
        SAFE_FREE(device_path);

        /* The IoTC Client was designed to be able to run on single threaded devices. As such it does not have its own
         * event loop thread. Instead you must regularly call the function iotc_events_process_blocking() to process
         * connection requests, and for the client to regularly check the sockets for incoming data. This implementation
         * has the loop operate endlessly. The loop will stop after closing the connection, using
         * iotc_shutdown_connection() as defined in on_connection_state_change logic, and exit the event handler by
         * calling iotc_events_stop().
         */
        iotc_events_process_blocking();
        ESP_LOGD(TAG, "iotc_events_process_blocking returned. Cleaning up and restarting...");
        mqtt_dispatch_connected(false);

        iotc_delete_context(iotc_context);
        iotc_context = IOTC_INVALID_CONTEXT_HANDLE;

        iotc_shutdown();

        /* Notify the system that there might be an issue with the network. */
        context_set_network_error(context, true);

        ESP_LOGW(TAG, "Task restarting");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

esp_err_t mqtt_init(context_t *ctx, const mqtt_config_t *config) {
    ARG_CHECK(ctx != NULL, ERR_PARAM_NULL);
    ARG_CHECK(config != NULL, ERR_PARAM_NULL);

    context = ctx;
    mqtt_config = config;
    mqtt_dispatch_connected(false);

    asprintf(&subscribe_topic_command, SUBSCRIBE_TOPIC_COMMAND, CONFIG_GIOT_DEVICE_ID);
    asprintf(&subscribe_topic_config, SUBSCRIBE_TOPIC_CONFIG, CONFIG_GIOT_DEVICE_ID);
    asprintf(&publish_topic_event, PUBLISH_TOPIC_EVENT, CONFIG_GIOT_DEVICE_ID);
    asprintf(&publish_topic_state, PUBLISH_TOPIC_STATE, CONFIG_GIOT_DEVICE_ID);

    xTaskCreatePinnedToCore(mqtt_task, "mqtt", 5120, NULL, tskIDLE_PRIORITY + 5, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
