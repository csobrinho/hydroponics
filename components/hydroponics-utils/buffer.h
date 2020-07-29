#ifndef HYDROPONICS_BUFFER_H
#define HYDROPONICS_BUFFER_H

#include <stdio.h>
#include <stdint.h>

#include "esp_err.h"

esp_err_t buffer_init(FILE **f, const uint8_t *buf, size_t len);

esp_err_t buffer_free(FILE *f);

esp_err_t buffer_read8(FILE *f, uint8_t *val);

esp_err_t buffer_read16(FILE *f, uint16_t *val);

esp_err_t buffer_read32(FILE *f, uint32_t *val);

esp_err_t buffer_peek32_b(const uint8_t *buf, uint32_t *val);

esp_err_t buffer_read8s(FILE *f, uint8_t *val, size_t len);

#endif //HYDROPONICS_BUFFER_H
