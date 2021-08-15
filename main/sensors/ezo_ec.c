#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "driver/ezo.h"

static const char *const TAG = "ezo_ec";
static ezo_sensor_t ec = {
        .probe = "CS150",
        .desc = "ec",
        .address = CONFIG_ESP_SENSOR_EC_ADDR, /*!< I2C address for Atlas EZO EC module. */
        .delay_ms = EZO_DELAY_MS_SHORT,
        .delay_read_ms = EZO_DELAY_MS_SLOW,
        .delay_calibration_ms = EZO_DELAY_MS_SLOWEST,
        .calibration = EZO_CALIBRATION_MODE_TWO_POINTS,
#ifdef CONFIG_ESP_SENSOR_SIMULATE
        .simulate = 1500.f,
        .threshold = 15.f,
#endif
};

static void ezo_ec_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);
    // Give it a little time to initialize.
    vTaskDelay(pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_EC_MS));
    ESP_ERROR_CHECK(ezo_init(&ec));

    while (true) {
        if (ec.pause) {
            vTaskDelay(pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_EC_MS));
            continue;
        }
        TickType_t last_wake_time = xTaskGetTickCount();
        float temp = context->sensors.temp.probe;
        ESP_ERROR_CHECK(context_set_ec(context, 0, ezo_read_and_print(&ec, temp, 0, "uS/cm")));
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_EC_MS));
    }
}

esp_err_t ezo_ec_init(context_t *context) {
    xTaskCreatePinnedToCore(ezo_ec_task, "ezo_ec", 3072, context, tskIDLE_PRIORITY + 7, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
