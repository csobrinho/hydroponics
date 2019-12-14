#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "driver/ezo.h"

#define EZO_EC_ADDR 0x64    /*!< Slave address for Atlas EZO EC module. */
#define SAMPLE_PERIOD 1000  /*!< Reading sample period. */

static const char *TAG = "ezo_ec";
static ezo_sensor_t ec = {
        .probe = "CS150",
        .address = EZO_EC_ADDR,
        .delay_ms = EZO_DELAY_MS_SHORT,
        .delay_read_ms = EZO_DELAY_MS_SLOW,
        .delay_calibration_ms = EZO_DELAY_MS_SLOWEST,
        .calibration = EZO_CALIBRATION_STEP_LOW | EZO_CALIBRATION_STEP_HIGH,
};

static void ezo_ec_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);
    ESP_ERROR_CHECK(ezo_init(&ec));

    float last_temp_water = 25.0f;
    float value;
    while (1) {
        TickType_t last_wake_time = xTaskGetTickCount();

        float temp_water = context->sensors.temp.water;
        if (last_temp_water != temp_water && CONTEXT_VALUE_IS_VALID(temp_water)) {
            last_temp_water = temp_water;
            ESP_ERROR_CHECK(ezo_read_temperature(&ec, &value, last_temp_water));
        } else {
            ESP_ERROR_CHECK(ezo_read(&ec, &value));
        }
        ESP_LOGI(TAG, "EC %.2f uS/cm", context->sensors.ec.value);
        ESP_ERROR_CHECK(context_set_ec(context, value));

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SAMPLE_PERIOD));
    }
}

esp_err_t ezo_ec_init(context_t *context) {
    xTaskCreatePinnedToCore(ezo_ec_task, "ezo_ec", 2048, context, 10, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
