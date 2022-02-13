/* Adapted from esp-idf/blob/master/examples/wifi/getting_started/station/main/station_example_main.c */
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

static const char *const TAG = "nwifi";

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

static const char *reason_str(uint8_t reason) {
    switch (reason) {
        case WIFI_REASON_UNSPECIFIED:
            return "UNSPECIFIED";
        case WIFI_REASON_AUTH_EXPIRE:
            return "AUTH_EXPIRE";
        case WIFI_REASON_AUTH_LEAVE:
            return "AUTH_LEAVE";
        case WIFI_REASON_ASSOC_EXPIRE:
            return "ASSOC_EXPIRE";
        case WIFI_REASON_ASSOC_TOOMANY:
            return "ASSOC_TOOMANY";
        case WIFI_REASON_NOT_AUTHED:
            return "NOT_AUTHED";
        case WIFI_REASON_NOT_ASSOCED:
            return "NOT_ASSOCED";
        case WIFI_REASON_ASSOC_LEAVE:
            return "ASSOC_LEAVE";
        case WIFI_REASON_ASSOC_NOT_AUTHED:
            return "ASSOC_NOT_AUTHED";
        case WIFI_REASON_DISASSOC_PWRCAP_BAD:
            return "DISASSOC_PWRCAP_BAD";
        case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
            return "DISASSOC_SUPCHAN_BAD";
        case WIFI_REASON_BSS_TRANSITION_DISASSOC:
            return "BSS_TRANSITION_DISASSOC";
        case WIFI_REASON_IE_INVALID:
            return "IE_INVALID";
        case WIFI_REASON_MIC_FAILURE:
            return "MIC_FAILURE";
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
            return "4WAY_HANDSHAKE_TIMEOUT";
        case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
            return "GROUP_KEY_UPDATE_TIMEOUT";
        case WIFI_REASON_IE_IN_4WAY_DIFFERS:
            return "IE_IN_4WAY_DIFFERS";
        case WIFI_REASON_GROUP_CIPHER_INVALID:
            return "GROUP_CIPHER_INVALID";
        case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
            return "PAIRWISE_CIPHER_INVALID";
        case WIFI_REASON_AKMP_INVALID:
            return "AKMP_INVALID";
        case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
            return "UNSUPP_RSN_IE_VERSION";
        case WIFI_REASON_INVALID_RSN_IE_CAP:
            return "INVALID_RSN_IE_CAP";
        case WIFI_REASON_802_1X_AUTH_FAILED:
            return "802_1X_AUTH_FAILED";
        case WIFI_REASON_CIPHER_SUITE_REJECTED:
            return "CIPHER_SUITE_REJECTED";
        case WIFI_REASON_INVALID_PMKID:
            return "INVALID_PMKID";
        case WIFI_REASON_BEACON_TIMEOUT:
            return "BEACON_TIMEOUT";
        case WIFI_REASON_NO_AP_FOUND:
            return "NO_AP_FOUND";
        case WIFI_REASON_AUTH_FAIL:
            return "AUTH_FAIL";
        case WIFI_REASON_ASSOC_FAIL:
            return "ASSOC_FAIL";
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
            return "HANDSHAKE_TIMEOUT";
        case WIFI_REASON_CONNECTION_FAIL:
            return "CONNECTION_FAIL";
        case WIFI_REASON_AP_TSF_RESET:
            return "AP_TSF_RESET";
        case WIFI_REASON_ROAMING:
            return "ROAMING";
        default: {
            static char buffer[16] = {0};
            snprintf(buffer, sizeof(buffer), "UNKNOWN %d", reason);
            return buffer;
        }
    }
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ARG_UNUSED(arg);
    ESP_LOGD(TAG, "Event base: %s, id: %d", event_base, event_id);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Connecting to %s...", args.ssid);
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *) event_data;
        ESP_LOGI(TAG, "Disconnected from %*s, bssid: %02x:%02x:%02x:%02x:%02x:%02x, reason: %s", event->ssid_len,
                 event->ssid, event->bssid[0], event->bssid[1], event->bssid[2], event->bssid[3], event->bssid[4],
                 event->bssid[5], reason_str(event->reason));
        ESP_ERROR_CHECK(context_set_network_error(args.context, true));
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        memcpy(&ip4_addr, &event->ip_info.ip, sizeof(ip4_addr));
        xEventGroupSetBits(wifi_event_group, GOT_IPV4_BIT);
    }
}

static esp_err_t initialize(wifi_config_t *wifi_config) {
    esp_log_level_set("wifi", ESP_LOG_VERBOSE);
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
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    initialized = true;
    return ESP_OK;
}

static void wifi_join(void) {
    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = CONFIG_ESP_WIFI_SSID,
                    .password = CONFIG_ESP_WIFI_PASSWORD,
                    // Setting a password implies station will connect to all security modes including WEP/WPA.
                    // However, these modes are deprecated and not advisable to be used. Only allow WPA2!
                    .threshold = {
                            .authmode = WIFI_AUTH_WPA2_PSK,
                            .rssi = -127,
                    },
                    // Do a full scan to better workaround mesh networks.
                    .scan_method = WIFI_ALL_CHANNEL_SCAN,
                    .pmf_cfg = {
                            .capable = true,
                            .required = false,
                    }
            },
    };
    if (args.ssid != NULL && args.password != NULL) {
        strlcpy((char *) wifi_config.sta.ssid, args.ssid, sizeof(wifi_config.sta.ssid));
        strlcpy((char *) wifi_config.sta.password, args.password, sizeof(wifi_config.sta.password));
    }
    ESP_ERROR_CHECK(initialize(&wifi_config));
    wifi_wait_and_notify();
}

static void wifi_reconnect(void) {
    ESP_LOGI(TAG, "Reconnecting to %s...", args.ssid);
    ESP_ERROR_CHECK(context_set_network_connected(args.context, false));

    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_connect());
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
            ESP_LOGW(TAG, "Network error, waiting 2s before reconnecting...");
            vTaskDelay(pdMS_TO_TICKS(2000));
            context_set_network_error(args.context, false);
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

    xTaskCreatePinnedToCore(wifi_task, "nwifi", 3072, context, configMAX_PRIORITIES - 7, NULL, tskNO_AFFINITY);
    return ESP_OK;
}