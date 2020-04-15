#include "esp_err.h"

#include "bme280.h"
#include "simulation.h"

int8_t humidity_pressure_hal_read(uint8_t sensor_comp, struct bme280_data *comp_data, struct bme280_dev *dev) {
    comp_data->temperature = WITH_THRESHOLD(21, 0.1);
    comp_data->pressure = WITH_THRESHOLD(10000, 100);
    comp_data->humidity = WITH_THRESHOLD(50, 1);
    return BME280_OK;
}

esp_err_t humidity_pressure_hal_init(struct bme280_dev *dev) {
    return ESP_OK;
}
