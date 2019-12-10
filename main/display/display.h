#ifndef HYDROPONICS_DISPLAY_DISPLAY_H
#define HYDROPONICS_DISPLAY_DISPLAY_H

#include "driver/gpio.h"

#include "context.h"

esp_err_t display_init(void);

esp_err_t display_draw_temp_humidity(context_t *context);

#endif //HYDROPONICS_DISPLAY_DISPLAY_H
