#include "esp_err.h"

#include "error.h"

#include "i2s_lcd8.h"
#include "rm68090.h"
#include "rm68090_regs.h"

static const char *TAG = "rm68090";

// Adapted from
static const uint16_t RM68090_REG_VALUES[] = {
        0x00e5, 0x78f0,                                               // Set SRAM internal timing
        RM68090_REG_DRIVER_OUTPUT_CONTROL, 0x0100,                    // Set Driver Output Control
        RM68090_REG_LCD_DRIVING_WAVE_CONTROL, 0x0200,                 // Set 1 line inversion
        RM68090_REG_ENTRY_MODE, 0x1030,                               // Set GRAM write direction and BGR=1.
        RM68090_REG_RESIZING_CONTROL, 0x0000,                         // Resize register
        RM68090_REG_16BITS_DATA_FORMAT_SELECTION, 0x0000,             // 16bits Data Format Selection
        RM68090_REG_DISPLAY_CONTROL_2, 0x0207,                        // Set the back porch and front porch
        RM68090_REG_DISPLAY_CONTROL_3, 0x0000,                        // Set non-display area refresh cycle ISC[3:0]
        RM68090_REG_DISPLAY_CONTROL_4, 0x0000,                        // FMARK function
        RM68090_REG_EXTERNAL_DISPLAY_INTERFACE_CONTROL_1, 0x0000,     // RGB interface setting
        RM68090_REG_FRAME_MARKER_POSITION, 0x0000,                    // Frame marker Position
        RM68090_REG_EXTERNAL_DISPLAY_INTERFACE_CONTROL_2, 0x0000,     // RGB interface polarity
        // ----------- Power On sequence ----------- //
        RM68090_REG_POWER_CONTROL_1, 0X0000,                          // SAP, BT[3:0], AP, DSTB, SLP, STB
        RM68090_REG_POWER_CONTROL_2, 0X0007,                          // DC1[2:0], DC0[2:0], VC[2:0]
        RM68090_REG_POWER_CONTROL_3, 0X0000,                          // VREG1OUT voltage
        RM68090_REG_POWER_CONTROL_4, 0X0000,                          // VDV[4:0] for VCOM amplitude
        RM68090_REG_DISPLAY_CONTROL_1, 0X0001,
        I2S_LCD8_DELAY, 200, // Dis-charge capacitor power voltage
        RM68090_REG_POWER_CONTROL_1, 0X1690,                          // SAP=1, BT=6, APE=1, AP=1, DSTB=0, SLP=0, STB=0
        RM68090_REG_POWER_CONTROL_2, 0X0227,                          // DC1=2, DC0=2, VC=7
        I2S_LCD8_DELAY, 50,  // Wait 50ms
        RM68090_REG_POWER_CONTROL_3, 0x000d,                          // VCIRE=1, PON=0, VRH=5
        I2S_LCD8_DELAY, 50,  // Wait 50ms
        RM68090_REG_POWER_CONTROL_4, 0x1200,                          // VDV=28 for VCOM amplitude
        RM68090_REG_POWER_CONTROL_7, 0x000a,                          // VCM=10 for VCOMH
        RM68090_REG_FRAME_RATE_AND_COLOR_CONTROL, 0x000d,             // Set Frame Rate
        I2S_LCD8_DELAY, 50,  // Wait 50ms
        RM68090_REG_RAM_ADDRESS_SET_HORIZONTAL_ADDRESS, 0X0000,       // GRAM horizontal Address
        RM68090_REG_RAM_ADDRESS_SET_VERTICAL_ADDRESS, 0X0000,         // GRAM Vertical Address
        // ----------- Adjust the Gamma Curve ----------//
        RM68090_REG_GAMMA_CONTROL_0, 0x0000,
        RM68090_REG_GAMMA_CONTROL_1, 0x0404,
        RM68090_REG_GAMMA_CONTROL_2, 0x0003,
        RM68090_REG_GAMMA_CONTROL_5, 0x0405,
        RM68090_REG_GAMMA_CONTROL_6, 0x0808,
        RM68090_REG_GAMMA_CONTROL_7, 0x0407,
        RM68090_REG_GAMMA_CONTROL_8, 0x0303,
        RM68090_REG_GAMMA_CONTROL_9, 0x0707,
        RM68090_REG_GAMMA_CONTROL_C, 0x0504,
        RM68090_REG_GAMMA_CONTROL_D, 0x0808,
        //------------------ Set GRAM area ---------------//
        RM68090_REG_BASE_IMAGE_DISPLAY_CONTROL_INSTRUCTION_0, 0x2700, // Gate Scan Line GS=0 [0xA700]
        RM68090_REG_BASE_IMAGE_DISPLAY_CONTROL_INSTRUCTION_1, 0x0001, // NDL,VLE, REV
        RM68090_REG_BASE_IMAGE_DISPLAY_CONTROL_INSTRUCTION_A, 0x0000, // set scrolling line
        //-------------- Partial Display Control ---------//
        RM68090_REG_PARTIAL_IMAGE_1_DISPLAY_POSITION, 0x0000,
        RM68090_REG_PARTIAL_IMAGE_1_AREA_START_LINE, 0x0000,
        RM68090_REG_PARTIAL_IMAGE_1_AREA_END_LINE, 0x0000,
        RM68090_REG_PARTIAL_IMAGE_2_DISPLAY_POSITION, 0x0000,
        RM68090_REG_PARTIAL_IMAGE_2_AREA_START_LINE, 0x0000,
        RM68090_REG_PARTIAL_IMAGE_2_AREA_END_LINE, 0x0000,
        //-------------- Panel Control -------------------//
        RM68090_REG_PANEL_INTERFACE_CONTROL_1, 0x0010,
        RM68090_REG_PANEL_INTERFACE_CONTROL_2, 0x0000,
        RM68090_REG_DISPLAY_CONTROL_1, 0x0133,                        // 262K color and display ON
};

