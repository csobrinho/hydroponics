#include <string.h>

#include "driver/gpio.h"
#include "esp_err.h"

#include "error.h"
#include "lcd.h"
#include "lcd_driver.h"
#include "utils.h"

static const char *const TAG = "lcd";

static esp_err_t lcd_init_gpio(lcd_dev_t *dev) {
    if (dev->config.led_io_num != GPIO_NUM_NC) {
        gpio_config_t conf = {
                .pin_bit_mask = BIT64(dev->config.led_io_num),
                .mode = GPIO_MODE_OUTPUT,
                .pull_up_en = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&conf));
    }
    return ESP_OK;
}

esp_err_t lcd_init(lcd_dev_t *dev) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    dev->rotation = ROTATION_UNKNOWN;

    ESP_ERROR_CHECK(dev->device->comm->init(dev));
    ESP_ERROR_CHECK(dev->device->init(dev));
    lcd_set_rotation(dev, dev->config.rotation);
    lcd_clear(dev, LCD_COLOR_BLACK);

    ESP_ERROR_CHECK(lcd_init_gpio(dev));
    ESP_ERROR_CHECK(lcd_set_backlight(dev, true));

    return ESP_OK;
}

inline void lcd_draw_pixel(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y) {
    dev->device->draw_pixel(dev, color, x, y);
}

void lcd_clear(lcd_dev_t *dev, uint16_t color) {
    LLOG(TAG, "[%s] clear color: 0x%04x", __FUNCTION__, color);
    dev->device->clear(dev, color);
}

inline void lcd_fill(lcd_dev_t *dev, uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    LLOG(TAG, "[%s] color: 0x%04x (%d, %d) -> (%d, %d)", __FUNCTION__, color, x1, y1, x2, y2);
    dev->device->fill(dev, color, x1, y1, x2, y2);
}

inline void lcd_fill_wh(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if (width <= 0 || height <= 0) {
        return;
    }
    lcd_fill(dev, color, x, y, x + width - 1, y + height - 1);
}

inline void lcd_hline(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t width) {
    if (width <= 0) {
        return;
    }
    lcd_fill(dev, color, x, y, x + width - 1, y);
}

inline void lcd_vline(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t height) {
    if (height <= 0) {
        return;
    }
    lcd_fill(dev, color, x, y, x, y + height - 1);
}

// Adapted from http://www.edepot.com/linee.html
void lcd_line(lcd_dev_t *dev, uint16_t color, int32_t x, int32_t y, int32_t x2, int32_t y2) {
    LLOG(TAG, "[%s] color: 0x%04x (%d, %d) -> (%d, %d)", __FUNCTION__, color, x, y, x2, y2);
    if (x == x2) {
        lcd_vline(dev, color, x, y, y2 - y);
        return;
    }
    if (y == y2) {
        lcd_hline(dev, color, x, y, x2 - x);
        return;
    }
    bool y_longer = false;
    int32_t short_len = y2 - y;
    int32_t long_len = x2 - x;
    if (abs(short_len) > abs(long_len)) {
        int32_t swap = short_len;
        short_len = long_len;
        long_len = swap;
        y_longer = true;
    }
    int dec_inc = long_len == 0 ? 0 : (short_len << 16) / long_len;
    if (y_longer) {
        if (long_len > 0) {
            long_len += y;
            for (int j = 0x8000 + (x << 16); y <= long_len; ++y) {
                lcd_draw_pixel(dev, color, j >> 16, y);
                j += dec_inc;
            }
            return;
        }
        long_len += y;
        for (int j = 0x8000 + (x << 16); y >= long_len; --y) {
            lcd_draw_pixel(dev, color, j >> 16, y);
            j -= dec_inc;
        }
        return;
    }

    if (long_len > 0) {
        long_len += x;
        for (int j = 0x8000 + (y << 16); x <= long_len; ++x) {
            lcd_draw_pixel(dev, color, x, j >> 16);
            j += dec_inc;
        }
        return;
    }
    long_len += x;
    for (int j = 0x8000 + (y << 16); x >= long_len; --x) {
        lcd_draw_pixel(dev, color, x, j >> 16);
        j -= dec_inc;
    }
}

