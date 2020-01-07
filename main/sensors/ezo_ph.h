#ifndef HYDROPONICS_DRIVERS_EZO_PH_H
#define HYDROPONICS_DRIVERS_EZO_PH_H

#include "esp_err.h"

#include "context.h"

extern ezo_sensor_t *ezo_ph;

esp_err_t ezo_ph_init(context_t *context);

esp_err_t ezo_ph_slope(float *acidPercentage, float *basePercentage);

#endif //HYDROPONICS_DRIVERS_EZO_PH_H
