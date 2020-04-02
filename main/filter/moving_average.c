#include <stdlib.h>

#include "esp_err.h"

#include "error.h"
#include "moving_average.h"

static const char *TAG = "moving_average";

moving_average_t *moving_average_create(uint16_t len, float initial) {
    moving_average_t *ma = malloc(sizeof(moving_average_t));
    if (ma == NULL) {
        return NULL;
    }
    ma->values_len = len;
    ma->values = calloc(len, sizeof(float));
    if (ma->values == NULL) {
        free(ma);
        return NULL;
    }
    ma->position = 0;
    ma->sum = initial * (float) len;
    if (initial != 0.0f) {
        for (uint16_t i = 0; i < len; ++i) {
            ma->values[i] = initial;
        }
    }
    return ma;
}

float moving_average_add(moving_average_t *ma, float value) {
    // Subtract the oldest number and add the new value.
    ma->sum = ma->sum - ma->values[ma->position] + value;
    // Add the new value to the array.
    ma->values[ma->position] = value;
    ma->position++;
    if (ma->position >= ma->values_len) {
        ma->position = 0;
    }
    return moving_average_latest(ma);
}

inline float moving_average_latest(moving_average_t *ma) {
    return ma->sum / (float) ma->values_len;
}

void moving_average_reset(moving_average_t *ma) {
    ma->position = 0;
    ma->sum = 0.0f;
    for (uint16_t i = 0; i < ma->values_len; ++i) {
        ma->values[i] = 0.0f;
    }
}

esp_err_t moving_average_destroy(moving_average_t *ma) {
    ARG_CHECK(ma != NULL, ERR_PARAM_NULL);
    free(ma->values);
    ma->values = NULL;
    free(ma);
    return ESP_OK;
}
