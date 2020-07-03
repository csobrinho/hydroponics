#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/adc.h"

#include "esp_log.h"

#include "context.h"
#include "error.h"
#include "tank.h"

#define NO_OF_SAMPLES 32
#define TANK_A ADC1_CHANNEL_0

static const char *TAG = "tank";

// Based on https://docs.google.com/spreadsheets/d/1LZo2zjm7wT2C7UeA40zMUXK-daBry_TDRjhAEd-a5a8/view
//                                             x^3                  x^2                x             b
static const double regression[] = {-0.0000000008986077278, 0.000004383302109, -0.007671056876, 5.622311802};

static void tank_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    while (true) {
        TickType_t last_wake_time = xTaskGetTickCount();
        float tank_a = 0.f;
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            tank_a += (float) adc1_get_raw(TANK_A);
        }
        tank_a /= NO_OF_SAMPLES;

        // Linear regression.
        double tank_2 = tank_a * tank_a;
        double tank_3 = tank_a * tank_a * tank_a;
        double tank_100 = regression[0] * tank_3 + regression[1] * tank_2 + regression[2] * tank_a + regression[3];

        ESP_ERROR_CHECK(context_set_tank(context, 0, (float) tank_100));
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Tank A: %d / %.1f %%", (int) tank_a, tank_100 * 100);
    }
}

esp_err_t tank_init(context_t *context) {
    // Setup the ADC.
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(TANK_A, ADC_ATTEN_DB_11);

    xTaskCreatePinnedToCore(tank_task, "tank", 2560, context, 15, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
