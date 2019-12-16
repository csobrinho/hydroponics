#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "rotary_encoder.h"

#include "buses.h"
#include "context.h"
#include "display/display.h"
#include "driver/status.h"
#include "driver/storage.h"
#include "sensors/ezo_ec.h"
#include "sensors/ezo_ph.h"
#include "sensors/humidity_pressure.h"
#include "sensors/temperature.h"

static const char *TAG = "main";
static context_t *context;

static void test_task(void *arg) {
    rotary_encoder_info_t info = {0};
    ESP_ERROR_CHECK(rotary_encoder_init(&info, ROTARY_DT_GPIO, ROTARY_CLK_GPIO));

    // Create a queue for events from the rotary encoder driver.
    // Tasks can read from this queue to receive up to date position information.
    QueueHandle_t event_queue = rotary_encoder_create_queue();
    ESP_ERROR_CHECK(rotary_encoder_set_queue(&info, event_queue));

    while (1) {
        // Wait for incoming events on the event queue.
        rotary_encoder_event_t event = {0};
        if (xQueueReceive(event_queue, &event, portMAX_DELAY) == pdTRUE) {
            ESP_ERROR_CHECK(context_set_rotary(context, event.state));
            ESP_LOGI(TAG, "Event: position %d, direction %s", event.state.position,
                     event.state.direction
                     ? (event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");
        }
    }
}

void app_main() {
    context = context_create();

    ESP_ERROR_CHECK(storage_init(context));
    buses_init();
    ESP_ERROR_CHECK(status_init(context));
    ESP_ERROR_CHECK(display_init(context));
    buses_scan();
    ESP_ERROR_CHECK(humidity_pressure_init(context));
    ESP_ERROR_CHECK(ezo_ec_init(context));
    ESP_ERROR_CHECK(ezo_ph_init(context));
    ESP_ERROR_CHECK(temperature_init(context));

    xTaskCreatePinnedToCore(test_task, "test", 2048, NULL, 15, NULL, tskNO_AFFINITY);
}