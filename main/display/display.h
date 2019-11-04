#ifndef HYDROPONICS_DISPLAY_DISPLAY_H
#define HYDROPONICS_DISPLAY_DISPLAY_H

#include "driver/gpio.h"

// OLED gpio pins.
#define OLED_CLOCK GPIO_NUM_15
#define OLED_RESET GPIO_NUM_16
#define OLED_DATA  GPIO_NUM_4

esp_err_t display_init(void);

#endif //HYDROPONICS_DISPLAY_DISPLAY_H
