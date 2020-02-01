#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "bme280.h"

#include "context.h"
#include "error.h"
#include "humidity_pressure.h"

static const char *TAG = "bme280";
static struct bme280_dev dev;

static void humidity_pressure_publish(context_t *context, struct bme280_data *comp_data) {
#ifdef BME280_FLOAT_ENABLE
    float indoor = comp_data->temperature;
    float pressure = comp_data->pressure;
    float humidity = comp_data->humidity;
#else
    float indoor = comp_data->temperature / 100.f;
    float pressure = comp_data->pressure / 100.f;
    float humidity = comp_data->humidity / 1024.f;
#endif
    ESP_LOGD(TAG, "Temp: %0.2f Pressure: %0.2f  Humidity: %0.2f", indoor, pressure, humidity);
    ESP_ERROR_CHECK(context_set_temp_indoor_humidity_pressure(context, indoor, humidity, pressure));
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
        ret = humidity_pressure_hal_read(BME280_ALL, &comp_data, &dev);
        if (ret != BME280_OK) {
            ESP_LOGE(TAG, "bme280_get_sensor_data err: %d", ret);
            break;
        }

        humidity_pressure_publish(context, &comp_data);
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_HUMIDITY_MS));
    }
    vTaskDelete(NULL);
}

esp_err_t humidity_pressure_init(context_t *context) {
    int8_t ret = humidity_pressure_hal_init(&dev);
    if (ret != BME280_OK) {
        ESP_LOGE(TAG, "error initializing BME280 err: %d", ret);
    }

    xTaskCreatePinnedToCore(humidity_pressure_task, TAG, 2048, context, 20, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
