#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/pcnt.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "buses.h"
#include "display/display.h"
#include "driver/rotary.h"
#include "sensors/ezo_ec.h"
#include "sensors/humidity_pressure.h"
#include "sensors/temperature.h"

typedef enum {
    LED_STATE_OFF = 0,
    LED_STATE_ON = 1,
    LED_STATE_BLINK = 2
} led_state_t;

#define BLINK_TASK_PRIO      10

static led_state_t led_state = LED_STATE_BLINK;
static rotary_config_t rotary_config = {
        .unit = PCNT_UNIT_0,
        .dt = ROTARY_DT_GPIO,
        .clk = ROTARY_CLK_GPIO,
        .sw = ROTARY_SW_GPIO,
};

static const char *TAG = "main";

void blink_task(void *arg) {
    gpio_set_direction(CONFIG_BLINK_GPIO, GPIO_MODE_OUTPUT);
    led_state_t level = LED_STATE_ON;
    rotary_evt_t evt;

    while (led_state == LED_STATE_BLINK) {
        gpio_set_level(CONFIG_BLINK_GPIO, level);
        level = !level;
        if (xQueueReceive(rotary_config.queue, &evt, pdMS_TO_TICKS(300))) {
            ESP_LOGW(TAG, "rotary unit: %d status: %d", evt.unit, evt.status);
        }
        ESP_ERROR_CHECK(rotary_value(&rotary_config, &evt.value));
        ESP_LOGW(TAG, "rotary value: %d", evt.value);
    }
    gpio_set_level(CONFIG_BLINK_GPIO, led_state);
    vTaskDelete(NULL);
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

    rotary_config.queue = xQueueCreate(10, sizeof(rotary_evt_t));
    ESP_ERROR_CHECK(rotary_init(&rotary_config));

    xTaskCreatePinnedToCore(blink_task, "blink", 2048, NULL, BLINK_TASK_PRIO, NULL, tskNO_AFFINITY);
}