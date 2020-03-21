#include "esp_event.h"

#include "buses.h"
#include "config.h"
#include "context.h"
#include "console/console.h"
#include "display/display.h"
#include "display/lcd.h"
#include "driver/status.h"
#include "driver/storage.h"
#include "network/iot.h"
#include "network/ntp.h"
#include "network/syslog.h"
#include "network/wifi.h"
#include "sensors/ezo_ec.h"
#include "sensors/ezo_ph.h"
#include "sensors/ezo_rtd.h"
#include "sensors/humidity_pressure.h"
#include "sensors/temperature.h"

static context_t *context;

void app_main() {
    context = context_create();
    buses_init();
    ESP_ERROR_CHECK(storage_init(context));
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(config_init(context));
    ESP_ERROR_CHECK(syslog_init(context));
    ESP_ERROR_CHECK(status_init(context));
    ESP_ERROR_CHECK(lcd_init(context));
    ESP_ERROR_CHECK(display_init(context));
    ESP_ERROR_CHECK(humidity_pressure_init(context));
    ESP_ERROR_CHECK(ezo_ec_init(context));
    ESP_ERROR_CHECK(ezo_ph_init(context));
    ESP_ERROR_CHECK(ezo_rtd_init(context));
    ESP_ERROR_CHECK(temperature_init(context));
    ESP_ERROR_CHECK(wifi_init(context, context->config.ssid, context->config.password));
    ESP_ERROR_CHECK(ntp_init(context));
    ESP_ERROR_CHECK(iot_init(context));

    ESP_ERROR_CHECK(console_init());
}