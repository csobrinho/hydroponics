#include <stdlib.h>
#include <stdint.h>

#include "utils.h"

int random_int(int min, int max) {
    return min + (int) random() % (max + 1 - min);
}

uint16_t clamp(uint16_t value, uint16_t min, uint16_t max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
