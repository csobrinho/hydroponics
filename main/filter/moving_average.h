#ifndef HYDROPONICS_FILTER_MOVING_AVERAGE_H
#define HYDROPONICS_FILTER_MOVING_AVERAGE_H

#include <stdint.h>

typedef struct {
    float *values;
    uint16_t values_len;
    uint16_t position;
    float sum;
} moving_average_t;

moving_average_t *moving_average_create(uint16_t len, float initial);

float moving_average_add(moving_average_t *ma, float value);

float moving_average_latest(moving_average_t *ma);

void moving_average_reset(moving_average_t *ma);

esp_err_t moving_average_destroy(moving_average_t *ma);

#endif //HYDROPONICS_FILTER_MOVING_AVERAGE_H
