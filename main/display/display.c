#include "display.h"

#include "u8g2.h"
#include "u8g2_esp32_hal.h"

u8g2_t u8g2;

esp_err_t display_init() {
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.sda = OLED_DATA;
    u8g2_esp32_hal.scl = OLED_CLOCK;
    u8g2_esp32_hal.reset = OLED_RESET;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_ssd1306_i2c_128x32_univision_f(
            &u8g2,
            U8G2_R0,
            u8g2_esp32_i2c_byte_cb,
            u8g2_esp32_gpio_and_delay_cb);  // init u8g2 structure
    u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);

    u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,

    u8g2_SetPowerSave(&u8g2, 0); // wake up display
    u8g2_ClearBuffer(&u8g2);

    u8g2_DrawBox(&u8g2, 0, 0, 128, 64);

    return ESP_OK;
}