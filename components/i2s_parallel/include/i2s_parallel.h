#pragma once

#include <stdio.h>
#include <stdint.h>
#include "driver/i2s.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t bit_width;
    uint8_t pin_clk;
    uint8_t data[24];
} i2s_parallel_pin_config_t;

void i2s_parallel_write_data(const uint8_t *data, size_t len);

void i2s_parallel_write_data16n(uint16_t data, size_t len);

void i2s_parallel_init(const i2s_parallel_pin_config_t *pin_config, uint32_t clk_div); // clk_fre = 40MHz / clk_div, clk_div > 1

#ifdef __cplusplus
}
#endif

