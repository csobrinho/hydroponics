#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <iotc.h>
#include <iotc_jwt.h>

#include "esp_err.h"
#include "esp_log.h"
#include "jsmn.h"

#include "context.h"
#include "error.h"
#include "mqtt.h"

#define DEVICE_PATH "projects/%s/locations/%s/registries/%s/devices/%s"
#define SUBSCRIBE_TOPIC_COMMAND "/devices/%s/commands/#"
#define SUBSCRIBE_TOPIC_CONFIG "/devices/%s/config"
#define PUBLISH_TOPIC_EVENT "/devices/%s/events"
#define PUBLISH_TOPIC_STATE "/devices/%s/state"

static const char *TAG = "mqtt";
static char *subscribe_topic_command;
static char *subscribe_topic_config;
static iotc_mqtt_qos_t iotc_example_qos = IOTC_MQTT_QOS_AT_LEAST_ONCE;
static iotc_timed_task_handle_t delayed_publish_task = IOTC_INVALID_TIMED_TASK_HANDLE;
static iotc_context_handle_t iotc_context = IOTC_INVALID_CONTEXT_HANDLE;
static context_t *context;
static const mqtt_config_t *mqtt_config;

extern const uint8_t EC_PV_KEY_START[] asm("_binary_ec_private_pem_start");
extern const uint8_t EC_PV_KEY_END[] asm("_binary_ec_private_pem_end");

static void publish_telemetry_event(iotc_context_handle_t context_handle, iotc_timed_task_handle_t timed_task,
                                    void *user_data) {
    ARG_UNUSED(timed_task);
    ARG_UNUSED(user_data);

    char *publish_topic = NULL;
    asprintf(&publish_topic, PUBLISH_TOPIC_EVENT, CONFIG_GIOT_DEVICE_ID);
    char *publish_message = NULL;
    ESP_ERROR_CHECK(mqtt_config->handle_publish(context, &publish_message));
    ESP_LOGI(TAG, "publishing msg '%s' to topic: '%s'", publish_message, publish_topic);

    iotc_publish(context_handle, publish_topic, publish_message, iotc_example_qos, /* callback= */ NULL,
            /* user_data= */ NULL);
    free(publish_topic);
    free(publish_message);
}

static void iotc_mqttlogic_subscribe_callback(iotc_context_handle_t in_context_handle, iotc_sub_call_type_t call_type,
                                              const iotc_sub_call_params_t *const params, iotc_state_t state,
                                              void *user_data) {
    ARG_UNUSED(in_context_handle);
    ARG_UNUSED(call_type);
    ARG_UNUSED(state);
    ARG_UNUSED(user_data);
    if (params != NULL && params->message.topic != NULL) {
        ESP_LOGI(TAG, "Subscription Topic: %s", params->message.topic);
        char *sub_message = (char *) malloc(params->message.temporary_payload_data_length + 1);
        if (sub_message == NULL) {
            ESP_LOGE(TAG, "Failed to allocate memory");
            return;
        }
        memcpy(sub_message, params->message.temporary_payload_data, params->message.temporary_payload_data_length);
        sub_message[params->message.temporary_payload_data_length] = '\0';
        ESP_LOGI(TAG, "Message Payload: %s", sub_message);

        if (strcmp(subscribe_topic_config, params->message.topic) == 0) {
            ESP_ERROR_CHECK(mqtt_config->handle_config(context, sub_message));
        } else if (strcmp(subscribe_topic_command, params->message.topic) == 0) {
            ESP_ERROR_CHECK(mqtt_config->handle_command(context, sub_message));
        }
        free(sub_message);
    }
}

