#include "driver/gpio.h"

#include "esp_err.h"

#include "error.h"
#include "lcd.h"
#include "lcd_driver.h"

static const char *TAG = "lcd_direct";
static uint16_t data_mask_clear = 0;
static uint16_t data_mask_set[256] = {0};

static uint16_t lcd_driver_read_id(const lcd_dev_t *dev);

static esp_err_t lcd_driver_set_direction(const lcd_dev_t *dev, gpio_mode_t mode);

esp_err_t lcd_driver_init(lcd_dev_t *dev) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    ARG_CHECK(dev->config.data_width == 8, "data_width must be 8");
    ARG_CHECK(dev->config.ws_io_num != GPIO_NUM_NC, "ws_io_num is GPIO_NUM_NC");
    ARG_CHECK(dev->config.rs_io_num != GPIO_NUM_NC, "rs_io_num is GPIO_NUM_NC");

    // No need for any buffer.
    dev->buffer_len = 0;
    dev->buffer = NULL;

    // Setup the GPIOs as general purpose outputs.
    uint64_t mask = 0;

    gpio_pad_select_gpio(dev->config.ws_io_num);
    mask |= BIT64(dev->config.ws_io_num);

    gpio_pad_select_gpio(dev->config.rs_io_num);
    mask |= BIT64(dev->config.rs_io_num);

    if (dev->config.rst_io_num != GPIO_NUM_NC) {
        gpio_pad_select_gpio(dev->config.rst_io_num);
        mask |= BIT64(dev->config.rst_io_num);
    }

    if (dev->config.rd_io_num != GPIO_NUM_NC) {
        gpio_pad_select_gpio(dev->config.rd_io_num);
        mask |= BIT64(dev->config.rd_io_num);
    }
    for (int i = 0; i < dev->config.data_width; ++i) {
        gpio_num_t pin = dev->config.data_io_num[i];
        ARG_CHECK(pin != GPIO_NUM_NC && pin <= GPIO_NUM_15, "data_io_num must be between [0, 15]");
        uint16_t bit = BIT(pin);
        data_mask_clear |= bit;
        // Setup the quick data mask.
        for (int idx = 0; idx < 256; idx++) {
            if (idx & BIT(i)) {
                data_mask_set[idx] |= bit;
            }
        }
    }
    const gpio_config_t conf = {
            .pin_bit_mask = mask,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&conf));
    ESP_ERROR_CHECK(lcd_driver_set_direction(dev, GPIO_MODE_OUTPUT));

    if (dev->config.rd_io_num != GPIO_NUM_NC) {
        lcd_driver_read_id(dev);
    }
    return ESP_OK;
}

static esp_err_t lcd_driver_set_direction(const lcd_dev_t *dev, gpio_mode_t mode) {
    for (int i = 0; i < dev->config.data_width; ++i) {
        gpio_num_t pin = dev->config.data_io_num[i];
        gpio_pad_select_gpio(pin);
        ESP_ERROR_CHECK(gpio_set_pull_mode(pin, GPIO_FLOATING));
        ESP_ERROR_CHECK(gpio_set_intr_type(pin, GPIO_INTR_DISABLE));
        ESP_ERROR_CHECK(gpio_set_direction(pin, mode));
    }
    return ESP_OK;
}

static inline uint16_t lcd_driver_read_data8(const lcd_dev_t *dev) {
    ARG_ERROR_CHECK(dev->config.rd_io_num != GPIO_NUM_NC, "rd_io_num is GPIO_NUM_NC");
    ARG_ERROR_CHECK(dev->handle == NULL, "Device already initialized, cannot read anymore");

    ESP_ERROR_CHECK(lcd_driver_set_direction(dev, GPIO_MODE_INPUT));
    RD_STROBE(dev);
    uint8_t data = 0;
    for (int i = 0; i < dev->config.data_width; ++i) {
        data |= gpio_get_level(dev->config.data_io_num[i]) << i;
    }
    RD_IDLE(dev);
    ESP_ERROR_CHECK(lcd_driver_set_direction(dev, GPIO_MODE_OUTPUT));
    return data;
}

static inline uint16_t lcd_driver_read_data16(const lcd_dev_t *dev) {
    return (lcd_driver_read_data8(dev) << 8) | lcd_driver_read_data8(dev);
}

static uint16_t lcd_driver_read_cmd(const lcd_dev_t *dev, uint16_t cmd) {
    ARG_ERROR_CHECK(dev->config.rd_io_num != GPIO_NUM_NC, "rd_io_num is GPIO_NUM_NC");
    ARG_ERROR_CHECK(dev->handle == NULL, "Device already initialized, cannot read anymore");

    lcd_write_cmd(dev, cmd);
    uint16_t ret = lcd_driver_read_data16(dev);
    RD_IDLE(dev);
    return ret;
}

static uint16_t lcd_driver_read_id(const lcd_dev_t *dev) {
    uint16_t id = lcd_driver_read_cmd(dev, 0x0000);
    ESP_LOGI(TAG, "Detected lcd id: 0x%04x", id);
    return id;
}

static inline void lcd_driver_write_data8(const lcd_dev_t *dev, uint8_t data) {
    GPIO.out_w1tc = data_mask_clear;
    GPIO.out_w1ts = data_mask_set[data];
    WR_STROBE(dev);
}

inline void lcd_driver_write_data16(const lcd_dev_t *dev, uint16_t data) {
    lcd_driver_write_data8(dev, data >> 8);
    lcd_driver_write_data8(dev, data);
}

void lcd_driver_write_data16n(const lcd_dev_t *dev, uint16_t data, size_t len) {
    ARG_ERROR_CHECK(len > 0, ERR_PARAM_LE_ZERO);
    uint16_t high_mask = data_mask_set[data >> 8];
    uint16_t low_mask = data_mask_set[data & 0xff];
    // len represents the len of bytes, not uint16_t.
    for (int i = 0; i < len / sizeof(uint16_t); ++i) {
        GPIO.out_w1tc = data_mask_clear;
        GPIO.out_w1ts = high_mask;
        WR_STROBE(dev);
        GPIO.out_w1tc = data_mask_clear;
        GPIO.out_w1ts = low_mask;
        WR_STROBE(dev);
    }
}

inline void lcd_driver_write_datan(const lcd_dev_t *dev, const uint16_t *buf, size_t len) {
    ARG_ERROR_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_ERROR_CHECK(len > 0, ERR_PARAM_LE_ZERO);

    LLOG(TAG, "[%s] buf: %p len: %2d", __FUNCTION__, buf, len);

    // len represents the len of bytes, not uint16_t.
    for (int i = 0; i < len / sizeof(uint16_t); ++i) {
        lcd_driver_write_data16(dev, *buf++);
    }
}
