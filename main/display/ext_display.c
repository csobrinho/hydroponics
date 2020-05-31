#include "esp_err.h"
#include "driver/gpio.h"

#include "ucg.h"

#include "buses.h"
#include "error.h"
#include "ext_display.h"
#include "lcd.h"
#include "rm68090.h"
#include "screens/ext_main.h"
#include "ucg_rm68090_hal.h"
#include "utils.h"

static const char *TAG = "ext_display";
static ucg_t ucg = {0};

static lcd_dev_t dev = {
        .id = RM68090_ID,
        .config = {
                .screen = RM68090_MAX_WIDTH * RM68090_MAX_HEIGHT * 2,
                .rotation = ROTATION_PORTRAIT,
                .data_width = 8,
                .data_io_num = LCD_DATA,
                .ws_io_num = LCD_WS,
                .rs_io_num = LCD_RS,
                .rd_io_num = LCD_RD,
                .rst_io_num = LCD_RST,
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
    ESP_ERROR_CHECK(ucg_rm68090_init(&dev, &ucg));
    ESP_ERROR_CHECK(ext_main_init(context, &dev, &ucg));

    while (1) {
        ESP_ERROR_CHECK(ext_main_draw(context, &ucg));
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

esp_err_t ext_display_init(context_t *context) {
    xTaskCreatePinnedToCore(ext_display_task, "ext_display", 4096, context, 15, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
