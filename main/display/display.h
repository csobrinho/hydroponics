#ifndef HYDROPONICS_DISPLAY_DISPLAY_H
#define HYDROPONICS_DISPLAY_DISPLAY_H

#include "driver/gpio.h"

esp_err_t display_init(void);

void display_draw_temp_humidity(float temp, float humidity);

#endif //HYDROPONICS_DISPLAY_DISPLAY_H
