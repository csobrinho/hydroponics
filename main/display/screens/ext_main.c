#include <stdio.h>
#include <time.h>
#include <sys/queue.h>

#include "esp_err.h"

#include "ucg.h"

#include "context.h"
#include "ext_main.h"
#include "error.h"
#include "lcd.h"
#include "utils.h"

#define MAX_VALUES 2
typedef struct log {
    time_t timestamp;
    float values[MAX_VALUES];
    TAILQ_ENTRY(log) next;
} log_t;
typedef TAILQ_HEAD(log_head, log) log_head_t;

static const lcd_rgb_t COLOR_BACKGROUND = {0x3f, 0x3c, 0x49};
static const lcd_rgb_t COLOR_PRIMARY[] = {
        {0x58, 0xa0, 0xfd}, // COLOR_PRIMARY1
        {0xff, 0x6c, 0x4b}, // COLOR_PRIMARY2
        {0x86, 0xdd, 0xbd}, // COLOR_PRIMARY3
};
static const lcd_rgb_t COLOR_TEXT = {0xfb, 0xfc, 0xfb};
static const lcd_rgb_t COLOR_ACTIVE = {0xae, 0xa6, 0xcb};
static const lcd_rgb_t COLOR_INACTIVE = {0x63, 0x5e, 0x73};
static const uint8_t COLUMN = 72;

static char buf[128] = {0};
static log_head_t head;

typedef struct {
    float min;
    float max;
    float m;
    float b;
} lerp_t;

static lerp_t lerp(float x1, float x2, float y1, float y2) {
    lerp_t ret = {.min = x1, .max = x2};
    ret.m = (y2 - y1) / (x2 - x1);
    ret.b = y1 - ret.m * x1;
    return ret;
}

static const char *render_value(const char *fmt, float value, const char *na) {
    if (!CONTEXT_VALUE_IS_VALID(value)) {
        return na;
    }
    snprintf(buf, sizeof(buf), fmt, value);
    return buf;
}

static const char *render_time(void) {
    time_t now = 0;
    struct tm t = {0};
    time(&now);
    localtime_r(&now, &t);
    strftime(buf, sizeof(buf), "%R", &t);
    return buf;
}

static void draw_graph_frame(ucg_t *ucg) {
    ucg_SetColor(ucg, 0, COLOR_INACTIVE.r, COLOR_INACTIVE.g, COLOR_INACTIVE.b);
    ucg_DrawVLine(ucg, 30, 89, 122);
    ucg_DrawVLine(ucg, 280, 89, 122);
    ucg_DrawHLine(ucg, 30, 210, 250);

    ucg_SetFont(ucg, ucg_font_helvR08_tr);
    ucg_SetColor(ucg, 0, COLOR_PRIMARY[0].r, COLOR_PRIMARY[0].g, COLOR_PRIMARY[0].b);
    ucg_DrawString(ucg, 9, 100, 0, "6.0");
    ucg_DrawString(ucg, 9, 150, 0, "5.5");
    ucg_DrawString(ucg, 9, 200, 0, "5.0");

    ucg_SetColor(ucg, 0, COLOR_PRIMARY[1].r, COLOR_PRIMARY[1].g, COLOR_PRIMARY[1].b);
    ucg_DrawString(ucg, 287, 100, 0, "1750");
    ucg_DrawString(ucg, 287, 150, 0, "1500");
    ucg_DrawString(ucg, 287, 200, 0, "1250");
}

static void draw_graph(ucg_t *ucg) {
    ucg_SetColor(ucg, 0, COLOR_BACKGROUND.r, COLOR_BACKGROUND.g, COLOR_BACKGROUND.b);
    ucg_DrawBox(ucg, 31, 89, 280 - 30 + 2, 208 - 89 + 1);
    ucg_SetColor(ucg, 0, COLOR_INACTIVE.r, COLOR_INACTIVE.g, COLOR_INACTIVE.b);
    ucg_DrawVLine(ucg, 280, 89, 122);

    lerp_t mxb[MAX_VALUES] = {
            lerp(5.8f, 6.8f, 100.f, 196.f),
            lerp(1750.f, 1250.f, 100.f, 196.f),
    };
    for (int i = 0; i < MAX_VALUES; ++i) {
        ucg_SetColor(ucg, 0, COLOR_PRIMARY[i].r, COLOR_PRIMARY[i].g, COLOR_PRIMARY[i].b);
        log_t *e = NULL;
        log_t *tmp = NULL;
        uint16_t x = 280;
        int16_t y_prev = -1;
        TAILQ_FOREACH_SAFE(e, &head, next, tmp) {
            if (CONTEXT_VALUE_IS_VALID(e->values[i])) {
                uint16_t y = (uint16_t) (mxb[i].m * e->values[i] + mxb[i].b);
                y = clamp(y, 89, 207);
                if (y_prev == -1) {
                    ucg_DrawBox(ucg, x - 1, y - 1, 3, 3);
                } else {
                    ucg_DrawLine(ucg, x, y, x + 2, y_prev);
                }
                y_prev = y;
            }
            if (x > 32) {
                x -= 2;
                continue;
            }
            TAILQ_REMOVE(&head, e, next);
            SAFE_FREE(e);
        }
    }
}

