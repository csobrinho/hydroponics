/* Adapted from esp-idf/examples/common_components/protocol_examples_common/connect.c */
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "sdkconfig.h"

#include "context.h"
#include "error.h"

#define GOT_IPV4_BIT BIT(0)
#define GOT_IPV6_BIT BIT(1)

#define CONNECTED_BITS (GOT_IPV6_BIT | GOT_IPV4_BIT)

static const char *TAG = "wifi";

typedef struct {
    context_t *context;
    const char *ssid;
    const char *password;
} args_t;

static EventGroupHandle_t connect_event_group;
static esp_ip4_addr_t ip4_addr;
static esp_ip6_addr_t ip6_addr;
static esp_netif_t *netif = NULL;
static args_t args = {0};

static void on_got_ipv4(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ARG_UNUSED(arg);
    ARG_UNUSED(event_base);
    ARG_UNUSED(event_id);
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    memcpy(&ip4_addr, &event->ip_info.ip, sizeof(ip4_addr));
    xEventGroupSetBits(connect_event_group, GOT_IPV4_BIT);
}

static void on_got_ipv6(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ARG_UNUSED(arg);
    ARG_UNUSED(event_base);
    ARG_UNUSED(event_id);
    ip_event_got_ip6_t *event = (ip_event_got_ip6_t *) event_data;
    memcpy(&ip6_addr, &event->ip6_info.ip, sizeof(ip6_addr));
    xEventGroupSetBits(connect_event_group, GOT_IPV6_BIT);
}

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ARG_UNUSED(arg);
    ARG_UNUSED(event_base);
    ARG_UNUSED(event_id);
    ARG_UNUSED(event_data);
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    ESP_ERROR_CHECK(context_set_network_connected(args.context, false));
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}

static void on_wifi_connect(void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ARG_UNUSED(event_base);
    ARG_UNUSED(event_id);
    ARG_UNUSED(event_data);
    esp_netif_create_ip6_linklocal(esp_netif);

    ESP_ERROR_CHECK(context_set_network_connected(args.context, true));
}

static void wifi_start() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT()
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_WIFI_STA();

    netif = esp_netif_new(&netif_config);
    assert(netif);

    esp_netif_attach_wifi_station(netif);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &on_wifi_connect, netif));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ipv4, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &on_got_ipv6, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));

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
    ESP_LOGI(TAG, "Connecting to %s...", args.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static void wifi_stop(void) {
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &on_wifi_connect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ipv4));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &on_got_ipv6));
    esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return;
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(netif));
    esp_netif_destroy(netif);
    netif = NULL;
}

static esp_err_t wifi_connect(void) {
    if (connect_event_group != NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    connect_event_group = xEventGroupCreate();
    wifi_start();
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&wifi_stop));
    ESP_LOGI(TAG, "Waiting for IP");
    xEventGroupWaitBits(connect_event_group, CONNECTED_BITS, true, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to %s", args.ssid);
    ESP_LOGI(TAG, "  IPv4 address: "
            IPSTR, IP2STR(&ip4_addr));
    ESP_LOGI(TAG, "  IPv6 address: "
            IPV6STR, IPV62STR(ip6_addr));
    return ESP_OK;
}

esp_err_t wifi_disconnect(void) {
    if (connect_event_group == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    vEventGroupDelete(connect_event_group);
    connect_event_group = NULL;
    wifi_stop();
    ESP_LOGI(TAG, "Disconnected from %s", args.ssid);
    return ESP_OK;
}

static void wifi_task(void *arg) {
    ARG_UNUSED(arg);
    ESP_ERROR_CHECK(wifi_connect());
    vTaskDelete(NULL);
}

esp_err_t wifi_init(context_t *context, const char *ssid, const char *password) {
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);
    ARG_ERROR_CHECK(ssid != NULL, ERR_PARAM_NULL);
    args.context = context;
    args.ssid = ssid;
    args.password = password;
    xTaskCreatePinnedToCore(wifi_task, "wifi", 4096, NULL, 20, NULL, tskNO_AFFINITY);
    return ESP_OK;
}