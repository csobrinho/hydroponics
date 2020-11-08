#include <math.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "error.h"

#include "lcd.h"
#include "lcd_spi.h"
#include "st7796s_regs.h"
#include "st7796s.h"
#include "utils.h"

#define _D0                                 0
#define _D1(d1)                             1, d1
#define _D2(d1, d2)                         2, d1, d2
#define _D3(d1, d2, d3)                     3, d1, d2, d3
#define _D4(d1, d2, d3, d4)                 4, d1, d2, d3, d4
#define _D8(d1, d2, d3, d4, d5, d6, d7, d8) 8, d1, d2, d3, d4, d5, d6, d7, d8
#define _D14(d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14) \
         14, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14

static const char *const TAG = "st7796s";

// Init registers adapted from https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_Drivers/ST7796_Init.h
DMA_ATTR const uint8_t ST7796S_REG_VALUES[] = {
        LCD_CMD8_DELAY, 120,                            // Wait 120 ms.
        ST7796S_REG_SWRESET, _D0,                       // Software reset.
        LCD_CMD8_DELAY, 120,                            // Wait 120 ms.
        ST7796S_REG_SLPOUT, _D0,                        // Sleep exit.
        LCD_CMD8_DELAY, 120,                            // Wait 120 ms.
        ST7796S_REG_CSCON, _D1(0xC3),                   // Enable extension command 2 part I.
        ST7796S_REG_CSCON, _D1(0x96),                   // Enable extension command 2 part II.
        ST7796S_REG_COLMOD, _D1(0x55),                  // Control interface color format set to 16.
        ST7796S_REG_IFMODE, _D1(0x80),                  // SPI_EN.
        ST7796S_REG_DFC, _D3(0x00,                      // Bypass.
                             0x02,                      // Source Output Scan [S1,S960], Gate Output Scan [G1,G480], scan cycle=2.
                             0x3b),                     // LCD Drive Line=8*(59+1).
        ST7796S_REG_BPC, _D4(0x02,                      // Blanking Porch Control. Front porch of 2 lines.
                             0x03,                      // Front porch of 3 lines.
                             0x00,
                             0x04),                     // Back porch of 4 lines.
        ST7796S_REG_FRMCTR1, _D2(0x80, 0x10),           // Frame Rate Control.
        ST7796S_REG_DIC, _D1(0x00),                     // Display Inversion Control.
        ST7796S_REG_EM, _D1(0xC6),                      // Entry Mode Set
        ST7796S_REG_VCMPCTL, _D1(0x24),                 // VCOM = 1.200.
        ST7796S_REG_DOCA, _D8(0x40, 0x8A, 0x00, 0x00,
                              0x29,                     // Source timing Control = 21us.
                              0x19,                     // Gate start: 26 Tclk
                              0xA5,                     // Gate end: 38 Tclk. Gate driver EQ function ON.
                              0x33),
        ST7796S_REG_PWR3, _D0,                          // Power Control 3.
        ST7796S_REG_PGC, _D14(0xF0, 0x09, 0x13, 0x12,   // Positive Gamma Control
                              0x12, 0x2B, 0x3C, 0x44,
                              0x4B, 0x1B, 0x18, 0x17,
                              0x1D, 0x21),
        ST7796S_REG_NGC, _D14(0xF0, 0x09, 0x13, 0x0C,   // Negative Gamma Control.
                              0x0D, 0x27, 0x3B, 0x44,
                              0x4D, 0x0B, 0x17, 0x17,
                              0x1D, 0x21),
        ST7796S_REG_MADCTL, _D1(ST7796S_MADCTL_MX | ST7796S_MADCTL_BGR),
        ST7796S_REG_CSCON, _D1(0x3c),                   // Disable extension command 2 part I.
        ST7796S_REG_CSCON, _D1(0x69),                   // Disable extension command 2 part II.
        ST7796S_REG_NORON, _D0,                         // Normal Display Mode On.
        ST7796S_REG_SLPOUT, _D0,                        // Sleep Out.
        ST7796S_REG_DISPON, _D0,                        // Display On.
};

static esp_err_t st7796s_vertical_scroll(lcd_dev_t *dev, int16_t top, int16_t scroll_lines, int16_t offset) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] top: %d scroll_lines: %d offset: %d", __FUNCTION__, top, scroll_lines, offset);

    int16_t bfa = ST7796S_MAX_HEIGHT - top - scroll_lines;
    if (offset <= -scroll_lines || offset >= scroll_lines) {
        offset = 0;                                          // Valid scroll.
    }
    int16_t vsp = top + offset;                              // Vertical start position.
    if (offset < 0) {
        vsp += scroll_lines;                                 // Keep in unsigned range.
    }
    const uint16_t vscrdef[3] = {                            // For multi-byte parameters. TFA, VSA, BFA.
            __bswap16(top),
            __bswap16(scroll_lines),
            __bswap16(bfa),
    };
    dev->device->comm->write_regn(dev, ST7796S_REG_VSCRDEF, (const uint8_t *) vscrdef, sizeof(vscrdef));
    dev->device->comm->write_reg16(dev, ST7796S_REG_VSCSAD, vsp);
    if (offset == 0) {
        dev->device->comm->write_regn(dev, ST7796S_REG_NORON, NULL, 0);
    }
    return ESP_OK;
}

