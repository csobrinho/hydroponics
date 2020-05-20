#ifndef HYDROPONICS_DRIVER_LCD_I2S_P_H
#define HYDROPONICS_DRIVER_LCD_I2S_P_H

#include "lcd.h"

esp_err_t lcd_i2s_parallel_init(lcd_dev_t *dev);

void lcd_i2s_parallel_write_data16(const lcd_dev_t *dev, uint16_t data);

void lcd_i2s_parallel_write_data16n(const lcd_dev_t *dev, uint16_t data, size_t len);

void lcd_i2s_parallel_write_datan(const lcd_dev_t *dev, const uint16_t *buf, size_t len);

#endif //HYDROPONICS_DRIVER_LCD_I2S_P_H
