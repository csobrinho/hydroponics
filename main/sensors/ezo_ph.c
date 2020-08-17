#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "driver/ezo.h"

static const char *TAG = "ezo_ph";
static ezo_sensor_t pha = {
        .probe = "PH2000",
        .desc = "pha",
        .address = CONFIG_ESP_SENSOR_PH_TANK_A_ADDR, /*!< Slave address for Atlas EZO PH module for tank A. */
        .delay_ms = EZO_DELAY_MS_SHORT,
        .delay_read_ms = EZO_DELAY_MS_SLOWEST,
        .delay_calibration_ms = EZO_DELAY_MS_SLOWEST,
        .calibration = EZO_CALIBRATION_MODE_THREE_POINTS,
#ifdef CONFIG_ESP_SENSOR_SIMULATE
        .simulate = 5.7f,
        .threshold = 0.05f,
#endif
};
static ezo_sensor_t phb = {
        .probe = "PH2000",
        .desc = "phb",
        .address = CONFIG_ESP_SENSOR_PH_TANK_B_ADDR, /*!< Slave address for Atlas EZO PH module for tank A. */
        .delay_ms = EZO_DELAY_MS_SHORT,
        .delay_read_ms = EZO_DELAY_MS_SLOWEST,
        .delay_calibration_ms = EZO_DELAY_MS_SLOWEST,
        .calibration = EZO_CALIBRATION_MODE_THREE_POINTS,
#ifdef CONFIG_ESP_SENSOR_SIMULATE
        .simulate = 5.7f,
        .threshold = 0.05f,
#endif
};

static void ezo_ph_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);
    // Give it a little time to initialize.
    vTaskDelay(pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_PH_MS));
    ESP_ERROR_CHECK(ezo_init(&pha));
    ESP_ERROR_CHECK(ezo_init(&phb));

    while (true) {
        if (pha.pause || phb.pause) {
            vTaskDelay(pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_PH_MS));
            continue;
        }
        TickType_t last_wake_time = xTaskGetTickCount();

        float temp = context->sensors.temp.probe;
        ESP_ERROR_CHECK(context_set_ph(context, 0, ezo_read_and_print(&pha, temp, 'A', 2, "")));
#if CONFIG_ESP_SENSOR_TANKS == 2
        ESP_ERROR_CHECK(context_set_ph(context, 1, ezo_read_and_print(&phb, temp, 'B', 2, "")));
#endif
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_PH_MS));
    }
}

esp_err_t ezo_ph_init(context_t *context) {
    xTaskCreatePinnedToCore(ezo_ph_task, "ezo_ph", 3072, context, tskIDLE_PRIORITY + 8, NULL, tskNO_AFFINITY);
    return ESP_OK;
}

esp_err_t ezo_ph_slope(float *acidPercentage, float *basePercentage) {
    ESP_ERROR_CHECK(ezo_send_command(&pha, pha.delay_ms, "Slope,?"));
    ESP_ERROR_CHECK(ezo_parse_response(&pha, 2, "?Slope,%f,%f", acidPercentage, basePercentage));
    return ESP_OK;
}