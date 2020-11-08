#include <math.h>

#include "esp_err.h"
#include "esp_log.h"

#include "error.h"

#include "lcd.h"
#include "rm68090_regs.h"
#include "rm68090.h"
#include "utils.h"

#define BGR_ON  BIT(12)
#define MV_ON   BIT(3)
#define MX_ON   BIT(8)
#define GS_ON   BIT(15)
#define I_D1_ON BIT(5)
#define I_D0_ON BIT(4)
#define VLE_ON  BIT(1)
#define REV_ON  BIT(0)

static const char *const TAG = "rm68090";

// Init registers adapted from https://github.com/prenticedavid/MCUFRIEND_kbv/blob/master/MCUFRIEND_kbv.cpp
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
        RM68090_REG_POWER_CONTROL_1, 0x0000,                          // SAP, BT[3:0], AP, DSTB, SLP, STB
        RM68090_REG_POWER_CONTROL_2, 0x0007,                          // DC1[2:0], DC0[2:0], VC[2:0]
        RM68090_REG_POWER_CONTROL_3, 0x0000,                          // VREG1OUT voltage
        RM68090_REG_POWER_CONTROL_4, 0x0000,                          // VDV[4:0] for VCOM amplitude
        RM68090_REG_DISPLAY_CONTROL_1, 0x0001,
        LCD_CMD_DELAY, 200, // Dis-charge capacitor power voltage
        RM68090_REG_POWER_CONTROL_1, 0x1690,                          // SAP=1, BT=6, APE=1, AP=1, DSTB=0, SLP=0, STB=0
        RM68090_REG_POWER_CONTROL_2, 0x0227,                          // DC1=2, DC0=2, VC=7
        LCD_CMD_DELAY, 50,  // Wait 50ms
        RM68090_REG_POWER_CONTROL_3, 0x000d,                          // VCIRE=1, PON=0, VRH=5
        LCD_CMD_DELAY, 50,  // Wait 50ms
        RM68090_REG_POWER_CONTROL_4, 0x1200,                          // VDV=28 for VCOM amplitude
        RM68090_REG_POWER_CONTROL_7, 0x000a,                          // VCM=10 for VCOMH
        RM68090_REG_FRAME_RATE_AND_COLOR_CONTROL, 0x000d,             // Set Frame Rate
        LCD_CMD_DELAY, 50,  // Wait 50ms
        RM68090_REG_RAM_ADDRESS_SET_HORIZONTAL_ADDRESS, 0x0000,       // GRAM horizontal Address
        RM68090_REG_RAM_ADDRESS_SET_VERTICAL_ADDRESS, 0x0000,         // GRAM Vertical Address
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

static esp_err_t rm68090_vertical_scroll(lcd_dev_t *dev, int16_t top, int16_t scroll_lines, int16_t offset) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] top: %d scroll_lines: %d offset: %d", __FUNCTION__, top, scroll_lines, offset);

    if (offset <= -scroll_lines || offset >= scroll_lines) {
        offset = 0; // Valid scroll.
    }
    int16_t vsp = top + offset; // Vertical start position.
    if (offset < 0) {
        vsp += scroll_lines;      // Keep in unsigned range.
    }
    // !NDL, VLE, REV
    dev->device->comm->write_reg16(dev, RM68090_REG_BASE_IMAGE_DISPLAY_CONTROL_INSTRUCTION_1,
                                   dev->registers.reverse | VLE_ON);
    // VL#
    dev->device->comm->write_reg16(dev, RM68090_REG_BASE_IMAGE_DISPLAY_CONTROL_INSTRUCTION_A, vsp);

    return ESP_OK;
}

static void rm68090_address_set(lcd_dev_t *dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL);

    dev->device->comm->write_reg16(dev, dev->registers.mc, x1);
    dev->device->comm->write_reg16(dev, dev->registers.ec, x2);

    dev->device->comm->write_reg16(dev, dev->registers.mp, y1);
    dev->device->comm->write_reg16(dev, dev->registers.ep, y2);
}