esp_err_t rm68090_init(i2s_lcd8_dev_t *dev) {
    ESP_ERROR_CHECK(i2s_lcd8_init(dev));
    ESP_ERROR_CHECK(i2s_lcd8_init_registers(dev, RM68090_REG_VALUES, sizeof(RM68090_REG_VALUES)));
    ESP_ERROR_CHECK(rm68090_set_rotation(dev, ROTATION_LANDSCAPE_REV));
    rm68090_clear(dev, LCD_COLOR_GREEN);
    return ESP_OK;
}

void rm68090_address_set(i2s_lcd8_dev_t *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL)

    i2s_lcd8_write_cmd(dev, RM68090_REG_RAM_ADDRESS_SET_HORIZONTAL_ADDRESS);
    i2s_lcd8_write_data(dev, x1);
    i2s_lcd8_write_data(dev, x2);

    i2s_lcd8_write_cmd(dev, RM68090_REG_RAM_ADDRESS_SET_VERTICAL_ADDRESS);
    i2s_lcd8_write_data(dev, y1);
    i2s_lcd8_write_data(dev, y2);

    i2s_lcd8_write_cmd(dev, RM68090_REG_WRITE_DATA_TO_GRAM);
}

void rm68090_clear(i2s_lcd8_dev_t *dev, uint16_t color) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL)

    rm68090_address_set(dev, 0, 0, RM68090_MAX_HEIGHT - 1, RM68090_MAX_WIDTH - 1);

    uint16_t *tmp = dev->buffer;
    for (int i = 0; i < RM68090_MAX_WIDTH; ++i) {
        *tmp++ = color;
    }
    int max = color == LCD_COLOR_WHITE ? RM68090_MAX_WIDTH : RM68090_MAX_WIDTH / 2;
    for (int w = 0; w < max; w++) {
        i2s_lcd8_write_datan(dev, dev->buffer, RM68090_MAX_HEIGHT * sizeof(uint16_t));
    }

    rm68090_address_set(dev, 10, 30, 10, 30);
    i2s_lcd8_write_data(dev, LCD_COLOR_WHITE);
}

esp_err_t rm68090_set_rotation(i2s_lcd8_dev_t *dev, rotation_t rotation) {
    if (rotation == dev->rotation) {
        return ESP_OK;
    }
    dev->width = (rotation & 1) ? RM68090_MAX_HEIGHT : RM68090_MAX_WIDTH;
    dev->height = (rotation & 1) ? RM68090_MAX_WIDTH : RM68090_MAX_HEIGHT;

    uint16_t val;
    switch (rotation) {
        case ROTATION_PORTRAIT:       // PORTRAIT:
            val = 0x48;               // MY=0, MX=1, MV=0, ML=0, BGR=1
            break;
        case ROTATION_LANDSCAPE:      // LANDSCAPE: 90 degrees
            val = 0x28;               // MY=0, MX=0, MV=1, ML=0, BGR=1
            break;
        case ROTATION_PORTRAIT_REV:   // PORTRAIT_REV: 180 degrees
            val = 0x98;               // MY=1, MX=0, MV=0, ML=1, BGR=1
            break;
        case ROTATION_LANDSCAPE_REV:  // LANDSCAPE_REV: 270 degrees
            val = 0xF8;               // MY=1, MX=1, MV=1, ML=1, BGR=1
            break;
        default:
            return ESP_ERR_INVALID_ARG;
    }
    val ^= 0x80;

//    config->width = RM68090_MAX_WIDTH;
//    config->height = RM68090_MAX_HEIGHT;
//    config->rotation = ROTATION_PORTRAIT;
    return ESP_OK;
}