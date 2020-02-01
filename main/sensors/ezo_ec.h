#ifndef HYDROPONICS_DRIVERS_EZO_EC_H
#define HYDROPONICS_DRIVERS_EZO_EC_H

#include "esp_err.h"

#include "context.h"
#include "driver/ezo.h"

esp_err_t ezo_ec_init(context_t *context);

#endif //HYDROPONICS_DRIVERS_EZO_EC_H
