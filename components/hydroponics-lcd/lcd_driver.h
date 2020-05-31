#ifndef HYDROPONICS_LCD_LCD_DRIVER_H
#define HYDROPONICS_LCD_LCD_DRIVER_H

#include "lcd.h"

esp_err_t lcd_driver_init(lcd_dev_t *dev);

void lcd_driver_write_data16(const lcd_dev_t *dev, uint16_t data);

void lcd_driver_write_data16n(const lcd_dev_t *dev, uint16_t data, size_t len);

void lcd_driver_write_datan(const lcd_dev_t *dev, const uint16_t *buf, size_t len);

#endif //HYDROPONICS_LCD_LCD_DRIVER_H
