#ifndef HYDROPONICS_SENSORS_TEMPERATURE_H
#define HYDROPONICS_SENSORS_TEMPERATURE_H

#include "esp_err.h"

#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"

#include "context.h"

#define OWB_MAX_DEVICES 2

typedef struct {
    owb_rmt_driver_info rmt_driver_info;
    OneWireBus *owb;
    DS18B20_Info *devices[OWB_MAX_DEVICES];
    int num_devices;
    float readings[OWB_MAX_DEVICES];
    DS18B20_ERROR errors[OWB_MAX_DEVICES];
} temperature_t;

esp_err_t temperature_init(context_t *context);

esp_err_t temperature_hal_init(temperature_t *dev);

void temperature_hal_read(temperature_t *dev);

#endif //HYDROPONICS_SENSORS_TEMPERATURE_H
