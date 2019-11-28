#ifndef HYDROPONICS_BUSES_H
#define HYDROPONICS_BUSES_H

#include "driver/i2c.h"

#define I2C_MASTER_NUM I2C_NUM_1             /*!< I2C port number for master dev. */
#define I2C_MASTER_TX_BUF_DISABLE 0          /*!< I2C master do not need buffer. */
#define I2C_MASTER_RX_BUF_DISABLE 0          /*!< I2C master do not need buffer. */
#define I2C_MASTER_FREQ_HZ 400000            /*!< I2C master clock frequency. */
#define I2C_TIMEOUT_MS 500                   /*!< I2C max timeout in ms. */

typedef enum {
    I2C_WRITE_ACK_CHECK = true,                    /*!< I2C enable ack check for a master write. */
    I2C_WRITE_ACK_IGNORE = false,                  /*!< I2C ignore ack check for a master write. */
} i2c_ack_t;

#define I2C_MASTER_SCL GPIO_NUM_15
#define I2C_MASTER_SDA GPIO_NUM_4
#define OLED_RESET     GPIO_NUM_16

typedef enum {
    BUSES_I2C_NO_STOP = 0x0,                 /*!< I2C don't send stop command. */
    BUSES_I2C_STOP = 0x1,                    /*!< I2C send stop command. */
    BUSES_I2C_STOP_MAX,
} buses_i2c_stop_t;

void buses_init(void);

void buses_scan(void);

esp_err_t buses_i2c_write(uint8_t address, uint8_t *buf, size_t len);

esp_err_t buses_i2c_read(uint8_t address, uint8_t *buf, size_t len, buses_i2c_stop_t stop);

esp_err_t buses_i2c_continue_read(uint8_t *buf, size_t len, i2c_ack_type_t ack);

esp_err_t buses_i2c_stop(void);

#endif //HYDROPONICS_BUSES_H
