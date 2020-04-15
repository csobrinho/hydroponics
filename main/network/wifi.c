/* Adapted from esp-idf/examples/common_components/protocol_examples_common/connect.c */
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"

#include "context.h"
#include "error.h"

#define GOT_IPV4_BIT BIT(0)
#define CONNECTED_BIT GOT_IPV4_BIT

static const char *TAG = "nwifi";

typedef struct {
    context_t *context;
    const char *ssid;
    const char *password;
} args_t;

static EventGroupHandle_t wifi_event_group;
static esp_ip4_addr_t ip4_addr;
static args_t args = {0};

static void wifi_wait_and_notify(void) {
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to %s", args.ssid);
    ESP_LOGI(TAG, "  IPv4 address: " IPSTR, IP2STR(&ip4_addr));
    ESP_ERROR_CHECK(context_set_network_connected(args.context, true));
}

static void wifi_connect(void) {
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ARG_UNUSED(arg);
    ARG_UNUSED(event_data);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected from %s", args.ssid);
        wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        memcpy(&ip4_addr, &event->ip_info.ip, sizeof(ip4_addr));
        xEventGroupSetBits(wifi_event_group, GOT_IPV4_BIT);
    }
}

static esp_err_t initialize(void) {
    ESP_ERROR_CHECK(context_set_network_connected(args.context, false));
    static bool initialized = false;
    if (initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing wifi...");
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT()
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_start());
    initialized = true;
    return ESP_OK;
}

static void wifi_join(void) {
    ESP_ERROR_CHECK(initialize());
    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = CONFIG_ESP_WIFI_SSID,
                    .password = CONFIG_ESP_WIFI_PASSWORD,
            }
    };
    if (args.ssid != NULL && args.password != NULL) {
        strlcpy((char *) wifi_config.sta.ssid, args.ssid, sizeof(wifi_config.sta.ssid));
        strlcpy((char *) wifi_config.sta.password, args.password, sizeof(wifi_config.sta.password));
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    ESP_LOGI(TAG, "Connecting to %s...", args.ssid);
    wifi_connect();
    wifi_wait_and_notify();
}

static void wifi_reconnect(void) {
    ESP_LOGI(TAG, "Reconnecting to %s...", args.ssid);
    ESP_ERROR_CHECK(context_set_network_connected(args.context, false));

    // Forcing a disconnect will trigger a reconnect from the event handler.
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
    ESP_ERROR_CHECK(context_set_network_connected(args.context, true));
}

static void wifi_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    while (true) {
        wifi_join();
        while (true) {
            // Wait until network error is dispatched.
            xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_NETWORK_ERROR, pdTRUE, pdTRUE, portMAX_DELAY);
            wifi_reconnect();
        }
    }
}

esp_err_t wifi_init(context_t *context, const char *ssid, const char *password) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    ARG_CHECK(ssid != NULL, ERR_PARAM_NULL);
    args.context = context;
    args.ssid = ssid;
    args.password = password;

    xTaskCreatePinnedToCore(wifi_task, "nwifi", 4096, context, configMAX_PRIORITIES - 7, NULL, tskNO_AFFINITY);
    return ESP_OK;
}