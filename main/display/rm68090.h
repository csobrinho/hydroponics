#ifndef HYDROPONICS_DISPLAY_RM68090_H
#define HYDROPONICS_DISPLAY_RM68090_H

#include "i2s_lcd8.h"
#include "iot_i2s_lcd.h"

esp_err_t rm68090_init(i2s_lcd8_dev_t *dev);

void rm68090_address_set(i2s_lcd8_dev_t *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void rm68090_draw_pixel(i2s_lcd8_dev_t *dev, uint16_t color, uint16_t x, uint16_t y);

void rm68090_prepare_draw(i2s_lcd8_dev_t *dev);

void rm68090_fill(i2s_lcd8_dev_t *dev, uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void rm68090_draw(i2s_lcd8_dev_t *dev, const uint16_t *buf, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void rm68090_clear(i2s_lcd8_dev_t *dev, uint16_t color);

esp_err_t rm68090_set_rotation(i2s_lcd8_dev_t *dev, rotation_t rotation);

esp_err_t rm68090_vertical_scroll(const i2s_lcd8_dev_t *dev, int16_t top, int16_t scroll_lines, int16_t offset);

esp_err_t rm68090_invert_display(i2s_lcd8_dev_t *dev, bool reverse);

#endif //HYDROPONICS_DISPLAY_RM68090_H
