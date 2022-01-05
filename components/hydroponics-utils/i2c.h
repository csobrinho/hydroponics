#ifndef HYDROPONICS_UTILS_I2C_H
#define HYDROPONICS_UTILS_I2C_H

#include "driver/i2c.h"

#include "esp_err.h"

esp_err_t i2c_master_write_reg_read(i2c_port_t i2c_num, uint8_t device_address, uint8_t reg_address,
                                    const uint8_t *write_buffer, size_t write_size, uint8_t *read_buffer,
                                    size_t read_size, TickType_t ticks_to_wait);

#endif //HYDROPONICS_UTILS_I2C_H
