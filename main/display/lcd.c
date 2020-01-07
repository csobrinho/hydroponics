#include "esp_err.h"
#include "esp_log.h"

#include "buses.h"
#include "rm68090.h"

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