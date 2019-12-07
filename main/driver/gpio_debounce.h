#ifndef HYDROPONICS_DRIVER_GPIO_DEBOUNCE_H
#define HYDROPONICS_DRIVER_GPIO_DEBOUNCE_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "esp_err.h"

typedef enum {
    GPIO_DEBOUNCE_TYPE_UNKNOWN = 0,
    GPIO_DEBOUNCE_TYPE_ON = 1,
    GPIO_DEBOUNCE_TYPE_OFF = 2,
} gpio_debounce_type_t;

typedef struct {
    gpio_num_t gpio;
    gpio_debounce_type_t type;
} gpio_debounce_evt_t;

esp_err_t gpio_debounce_init(TickType_t debounce_ticks);

esp_err_t gpio_debounce_register(gpio_num_t gpio, QueueHandle_t *queue);

esp_err_t gpio_debounce_get(gpio_num_t gpio, gpio_debounce_type_t *value);

QueueHandle_t *gpio_debounce_create_queue(void);

esp_err_t gpio_debounce_deregister(gpio_num_t gpio);

#endif //HYDROPONICS_DRIVER_GPIO_DEBOUNCE_H
