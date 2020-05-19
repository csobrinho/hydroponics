#include <string.h>

#include "esp_err.h"

#include "driver/gpio.h"
#include "ucg.h"

#include "buses.h"
#include "driver/lcd/lcd.h"
#include "driver/lcd/rm68090.h"
#include "driver/lcd/ucg_rm68090_hal.h"
#include "embedded.h"
#include "error.h"
#include "ext_display.h"
#include "utils.h"

static const char *TAG = "ext_display";
static ucg_t ucg = {0};

static lcd_dev_t dev = {
        .id = RM68090_ID,
        .config = {
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

static size_t snprintf_append(char *buf, size_t len, size_t max_size, float value) {
    if (CONTEXT_VALUE_IS_VALID(value)) {
        return snprintf(buf + len, max_size - len, " %.1f", value);
    }
    return snprintf(buf + len, max_size - len, " ??");
}

static void snprintf_value(char *buf, size_t max_size, const char *format, const char *format_off, float value) {
    if (CONTEXT_VALUE_IS_VALID(value)) {
        snprintf(buf, max_size, format, value);
    } else {
        strncpy(buf, format_off, max_size);
    }
}

static void ext_display_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    ESP_ERROR_CHECK(lcd_init(&dev));
    ESP_ERROR_CHECK(ucg_rm68090_init(&dev, &ucg));

    while (1) {
        ucg_SetColor(&ucg, 0, 0xff, 0xff, 0xff);
        ucg_SetFont(&ucg, ucg_font_7x13_tf);
        char buf[128] = {0};

        context_lock(context);
        float indoor = context->sensors.temp.indoor;
        float probe = context->sensors.temp.probe;
        float humidity = context->sensors.humidity;
        float ec = context->sensors.ec.value;
        float ph = context->sensors.ph.value;
        EventBits_t bits = xEventGroupGetBits(context->event_group);
        bool connected = (bits & CONTEXT_EVENT_NETWORK) == CONTEXT_EVENT_NETWORK;
        bool time_updated = (bits & CONTEXT_EVENT_TIME) == CONTEXT_EVENT_TIME;
        bool iot_connected = (bits & CONTEXT_EVENT_IOT) == CONTEXT_EVENT_IOT;
        context_unlock(context);

        uint8_t dir = 0;

        size_t len = strlcpy(buf, "Tmp:", sizeof(buf));
        len += snprintf_append(buf, len, sizeof(buf), indoor);
        len += snprintf(buf + len, sizeof(buf) - len, " |");
        len += snprintf_append(buf, len, sizeof(buf), probe);
        snprintf(buf + len, sizeof(buf) - len, " \260C");
        ucg_DrawString(&ucg, 0, 13, dir, buf);

        snprintf_value(buf, sizeof(buf), "Hum: %.f %%", "Hum: ?? %%", humidity);
        ucg_DrawString(&ucg, 0, 26, dir, buf);

        snprintf(buf, sizeof(buf), "%c%c%c", connected ? 'W' : '*', time_updated ? 'T' : '*',
                 iot_connected ? 'G' : '*');
        ucg_DrawString(&ucg, ucg_GetWidth(&ucg) - (8 * 3), 13, dir, buf);

        ucg_SetFont(&ucg, ucg_font_inr24_tf);

        snprintf_value(buf, sizeof(buf), "EC: %.f uS/cm", "EC: ?? uS/cm", ec);
        ucg_DrawString(&ucg, 0, 100, dir, buf);
        snprintf_value(buf, sizeof(buf), "PH: %.2f", "PH: ??", ph);
        ucg_DrawString(&ucg, 0, 134, dir, buf);

        vTaskDelay(pdMS_TO_TICKS(2000));
        ucg_SetColor(&ucg, 0, 0, 0, 0);
        ucg_DrawBox(&ucg, 0, 0, ucg_GetWidth(&ucg), 140);
    }
}

esp_err_t ext_display_init(context_t *context) {
    xTaskCreatePinnedToCore(ext_display_task, "ext_display", 4096, context, 15, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
