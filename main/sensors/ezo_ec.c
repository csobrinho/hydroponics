#include <string.h>

#include "driver/i2c.h"

#include "esp_log.h"
#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "display/display.h"
#include "driver/ezo.h"

#define EZO_EC_ADDR 0x64                /*!< Slave address for Atlas EZO EC module. */
#define SAMPLE_PERIOD 1000              /*!< Reading sample period. */

static const char *TAG = "ezo_ec";
static ezo_sensor_t ec = {
        .type = "EC",
        .probe = "CS150",
        .address = EZO_EC_ADDR,
        .cmd_device_info = {.cmd = "I", .delay_ms = 300, .has_read = true},
        .cmd_read = {.cmd="R", .delay_ms=600, .has_read = true},
};

static const ezo_cmd_t cmd_temperature = {.cmd = "T", .delay_ms=300, .has_read= true};

static esp_err_t ezo_ec_set_temperature(float temp) {
    char args[20] = {0};
    snprintf(args, sizeof(args), ",%.2f", temp);

    ESP_ERROR_CHECK(ezo_send_command(&ec, cmd_temperature, args));
    if (ec.status != EZO_SENSOR_RESPONSE_SUCCESS) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}

static void ezo_ec_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);
    ESP_ERROR_CHECK(ezo_init(&ec));

    float last_temperature = 25.0f;
    while (1) {
        TickType_t last_wake_time = xTaskGetTickCount();

        float temperature = context->sensors.temp.water;
        if (last_temperature != temperature && temperature != CONTEXT_UNKNOWN_VALUE) {
            ESP_ERROR_CHECK(ezo_ec_set_temperature(temperature));
            last_temperature = temperature;
        }
        ESP_ERROR_CHECK(ezo_read_command(&ec, (float *) (&context->sensors.ec.value)));
        ESP_LOGI(TAG, "EC %.2f uS/cm", context->sensors.ec.value);

        display_draw_temp_humidity(context);

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SAMPLE_PERIOD));
    }
    vTaskDelete(NULL);
}

esp_err_t ezo_ec_init(context_t *context) {
    xTaskCreatePinnedToCore(ezo_ec_task, "ezo_ec", 2048, context, 10, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
