#include "freertos/FreeRTOS.h"

#include "driver/ledc.h"
#include "esp_err.h"

#include "context.h"
#include "error.h"

#define LEDC_INITIAL_DUTY 3

static const char *TAG = "status";

esp_err_t status_init(context_t *context) {
    gpio_set_direction(CONFIG_BLINK_GPIO, GPIO_MODE_OUTPUT);

    /* Prepare and set configuration of timers that will be used by LED Controller. */
    ledc_timer_config_t ledc_timer = {
            .duty_resolution = LEDC_TIMER_10_BIT, // Resolution of PWM duty.
            .freq_hz = LEDC_INITIAL_DUTY,                 // Frequency of PWM signal.
            .speed_mode = LEDC_LOW_SPEED_MODE,    // Timer mode.
            .timer_num = LEDC_TIMER_0,            // Timer index.
            .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock.
    };
    // Set configuration of timer0 for high speed channels.
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    /*
     * Prepare individual configuration for each channel of LED Controller by selecting:
     * - controller's channel number.
     * - output duty cycle, set initially to 0.
     * - GPIO number where LED is connected to.
     * - speed mode, either high or low.
     * - timer servicing selected channel.
     *   Note: if different channels use one timer, then frequency and bit_num of these channels will be the same.
     */
    ledc_channel_config_t ledc_channel = {
            .channel    = LEDC_CHANNEL_0,
            .duty       = 500,
            .gpio_num   = CONFIG_BLINK_GPIO,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    return ESP_OK;
}