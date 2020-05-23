#include "driver/gpio.h"
#include "esp_err.h"

#include "error.h"
#include "lcd.h"
#include "utils.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#define USE_I2S
#include "i2s.h"
#else
#include "direct.h"
#endif

static const char *TAG = "lcd";

esp_err_t lcd_init(lcd_dev_t *dev) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    dev->buffer_len = sizeof(uint32_t) * LCD_BUFFER_SIZE;
    dev->buffer = calloc(1, dev->buffer_len);
    if (!dev->buffer) {
        return ESP_ERR_NO_MEM;
    }
    dev->rotation = ROTATION_UNKNOWN;

#ifdef USE_I2S
    ESP_ERROR_CHECK(lcd_i2s_init(dev));
#else
    ESP_ERROR_CHECK(lcd_direct_init(dev));
#endif
    ESP_ERROR_CHECK(lcd_reset(dev));
    ESP_ERROR_CHECK(dev->device.init(dev));
    lcd_clear(dev, LCD_COLOR_BLACK);

    return ESP_OK;
}

esp_err_t lcd_reset(const lcd_dev_t *dev) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    if (dev->config.rd_io_num != GPIO_NUM_NC) {
        RD_IDLE(dev);
    }
    WR_IDLE(dev);
    if (dev->config.rst_io_num != GPIO_NUM_NC) {
        RESET_IDLE(dev);
        safe_delay_ms(50);
        RESET_ACTIVE(dev);
        safe_delay_ms(100);
        RESET_IDLE(dev);
        safe_delay_ms(100);
    }
    return ESP_OK;
}

esp_err_t lcd_init_registers(const lcd_dev_t *dev, const uint16_t *table, size_t size) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    ARG_CHECK(table != NULL, ERR_PARAM_NULL);
    ARG_CHECK(size > 0, ERR_PARAM_LE_ZERO);
    LLOG(TAG, "[%s] count: %d", __FUNCTION__, (int) (size / sizeof(uint16_t)));

    while (size > 0) {
        uint16_t cmd = *table++;
        uint16_t d = *table++;
        if (cmd == LCD_CMD_DELAY)
            safe_delay_ms(d);
        else {
            lcd_write_reg(dev, cmd, d);
        }
        size -= 2 * sizeof(int16_t);
    }
    return ESP_OK;
}

inline void lcd_write_data16(const lcd_dev_t *dev, uint16_t data) {
#ifdef USE_I2S
    lcd_i2s_write_data16(dev, data);
#else
    lcd_direct_write_data16(dev, data);
#endif
}

void lcd_write_data16n(const lcd_dev_t *dev, uint16_t data, size_t len) {
#ifdef USE_I2S
    lcd_i2s_write_data16n(dev, data);
#else
    lcd_direct_write_data16n(dev, data, len);
#endif
}

inline void lcd_write_datan(const lcd_dev_t *dev, const uint16_t *buf, size_t len) {
#ifdef USE_I2S
    lcd_i2s_write_datan(dev, buf, len);
#else
    lcd_direct_write_datan(dev, buf, len);
#endif
}

inline void lcd_write_cmd(const lcd_dev_t *dev, uint16_t cmd) {
    CD_COMMAND(dev);
    lcd_write_data16(dev, cmd);
    CD_DATA(dev);
}

inline void lcd_write_reg(const lcd_dev_t *dev, uint16_t cmd, uint16_t data) {
    LLOG(TAG, "[%s] cmd: 0x%04x data: 0x%04x", __FUNCTION__, cmd, data);
    lcd_write_cmd(dev, cmd);
    lcd_write_data16(dev, data);
}

inline void lcd_clear(lcd_dev_t *dev, uint16_t color) {
    dev->device.clear(dev, color);
}

inline void lcd_draw_pixel(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y) {
    dev->device.draw_pixel(dev, color, x, y);
}

inline void lcd_fill(lcd_dev_t *dev, uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    LLOG(TAG, "[%s] color: 0x%04x (%d, %d) -> (%d, %d)", __FUNCTION__, color, x1, y1, x2, y2);
    dev->device.fill(dev, color, x1, y1, x2, y2);
}

inline void lcd_fill_wh(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if (width <= 0 || height <= 0) return;
    lcd_fill(dev, color, x, y, x + width - 1, y + height - 1);
}

inline void lcd_hline(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t width) {
    if (width <= 0) return;
    lcd_fill(dev, color, x, y, x + width - 1, y);
}

inline void lcd_vline(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t height) {
    if (height <= 0) return;
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
    dev->device.draw(dev, img, x, y, width, height);
}

esp_err_t lcd_set_rotation(lcd_dev_t *dev, rotation_t rotation) {
    return dev->device.set_rotation(dev, rotation);
}

esp_err_t lcd_vertical_scroll(lcd_dev_t *dev, int16_t top, int16_t scroll_lines, int16_t offset) {
    return dev->device.vertical_scroll(dev, top, scroll_lines, offset);
}

esp_err_t lcd_invert_display(lcd_dev_t *dev, bool reverse) {
    return dev->device.invert_display(dev, reverse);
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
