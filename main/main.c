#include <stdio.h>
#include <sensors/temperature.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"

#include "display/display.h"

typedef enum {
    LED_STATE_OFF = 0,
    LED_STATE_ON = 1,
    LED_STATE_BLINK = 2
} led_state_t;

#define BLINK_TASK_PRIO      10

static const char *TAG = "main";

static led_state_t led_state = LED_STATE_BLINK;

void blink_task(void *arg) {
    gpio_set_direction(CONFIG_BLINK_GPIO, GPIO_MODE_OUTPUT);
    led_state_t level = LED_STATE_ON;
    while (led_state == LED_STATE_BLINK) {
        gpio_set_level(CONFIG_BLINK_GPIO, level);
        level = !level;
        vTaskDelay(pdMS_TO_TICKS(300));
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

    ESP_ERROR_CHECK(display_init());
    ESP_ERROR_CHECK(temperature_init());

    xTaskCreatePinnedToCore(blink_task, "blink", 1024, NULL, BLINK_TASK_PRIO, NULL, tskNO_AFFINITY);
}