static esp_err_t st7796s_address_set(lcd_dev_t *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] (%d, %d) -> (%d, %d)", __FUNCTION__, x1, y1, x2, y2);

    const uint16_t x[2] = {__bswap16(x1), __bswap16(x2)};
    const uint16_t y[2] = {__bswap16(y1), __bswap16(y2)};
    dev->device->comm->write_regn(dev, dev->registers.mc, (const uint8_t *) x, sizeof(x));
    dev->device->comm->write_regn(dev, dev->registers.mp, (const uint8_t *) y, sizeof(y));

    return ESP_OK;
}

static esp_err_t st7796s_set_rotation(lcd_dev_t *dev, rotation_t rotation) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    ARG_CHECK(rotation >= 0 && rotation <= ROTATION_MAX, "rotation is invalid");
    LLOG(TAG, "[%s] rotation: %d", __FUNCTION__, rotation);

    if (rotation == dev->rotation) {
        return ESP_OK;
    }
    bool is_landscape = rotation == ROTATION_LANDSCAPE || rotation == ROTATION_LANDSCAPE_REV;
    dev->registers.width = is_landscape ? ST7796S_MAX_HEIGHT : ST7796S_MAX_WIDTH;
    dev->registers.height = is_landscape ? ST7796S_MAX_WIDTH : ST7796S_MAX_HEIGHT;

    uint8_t val = ST7796S_MADCTL_BGR;
    switch (rotation) {
        case ROTATION_PORTRAIT:                                               // PORTRAIT:
            val |= ST7796S_MADCTL_MX;                                         // MY=0, MX=1, MV=0, BGR=1
            break;
        case ROTATION_LANDSCAPE:                                              // LANDSCAPE: 90 degrees
            val |= ST7796S_MADCTL_MV;                                         // MY=0, MX=0, MV=1, BGR=1
            break;
        case ROTATION_PORTRAIT_REV:                                           // PORTRAIT_REV: 180 degrees
            val |= ST7796S_MADCTL_MY | ST7796S_MADCTL_ML;                     // MY=1, MX=0, MV=1, ML=0, BGR=1
            break;
        case ROTATION_LANDSCAPE_REV:                                          // LANDSCAPE_REV: 270 degrees
            //                                                                   MY=1, MX=1, MV=1, ML=1, BGR=1
            val |= ST7796S_MADCTL_MX | ST7796S_MADCTL_MY | ST7796S_MADCTL_MV | ST7796S_MADCTL_ML;
            break;
        default:
            return ESP_ERR_INVALID_ARG;
    }
    dev->device->comm->write_reg8(dev, ST7796S_REG_MADCTL, val);

    st7796s_address_set(dev, 0, 0, dev->registers.width - 1, dev->registers.height - 1);
    ESP_ERROR_CHECK(st7796s_vertical_scroll(dev, 0, ST7796S_MAX_HEIGHT, 0));
    return ESP_OK;
}

static esp_err_t st7796s_invert_display(lcd_dev_t *dev, bool reverse) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] reverse: %d", __FUNCTION__, reverse);

    dev->registers.reverse = reverse;
    dev->device->comm->write_regn(dev, dev->registers.reverse ? ST7796S_REG_INVON : ST7796S_REG_INVOFF, NULL, 0);

    return ESP_OK;
}

static esp_err_t st7796s_init_registers(const lcd_dev_t *dev, const uint8_t *table, size_t size) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    ARG_CHECK(table != NULL, ERR_PARAM_NULL);
    ARG_CHECK(size > 0, ERR_PARAM_LE_ZERO);
    LLOG(TAG, "[%s] count: %d", __FUNCTION__, size);

    while (size > 0) {
        uint8_t cmd = *table++;
        uint8_t len = *table++;
        if (cmd == LCD_CMD8_DELAY) {
            safe_delay_ms(len);
        } else {
            dev->device->comm->write_regn(dev, cmd, table, len);
            table += len;
            size -= len;
        }
        size -= 2;
    }
    return ESP_OK;
}

