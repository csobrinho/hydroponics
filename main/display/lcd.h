#ifndef HYDROPONICS_DISPLAY_LCD_H
#define HYDROPONICS_DISPLAY_LCD_H

#include "esp_err.h"

#include "context.h"

esp_err_t lcd_init(context_t *context);

void lcd_clear(uint16_t color);

void lcd_fill(uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void lcd_hline(uint16_t color, uint16_t x, uint16_t y, uint16_t width);

void lcd_vline(uint16_t color, uint16_t x, uint16_t y, uint16_t height);

void lcd_line(uint16_t color, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

void lcd_rect(uint16_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

uint16_t lcd_width();

uint16_t lcd_height();

#endif //HYDROPONICS_DISPLAY_LCD_H
