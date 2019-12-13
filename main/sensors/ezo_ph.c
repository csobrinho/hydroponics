#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "driver/ezo.h"

#define EZO_PH_ADDR 0x63                /*!< Slave address for Atlas EZO PH module. */
#define SAMPLE_PERIOD 1000              /*!< Reading sample period. */

static const char *TAG = "ezo_ph";
static ezo_sensor_t ph = {
        .type = "pH",
        .probe = "PH2000",
        .address = EZO_PH_ADDR,
        .delay_read_ms = EZO_DELAY_MS_SLOWEST,
        .delay_calibration_ms = EZO_DELAY_MS_SLOWEST,
        .calibration = EZO_CALIBRATION_LOW | EZO_CALIBRATION_MID | EZO_CALIBRATION_HIGH,
};
static const ezo_cmd_t cmd_slope = {.cmd = "Slope,?"};

static void ezo_ph_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL)
    ESP_ERROR_CHECK(ezo_init(&ph));

    float last_temp_water = 25.0f;
    float value;
    while (1) {
        TickType_t last_wake_time = xTaskGetTickCount();

        float temp_water = context->sensors.temp.water;
        if (last_temp_water != temp_water && CONTEXT_VALUE_IS_VALID(temp_water)) {
            last_temp_water = temp_water;
            ESP_ERROR_CHECK(ezo_read_temperature_command(&ph, &value, last_temp_water));
        } else {
            ESP_ERROR_CHECK(ezo_read_command(&ph, &value));
        }
        ESP_LOGI(TAG, "PH %.2f", context->sensors.ph.value);
        ESP_ERROR_CHECK(context_set_ph(context, value));

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SAMPLE_PERIOD));
    }
}

esp_err_t ezo_ph_init(context_t *context) {
    xTaskCreatePinnedToCore(ezo_ph_task, "ezo_ph", 2048, context, 10, NULL, tskNO_AFFINITY);
    return ESP_OK;
}

esp_err_t ezo_ph_slope(float *acidPercentage, float *basePercentage) {
    ESP_ERROR_CHECK(ezo_send_command(&ph, &cmd_slope, ph.delay_ms, NULL));

    int ret = sscanf(ph.buf, "?Slope,%f,%f", acidPercentage, basePercentage);
    if (ret != 2) {
        ESP_LOGW(TAG, "[0x%.2x] Unexpected response: %d '%s'", ret, ph.address, ph.buf);
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}