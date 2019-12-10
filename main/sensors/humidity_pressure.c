#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"

#include "esp_err.h"
#include "esp_log.h"

#include "bme280.h"

#include "buses.h"
#include "context.h"
#include "error.h"
#include "display/display.h"

static const char *TAG = "bme280";
static struct bme280_dev dev;

static int8_t hal_bme280_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_WRITE, I2C_WRITE_ACK_CHECK);
    i2c_master_write_byte(cmd, reg_addr, I2C_WRITE_ACK_CHECK);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_READ, I2C_WRITE_ACK_CHECK);

    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    return (err == ESP_OK) ? BME280_OK : BME280_E_COMM_FAIL;
}

static int8_t hal_bme280_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_WRITE, I2C_WRITE_ACK_CHECK);

    i2c_master_write_byte(cmd, reg_addr, I2C_WRITE_ACK_CHECK);
    i2c_master_write(cmd, data, len, I2C_WRITE_ACK_CHECK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    return (err == ESP_OK) ? BME280_OK : BME280_E_COMM_FAIL;
}

static void hal_bme280_delay_ms(uint32_t period) {
    vTaskDelay(pdMS_TO_TICKS(period));
}

static void publish(context_t *context, struct bme280_data *comp_data) {
#ifdef BME280_FLOAT_ENABLE
    context->sensors.temp.indoor = comp_data->temperature;
    context->sensors.humidity = comp_data->humidity;
    context->sensors.pressure = comp_data->pressure;
#else
    context->sensors.temp.indoor = comp_data->temperature / 100.f;
    context->sensors.humidity = comp_data->humidity / 1024.f;
    context->sensors.pressure = comp_data->pressure / 100.f;
#endif
    ESP_LOGI(TAG, "Temp: %0.2f Pressure: %0.2f  Humidity: %0.2f", context->sensors.temp.indoor,
             context->sensors.pressure, context->sensors.humidity);

    // FIXME: Add event group dispatch.
    display_draw_temp_humidity(context);
}

static void humidity_pressure_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    struct bme280_data comp_data;
    while (1) {
        TickType_t last_wake_time = xTaskGetTickCount();

        int8_t ret;
#ifdef HUMIDITY_FORCED_MODE
        ret = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
        if (ret != BME280_OK) {
            ESP_LOGE(TAG, "bme280_set_sensor_mode err: %d", ret);
            break;
        }
        /* Wait for the measurement to complete and print data @25Hz */
        dev.delay_ms(70);
#endif
        ret = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
        if (ret != BME280_OK) {
            ESP_LOGE(TAG, "bme280_get_sensor_data err: %d", ret);
            break;
        }

        publish(context, &comp_data);
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_HUMIDITY_MS));
    }
    vTaskDelete(NULL);
}

esp_err_t humidity_pressure_init(context_t *context) {
    dev.dev_id = BME280_I2C_ADDR_PRIM;
    dev.intf = BME280_I2C_INTF;
    dev.read = hal_bme280_i2c_read;
    dev.write = hal_bme280_i2c_write;
    dev.delay_ms = hal_bme280_delay_ms;

    int8_t ret = bme280_init(&dev);
    uint8_t settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    dev.settings.filter = BME280_FILTER_COEFF_16;

#ifdef HUMIDITY_FORCED_MODE
    /* Initialize forced mode. Recommended mode of operation: Indoor navigation */
    ret += bme280_set_sensor_settings(settings_sel, &dev);
#else
    /* Initialize in normal mode. Recommended mode of operation: Indoor navigation */
    dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

    settings_sel |= BME280_STANDBY_SEL;
    ret += bme280_set_sensor_settings(settings_sel, &dev);
    ret += bme280_set_sensor_mode(BME280_NORMAL_MODE, &dev);
#endif
    if (ret != BME280_OK) {
        ESP_LOGE(TAG, "error initializing BME280 err: %d", ret);
    }

    xTaskCreatePinnedToCore(humidity_pressure_task, TAG, 2048, context, 20, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
