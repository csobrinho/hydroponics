#include "utils.h"

int random_int(int min, int max) {
    return min + (int) random() % (max + 1 - min);
}