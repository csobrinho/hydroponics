#include "esp_err.h"

#include "ucg.h"

#include "error.h"
#include "ext_display.h"
#include "lcd.h"
#include "screens/ext_main.h"
#include "lcd_ucg_hal.h"
#include "st7796s.h"

static const char *const TAG = "ext_display";
static ucg_t ucg = {0};

lcd_dev_t dev = {
        .id = ST7796S_ID,
        .config = {
                .type = LCD_TYPE_SPI,
                .screen = {
                        .width = ST7796S_MAX_WIDTH,
                        .height = ST7796S_MAX_HEIGHT,
                        .bytes = sizeof(uint16_t),
                        .divisor = 32,
                        .caps = MALLOC_CAP_DMA
                },
                .rst_io_num = ST7796S_RST,
                .led_io_num = ST7796S_LED,
                .rotation = ROTATION_PORTRAIT,
                .spi = {
                        .mosi_io_num = ST7796S_MOSI,
                        .miso_io_num = ST7796S_MISO,
                        .sclk_io_num = ST7796S_SCK,
                        .cs_io_num = ST7796S_CS,
                        .dc_io_num = ST7796S_DC,
                        .mode = 0,                          // SPI mode 0.
                        .clock_speed_hz = 80 * 1000 * 1000, // Clock out at 80 MHz
                        .host = SPI2_HOST,
                        .dma_chan = SPI2_HOST,
                },
        },
        .device = &st7796s,
};

static void ext_display_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    ESP_ERROR_CHECK(lcd_init(&dev));
    ESP_ERROR_CHECK(lcd_ucg_hal_init(&dev, &ucg));
    ESP_ERROR_CHECK(ext_main_init(context, &dev, &ucg));

    while (true) {
        ESP_ERROR_CHECK(ext_main_draw(context, &dev, &ucg));
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

esp_err_t ext_display_init(context_t *context) {
    xTaskCreatePinnedToCore(ext_display_task, "ext_display", 4096, context, 15, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
