#ifndef HYDROPONICS_DISPLAY_LCD_H
#define HYDROPONICS_DISPLAY_LCD_H

#include "esp_err.h"

typedef struct {
    gpio_num_t gpio_rst;
    gpio_num_t gpio_cs;
    gpio_num_t gpio_rs;
    gpio_num_t gpio_wr;
    gpio_num_t gpio_rd;
    gpio_num_t gpio_data[8];
    uint32_t data_set_mask[256];
    uint32_t data_clear_mask[256];
    uint16_t wr_delay_us;
    uint32_t wr_set_mask;
    uint32_t wr_clear_mask;
} lcd_config_t;

esp_err_t lcd_init(lcd_config_t *config);

void Lcd_Write_Com_Data(uint16_t com, uint16_t dat);

#endif //HYDROPONICS_DISPLAY_LCD_H
