#ifndef HYDROPONICS_DRIVERS_EZO_EC_H
#define HYDROPONICS_DRIVERS_EZO_EC_H

#include "esp_err.h"

extern float ec_value;

esp_err_t ezo_ec_init(void);

#endif //HYDROPONICS_DRIVERS_EZO_EC_H
