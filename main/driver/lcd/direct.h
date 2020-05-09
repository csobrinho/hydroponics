#ifndef HYDROPONICS_DRIVER_LCD_DIRECT_H
#define HYDROPONICS_DRIVER_LCD_DIRECT_H

#include "lcd.h"

esp_err_t lcd_direct_init(lcd_dev_t *dev);

void lcd_direct_write_data16(const lcd_dev_t *dev, uint16_t data);

void lcd_direct_write_datan(const lcd_dev_t *dev, const uint16_t *buf, size_t len);

#endif //HYDROPONICS_DRIVER_LCD_DIRECT_H