static void on_connection_state_changed(iotc_context_handle_t in_context_handle, void *data, iotc_state_t state) {
    iotc_connection_data_t *conn_data = (iotc_connection_data_t *) data;

    switch (conn_data->connection_state) {
        /* IOTC_CONNECTION_STATE_OPENED means that the connection has been established and the IoTC Client is ready to
         * send/recv messages. */
        case IOTC_CONNECTION_STATE_OPENED:
            ESP_LOGI(TAG, "connected");

            /* Publish immediately upon connect. 'publish_function' is defined in this example file and invokes the IoTC
             * API to publish a message. */
            asprintf(&subscribe_topic_command, SUBSCRIBE_TOPIC_COMMAND, CONFIG_GIOT_DEVICE_ID);
            ESP_LOGI(TAG, "subscribed to topic: '%s'", subscribe_topic_command);
            iotc_subscribe(in_context_handle, subscribe_topic_command, IOTC_MQTT_QOS_AT_LEAST_ONCE,
                           &iotc_mqttlogic_subscribe_callback, /* user_data= */ NULL);

            asprintf(&subscribe_topic_config, SUBSCRIBE_TOPIC_CONFIG, CONFIG_GIOT_DEVICE_ID);
            ESP_LOGI(TAG, "subscribed to topic: '%s'", subscribe_topic_config);
            iotc_subscribe(in_context_handle, subscribe_topic_config, IOTC_MQTT_QOS_AT_LEAST_ONCE,
                           &iotc_mqttlogic_subscribe_callback, /* user_data= */ NULL);

            /* Create a timed task to publish every 10 seconds. */
            delayed_publish_task = iotc_schedule_timed_task(in_context_handle, publish_telemetry_event, 10, 15, NULL);
            break;

            /* IOTC_CONNECTION_STATE_OPEN_FAILED is set when there was a problem
               when establishing a connection to the server. The reason for the error
               is contained in the 'state' variable. Here we log the error state and
               exit out of the application. */

            /* Publish immediately upon connect. 'publish_function' is defined
               in this example file and invokes the IoTC API to publish a
               message. */
        case IOTC_CONNECTION_STATE_OPEN_FAILED:
            ESP_LOGE(TAG, "Connection has failed reason %d", state);

            /* Exit it out of the application by stopping the event loop. */
            iotc_events_stop();
            break;

            /* IOTC_CONNECTION_STATE_CLOSED is set when the IoTC Client has been disconnected. The disconnection may
             * have been caused by some external issue, or user may have requested a disconnection. In order to
             * distinguish between those two situation it is advised to check the state variable value. If the
             * state == IOTC_STATE_OK then the application has requested a disconnection via 'iotc_shutdown_connection'.
             * If the state != IOTC_STATE_OK then the connection has been closed from one side. */
        case IOTC_CONNECTION_STATE_CLOSED:
            free(subscribe_topic_command);
            free(subscribe_topic_config);
            /* When the connection is closed it's better to cancel some of previously registered activities. Using
             * cancel function on handler will remove the handler from the timed queue which prevents the registered
             * handle to be called when there is no connection. */
            if (IOTC_INVALID_TIMED_TASK_HANDLE != delayed_publish_task) {
                iotc_cancel_timed_task(delayed_publish_task);
                delayed_publish_task = IOTC_INVALID_TIMED_TASK_HANDLE;
            }

            if (state == IOTC_STATE_OK) {
                /* The connection has been closed intentionally. Therefore, stop the event processing loop as there's
                 * nothing left to do in this example. */
                iotc_events_stop();
            } else {
                ESP_LOGI(TAG, "connection closed - reason %d!", state);
                /* The disconnection was unforeseen.  Try reconnect to the server with previously set configuration,
                 * which has been provided to this callback in the conn_data structure. */

                // Wait until network is connected and time is updated from the network.
                xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_NETWORK | CONTEXT_EVENT_TIME, pdFALSE, pdTRUE,
                                    portMAX_DELAY);

                iotc_connect(in_context_handle, conn_data->username, conn_data->password, conn_data->client_id,
                             conn_data->connection_timeout, conn_data->keepalive_timeout,
                             &on_connection_state_changed);
            }
            break;

        default:
            ESP_LOGW(TAG, "unsupported connection state: %d", conn_data->connection_state);
            break;
    }
}

