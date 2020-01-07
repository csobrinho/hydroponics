#include "driver/gpio.h"
#include "esp_err.h"

#include "iot_i2s_lcd.h"
#include "i2s_lcd_com.h"

#include "error.h"

#include "i2s_lcd8.h"

#define DMA_SIZE 2000

#define WS_ON   GPIO.out_w1ts = (1 << dev->base.i2s_lcd_conf.ws_io_num)
#define RS_ON   GPIO.out_w1ts = (1 << dev->base.i2s_lcd_conf.rs_io_num)
#define RS_OFF  GPIO.out_w1tc = (1 << dev->base.i2s_lcd_conf.rs_io_num)
#define RST_ON  GPIO.out_w1ts = (1 << dev->rst_io_num)
#define RST_OFF GPIO.out_w1tc = (1 << dev->rst_io_num)

static const char *TAG = "i2s_lcd8";
static i2s_dev_t *I2S[I2S_NUM_MAX] = {&I2S0, &I2S1};

void inline i2s_lcd8_delay_us(uint8_t us) {
    ets_delay_us(us);
}

void i2s_lcd8_delay_ms(uint8_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

esp_err_t i2s_lcd8_init(i2s_lcd8_dev_t *dev) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL)
    ARG_CHECK(dev->rst_io_num != GPIO_NUM_NC, "rst is GPIO_NUM_NC")
    ARG_CHECK(dev->base.i2s_lcd_conf.data_width == 8, "data_width must be 8")

    // Setup the GPIOs as general purpose outputs.
    gpio_pad_select_gpio(dev->rst_io_num);
    const gpio_config_t conf = {
            .pin_bit_mask = BIT(dev->rst_io_num),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&conf));

    i2s_lcd8_reset(dev);

    dev->handle = iot_i2s_lcd_pin_cfg(dev->base.i2s_port, &dev->base.i2s_lcd_conf);
    if (dev->handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    dev->buffer_len = sizeof(uint32_t) * DMA_SIZE;
    dev->buffer = calloc(1, dev->buffer_len);
    if (!dev->buffer) {
        return ESP_ERR_NO_MEM;
    }

    // Fix the clock. It needs to be at least 4 for 8 bits to work properly.
    I2S[dev->base.i2s_port]->clkm_conf.clk_en = 0;
    I2S[dev->base.i2s_port]->clkm_conf.clkm_div_num = 4;
    I2S[dev->base.i2s_port]->clkm_conf.clk_en = 1;

    return ESP_OK;
}

void i2s_lcd8_reset(const i2s_lcd8_dev_t *dev) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL)

    WS_ON;
    RS_ON;
    RST_ON;
    i2s_lcd8_delay_ms(5);
    RST_OFF;
    i2s_lcd8_delay_ms(15);
    RST_ON;
    i2s_lcd8_delay_ms(15);
}

void i2s_lcd8_write_data(const i2s_lcd8_dev_t *dev, uint16_t data) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL)

    iot_i2s_lcd_write_data(dev->handle, data);
}

void i2s_lcd8_write_datan(const i2s_lcd8_dev_t *dev, const uint16_t *buf, size_t len) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL)
    ARG_ERROR_CHECK(buf != NULL, ERR_PARAM_NULL)
    ARG_ERROR_CHECK(len > 0, ERR_PARAM_LE_ZERO)

    iot_i2s_lcd_write(dev->handle, (uint16_t *) buf, len);
}

void i2s_lcd8_write_cmd(const i2s_lcd8_dev_t *dev, uint16_t cmd) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL)

    RS_OFF;
    i2s_lcd8_write_data(dev, cmd);
    RS_ON;
}

void i2s_lcd8_write_reg(const i2s_lcd8_dev_t *dev, uint16_t cmd, uint16_t data) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL)

    i2s_lcd8_write_cmd(dev, cmd);
    i2s_lcd8_write_data(dev, data);
}

esp_err_t i2s_lcd8_init_registers(const i2s_lcd8_dev_t *dev, const uint16_t *table, size_t size) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL)
    ARG_CHECK(table != NULL, ERR_PARAM_NULL)
    ARG_CHECK(size > 0, ERR_PARAM_LE_ZERO)

    while (size > 0) {
        uint16_t cmd = *table++;
        uint16_t d = *table++;
        if (cmd == I2S_LCD8_DELAY)
            i2s_lcd8_delay_ms(d);
        else {
            i2s_lcd8_write_reg(dev->handle, cmd, d);
        }
        size -= 2 * sizeof(int16_t);
    }

    return ESP_OK;
}
