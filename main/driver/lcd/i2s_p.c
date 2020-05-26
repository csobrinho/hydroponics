#include "driver/gpio.h"

#include "esp_err.h"

#include "i2s_parallel.h"

#include "error.h"
#include "i2s_p.h"
#include "lcd.h"

static const char *TAG = "lcd_i2s_p";

esp_err_t lcd_i2s_parallel_init(lcd_dev_t *dev) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    ARG_CHECK(dev->config.data_width == 8, "data_width must be 8");
    ARG_CHECK(dev->config.ws_io_num != GPIO_NUM_NC, "ws_io_num is GPIO_NUM_NC");
    ARG_CHECK(dev->config.rs_io_num != GPIO_NUM_NC, "rs_io_num is GPIO_NUM_NC");

    dev->handle = NULL;

    // Setup the GPIOs as general purpose outputs.
    uint64_t mask = 0;
    if (dev->config.rst_io_num != GPIO_NUM_NC) {
        gpio_pad_select_gpio(dev->config.rst_io_num);
        mask |= BIT64(dev->config.rst_io_num);
    }
    if (dev->config.rd_io_num != GPIO_NUM_NC) {
        gpio_pad_select_gpio(dev->config.rd_io_num);
        mask |= BIT64(dev->config.rd_io_num);
    }
    gpio_pad_select_gpio(dev->config.rs_io_num);
    mask |= BIT64(dev->config.rs_io_num);
    const gpio_config_t conf = {
            .pin_bit_mask = mask,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&conf));

    i2s_parallel_pin_config_t *lcd_conf = calloc(1, sizeof(i2s_parallel_pin_config_t));
    lcd_conf->bit_width = dev->config.data_width;
    for (int i = 0; i < lcd_conf->bit_width; ++i) {
        ARG_CHECK(dev->config.data_io_num[i] != GPIO_NUM_NC, "dev->config.data_io_num is GPIO_NUM_NC");
        lcd_conf->data[i] = dev->config.data_io_num[i];
    }
    lcd_conf->pin_clk = dev->config.ws_io_num;
    i2s_parallel_init(lcd_conf, 2);
    return ESP_OK;
}

inline void lcd_i2s_parallel_write_data16(const lcd_dev_t *dev, uint16_t data) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL);
    i2s_parallel_write_data16n(data, sizeof(uint16_t));
}

inline void lcd_i2s_parallel_write_data16n(const lcd_dev_t *dev, uint16_t data, size_t len) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL);
    i2s_parallel_write_data16n(data, len);
}

inline void lcd_i2s_parallel_write_datan(const lcd_dev_t *dev, const uint16_t *buf, size_t len) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL);
    ARG_ERROR_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_ERROR_CHECK(len > 0, ERR_PARAM_LE_ZERO);
    LLOG(TAG, "[%s] buf: %p len: %2d", __FUNCTION__, buf, len);
    i2s_parallel_write_data((const uint8_t *) buf, len);
}
