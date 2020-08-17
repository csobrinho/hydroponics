#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "driver/ezo.h"

static const char *TAG = "ezo_rtd";
static ezo_sensor_t rtd = {
        .probe = "CS150",
        .desc = "rtd",
        .address = CONFIG_ESP_SENSOR_RTD_ADDR, /*!< Slave address for Atlas EZO RTD module. */
        .delay_ms = EZO_DELAY_MS_SHORT,
        .delay_read_ms = EZO_DELAY_MS_SLOW,
        .delay_calibration_ms = EZO_DELAY_MS_SLOW,
        .calibration = EZO_CALIBRATION_MODE_ONE_POINT,
#ifdef CONFIG_ESP_SENSOR_SIMULATE
        .simulate = 19.0f,
        .threshold = 0.2f,
#endif
};

static void ezo_rtd_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);
    // Give it a little time to initialize.
    vTaskDelay(pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_RTD_MS));
    ESP_ERROR_CHECK(ezo_init(&rtd));

    float value = 0.f;
    while (true) {
        if (rtd.pause) {
            vTaskDelay(pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_RTD_MS));
            continue;
        }
        TickType_t last_wake_time = xTaskGetTickCount();

        ESP_ERROR_CHECK(ezo_read(&rtd, &value));
        ESP_LOGD(TAG, "RTD %.2f", value);
        ESP_ERROR_CHECK(context_set_temp_probe(context, value));

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_RTD_MS));
    }
}

esp_err_t ezo_rtd_init(context_t *context) {
    xTaskCreatePinnedToCore(ezo_rtd_task, "ezo_rtd", 2560, context, tskIDLE_PRIORITY + 9, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
