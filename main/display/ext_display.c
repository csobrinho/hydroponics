#include "esp_err.h"

#include "driver/gpio.h"

#include "driver/lcd/lcd.h"
#include "driver/lcd/rm68090.h"
#include "embedded.h"
#include "error.h"
#include "ext_display.h"
#include "utils.h"

static const char *TAG = "ext_display";

static lcd_dev_t dev = {
        .id = RM68090_ID,
        .config = {
                .rotation = ROTATION_LANDSCAPE,
                .data_width = 8,
#ifdef CONFIG_IDF_TARGET_ESP32
                .data_io_num = {
                        GPIO_NUM_23, GPIO_NUM_18, GPIO_NUM_14, GPIO_NUM_27,
                        GPIO_NUM_26, GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_19,
                },
                .ws_io_num = GPIO_NUM_17,
                .rs_io_num = GPIO_NUM_12,
                .rd_io_num = GPIO_NUM_NC,
                .rst_io_num = GPIO_NUM_NC,
#else
                .data_io_num = {
                        GPIO_NUM_2, GPIO_NUM_1, GPIO_NUM_8, GPIO_NUM_7,
                        GPIO_NUM_6, GPIO_NUM_5, GPIO_NUM_4, GPIO_NUM_3,
                },
                .ws_io_num = GPIO_NUM_38,
                .rs_io_num = GPIO_NUM_45,
                .rd_io_num = GPIO_NUM_NC,
                .rst_io_num = GPIO_NUM_NC,
#endif
        },
        .device = {
                .init = rm68090_init,
                .draw_pixel = rm68090_draw_pixel,
                .fill = rm68090_fill,
                .draw = rm68090_draw,
                .clear = rm68090_clear,
                .set_rotation = rm68090_set_rotation,
                .vertical_scroll = rm68090_vertical_scroll,
                .invert_display = rm68090_invert_display,
        }
};

static void ext_display_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    ESP_ERROR_CHECK(lcd_init(&dev));

    uint16_t w = lcd_width(&dev);
    uint16_t h = lcd_height(&dev);
    uint16_t logo_w = HYDROPONICS_LOGO_BIN_WIDTH;
    uint16_t logo_h = HYDROPONICS_LOGO_BIN_HEIGHT;
    int16_t x = random_int(0, w - logo_w);
    int16_t y = random_int(0, h - logo_h);
    uint16_t x_inc = 1;
    uint16_t y_inc = 1;
    while (1) {
        lcd_draw(&dev, (const uint16_t *) HYDROPONICS_LOGO_BIN_START, x, y, logo_w, logo_h);
        vTaskDelay(pdMS_TO_TICKS(200));
        lcd_fill(&dev, LCD_COLOR_BLACK, x, y, x + logo_w, y + logo_h);

        x += x_inc;
        y += y_inc;
        if (x < 0) {
            x = 1;
            x_inc = 1;
        }
        if (x > w - logo_w) {
            x = w - logo_w;
            x_inc = -1;
        }
        if (y < 0) {
            y = 1;
            y_inc = 1;
        }
        if (y > h - logo_h) {
            y = h - logo_h;
            y_inc = -1;
        }
    }
}

esp_err_t ext_display_init(context_t *context) {
    xTaskCreatePinnedToCore(ext_display_task, "ext_display", 4096, context, 15, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
