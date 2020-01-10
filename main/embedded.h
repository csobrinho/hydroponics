#ifndef HYDROPONICS_EMBEDDED_H
#define HYDROPONICS_EMBEDDED_H

#include <stdint.h>

extern const uint8_t HYDROPONICS_LOGO_BIN_START[] asm("_binary_hydroponics_logo_bin_start");
extern const uint8_t HYDROPONICS_LOGO_BIN_END[] asm("_binary_hydroponics_logo_bin_end");
extern const uint16_t HYDROPONICS_LOGO_BIN_WIDTH;
extern const uint16_t HYDROPONICS_LOGO_BIN_HEIGHT;

#endif //HYDROPONICS_EMBEDDED_H
