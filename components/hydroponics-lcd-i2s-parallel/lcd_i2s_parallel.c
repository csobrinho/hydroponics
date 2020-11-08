#include "driver/gpio.h"

#include "esp_err.h"

#include "i2s_parallel.h"

#include "error.h"
#include "lcd.h"
#include "utils.h"

#ifdef LCD_CONTROL_LOW
#define WR_IDLE(d)      GPIO.out_w1ts = (1 << d->config.ws_io_num)
#define WR_ACTIVE(d)    GPIO.out_w1tc = (1 << d->config.ws_io_num)
#else
#define WR_IDLE(d)      GPIO.out1_w1ts.data = (1 << (d->config.parallel.ws_io_num - 32))
#define WR_ACTIVE(d)    GPIO.out1_w1tc.data = (1 << (d->config.parallel.ws_io_num - 32))
#endif
#define WR_STROBE(d)    WR_ACTIVE(d); WR_IDLE(d)
#define RD_IDLE(d)      gpio_set_level(d->config.parallel.rd_io_num, 1)
#define RD_ACTIVE(d)    gpio_set_level(d->config.parallel.rd_io_num, 0)
#define RD_STROBE(d)    RD_IDLE(d); RD_ACTIVE(d); RD_ACTIVE(d); RD_ACTIVE(d)
#define CD_DATA(d)      gpio_set_level(d->config.parallel.rs_io_num, 1)
#define CD_COMMAND(d)   gpio_set_level(d->config.parallel.rs_io_num, 0)
#define RESET_IDLE(d)   gpio_set_level(d->config.rst_io_num, 1)
#define RESET_ACTIVE(d) gpio_set_level(d->config.rst_io_num, 0)

static const char *TAG = "lcd_i2s_p";

static esp_err_t lcd_i2s_parallel_init(lcd_dev_t *dev) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    ARG_CHECK(dev->config.parallel.data_width == 8, "data_width must be 8");
    ARG_CHECK(dev->config.parallel.ws_io_num != GPIO_NUM_NC, "ws_io_num is GPIO_NUM_NC");
    ARG_CHECK(dev->config.parallel.rs_io_num != GPIO_NUM_NC, "rs_io_num is GPIO_NUM_NC");

    // No need for any buffer.
    dev->buffer_len = 0;
    dev->buffer = NULL;

    dev->handle = NULL;

    // Setup the GPIOs as general purpose outputs.
    uint64_t mask = 0;
    if (dev->config.rst_io_num != GPIO_NUM_NC) {
        gpio_pad_select_gpio(dev->config.rst_io_num);
        mask |= BIT64(dev->config.rst_io_num);
    }
    if (dev->config.parallel.rd_io_num != GPIO_NUM_NC) {
        gpio_pad_select_gpio(dev->config.parallel.rd_io_num);
        mask |= BIT64(dev->config.parallel.rd_io_num);
    }
    gpio_pad_select_gpio(dev->config.parallel.rs_io_num);
    mask |= BIT64(dev->config.parallel.rs_io_num);
    const gpio_config_t conf = {
            .pin_bit_mask = mask,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&conf));

    i2s_parallel_pin_config_t *lcd_conf = calloc(1, sizeof(i2s_parallel_pin_config_t));
    lcd_conf->bit_width = dev->config.parallel.data_width;
    for (int i = 0; i < lcd_conf->bit_width; ++i) {
        ARG_CHECK(dev->config.parallel.data_io_num[i] != GPIO_NUM_NC,
                  "dev->config.parallel.data_io_num is GPIO_NUM_NC");
        lcd_conf->data[i] = dev->config.parallel.data_io_num[i];
    }
    lcd_conf->pin_clk = dev->config.parallel.ws_io_num;
    i2s_parallel_init(lcd_conf, 2);
    return ESP_OK;
}

static esp_err_t lcd_i2s_parallel_reset(lcd_dev_t *dev) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    if (dev->config.parallel.rd_io_num != GPIO_NUM_NC) {
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

static void lcd_i2s_parallel_write_data16(const lcd_dev_t *dev, uint16_t data) {
    ARG_UNUSED(dev);
    i2s_parallel_write_data16n(data, sizeof(uint16_t));
}

static void lcd_i2s_parallel_write_data8(const lcd_dev_t *dev, uint8_t data) {
    lcd_i2s_parallel_write_data16(dev, data);
}

static void lcd_i2s_parallel_write_data16n(const lcd_dev_t *dev, uint16_t data, size_t len) {
    ARG_UNUSED(dev);
    i2s_parallel_write_data16n(data, len);
}

static void lcd_i2s_parallel_write_datan(const lcd_dev_t *dev, const uint16_t *buf, size_t len) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL);
    ARG_ERROR_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_ERROR_CHECK(len > 0, ERR_PARAM_LE_ZERO);
    LLOG(TAG, "[%s] buf: %p len: %2d", __FUNCTION__, buf, len);
    i2s_parallel_write_data((const uint8_t *) buf, len);
}

static void lcd_i2s_parallel_write_regn(const lcd_dev_t *dev, uint16_t cmd, const uint8_t *buf, size_t len) {
    CD_COMMAND(dev);
    lcd_i2s_parallel_write_data16(dev, cmd);
    CD_DATA(dev);
    if (len > 0) {
        lcd_i2s_parallel_write_datan(dev, (const uint16_t *) buf, len);
    }
}

static void lcd_i2s_parallel_write_reg16(const lcd_dev_t *dev, uint16_t cmd, uint16_t data) {
    lcd_i2s_parallel_write_regn(dev, cmd, (const uint8_t *) &data, sizeof(uint16_t));
}

static void lcd_i2s_parallel_write_reg8(const lcd_dev_t *dev, uint16_t cmd, uint8_t data) {
    lcd_i2s_parallel_write_reg16(dev, cmd, data);
}

const lcd_com_t lcd_i2s_parallel = {
        .init = lcd_i2s_parallel_init,
        .reset = lcd_i2s_parallel_reset,
        .read_reg8 = NULL, // lcd_i2s_parallel_read_reg8,
        .read_reg16 = NULL, // lcd_i2s_parallel_read_reg16,
        .read_regn = NULL, // lcd_i2s_parallel_read_regn,
        .write_reg8 = lcd_i2s_parallel_write_reg8,
        .write_reg16 = lcd_i2s_parallel_write_reg16,
        .write_regn = lcd_i2s_parallel_write_regn,
        .write_data8 = lcd_i2s_parallel_write_data8,
        .write_data16  = lcd_i2s_parallel_write_data16,
        .write_data16n  = lcd_i2s_parallel_write_data16n,
};
