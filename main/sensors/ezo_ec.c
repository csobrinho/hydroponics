#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "driver/ezo.h"

static const char *TAG = "ezo_ec";
static ezo_sensor_t eca = {
        .probe = "CS150",
        .desc = "eca",
        .address = CONFIG_ESP_SENSOR_EC_TANK_A_ADDR, /*!< Slave address for Atlas EZO EC module for tank A. */
        .delay_ms = EZO_DELAY_MS_SHORT,
        .delay_read_ms = EZO_DELAY_MS_SLOW,
        .delay_calibration_ms = EZO_DELAY_MS_SLOWEST,
        .calibration = EZO_CALIBRATION_MODE_TWO_POINTS,
#ifdef CONFIG_ESP_SENSOR_SIMULATE
        .simulate = 1500.f,
        .threshold = 15.f,
#endif
};
static ezo_sensor_t ecb = {
        .probe = "CS150",
        .desc = "ecb",
        .address = CONFIG_ESP_SENSOR_EC_TANK_B_ADDR, /*!< Slave address for Atlas EZO EC module for tank B. */
        .delay_ms = EZO_DELAY_MS_SHORT,
        .delay_read_ms = EZO_DELAY_MS_SLOW,
        .delay_calibration_ms = EZO_DELAY_MS_SLOWEST,
        .calibration = EZO_CALIBRATION_MODE_TWO_POINTS,
#ifdef CONFIG_ESP_SENSOR_SIMULATE
        .simulate = 1900.f,
        .threshold = 15.f,
#endif
};

static void ezo_ec_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);
    ESP_ERROR_CHECK(ezo_init(&eca));
    ESP_ERROR_CHECK(ezo_init(&ecb));

    while (true) {
        if (eca.pause | ecb.pause) {
            vTaskDelay(pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_EC_MS));
            continue;
        }
        TickType_t last_wake_time = xTaskGetTickCount();
        float temp = context->sensors.temp.probe;
        ESP_ERROR_CHECK(context_set_ec(context, 0, ezo_read_and_print(&eca, temp, 'A', 0, "uS/cm")));
#if CONFIG_ESP_SENSOR_TANKS == 2
        ESP_ERROR_CHECK(context_set_ec(context, 1, ezo_read_and_print(&ecb, temp, 'B', 0, "uS/cm")));
#endif
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_EC_MS));
    }
}

esp_err_t ezo_ec_init(context_t *context) {
    xTaskCreatePinnedToCore(ezo_ec_task, "ezo_ec", 3072, context, tskIDLE_PRIORITY + 7, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
