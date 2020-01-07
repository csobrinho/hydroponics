#ifndef HYDROPONICS_DRIVER_I2S_LCD8_H
#define HYDROPONICS_DRIVER_I2S_LCD8_H

#include "iot_i2s_lcd.h"

#define I2S_LCD8_DELAY 0xffff

typedef enum {
    LCD_COLOR_BLACK = 0x0000,
    LCD_COLOR_BLUE = 0x001F,
    LCD_COLOR_RED = 0xF800,
    LCD_COLOR_GREEN = 0x07E0,
    LCD_COLOR_CYAN = 0x07FF,
    LCD_COLOR_MAGENTA = 0xF81F,
    LCD_COLOR_YELLOW = 0xFFE0,
    LCD_COLOR_WHITE = 0xFFFF,
} lcd_color_t;

typedef enum {
    ROTATION_PORTRAIT = 0,
    ROTATION_LANDSCAPE = 1,
    ROTATION_PORTRAIT_REV = 2,
    ROTATION_LANDSCAPE_REV = 3,
} rotation_t;

typedef struct {
    i2s_lcd_t base;
    i2s_lcd_handle_t handle;
    gpio_num_t rst_io_num;
    uint16_t *buffer;
    size_t buffer_len;
    uint16_t width;
    uint16_t height;
    rotation_t rotation;
} i2s_lcd8_dev_t;

esp_err_t i2s_lcd8_init(i2s_lcd8_dev_t *dev);

void i2s_lcd8_reset(const i2s_lcd8_dev_t *dev);

void i2s_lcd8_delay_us(uint8_t us);

void i2s_lcd8_delay_ms(uint8_t ms);

void i2s_lcd8_write_data(const i2s_lcd8_dev_t *dev, uint16_t data);

void i2s_lcd8_write_datan(const i2s_lcd8_dev_t *dev, const uint16_t *buf, size_t len);

void i2s_lcd8_write_cmd(const i2s_lcd8_dev_t *dev, uint16_t cmd);

void i2s_lcd8_write_reg(const i2s_lcd8_dev_t *dev, uint16_t cmd, uint16_t data);

esp_err_t i2s_lcd8_init_registers(const i2s_lcd8_dev_t *dev, const uint16_t *table, size_t size);

#endif //HYDROPONICS_DRIVER_I2S_LCD8_H