static esp_err_t st7796s_init(lcd_dev_t *dev) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);

    dev->registers.mw = ST7796S_REG_RAMWR;
    dev->registers.ec = ST7796S_REG_NOP; // Not used.
    dev->registers.ep = ST7796S_REG_NOP; // Not used.
    dev->registers.sc = ST7796S_REG_NOP; // Scroll not implemented.
    dev->registers.sp = ST7796S_REG_NOP; // Scroll not implemented.
    dev->registers.mc = ST7796S_REG_CASET;
    dev->registers.mp = ST7796S_REG_RASET;

    gpio_config_t conf = {
            .pin_bit_mask = 0,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    if (dev->config.rst_io_num != GPIO_NUM_NC) {
        conf.pin_bit_mask |= BIT64(dev->config.rst_io_num);
    }
    if (dev->config.spi.dc_io_num != GPIO_NUM_NC) {
        conf.pin_bit_mask |= BIT64(dev->config.spi.dc_io_num);
    }
    if (conf.pin_bit_mask) {
        ESP_ERROR_CHECK(gpio_config(&conf));
    }
    if (dev->config.rst_io_num != GPIO_NUM_NC) {
        ESP_ERROR_CHECK(gpio_set_level(dev->config.rst_io_num, false));
        safe_delay_ms(2);
        ESP_ERROR_CHECK(gpio_set_level(dev->config.rst_io_num, true));
        safe_delay_ms(200);
    }

    // Try to release any stuck device.
    dev->device->comm->write_regn(dev, ST7796S_REG_NOP, NULL, 0);
    dev->device->comm->write_regn(dev, ST7796S_REG_NOP, NULL, 0);
    dev->device->comm->write_regn(dev, ST7796S_REG_NOP, NULL, 0);

    ESP_ERROR_CHECK(st7796s_init_registers(dev, ST7796S_REG_VALUES, sizeof(ST7796S_REG_VALUES)));

    return ESP_OK;
}

static void st7796s_draw_pixel(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y) {
    LLOG(TAG, "[%s] color: 0x%04x (%d, %d)", __FUNCTION__, color, x, y);

    st7796s_address_set(dev, x, y, x, y);
    dev->device->comm->write_reg16(dev, dev->registers.mw, color);
}

static void st7796s_prepare_draw(lcd_dev_t *dev) {
    dev->device->comm->write_regn(dev, dev->registers.mw, NULL, 0);
}

static void st7796s_fill(lcd_dev_t *dev, uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] color: 0x%04x (%d, %d) -> (%d, %d)", __FUNCTION__, color, x1, y1, x2, y2);
    x1 = clamp(x1, 0, dev->registers.width - 1);
    x2 = clamp(x2, 0, dev->registers.width - 1);
    y1 = clamp(y1, 0, dev->registers.height - 1);
    y2 = clamp(y2, 0, dev->registers.height - 1);
    ARG_ERROR_CHECK(x1 <= x2, "x1 > x2");
    ARG_ERROR_CHECK(y1 <= y2, "y1 > y2");

    if (x1 == x2 && y1 == y2) {
        st7796s_draw_pixel(dev, color, x1, y1);
        return;
    }
    st7796s_address_set(dev, x1, y1, x2, y2);
    st7796s_prepare_draw(dev);
    size_t size_remain = (((x2 - x1) + 1) * ((y2 - y1) + 1)) * sizeof(uint16_t);
    dev->device->comm->write_data16n(dev, color, size_remain);
}

static void st7796s_draw(lcd_dev_t *dev, const uint16_t *buf, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] (%d, %d) -> (%d, %d)", __FUNCTION__, x, y, width, height);

    st7796s_address_set(dev, x, y, x + width - 1, y + height - 1);

    size_t len = width * height * sizeof(uint16_t);
    while (len > 0) {
        size_t trans_len = len > dev->buffer_len ? dev->buffer_len : len;
        for (int i = 0; i < trans_len; i += sizeof(uint16_t)) {
            dev->buffer[i] = __bswap16(buf[i]);
        }
        dev->device->comm->write_regn(dev, dev->registers.mw, (const uint8_t *) dev->buffer, trans_len);
        len -= trans_len;
    }
}

static void st7796s_clear(lcd_dev_t *dev, uint16_t color) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] color: 0x%04x", __FUNCTION__, color);

    st7796s_fill(dev, color, 0, 0, dev->registers.width - 1, dev->registers.height - 1);
}

const lcd_device_t st7796s = {
        .comm = &lcd_com_spi,
        .init = st7796s_init,
        .draw_pixel = st7796s_draw_pixel,
        .fill = st7796s_fill,
        .draw = st7796s_draw,
        .clear = st7796s_clear,
        .set_rotation = st7796s_set_rotation,
        .vertical_scroll = st7796s_vertical_scroll,
        .invert_display = st7796s_invert_display,
};
