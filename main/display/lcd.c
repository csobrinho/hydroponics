#include "esp_err.h"
#include "esp_log.h"

#include "buses.h"
#include "rm68090.h"

// #define LOG(args...) ESP_LOGD(args)
#define LOG(args...) do {} while (0)

static const char *TAG = "lcd";
static i2s_lcd8_dev_t dev = {
        .base = {
                .i2s_lcd_conf = {
                        .data_width = 8,
                        .data_io_num = {
                                GPIO_NUM_23, GPIO_NUM_18, GPIO_NUM_14, GPIO_NUM_27,
                                GPIO_NUM_26, GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_19,
                        },
                        .ws_io_num = GPIO_NUM_17,
                        .rs_io_num = GPIO_NUM_12,
                },
                .i2s_port = I2S_NUM_1,
        },
        .rst_io_num = GPIO_NUM_16,
};

esp_err_t lcd_init() {
    ESP_ERROR_CHECK(rm68090_init(&dev));
    return ESP_OK;
}

void lcd_clear(uint16_t color) {
    rm68090_clear(&dev, color);
}

void lcd_draw_pixel(uint16_t color, uint16_t x, uint16_t y) {
    rm68090_draw_pixel(&dev, color, x, y);
}

void lcd_fill(uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    LOG(TAG, "[%s] color: 0x%04x (%d, %d) -> (%d, %d)", __FUNCTION__, color, x1, y1, x2, y2);
    rm68090_fill(&dev, color, x1, y1, x2, y2);
}

void lcd_hline(uint16_t color, uint16_t x, uint16_t y, uint16_t width) {
    if (width <= 0) return;
    lcd_fill(color, x, y, x + width, y);
}

void lcd_vline(uint16_t color, uint16_t x, uint16_t y, uint16_t height) {
    if (height <= 0) return;
    lcd_fill(color, x, y, x, y + height);
}

// Adapted from http://www.edepot.com/linee.html
void lcd_line(uint16_t color, int32_t x, int32_t y, int32_t x2, int32_t y2) {
    LOG(TAG, "[%s] color: 0x%04x (%d, %d) -> (%d, %d)", __FUNCTION__, color, x, y, x2, y2);
//    if (x == x2) {
//        lcd_vline(color, x, y, y2 - y);
//        return;
//    }
//    if (y == y2) {
//        lcd_hline(color, x, y, x2 - x);
//        return;
//    }
    bool y_longer = false;
    int32_t short_len = y2 - y;
    int32_t long_len = x2 - x;
    if (abs(short_len) > abs(long_len)) {
        int32_t swap = short_len;
        short_len = long_len;
        long_len = swap;
        y_longer = true;
    }
    int dec_inc;
    if (long_len == 0) dec_inc = 0;
    else dec_inc = (short_len << 16) / long_len;

    if (y_longer) {
        if (long_len > 0) {
            long_len += y;
            for (int j = 0x8000 + (x << 16); y <= long_len; ++y) {
                lcd_draw_pixel(color, j >> 16, y);
                j += dec_inc;
            }
            return;
        }
        long_len += y;
        for (int j = 0x8000 + (x << 16); y >= long_len; --y) {
            lcd_draw_pixel(color, j >> 16, y);
            j -= dec_inc;
        }
        return;
    }

    if (long_len > 0) {
        long_len += x;
        for (int j = 0x8000 + (y << 16); x <= long_len; ++x) {
            lcd_draw_pixel(color, x, j >> 16);
            j += dec_inc;
        }
        return;
    }
    long_len += x;
    for (int j = 0x8000 + (y << 16); x >= long_len; --x) {
        lcd_draw_pixel(color, x, j >> 16);
        j -= dec_inc;
    }
}

void lcd_rect(uint16_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if (width == 0 || height == 0) return;
    if (width == 1) {
        lcd_vline(color, x, y, height);
        return;
    }
    if (height == 1) {
        lcd_hline(color, x, y, width);
        return;
    }
    lcd_hline(color, x, y, width);
    lcd_hline(color, x, y + height, width);
    lcd_vline(color, x, y + 1, height - 2);
    lcd_vline(color, x + width, y + 1, height - 2);
}

uint16_t lcd_width() {
    return dev.width;
}

uint16_t lcd_height() {
    return dev.height;
}