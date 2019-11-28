#include <string.h>

#include "driver/i2c.h"

#include "esp_log.h"
#include "esp_err.h"

#include "ezo.h"
#include "temperature.h"

static const char *TAG = "ezo_ec";
float ec_value = 0.0f;

#define EZO_EC_ADDR 0x64                /*!< Slave address for Atlas EZO EC module. */
#define SAMPLE_PERIOD 1000              /*!< Reading sample period. */

#if 0
#define ESP_ERROR_CHECK(x) do {                                                    \
        esp_err_t __err_rc = (x);                                                  \
        if (__err_rc != ESP_OK) {                                                  \
            _esp_error_check_failed_without_abort(__err_rc, __FILE__, __LINE__,    \
                                                  __ASSERT_FUNC, #x);              \
        }                                                                          \
        return __err_rc;                                                           \
    } while(0)
#endif

const ezo_cmd_t cmd_temperature = {.cmd = "T", .delay_ms=300, .has_read= true};

ezo_sensor_t ec = {
        .type = "EC",
        .probe = "CS150",
        .address = EZO_EC_ADDR,
        .cmd_device_info = {.cmd = "I", .delay_ms = 300, .has_read = true},
        .cmd_read = {.cmd="R", .delay_ms=600, .has_read = true},
};

esp_err_t ezo_ec_set_temperature(float temp) {
    char args[20] = {0};
    snprintf(args, sizeof(args), ",%.2f", temp);

    ESP_ERROR_CHECK(ezo_send_command(&ec, cmd_temperature, args));
    if (ec.status != EZO_SENSOR_RESPONSE_SUCCESS) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}

void ezo_ec_task(void *arg) {
    ESP_ERROR_CHECK(ezo_init(&ec));

    float last_temperature = 25.0f;
    while (1) {
        TickType_t last_wake_time = xTaskGetTickCount();

        if (last_temperature != temperature) {
            ESP_ERROR_CHECK(ezo_ec_set_temperature(temperature));
            last_temperature = temperature;
        }
        ESP_ERROR_CHECK(ezo_read_command(&ec, &ec_value));
        ESP_LOGI(TAG, "EC %.2f uS/cm", ec_value);

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SAMPLE_PERIOD));
    }
    vTaskDelete(NULL);
}

esp_err_t ezo_ec_init(void) {
    xTaskCreatePinnedToCore(ezo_ec_task, "ezo_ec", 2048, NULL, 10, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
