#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "buses.h"
#include "context.h"
#include "error.h"
#include "temperature.h"

static const char *TAG = "temperature";
static temperature_t dev = {0};

static void temperature_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL)

    int errors_count[OWB_MAX_DEVICES] = {0};
    int sample_count = 0;

    while (1) {
        TickType_t last_wake_time = xTaskGetTickCount();
        temperature_hal_read(&dev);
        ESP_LOGI(TAG, "Temperature readings (degrees C): sample %d", ++sample_count);
        if (dev.num_devices > 0) {
            for (int i = 0; i < dev.num_devices; i++) {
                if (dev.errors[i] != DS18B20_OK) {
                    ++errors_count[i];
                }
                ESP_LOGI(TAG, "  %d: %.1f    %d errors", i, dev.readings[i], errors_count[i]);
            }
            ESP_ERROR_CHECK(context_set_temp_water(context, dev.readings[0]));
        }
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_TEMPERATURE_MS));
    }
}

esp_err_t temperature_init(context_t *context) {
    // Setup the GPIOs.
    gpio_set_direction(ONE_WRITE_GPIO, GPIO_MODE_INPUT_OUTPUT);

    ESP_ERROR_CHECK(temperature_hal_init(&dev));

    xTaskCreatePinnedToCore(temperature_task, "temperature", 2048, context, 15, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
