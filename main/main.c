#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "rotary_encoder.h"

#include "buses.h"
#include "display/display.h"
#include "sensors/ezo_ec.h"
#include "sensors/humidity_pressure.h"
#include "sensors/temperature.h"

typedef enum {
    LED_STATE_OFF = 0,
    LED_STATE_ON = 1,
    LED_STATE_BLINK = 2
} led_state_t;

#define BLINK_TASK_PRIO      10

static const char *TAG = "main";
static led_state_t led_state = LED_STATE_BLINK;
int16_t rotary_current;

void blink_task(void *arg) {
    gpio_set_direction(CONFIG_BLINK_GPIO, GPIO_MODE_OUTPUT);
    led_state_t level = LED_STATE_ON;
    while (led_state == LED_STATE_BLINK) {
        gpio_set_level(CONFIG_BLINK_GPIO, level);
        level = !level;
        vTaskDelay(pdMS_TO_TICKS(333));
    }
    gpio_set_level(CONFIG_BLINK_GPIO, led_state);
    vTaskDelete(NULL);
}

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
        if (xQueueReceive(event_queue, &event, pdMS_TO_TICKS(500)) == pdTRUE) {
            rotary_current = event.state.position;
            ESP_LOGI(TAG, "Event: position %d, direction %s", event.state.position,
                     event.state.direction
                     ? (event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW")
                     : "NOT_SET");
        } else {
            // Poll current position and direction
            rotary_encoder_state_t state = {0};
            ESP_ERROR_CHECK(rotary_encoder_get_state(&info, &state));
            rotary_current = state.position;
            ESP_LOGI(TAG, "Poll: position %d, direction %s", state.position,
                     state.direction
                     ? (state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW")
                     : "NOT_SET");
        }
    }
}

void app_main() {
    printf("Hello world!\n");

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    buses_init();
    ESP_ERROR_CHECK(display_init());
    buses_scan();
    ESP_ERROR_CHECK(humidity_pressure_init());
    ESP_ERROR_CHECK(ezo_ec_init());
    ESP_ERROR_CHECK(temperature_init());

    xTaskCreatePinnedToCore(blink_task, "blink", 2048, NULL, BLINK_TASK_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(test_task, "test", 2048, NULL, BLINK_TASK_PRIO, NULL, tskNO_AFFINITY);
}