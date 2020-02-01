#ifndef HYDROPONICS_SENSORS_EZO_RTD_H
#define HYDROPONICS_SENSORS_EZO_RTD_H

#include "esp_err.h"

#include "context.h"

extern ezo_sensor_t *ezo_rtd;

esp_err_t ezo_rtd_init(context_t *context);

#endif //HYDROPONICS_SENSORS_EZO_RTD_H
