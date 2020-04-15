#ifndef HYDROPONICS_DRIVER_EXT_GPIO_H
#define HYDROPONICS_DRIVER_EXT_GPIO_H

#include "driver/gpio.h"

#define EXT_GPIO_ADDRESS 0x20

typedef enum {
    EXT_GPIO_A_0 = GPIO_NUM_0,
    EXT_GPIO_A_1 = GPIO_NUM_1,
    EXT_GPIO_A_2 = GPIO_NUM_2,
    EXT_GPIO_A_3 = GPIO_NUM_3,
    EXT_GPIO_A_4 = GPIO_NUM_4,
    EXT_GPIO_A_5 = GPIO_NUM_5,
    EXT_GPIO_A_6 = GPIO_NUM_6,
    EXT_GPIO_A_7 = GPIO_NUM_7,
    EXT_GPIO_B_0 = GPIO_NUM_8,
    EXT_GPIO_B_1 = GPIO_NUM_9,
    EXT_GPIO_B_2 = GPIO_NUM_10,
    EXT_GPIO_B_3 = GPIO_NUM_11,
    EXT_GPIO_B_4 = GPIO_NUM_12,
    EXT_GPIO_B_5 = GPIO_NUM_13,
    EXT_GPIO_B_6 = GPIO_NUM_14,
    EXT_GPIO_B_7 = GPIO_NUM_15,
    EXT_GPIO_MAX = GPIO_NUM_16,
} ext_gpio_num_t;

esp_err_t ext_gpio_init(void);

esp_err_t ext_gpio_config(const gpio_config_t *config);

uint16_t ext_gpio_get(void);

int ext_gpio_get_level(ext_gpio_num_t gpio_num);

esp_err_t ext_gpio_set_level(ext_gpio_num_t gpio_num, uint32_t level);

esp_err_t ext_gpio_set(uint16_t value);

esp_err_t ext_gpio_set_direction(ext_gpio_num_t gpio_num, gpio_mode_t mode);

esp_err_t ext_gpio_set_pull_mode(ext_gpio_num_t gpio_num, gpio_pull_mode_t pull);

esp_err_t ext_gpio_set_intr_type(ext_gpio_num_t gpio_num, gpio_int_type_t intr_type);

esp_err_t ext_gpio_invert_input(ext_gpio_num_t gpio_num, bool invert);

#endif //HYDROPONICS_DRIVER_EXT_GPIO_H
