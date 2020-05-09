#ifndef HYDROPONICS_EMBEDDED_H
#define HYDROPONICS_EMBEDDED_H

#include <stdint.h>

#define HYDROPONICS_LOGO_BIN_WIDTH 126
#define HYDROPONICS_LOGO_BIN_HEIGHT 69
extern const uint8_t HYDROPONICS_LOGO_BIN_START[] asm("_binary_hydroponics_logo_bin_start");
extern const uint8_t HYDROPONICS_LOGO_BIN_END[] asm("_binary_hydroponics_logo_bin_end");

#endif //HYDROPONICS_EMBEDDED_H
