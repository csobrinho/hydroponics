#ifndef HYDROPONICS_SENSORS_HUMIDITY_PRESSURE_H
#define HYDROPONICS_SENSORS_HUMIDITY_PRESSURE_H

#include "bme280.h"
#include "context.h"

esp_err_t humidity_pressure_init(context_t *context);

int8_t humidity_pressure_hal_init(struct bme280_dev *dev);

int8_t humidity_pressure_hal_read(uint8_t sensor_comp, struct bme280_data *comp_data, struct bme280_dev *dev);

#endif //HYDROPONICS_SENSORS_HUMIDITY_PRESSURE_H
