#include <string.h>

#include "driver/i2c.h"

#include "esp_err.h"

#include "error.h"
#include "i2c.h"

static const char *const TAG = "i2c";

esp_err_t i2c_master_write_reg_read(i2c_port_t i2c_num, uint8_t device_address, uint8_t reg_address,
                                    const uint8_t *const write_buffer, size_t write_size, uint8_t *read_buffer,
                                    size_t read_size, TickType_t ticks_to_wait) {
    uint8_t buf[1 + write_size];
    buf[0] = reg_address;
    if (write_size > 0) {
        ARG_CHECK(write_buffer != NULL, ERR_PARAM_NULL);
        memcpy(&buf[1], write_buffer, write_size);
    }
    if (read_size > 0) {
        ARG_CHECK(read_buffer != NULL, ERR_PARAM_NULL);
        return i2c_master_write_read_device(i2c_num, device_address, buf, 1 + write_size, read_buffer, read_size,
                                            ticks_to_wait);
    } else {
        return i2c_master_write_to_device(i2c_num, device_address, buf, 1 + write_size, ticks_to_wait);
    }
}