static void mqtt_task(void *args) {
    ARG_UNUSED(args);

    // Wait until network is connected and time is updated from the network.
    xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_NETWORK | CONTEXT_EVENT_TIME, pdFALSE, pdTRUE,
                        portMAX_DELAY);

    ESP_LOGI(TAG, "Connecting to Google IoT Core");

    /* Format the key type descriptors so the client understands which type of key is being represented. In this case,
     * a PEM encoded byte array of a ES256 key. */
    iotc_crypto_key_data_t iotc_connect_private_key_data;
    iotc_connect_private_key_data.crypto_key_signature_algorithm = IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256;
    iotc_connect_private_key_data.crypto_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM;
    iotc_connect_private_key_data.crypto_key_union.key_pem.key = (char *) EC_PV_KEY_START;

    /* Initialize the iotc library and create a context to use to connect to the GCP IoT Core Service. */
    const iotc_state_t error_init = iotc_initialize();

    if (IOTC_STATE_OK != error_init) {
        ESP_LOGW(TAG, "failed to initialize, error: %d", error_init);
        vTaskDelete(NULL);
    }

    /* Create a connection context. A context represents a Connection on a single socket, and can be used to publish
     * and subscribe to numerous topics. */
    iotc_context = iotc_create_context();
    if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context) {
        ESP_LOGW(TAG, "failed to create context, error: %d", -iotc_context);
        vTaskDelete(NULL);
    }

    /* Queue a connection request to be completed asynchronously. The 'on_connection_state_changed' parameter is the
     * name of the callback function after the connection request completes, and its implementation should handle both
     * successful connections and unsuccessful connections as well as disconnections. */
    const uint16_t connection_timeout = 0;
    const uint16_t keepalive_timeout = 20;

    /* Generate the client authentication JWT, which will serve as the MQTT password. */
    char jwt[IOTC_JWT_SIZE] = {0};
    size_t bytes_written = 0;
    iotc_state_t state = iotc_create_iotcore_jwt(CONFIG_GIOT_PROJECT_ID, /* expiration_period_sec= */ 3600,
                                                 &iotc_connect_private_key_data, jwt, IOTC_JWT_SIZE, &bytes_written);

    if (IOTC_STATE_OK != state) {
        ESP_LOGE(TAG, "iotc_create_iotcore_jwt returned with error: %ul", state);
        vTaskDelete(NULL);
    }

    char *device_path = NULL;
    asprintf(&device_path, DEVICE_PATH, CONFIG_GIOT_PROJECT_ID, CONFIG_GIOT_LOCATION, CONFIG_GIOT_REGISTRY_ID,
             CONFIG_GIOT_DEVICE_ID);
    iotc_connect(iotc_context, NULL, jwt, device_path, connection_timeout, keepalive_timeout,
                 &on_connection_state_changed);
    free(device_path);

    /* The IoTC Client was designed to be able to run on single threaded devices. As such it does not have its own event
     * loop thread. Instead you must regularly call the function iotc_events_process_blocking() to process connection
     * requests, and for the client to regularly check the sockets for incoming data. This implementation has the loop
     * operate endlessly. The loop will stop after closing the connection, using iotc_shutdown_connection() as defined
     * in on_connection_state_change logic, and exit the event handler handler by calling iotc_events_stop();
     */
    iotc_events_process_blocking();

    iotc_delete_context(iotc_context);
    iotc_shutdown();

    vTaskDelete(NULL);
}

esp_err_t mqtt_init(context_t *ctx, const mqtt_config_t *config) {
    ARG_CHECK(ctx != NULL, ERR_PARAM_NULL);

    context = ctx;
    mqtt_config = config;
    xTaskCreatePinnedToCore(mqtt_task, "mqtt", 8192, NULL, 5, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
