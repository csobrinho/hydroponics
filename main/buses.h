#ifndef HYDROPONICS_BUSES_H
#define HYDROPONICS_BUSES_H

#include "driver/adc.h"
#include "driver/i2c.h"

#define I2C_MASTER_NUM I2C_NUM_1             /*!< I2C port number for master dev. */
#define I2C_MASTER_TX_BUF_DISABLE 0          /*!< I2C master do not need buffer. */
#define I2C_MASTER_RX_BUF_DISABLE 0          /*!< I2C master do not need buffer. */
#define I2C_MASTER_FREQ_HZ 350000            /*!< I2C master clock frequency. */
#define I2C_TIMEOUT_MS 500                   /*!< I2C max timeout in ms. */
#define I2C_NO_DEVICE 0xff                   /*!< I2C no device address. */

typedef enum {
    I2C_WRITE_ACK_CHECK = true,              /*!< I2C enable ack check for a master write. */
    I2C_WRITE_ACK_IGNORE = false,            /*!< I2C ignore ack check for a master write. */
} i2c_ack_t;

#define I2C_MASTER_SCL  GPIO_NUM_15
#ifdef CONFIG_IDF_TARGET_ESP32
#define I2C_MASTER_SDA  GPIO_NUM_4
#else
#define I2C_MASTER_SDA  GPIO_NUM_14
#endif
#define OLED_RESET      GPIO_NUM_16
#define ONE_WRITE_GPIO  CONFIG_ESP_ONE_WIRE_GPIO
#define ROTARY_DT_GPIO  GPIO_NUM_37
#define ROTARY_CLK_GPIO GPIO_NUM_36
#define ROTARY_SW_GPIO  GPIO_NUM_NC

#ifdef CONFIG_IDF_TARGET_ESP32
#define LCD_DATA  { GPIO_NUM_23, GPIO_NUM_18, GPIO_NUM_14, GPIO_NUM_27, GPIO_NUM_26, GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_19 }
#define LCD_CONTROL_LOW
#define LCD_WS    GPIO_NUM_17
#define LCD_RS    GPIO_NUM_12
#define LCD_RD    GPIO_NUM_NC
#define LCD_RST   OLED_RESET
#else
#define LCD_DATA  { GPIO_NUM_2, GPIO_NUM_1, GPIO_NUM_8, GPIO_NUM_7, GPIO_NUM_6, GPIO_NUM_5, GPIO_NUM_4, GPIO_NUM_3 }
#define LCD_CONTROL_HIGH
#define LCD_WS    GPIO_NUM_38
#define LCD_RS    GPIO_NUM_45
#define LCD_RD    GPIO_NUM_NC
#define LCD_RST   GPIO_NUM_NC
#endif

#define TANK_A_CHANNEL ADC1_CHANNEL_0        /*!< GPIO36 Tank A water level analog pin. */

typedef enum {
    BUSES_I2C_NO_STOP = 0x0,                 /*!< I2C don't send stop command. */
    BUSES_I2C_STOP = 0x1,                    /*!< I2C send stop command. */
    BUSES_I2C_STOP_MAX,
} buses_i2c_stop_t;

void buses_init(void);

void buses_scan(void);

#endif //HYDROPONICS_BUSES_H