static void draw_indicators(ucg_t *ucg) {
    ucg_SetFont(ucg, ucg_font_helvB14_tr);
    ucg_SetColor(ucg, 0, COLOR_ACTIVE.r, COLOR_ACTIVE.g, COLOR_ACTIVE.b);
    ucg_DrawFrame(ucg, 247, 50, 24, 24);
    ucg_DrawString(ucg, 252, 69, 0, "A");

    ucg_SetColor(ucg, 0, COLOR_INACTIVE.r, COLOR_INACTIVE.g, COLOR_INACTIVE.b);
    ucg_DrawFrame(ucg, 280, 50, 24, 24);
    ucg_DrawString(ucg, 285, 69, 0, "B");
}

static void draw_wifi(ucg_t *ucg, uint16_t x, uint16_t y) {
    ucg_DrawHLine(ucg, x + 4, y++, 7);
    ucg_DrawHLine(ucg, x + 2, y++, 11);
    ucg_DrawHLine(ucg, x + 1, y++, 13);
    ucg_DrawHLine(ucg, x++, y++, 15);
    ucg_DrawHLine(ucg, x++, y++, 13);
    ucg_DrawHLine(ucg, x++, y++, 11);
    ucg_DrawHLine(ucg, x++, y++, 9);
    ucg_DrawHLine(ucg, x++, y++, 7);
    ucg_DrawHLine(ucg, x++, y++, 5);
    ucg_DrawHLine(ucg, x++, y++, 3);
    ucg_DrawHLine(ucg, x, y, 1);
}

static void draw_statusbar(ucg_t *ucg) {
    ucg_SetFont(ucg, ucg_font_helvB12_tr);
    uint8_t font_height = ucg_GetFontCapitalAHeight(ucg);

    ucg_SetColor(ucg, 0, COLOR_BACKGROUND.r, COLOR_BACKGROUND.g, COLOR_BACKGROUND.b);
    ucg_DrawBox(ucg, 250, 18 - font_height, ucg_GetWidth(ucg) - 253, font_height);

    ucg_SetColor(ucg, 0, COLOR_TEXT.r, COLOR_TEXT.g, COLOR_TEXT.b);
    ucg_DrawString(ucg, 274, 18, 0, render_time());
    draw_wifi(ucg, 253, 6);
}

static void draw_labels(ucg_t *ucg) {
    ucg_SetFont(ucg, ucg_font_helvR14_tf);
    ucg_SetColor(ucg, 0, COLOR_TEXT.r, COLOR_TEXT.g, COLOR_TEXT.b);
    ucg_DrawString(ucg, 32, 40, 0, "PH");
    ucg_DrawString(ucg, 32 + COLUMN, 40, 0, "EC");
    ucg_DrawString(ucg, 32 + 2 * COLUMN, 40, 0, "\260C");
}

static void draw_values(context_t *context, ucg_t *ucg) {
    context_lock(context);
    float pha = context->sensors.ph[0].value;
    float eca = context->sensors.ec[0].value;
    float temp = context->sensors.temp.probe;
#if CONFIG_ESP_SENSOR_TANKS == 2
    // float ecb = context->sensors.ec[1].value;
    // float phb = context->sensors.ph[1].value;
#endif
    context_unlock(context);

    log_t *entry = calloc(1, sizeof(log_t));
    entry->timestamp = time(NULL);
    entry->values[0] = pha;
    entry->values[1] = eca;
    TAILQ_INSERT_HEAD(&head, entry, next);

    ucg_SetFont(ucg, ucg_font_helvB18_tr);
    ucg_SetColor(ucg, 0, COLOR_BACKGROUND.r, COLOR_BACKGROUND.g, COLOR_BACKGROUND.b);
    uint8_t font_height = ucg_GetFontCapitalAHeight(ucg);
    ucg_DrawBox(ucg, 32, 70 - font_height, 3 * COLUMN - 6, font_height);

    ucg_SetColor(ucg, 0, COLOR_PRIMARY[0].r, COLOR_PRIMARY[0].g, COLOR_PRIMARY[0].b);
    ucg_DrawString(ucg, 32, 70, 0, render_value("%.2f", pha, "??"));

    ucg_SetColor(ucg, 0, COLOR_PRIMARY[1].r, COLOR_PRIMARY[1].g, COLOR_PRIMARY[1].b);
    ucg_DrawString(ucg, 32 + COLUMN, 70, 0, render_value("%.f", eca, "??"));

    ucg_SetColor(ucg, 0, COLOR_PRIMARY[2].r, COLOR_PRIMARY[2].g, COLOR_PRIMARY[2].b);
    ucg_DrawString(ucg, 32 + 2 * COLUMN, 70, 0, render_value("%.1f", temp, "??"));
}

esp_err_t ext_main_init(context_t *context, lcd_dev_t *dev, ucg_t *ucg) {
    ARG_UNUSED(context);
    TAILQ_INIT(&head);

    lcd_clear(dev, lcd_rgb565s(COLOR_BACKGROUND));
    draw_labels(ucg);
    draw_indicators(ucg);
    draw_graph_frame(ucg);

    return ESP_OK;
}

esp_err_t ext_main_draw(context_t *context, ucg_t *ucg) {
    draw_values(context, ucg);
    draw_graph(ucg);
    draw_statusbar(ucg);

    return ESP_OK;
}
