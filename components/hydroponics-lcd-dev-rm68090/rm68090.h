#ifndef HYDROPONICS_LCD_DEV_RM68090_H
#define HYDROPONICS_LCD_DEV_RM68090_H

#include "lcd.h"

#define RM68090_ID         0x6809
#define RM68090_MAX_WIDTH  240
#define RM68090_MAX_HEIGHT 320

esp_err_t rm68090_init(lcd_dev_t *dev);

void rm68090_address_set(lcd_dev_t *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void rm68090_draw_pixel(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y);

void rm68090_prepare_draw(lcd_dev_t *dev);

void rm68090_fill(lcd_dev_t *dev, uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void rm68090_draw(lcd_dev_t *dev, const uint16_t *buf, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void rm68090_clear(lcd_dev_t *dev, uint16_t color);

esp_err_t rm68090_set_rotation(lcd_dev_t *dev, rotation_t rotation);

esp_err_t rm68090_vertical_scroll(lcd_dev_t *dev, int16_t top, int16_t scroll_lines, int16_t offset);

esp_err_t rm68090_invert_display(lcd_dev_t *dev, bool reverse);

#endif //HYDROPONICS_LCD_DEV_RM68090_H
