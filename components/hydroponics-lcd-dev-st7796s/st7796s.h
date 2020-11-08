#ifndef HYDROPONICS_ST7796S_H
#define HYDROPONICS_ST7796S_H

#include "esp_err.h"
#include "lcd.h"

#define ST7796S_ID         0x7796
#define ST7796S_MAX_WIDTH  320
#define ST7796S_MAX_HEIGHT 480

/*
 * MHS 4.0 pinout.
 *
 *  1. VCC
 *  2. GND
 *  3. CS
 *  4. RESET
 *  5. DC/RS
 *  6. SDI (MOSI)
 *  7. SCK
 *  8. LED
 *  9. SDO (MISO)
 * 10. T_CLK
 * 11. T_CS
 * 12. T_DIN
 * 13. T_DO
 * 14. T_IRQ
 */

#define ST7796S_MISO GPIO_NUM_37 // ESP32-S2: FSPI_IOMUX_PIN_NUM_MISO  13
#define ST7796S_MOSI GPIO_NUM_35 // ESP32-S2: FSPI_IOMUX_PIN_NUM_MOSI  11
#define ST7796S_SCK  GPIO_NUM_36 // ESP32-S2: FSPI_IOMUX_PIN_NUM_CLK   12
#define ST7796S_CS   GPIO_NUM_34 // ESP32-S2: FSPI_IOMUX_PIN_NUM_CS    10
#define ST7796S_DC   GPIO_NUM_4
#define ST7796S_RST  GPIO_NUM_5
#define ST7796S_LED  GPIO_NUM_6

extern const lcd_device_t st7796s;

#endif //HYDROPONICS_ST7796S_H
