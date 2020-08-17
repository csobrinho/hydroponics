#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/queue.h>

#include "iot_button.h"

#include "esp_err.h"

#include "ucg.h"

#include "buses.h"
#include "context.h"
#include "config.h"
#include "ext_main.h"
#include "error.h"
#include "lcd.h"
#include "utils.h"

typedef enum {
    VALUE_PH_A = 0,
    VALUE_EC_A = 1,
    VALUE_PH_B = 2,
    VALUE_EC_B = 3,
    VALUE_MAX = 4,
} value_t;

typedef enum {
    TANK_A = CONFIG_TANK_A,
    TANK_B = CONFIG_TANK_B,
} tank_t;

typedef struct {
    float min;
    float max;
    float m;
    float b;
    struct {
        char min[6];
        char mid[6];
        char max[6];
    } str;
} lerp_t;

typedef struct log {
    time_t timestamp;
    float values[VALUE_MAX];
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

static tank_t current_tank = TANK_A;
static bool refresh = true;
static button_handle_t button_handle;
static lerp_t mxb[VALUE_MAX] = {0};

static const char *TAG = "ext_main";

static void lerp(lerp_t *s, float x1, float x2, float y1, float y2, const char *fmt) {
    s->min = x1;
    s->max = x2;
    s->m = (y2 - y1) / (x2 - x1);
    s->b = y1 - s->m * x1;
    memset(s->str.min, 0, sizeof(s->str.min));
    memset(s->str.mid, 0, sizeof(s->str.mid));
    memset(s->str.max, 0, sizeof(s->str.max));
    snprintf(s->str.min, sizeof(s->str.min) - 1, fmt, s->min);
    snprintf(s->str.mid, sizeof(s->str.mid) - 1, fmt, (s->max + s->min) / 2.f);
    snprintf(s->str.max, sizeof(s->str.max) - 1, fmt, s->max);
}

#define SWAP(a, b) do { typeof(a) temp = a; a = b; b = temp; } while (0)
#define IS_PH(t) (t == VALUE_PH_A || t == VALUE_PH_B)

static void setup_lerp(value_t type, Hydroponics__Controller__Entry *entry, float min, float max) {
    if (entry != NULL && entry->min_graph != 0.f && entry->max_graph != 0.f) {
        min = entry->min_graph;
        max = entry->max_graph;
    }
    lerp(&mxb[type], min, max, 196.f, 100.f, IS_PH(type) ? "%.2f" : "%.f");
}

static void config_callback(const Hydroponics__Config *config) {
    refresh = true;
    if (config == NULL) {
        return;
    }
    if (config->controller != NULL) {
        setup_lerp(VALUE_PH_A, config->controller->pha, 5.8f, 6.8f);
        setup_lerp(VALUE_EC_A, config->controller->eca, 1000.f, 2000.f);
        setup_lerp(VALUE_PH_B, config->controller->phb, 5.8f, 6.8f);
        setup_lerp(VALUE_EC_B, config->controller->ecb, 1000.f, 2000.f);
    }
    hydroponics__config__free_unpacked((Hydroponics__Config *) config, NULL);
}

static void IRAM_ATTR button_callback(void *data) {
    ARG_UNUSED(data);
    current_tank = current_tank == TANK_A ? TANK_B : TANK_A;
    refresh = true;
    ESP_LOGI(TAG, "Button pressed. Now showing Tank %c", current_tank == TANK_A ? 'A' : 'B');
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

    lerp_t current_mxb = mxb[current_tank * 2];
    ucg_SetFont(ucg, ucg_font_helvR08_tr);
    ucg_SetColor(ucg, 0, COLOR_PRIMARY[0].r, COLOR_PRIMARY[0].g, COLOR_PRIMARY[0].b);
    ucg_DrawString(ucg, 9, 100, 0, current_mxb.str.max);
    ucg_DrawString(ucg, 9, 150, 0, current_mxb.str.mid);
    ucg_DrawString(ucg, 9, 200, 0, current_mxb.str.min);

    current_mxb = mxb[current_tank * 2 + 1];
    ucg_SetColor(ucg, 0, COLOR_PRIMARY[1].r, COLOR_PRIMARY[1].g, COLOR_PRIMARY[1].b);
    ucg_DrawString(ucg, 287, 100, 0, current_mxb.str.max);
    ucg_DrawString(ucg, 287, 150, 0, current_mxb.str.mid);
    ucg_DrawString(ucg, 287, 200, 0, current_mxb.str.min);
}

static void draw_graph(ucg_t *ucg) {
    ucg_SetColor(ucg, 0, COLOR_BACKGROUND.r, COLOR_BACKGROUND.g, COLOR_BACKGROUND.b);
    ucg_DrawBox(ucg, 31, 89, 280 - 30 + 2, 208 - 89 + 1);
    ucg_SetColor(ucg, 0, COLOR_INACTIVE.r, COLOR_INACTIVE.g, COLOR_INACTIVE.b);
    ucg_DrawVLine(ucg, 280, 89, 122);

    int start = current_tank * 2;
    for (int i = start; i < start + 2; ++i) {
        int color = i - start;
        ucg_SetColor(ucg, 0, COLOR_PRIMARY[color].r, COLOR_PRIMARY[color].g, COLOR_PRIMARY[color].b);
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
    lcd_rgb_t color_a = current_tank == TANK_A ? COLOR_ACTIVE : COLOR_INACTIVE;
    ucg_SetColor(ucg, 0, color_a.r, color_a.g, color_a.b);
    ucg_DrawFrame(ucg, 247, 50, 24, 24);
    ucg_DrawString(ucg, 252, 69, 0, "A");

    lcd_rgb_t color_b = current_tank == TANK_B ? COLOR_ACTIVE : COLOR_INACTIVE;
    ucg_SetColor(ucg, 0, color_b.r, color_b.g, color_b.b);
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
    ucg_DrawBox(ucg, 248, 18 - font_height, ucg_GetWidth(ucg) - 252, font_height);

    ucg_SetColor(ucg, 0, COLOR_TEXT.r, COLOR_TEXT.g, COLOR_TEXT.b);
    ucg_DrawString(ucg, 272, 18, 0, render_time());
    draw_wifi(ucg, 251, 6);
}

static void draw_labels(ucg_t *ucg) {
    ucg_SetFont(ucg, ucg_font_helvR14_tf);
    ucg_SetColor(ucg, 0, COLOR_TEXT.r, COLOR_TEXT.g, COLOR_TEXT.b);
    ucg_DrawString(ucg, 32, 40, 0, "PH");
    ucg_DrawString(ucg, 32 + COLUMN, 40, 0, "EC");
    ucg_DrawString(ucg, 32 + 2 * COLUMN, 40, 0, "\260C");
}

static void draw_values(context_t *context, ucg_t *ucg) {
    log_t *entry = calloc(1, sizeof(log_t));
    ARG_ERROR_CHECK(entry != NULL, "Out of memory");

    entry->timestamp = time(NULL);

    context_lock(context);
    float temp = context->sensors.temp.probe;
    entry->values[VALUE_PH_A] = context->sensors.ph[TANK_A].value;
    entry->values[VALUE_EC_A] = context->sensors.ec[TANK_A].value;
    entry->values[VALUE_PH_B] = context->sensors.ph[TANK_B].value;
    entry->values[VALUE_EC_B] = context->sensors.ec[TANK_B].value;
    context_unlock(context);

    TAILQ_INSERT_HEAD(&head, entry, next);

    ucg_SetFont(ucg, ucg_font_helvB18_tr);
    ucg_SetColor(ucg, 0, COLOR_BACKGROUND.r, COLOR_BACKGROUND.g, COLOR_BACKGROUND.b);
    uint8_t font_height = ucg_GetFontCapitalAHeight(ucg);
    ucg_DrawBox(ucg, 32, 70 - font_height, 3 * COLUMN - 6, font_height);

    int value_idx = current_tank * 2;
    ucg_SetColor(ucg, 0, COLOR_PRIMARY[0].r, COLOR_PRIMARY[0].g, COLOR_PRIMARY[0].b);
    ucg_DrawString(ucg, 32, 70, 0, render_value("%.2f", entry->values[value_idx], "??"));
    ucg_SetColor(ucg, 0, COLOR_PRIMARY[1].r, COLOR_PRIMARY[1].g, COLOR_PRIMARY[1].b);
    ucg_DrawString(ucg, 32 + COLUMN, 70, 0, render_value("%.f", entry->values[value_idx + 1], "??"));

    ucg_SetColor(ucg, 0, COLOR_PRIMARY[2].r, COLOR_PRIMARY[2].g, COLOR_PRIMARY[2].b);
    ucg_DrawString(ucg, 32 + 2 * COLUMN, 70, 0, render_value("%.1f", temp, "??"));
}

esp_err_t ext_main_init(context_t *context, lcd_dev_t *dev, ucg_t *ucg) {
    ARG_UNUSED(context);
    ARG_UNUSED(dev);
    ARG_UNUSED(ucg);

    TAILQ_INIT(&head);

    button_handle = iot_button_create(LCD_BUTTON_GPIO, BUTTON_ACTIVE_LOW);
    CHECK_NO_MEM(button_handle);
    ESP_ERROR_CHECK(iot_button_set_evt_cb(button_handle, BUTTON_CB_TAP, button_callback, NULL));

    const Hydroponics__Config *config;
    ESP_ERROR_CHECK(context_get_config(context, &config));
    config_callback(config);
    config_register(config_callback);

    return ESP_OK;
}

esp_err_t ext_main_draw(context_t *context, lcd_dev_t *dev, ucg_t *ucg) {
    if (refresh) {
        lcd_clear(dev, lcd_rgb565s(COLOR_BACKGROUND));
        draw_labels(ucg);
        draw_indicators(ucg);
        draw_graph_frame(ucg);
        refresh = !refresh;
    }
    draw_values(context, ucg);
    draw_graph(ucg);
    draw_statusbar(ucg);

    return ESP_OK;
}
