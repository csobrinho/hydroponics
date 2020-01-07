#ifndef HYDROPONICS_DISPLAY_RM68090_H
#define HYDROPONICS_DISPLAY_RM68090_H

#include "i2s_lcd8.h"
#include "iot_i2s_lcd.h"

esp_err_t rm68090_init(i2s_lcd8_dev_t *dev);

void rm68090_address_set(i2s_lcd8_dev_t *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void rm68090_clear(i2s_lcd8_dev_t *dev, uint16_t color);

esp_err_t rm68090_set_rotation(i2s_lcd8_dev_t *dev, rotation_t rotation);

#endif //HYDROPONICS_DISPLAY_RM68090_H