static esp_err_t rm68090_set_rotation(lcd_dev_t *dev, rotation_t rotation) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    ARG_CHECK(rotation >= 0 && rotation <= ROTATION_MAX, "rotation is invalid");
    LLOG(TAG, "[%s] rotation: %d", __FUNCTION__, rotation);

    if (rotation == dev->rotation) {
        return ESP_OK;
    }
    bool is_landscape = rotation == ROTATION_LANDSCAPE || rotation == ROTATION_LANDSCAPE_REV;
    dev->registers.width = is_landscape ? RM68090_MAX_HEIGHT : RM68090_MAX_WIDTH;
    dev->registers.height = is_landscape ? RM68090_MAX_WIDTH : RM68090_MAX_HEIGHT;

    uint16_t val;
    switch (rotation) {
        case ROTATION_PORTRAIT:           // PORTRAIT:
            val = GS_ON | MX_ON | BGR_ON; // MY=0, MX=1, MV=0, ML=0, BGR=1
            break;
        case ROTATION_LANDSCAPE:          // LANDSCAPE: 90 degrees
            val = GS_ON | MV_ON | BGR_ON; // MY=0, MX=0, MV=1, ML=0, BGR=1
            break;
        case ROTATION_PORTRAIT_REV:       // PORTRAIT_REV: 180 degrees
            // TODO(sobrinho): Double check if ROTATION_PORTRAIT_REV is missing MY=1.
            val = BGR_ON;                 // MY=1, MX=0, MV=0, ML=1, BGR=1
            break;
        case ROTATION_LANDSCAPE_REV:      // LANDSCAPE_REV: 270 degrees
            val = MX_ON | MV_ON | BGR_ON; // MY=1, MX=1, MV=1, ML=1, BGR=1
            break;
        default:
            return ESP_ERR_INVALID_ARG;
    }
    dev->registers.mw = RM68090_REG_WRITE_DATA_TO_GRAM;
    if (is_landscape) {
        // Switch the horizontal<->vertical registers.
        dev->registers.mp = RM68090_REG_RAM_ADDRESS_SET_HORIZONTAL_ADDRESS;
        dev->registers.mc = RM68090_REG_RAM_ADDRESS_SET_VERTICAL_ADDRESS;
        dev->registers.sp = RM68090_REG_WINDOW_ADDRESS_WRITE_CONTROL_INSTRUCTION_0;
        dev->registers.sc = RM68090_REG_WINDOW_ADDRESS_WRITE_CONTROL_INSTRUCTION_2;
        dev->registers.ep = RM68090_REG_WINDOW_ADDRESS_WRITE_CONTROL_INSTRUCTION_1;
        dev->registers.ec = RM68090_REG_WINDOW_ADDRESS_WRITE_CONTROL_INSTRUCTION_3;
    } else {
        dev->registers.mc = RM68090_REG_RAM_ADDRESS_SET_HORIZONTAL_ADDRESS;
        dev->registers.mp = RM68090_REG_RAM_ADDRESS_SET_VERTICAL_ADDRESS;
        dev->registers.sc = RM68090_REG_WINDOW_ADDRESS_WRITE_CONTROL_INSTRUCTION_0;
        dev->registers.sp = RM68090_REG_WINDOW_ADDRESS_WRITE_CONTROL_INSTRUCTION_2;
        dev->registers.ec = RM68090_REG_WINDOW_ADDRESS_WRITE_CONTROL_INSTRUCTION_1;
        dev->registers.ep = RM68090_REG_WINDOW_ADDRESS_WRITE_CONTROL_INSTRUCTION_3;
    }

    // Gate Scan Line (0xa700)
    uint16_t gs = 0x2700 | (val & GS_ON);                         // GS:1. Gate Line No = G320.
    dev->device->comm->write_reg16(dev, RM68090_REG_BASE_IMAGE_DISPLAY_CONTROL_INSTRUCTION_0, gs);

    // Set Driver Output Control
    uint16_t ss = val & MX_ON;                                    // SS:1. Source driver output shift from S720 to S1.
    dev->device->comm->write_reg16(dev, RM68090_REG_DRIVER_OUTPUT_CONTROL, ss);

    // set GRAM write direction and BGR=1.
    uint16_t org = (val & (MV_ON | BGR_ON)) | I_D1_ON | I_D0_ON;  // Sets the vertical direction.
    dev->device->comm->write_reg16(dev, RM68090_REG_ENTRY_MODE,
                                   org);              // I/D1-0: 1. Horizontal and Vertical increment.

    rm68090_address_set(dev, 0, 0, dev->registers.width - 1, dev->registers.height - 1);
    rm68090_vertical_scroll(dev, 0, RM68090_MAX_HEIGHT, 0);
    return ESP_OK;
}

