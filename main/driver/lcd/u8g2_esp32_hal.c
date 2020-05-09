/* Adapted from: kolban */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "buses.h"
#include "error.h"
#include "utils.h"
#include "u8g2_esp32_hal.h"

static const char *TAG = "u8g2_hal";

static i2c_cmd_handle_t handle_i2c;      // I2C handle.
static u8g2_esp32_hal_t u8g2_esp32_hal;  // HAL state data.

/* Initialize the ESP32 HAL. */
void u8g2_esp32_hal_init(u8g2_esp32_hal_t u8g2_esp32_hal_param) {
    u8g2_esp32_hal = u8g2_esp32_hal_param;
}

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is invoked
 * to handle I2C communications.
 */
uint8_t u8g2_esp32_i2c_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    ESP_LOGD(TAG, "i2c_cb: Received a msg: %d, arg_int: %d, arg_ptr: %p", msg, arg_int, arg_ptr);

    switch (msg) {
        case U8X8_MSG_BYTE_SEND: {
            uint8_t *data_ptr = (uint8_t *) arg_ptr;
            ESP_LOG_BUFFER_HEXDUMP(TAG, data_ptr, arg_int, ESP_LOG_VERBOSE);

            while (arg_int > 0) {
                ESP_ERROR_CHECK(i2c_master_write_byte(handle_i2c, *data_ptr, I2C_WRITE_ACK_CHECK));
                data_ptr++;
                arg_int--;
            }
            break;
        }

        case U8X8_MSG_BYTE_START_TRANSFER: {
            uint8_t i2c_address = u8x8_GetI2CAddress(u8x8);
            handle_i2c = i2c_cmd_link_create();
            ESP_LOGD(TAG, "Start I2C transfer to %02X.", i2c_address >> 1);
            ESP_ERROR_CHECK(i2c_master_start(handle_i2c));
            ESP_ERROR_CHECK(i2c_master_write_byte(handle_i2c, i2c_address | I2C_MASTER_WRITE, I2C_WRITE_ACK_CHECK));
            break;
        }

        case U8X8_MSG_BYTE_END_TRANSFER: {
            ESP_LOGD(TAG, "End I2C transfer.");
            ESP_ERROR_CHECK(i2c_master_stop(handle_i2c));
            ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, handle_i2c, pdMS_TO_TICKS(I2C_TIMEOUT_MS)));
            i2c_cmd_link_delete(handle_i2c);
            break;
        }

        case U8X8_MSG_BYTE_INIT: {
            // Initialization is done by the buses_init.
            break;
        }

        default:
            break;
    }
    return 0;
}

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is invoked
 * to handle callbacks for GPIO and delay functions.
 */
uint8_t u8g2_esp32_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    ARG_UNUSED(u8x8);
    ARG_UNUSED(arg_ptr);
    ESP_LOGD(TAG, "gpio_and_delay_cb: Received a msg: %d, arg_int: %d, arg_ptr: %p", msg, arg_int, arg_ptr);

    switch (msg) {
        // Initialize the GPIO and DELAY HAL functions.  If the pins for DC and RESET have been
        // specified then we define those pins as GPIO outputs.
        case U8X8_MSG_GPIO_AND_DELAY_INIT: {
            uint64_t bitmask = 0;
            if (u8g2_esp32_hal.reset != U8G2_ESP32_HAL_UNDEFINED) {
                bitmask = bitmask | (1ull << u8g2_esp32_hal.reset);
            }
            if (bitmask == 0) {
                break;
            }
            gpio_config_t gpioConfig = {
                    .pin_bit_mask = bitmask,
                    .mode = GPIO_MODE_OUTPUT,
                    .pull_up_en = GPIO_PULLUP_DISABLE,
                    .pull_down_en = GPIO_PULLDOWN_ENABLE,
                    .intr_type = GPIO_INTR_DISABLE,
            };
            gpio_config(&gpioConfig);
            break;
        }

        case U8X8_MSG_GPIO_RESET: {
            // Set the GPIO reset pin to the value passed in through arg_int.
            if (u8g2_esp32_hal.reset != U8G2_ESP32_HAL_UNDEFINED) {
                gpio_set_level(u8g2_esp32_hal.reset, arg_int);
            }
            break;
        }
            // Set the Hardware I²C pin to the value passed in through arg_int.
        case U8X8_MSG_GPIO_I2C_CLOCK:
            if (u8g2_esp32_hal.scl != U8G2_ESP32_HAL_UNDEFINED) {
                gpio_set_level(u8g2_esp32_hal.scl, arg_int);
            }
            break;
            // Set the Hardware I²C pin to the value passed in through arg_int.
        case U8X8_MSG_GPIO_I2C_DATA:
            if (u8g2_esp32_hal.sda != U8G2_ESP32_HAL_UNDEFINED) {
                gpio_set_level(u8g2_esp32_hal.sda, arg_int);
            }
            break;
            // Delay for the number of milliseconds passed in through arg_int.
        case U8X8_MSG_DELAY_MILLI:
            safe_delay_ms(arg_int);
            break;

        default:
            break;
    }
    return 0;
}