void lcd_rect(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if (width == 0 || height == 0) return;
    if (width <= 2 || height <= 2) {
        lcd_fill_wh(dev, color, x, y, width, height);
        return;
    }
    lcd_hline(dev, color, x, y, width);
    lcd_hline(dev, color, x, y + height - 1, width);
    lcd_vline(dev, color, x, y + 1, height - 2);
    lcd_vline(dev, color, x + width - 1, y + 1, height - 2);
}

void lcd_draw(lcd_dev_t *dev, const uint16_t *img, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    dev->device->draw(dev, img, x, y, width, height);
}

esp_err_t lcd_set_rotation(lcd_dev_t *dev, rotation_t rotation) {
    return dev->device->set_rotation(dev, rotation);
}

esp_err_t lcd_vertical_scroll(lcd_dev_t *dev, int16_t top, int16_t scroll_lines, int16_t offset) {
    return dev->device->vertical_scroll(dev, top, scroll_lines, offset);
}

esp_err_t lcd_invert_display(lcd_dev_t *dev, bool reverse) {
    return dev->device->invert_display(dev, reverse);
}

esp_err_t lcd_set_backlight(lcd_dev_t *dev, bool on) {
    if (dev->config.led_io_num == GPIO_NUM_NC) {
        return ESP_OK;
    }
    ESP_ERROR_CHECK(gpio_set_level(dev->config.led_io_num, on));
    return ESP_OK;
}

inline uint16_t lcd_width(const lcd_dev_t *dev) {
    return dev->registers.width;
}

inline uint16_t lcd_height(const lcd_dev_t *dev) {
    return dev->registers.height;
}

inline uint16_t lcd_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return (((r >> 3) & 0b00011111) << 11)  // Red
           | (((g >> 2) & 0b00111111) << 5) // Green
           | ((b >> 3) & 0b00011111);       // Blue
}

inline uint16_t lcd_rgb565s(const lcd_rgb_t color) {
    return lcd_rgb565(color.r, color.g, color.b);
}

inline lcd_rgb_t lcd_rgb888(uint16_t color) {
    return (lcd_rgb_t) {
            .r = ((((color >> 11) & 0b00011111) * 527) + 23) >> 6,
            .g = ((((color >> 5) & 0b00111111) * 259) + 33) >> 6,
            .b = (((color & 0b00011111) * 527) + 23) >> 6,
    };
}

#define LCD_FILL_BLOCK 16 // Should be 8 or 16.

void lcd_buf_fill(uint16_t *buf, uint16_t data, size_t len) {
    LLOG(TAG, "[%s] len: %d data: 0x%02x 0x%02x", __FUNCTION__, len, data & 0xff, data >> 8);
    // Fill the buffer with the data.
    uint16_t swapped = __bswap16(data);
    if (data == swapped) {
        memset(buf, data, len);
        return;
    }
    ARG_ERROR_CHECK(len % 2 == 0, "len %d must be an even number", len);
    uint32_t swapped32 = swapped << 16 | swapped;
    void *end = buf + len / 2;
    uint32_t *buf32 = (uint32_t *) buf;
    while (len >= LCD_FILL_BLOCK * sizeof(uint32_t)) {
        buf32[0] = swapped32;
        buf32[1] = swapped32;
        buf32[2] = swapped32;
        buf32[3] = swapped32;
        buf32[4] = swapped32;
        buf32[5] = swapped32;
        buf32[6] = swapped32;
        buf32[7] = swapped32;
#if LCD_FILL_BLOCK == 16
        buf32[8] = swapped32;
        buf32[9] = swapped32;
        buf32[10] = swapped32;
        buf32[11] = swapped32;
        buf32[12] = swapped32;
        buf32[13] = swapped32;
        buf32[14] = swapped32;
        buf32[15] = swapped32;
#endif
        buf32 += LCD_FILL_BLOCK;
        len -= LCD_FILL_BLOCK * sizeof(uint32_t);
    }
    buf = (uint16_t *) buf32;
    while (len > 0) {
        *buf++ = swapped;
        len -= sizeof(uint16_t);
    }
    ARG_ERROR_CHECK(buf == end, "buf %p != predicted end %p", buf, end);
}
