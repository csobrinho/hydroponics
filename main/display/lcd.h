#ifndef HYDROPONICS_DISPLAY_LCD_H
#define HYDROPONICS_DISPLAY_LCD_H

#include "esp_err.h"

esp_err_t lcd_init(void);

void lcd_clear(uint16_t color);

#endif //HYDROPONICS_DISPLAY_LCD_H
