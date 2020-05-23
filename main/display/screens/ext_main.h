#ifndef HYDROPONICS_DISPLAY_SCREENS_EXT_MAIN_H
#define HYDROPONICS_DISPLAY_SCREENS_EXT_MAIN_H

#include "esp_err.h"

#include "ucg.h"

#include "context.h"
#include "lcd.h"

esp_err_t ext_main_init(context_t *context);

esp_err_t ext_main_draw(context_t *context, lcd_dev_t *dev, ucg_t *ucg);

#endif //HYDROPONICS_DISPLAY_SCREENS_EXT_MAIN_H