static esp_err_t rm68090_invert_display(lcd_dev_t *dev, bool reverse) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] reverse: %d", __FUNCTION__, reverse);

    dev->registers.reverse = reverse ? 0x0 : REV_ON;
    dev->device->comm->write_reg16(dev, RM68090_REG_BASE_IMAGE_DISPLAY_CONTROL_INSTRUCTION_1, dev->registers.reverse);

    return ESP_OK;
}

static esp_err_t rm68090_init_registers(const lcd_dev_t *dev, const uint16_t *table, size_t size) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    ARG_CHECK(table != NULL, ERR_PARAM_NULL);
    ARG_CHECK(size > 0, ERR_PARAM_LE_ZERO);
    LLOG(TAG, "[%s] count: %d", __FUNCTION__, (int) (size / sizeof(uint16_t)));

    while (size > 0) {
        uint16_t cmd = *table++;
        uint16_t d = *table++;
        if (cmd == LCD_CMD_DELAY)
            safe_delay_ms(d);
        else {
            lcd_write_reg(dev, cmd, d);
        }
        size -= 2 * sizeof(int16_t);
    }
    return ESP_OK;
}

static esp_err_t rm68090_init(lcd_dev_t *dev) {
    ESP_ERROR_CHECK(rm68090_init_registers(dev, RM68090_REG_VALUES, sizeof(RM68090_REG_VALUES)));
    dev->registers.reverse = 0x0;
    dev->rotation = ROTATION_UNKNOWN;
    ESP_ERROR_CHECK(rm68090_set_rotation(dev, dev->config.rotation));
    ESP_ERROR_CHECK(rm68090_invert_display(dev, false));

    return ESP_OK;
}

static void rm68090_draw_pixel(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y) {
    LLOG(TAG, "[%s] color: 0x%04x (%d, %d)", __FUNCTION__, color, x, y);

    rm68090_address_set(dev, x, y, x, y);
    dev->device->comm->write_reg16(dev, dev->registers.mw, color);
}

static void rm68090_prepare_draw(lcd_dev_t *dev) {
    dev->device->comm->write_regn(dev, dev->registers.mw, NULL, 0);
}

static void rm68090_fill(lcd_dev_t *dev, uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] color: 0x%04x (%d, %d) -> (%d, %d)", __FUNCTION__, color, x1, y1, x2, y2);
    x1 = clamp(x1, 0, dev->registers.width - 1);
    x2 = clamp(x2, 0, dev->registers.width - 1);
    y1 = clamp(y1, 0, dev->registers.height - 1);
    y2 = clamp(y2, 0, dev->registers.height - 1);
    ARG_ERROR_CHECK(x1 <= x2, "x1 > x2");
    ARG_ERROR_CHECK(y1 <= y2, "y1 > y2");

    if (x1 == x2 && y1 == y2) {
        rm68090_draw_pixel(dev, color, x1, y1);
        return;
    }
    rm68090_address_set(dev, x1, y1, x2, y2);
    rm68090_prepare_draw(dev);

    size_t size_remain = (((x2 - x1) + 1) * ((y2 - y1) + 1)) * sizeof(uint16_t);
    lcd_write_data16n(dev, color, size_remain);
}

static void rm68090_draw(lcd_dev_t *dev, const uint16_t *buf, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] (%d, %d) -> (%d, %d)", __FUNCTION__, x, y, width, height);

    rm68090_address_set(dev, x, y, x + width - 1, y + height - 1);
    rm68090_prepare_draw(dev);

    size_t to_write = width * height * sizeof(uint16_t);
    lcd_write_datan(dev, buf, to_write);
}

static void rm68090_clear(lcd_dev_t *dev, uint16_t color) {
    ARG_ERROR_CHECK(dev != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] color: 0x%04x", __FUNCTION__, color);

    rm68090_fill(dev, color, 0, 0, dev->registers.width - 1, dev->registers.height - 1);
}

const lcd_device_t rm68090 = {
        .init = rm68090_init,
        .draw_pixel = rm68090_draw_pixel,
        .fill = rm68090_fill,
        .draw = rm68090_draw,
        .clear = rm68090_clear,
        .set_rotation = rm68090_set_rotation,
        .vertical_scroll = rm68090_vertical_scroll,
        .invert_display = rm68090_invert_display,
};
