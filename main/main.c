#include "esp_event.h"
#include "esp_log.h"

#include "buses.h"
#include "config.h"
#include "context.h"
#include "console/console.h"
#include "display/ext_display.h"
#include "driver/ext_gpio.h"
#include "network/iot.h"
#include "network/ntp.h"
#include "network/syslog.h"
#include "network/wifi.h"
#include "sensors/ezo_ec.h"
#include "sensors/ezo_ph.h"
#include "sensors/ezo_rtd.h"
#include "sensors/humidity_pressure.h"
#include "sensors/tank.h"
#include "storage.h"
#include "tasks/cron.h"
#include "tasks/io.h"
#include "tasks/monitor.h"
#include "tasks/tuya_io.h"

#include "st7796s.h"
#include "lcd_spi.h"

#ifdef CONFIG_IDF_TARGET_ESP32

#include "display/display.h"
#include "driver/status.h" /*TODO(sobrinho): Adapt this module to support the RGB led.*/

#endif

static context_t *context;

static void spi_pre_transfer_callback(spi_transaction_t *t) {
    lcd_spi_dc_t dc = (lcd_spi_dc_t) (intptr_t) t->user;
    gpio_set_level(ST7796S_DC, dc);
}

static lcd_dev_t dev = {
        .id = 0x7796,
        .config = {
                .type = LCD_TYPE_SPI,
                .screen = ST7796S_MAX_WIDTH * ST7796S_MAX_HEIGHT * sizeof(uint16_t),
                .spi = {
                        .bus = {
                                .miso_io_num=ST7796S_MISO,
                                .mosi_io_num=ST7796S_MOSI,
                                .sclk_io_num=ST7796S_SCK,
                                .quadwp_io_num=GPIO_NUM_NC,
                                .quadhd_io_num=GPIO_NUM_NC,
                                .max_transfer_sz=ST7796S_MAX_HEIGHT * PARALLEL_LINES * sizeof(uint16_t) + 8,
                        },
                        .dev = {
                                .clock_speed_hz=1 * 1000 * 1000,           // Clock out at 10 MHz
                                .mode=0,                                   // SPI mode 0
                                .spics_io_num=ST7796S_CS,                  // CS pin
                                .queue_size=7,                             // We want to be able to queue 7 transactions at a time
.pre_cb=spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line
.cs_ena_pretrans = 5,
.cs_ena_posttrans = 5,
.input_delay_ns = 20,
},
.host = SPI2_HOST,
.dma_chan = SPI2_HOST,
.dc_io_num = ST7796S_DC,
}
},
.device = &st7796s,
};


void app_main() {
context = context_create();
//    ESP_ERROR_CHECK(syslog_init(context));
buses_init();
lcd_init(&dev);
lcd_clear(&dev, LCD_COLOR_GREEN);
//    ESP_ERROR_CHECK(storage_init(context));
//    ESP_ERROR_CHECK(esp_netif_init());
//    ESP_ERROR_CHECK(esp_event_loop_create_default());
//    ESP_ERROR_CHECK(config_init(context));
//    ESP_ERROR_CHECK(cron_init(context));
//    ESP_ERROR_CHECK(iot_init(context));
//    ESP_ERROR_CHECK(ext_gpio_init());
//    ESP_ERROR_CHECK(tuya_io_init(context));
//    ESP_ERROR_CHECK(io_init(context));
#ifdef
CONFIG_IDF_TARGET_ESP32
    //    ESP_ERROR_CHECK(status_init(context));
    //    ESP_ERROR_CHECK(display_init(context));
#endif
//    ESP_ERROR_CHECK(ext_display_init(context));
//    ESP_ERROR_CHECK(humidity_pressure_init(context));
//    ESP_ERROR_CHECK(ezo_ec_init(context));
//    ESP_ERROR_CHECK(ezo_ph_init(context));
//    ESP_ERROR_CHECK(ezo_rtd_init(context));
//    ESP_ERROR_CHECK(tank_init(context));
//    ESP_ERROR_CHECK(wifi_init(context, context->config.ssid, context->config.password));
//    ESP_ERROR_CHECK(ntp_init(context));
//    ESP_ERROR_CHECK(monitor_init(context));
//    ESP_ERROR_CHECK(console_init());
}
