#include "sensors/ezo_ec.h"
#include "sensors/temperature.h"
#include "buses.h"
#include "display.h"
#include "u8g2.h"
#include "u8g2_esp32_hal.h"

#define I2C_ADDRESS_OLED 0x78  /*!< Slave address for OLED display. */

u8g2_t u8g2;

esp_err_t display_init() {
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.sda = I2C_MASTER_SDA;
    u8g2_esp32_hal.scl = I2C_MASTER_SCL;
    u8g2_esp32_hal.reset = OLED_RESET;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_ssd1306_i2c_128x32_univision_f(
            &u8g2,
            U8G2_R0,
            u8g2_esp32_i2c_byte_cb,
            u8g2_esp32_gpio_and_delay_cb);  // Init u8g2 structure.
    u8x8_SetI2CAddress(&u8g2.u8x8, I2C_ADDRESS_OLED);

    u8g2_InitDisplay(&u8g2);                // Send init sequence to the display.
    u8g2_SetPowerSave(&u8g2, 0);            // Init left the display in sleep mode, so wake it up.
    u8g2_ClearBuffer(&u8g2);
    u8g2_SendBuffer(&u8g2);

    return ESP_OK;
}

void display_draw_temp_humidity(float temp, float humidity) {
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    char buf[128] = {0};
    snprintf(buf, 128, "Tmp: %.1f | %.1f \260C", temp, temperature);
    u8g2_DrawStr(&u8g2, 0, 10, buf);
    snprintf(buf, 128, "Hum: %.1f %%", humidity);
    u8g2_DrawStr(&u8g2, 0, 20, buf);
    snprintf(buf, 128, "EC:  %.1f uS/cm", ec_value);
    u8g2_DrawStr(&u8g2, 0, 30, buf);
    u8g2_SendBuffer(&u8g2);
}