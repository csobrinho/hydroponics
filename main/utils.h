#ifndef HYDROPONICS_UTILS_H
#define HYDROPONICS_UTILS_H

#include "esp_err.h"

#define SAFE_FREE(f) do {    \
        if ((f) != NULL) {   \
            free(f);         \
            (f) = NULL;      \
        }                    \
    } while(0)

int random_int(int min, int max);

uint16_t clamp(uint16_t value, uint16_t min, uint16_t max);

esp_err_t sha256(const uint8_t *buf, size_t len, unsigned char output[32]);

#endif //HYDROPONICS_UTILS_H
