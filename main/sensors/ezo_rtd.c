#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "driver/ezo.h"

#define EZO_RTD_ADDR 0x66  /*!< Slave address for Atlas EZO RTD module. */

static const char *TAG = "ezo_rtd";
static ezo_sensor_t rtd = {
        .probe = "CS150",
        .address = EZO_RTD_ADDR,
        .delay_ms = EZO_DELAY_MS_SHORT,
        .delay_read_ms = EZO_DELAY_MS_SLOW,
        .delay_calibration_ms = EZO_DELAY_MS_SLOW,
        .calibration = EZO_CALIBRATION_MODE_ONE_POINT,
#ifdef CONFIG_ESP_SIMULATE_SENSORS
.simulate = 19.0f,
.threshold = 0.2f,
#endif
};
ezo_sensor_t *ezo_rtd = &rtd;

static void ezo_rtd_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL)
    ESP_ERROR_CHECK(ezo_init(&rtd));

    float value;
    while (1) {
        TickType_t last_wake_time = xTaskGetTickCount();

        ESP_ERROR_CHECK(ezo_read(&rtd, &value));
        ESP_LOGD(TAG, "RTD %.2f", value);
        ESP_ERROR_CHECK(context_set_temp_probe(context, value));

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_RTD_MS));
        // vTaskDelay(portMAX_DELAY);
    }
}

esp_err_t ezo_rtd_init(context_t *context) {
    xTaskCreatePinnedToCore(ezo_rtd_task, "ezo_rtd", 2048, context, 10, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
