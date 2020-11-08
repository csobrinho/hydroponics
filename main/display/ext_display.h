#ifndef HYDROPONICS_DISPLAY_EXT_DISPLAY_H
#define HYDROPONICS_DISPLAY_EXT_DISPLAY_H

#include "esp_err.h"

#include "context.h"
#include "lcd.h"

esp_err_t ext_display_init(context_t *context);

extern lcd_dev_t dev;

#endif //HYDROPONICS_DISPLAY_EXT_DISPLAY_H
