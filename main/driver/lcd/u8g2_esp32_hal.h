/* Adapted from: kolban */

#ifndef HYDROPONICS_DISPLAY_U8G2_ESP32_HAL_H
#define HYDROPONICS_DISPLAY_U8G2_ESP32_HAL_H

#include "u8g2.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"

#define U8G2_ESP32_HAL_UNDEFINED (GPIO_NUM_NC)

typedef struct {
    gpio_num_t sda;   // data for I²C.
    gpio_num_t scl;   // clock for I²C.
    gpio_num_t reset; // reset pin.
} u8g2_esp32_hal_t;

#define U8G2_ESP32_HAL_DEFAULT {U8G2_ESP32_HAL_UNDEFINED, U8G2_ESP32_HAL_UNDEFINED, U8G2_ESP32_HAL_UNDEFINED}

void u8g2_esp32_hal_init(u8g2_esp32_hal_t u8g2_esp32_hal_param);

uint8_t u8g2_esp32_i2c_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

uint8_t u8g2_esp32_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#endif // HYDROPONICS_DISPLAY_U8G2_ESP32_HAL_H
