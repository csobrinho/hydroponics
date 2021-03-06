#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"

#include "esp_err.h"
#include "esp_log.h"

#include "bme280.h"

#include "buses.h"
#include "error.h"
#include "utils.h"

static const uint8_t BME280_ADDR = BME280_I2C_ADDR_PRIM;

static int8_t humidity_pressure_hal_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    uint8_t dev_id = *(uint8_t *) intf_ptr;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_WRITE, I2C_WRITE_ACK_CHECK);
    i2c_master_write_byte(cmd, reg_addr, I2C_WRITE_ACK_CHECK);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_READ, I2C_WRITE_ACK_CHECK);

    if (len > 1) {
        i2c_master_read(cmd, reg_data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, reg_data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    return (err == ESP_OK) ? BME280_OK : BME280_E_COMM_FAIL;
}

static int8_t humidity_pressure_hal_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    uint8_t dev_id = *(uint8_t *) intf_ptr;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_WRITE, I2C_WRITE_ACK_CHECK);

    i2c_master_write_byte(cmd, reg_addr, I2C_WRITE_ACK_CHECK);
    i2c_master_write(cmd, reg_data, len, I2C_WRITE_ACK_CHECK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    return (err == ESP_OK) ? BME280_OK : BME280_E_COMM_FAIL;
}

static void humidity_pressure_hal_delay_us(uint32_t period, void *intf_ptr) {
    ARG_UNUSED(intf_ptr);
    safe_delay_us(period);
}

int8_t humidity_pressure_hal_read(uint8_t sensor_comp, struct bme280_data *comp_data, struct bme280_dev *dev) {
    return bme280_get_sensor_data(sensor_comp, comp_data, dev);
}

int8_t humidity_pressure_hal_init(struct bme280_dev *dev) {
    dev->intf_ptr = (void *) &BME280_ADDR;
    dev->intf = BME280_I2C_INTF;
    dev->read = humidity_pressure_hal_i2c_read;
    dev->write = humidity_pressure_hal_i2c_write;
    dev->delay_us = humidity_pressure_hal_delay_us;

    int8_t ret = bme280_init(dev);
    uint8_t settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
    dev->settings.osr_h = BME280_OVERSAMPLING_1X;
    dev->settings.osr_p = BME280_OVERSAMPLING_16X;
    dev->settings.osr_t = BME280_OVERSAMPLING_2X;
    dev->settings.filter = BME280_FILTER_COEFF_16;

#ifdef HUMIDITY_FORCED_MODE
    /* Initialize forced mode. Recommended mode of operation: Indoor navigation */
    ret += bme280_set_sensor_settings(settings_sel, dev);
#else
    /* Initialize in normal mode. Recommended mode of operation: Indoor navigation */
    dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

    settings_sel |= BME280_STANDBY_SEL;
    ret += bme280_set_sensor_settings(settings_sel, dev);
    ret += bme280_set_sensor_mode(BME280_NORMAL_MODE, dev);
#endif
    return ret;
}
