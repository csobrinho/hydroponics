#ifndef HYDROPONICS_LCD_SPI_H
#define HYDROPONICS_LCD_SPI_H

#include "lcd.h"

#include "esp_err.h"

typedef enum {
    LCD_SPI_COMMAND = 0,
    LCD_SPI_DATA = 1,
} lcd_spi_dc_t;

extern const lcd_com_t lcd_com_spi;

esp_err_t lcd_spi_init(lcd_dev_t *dev);

uint8_t lcd_spi_read_reg8(const lcd_dev_t *dev, uint16_t cmd);

uint16_t lcd_spi_read_reg16(const lcd_dev_t *dev, uint16_t cmd);

void lcd_spi_read_regn(const lcd_dev_t *dev, uint16_t cmd, uint8_t *buf, size_t len);

void lcd_spi_write_reg8(const lcd_dev_t *dev, uint16_t cmd, uint8_t data);

void lcd_spi_write_reg16(const lcd_dev_t *dev, uint16_t cmd, uint16_t data);

void lcd_spi_write_regn(const lcd_dev_t *dev, uint16_t cmd, const uint8_t *buf, size_t len);

void lcd_spi_write_data8(const lcd_dev_t *dev, uint8_t data);

void lcd_spi_write_data16(const lcd_dev_t *dev, uint16_t data);

#endif //HYDROPONICS_LCD_SPI_H
