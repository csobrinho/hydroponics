#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "buses.h"
#include "lcd.h"

#define TFTLCD_DELAY 0xFFFF
#define HIGH 1
#define LOW 0
#define LOG(...) ESP_LOGD(__VA_ARGS__)

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// RM68090 is an ILI9320-style controller.
#define REG_ADDRESS_SET_HORIZONTAL 0x20
#define REG_ADDRESS_SET_VERTICAL   0x20

static const char *TAG = "lcd";
//static const gpio_num_t LCD_RST = GPIO_NUM_16;
//static const gpio_num_t LCD_CS = GPIO_NUM_15;
//static const gpio_num_t LCD_RS = GPIO_NUM_2;
//static const gpio_num_t LCD_WR = GPIO_NUM_17;
//static const gpio_num_t LCD_RD = GPIO_NUM_0;

//static const gpio_num_t LCD_CONTROL[] = {LCD_RST, LCD_CS, LCD_RS, LCD_WR, LCD_RD};
//static const gpio_num_t LCD_DATA[] = {GPIO_NUM_23, GPIO_NUM_18, GPIO_NUM_14, GPIO_NUM_27,
//                                      GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_22, GPIO_NUM_19};

static const uint16_t RM68090_REG_VALUES[] = {
#if 1
        0x00E5, 0x78F0,     // set SRAM internal timing
        0x0001, 0x0100,     // set Driver Output Control
        0x0002, 0x0200,     // set 1 line inversion
        0x0003, 0x1030,     // set GRAM write direction and BGR=1.
        0x0004, 0x0000,     // Resize register
        0x0005, 0x0000,     // .kbv 16bits Data Format Selection
        0x0008, 0x0207,     // set the back porch and front porch
        0x0009, 0x0000,     // set non-display area refresh cycle ISC[3:0]
        0x000A, 0x0000,     // FMARK function
        0x000C, 0x0000,     // lcd_rgb interface setting
        0x000D, 0x0000,     // Frame marker Position
        0x000F, 0x0000,     // lcd_rgb interface polarity
        // ----------- Power On sequence ----------- //
        0x0010, 0x0000,     // SAP, BT[3:0], AP, DSTB, SLP, STB
        0x0011, 0x0007,     // DC1[2:0], DC0[2:0], VC[2:0]
        0x0012, 0x0000,     // VREG1OUT voltage
        0x0013, 0x0000,     // VDV[4:0] for VCOM amplitude
        0x0007, 0x0001,
        TFTLCD_DELAY, 200,  // Dis-charge capacitor power voltage
        0x0010, 0x1690,     // SAP=1, BT=6, APE=1, AP=1, DSTB=0, SLP=0, STB=0
        0x0011, 0x0227,     // DC1=2, DC0=2, VC=7
        TFTLCD_DELAY, 50,   // wait_ms 50ms
        0x0012, 0x000D,     // VCIRE=1, PON=0, VRH=5
        TFTLCD_DELAY, 50,   // wait_ms 50ms
        0x0013, 0x1200,     // VDV=28 for VCOM amplitude
        0x0029, 0x000A,     // VCM=10 for VCOMH
        0x002B, 0x000D,     // Set Frame Rate
        TFTLCD_DELAY, 50,   // wait_ms 50ms
        0x0020, 0x0000,     // GRAM horizontal Address
        0x0021, 0x0000,     // GRAM Vertical Address
        // ----------- Adjust the Gamma Curve ----------//
        0x0030, 0x0000,
        0x0031, 0x0404,
        0x0032, 0x0003,
        0x0035, 0x0405,
        0x0036, 0x0808,
        0x0037, 0x0407,
        0x0038, 0x0303,
        0x0039, 0x0707,
        0x003C, 0x0504,
        0x003D, 0x0808,
        //------------------ Set GRAM area ---------------//
        0x0060, 0x2700,     // Gate Scan Line GS=0 [0xA700]
        0x0061, 0x0001,     // NDL,VLE, REV .kbv
        0x006A, 0x0000,     // set scrolling line
        //-------------- Partial Display Control ---------//
        0x0080, 0x0000,
        0x0081, 0x0000,
        0x0082, 0x0000,
        0x0083, 0x0000,
        0x0084, 0x0000,
        0x0085, 0x0000,
        //-------------- Panel Control -------------------//
        0x0090, 0x0010,
        0x0092, 0x0000,
        0x0007, 0x0133,     // 262K color and display ON
#else
0x00e5, 0x8000,
0x0000, 0x0001,
0x0001, 0x0100,
0x0002, 0x0700,
0x0003, 0x1030,
0x0004, 0x0000,
0x0008, 0x0202,
0x0009, 0x0000,
0x000a, 0x0000,
0x000c, 0x0000,
0x000d, 0x0000,
0x000f, 0x0000,
//*********************************************Power On
0x0010, 0x0000,
0x0011, 0x0000,
0x0012, 0x0000,
0x0013, 0x0000,
0x0010, 0x17b0,
0x0011, 0x0037,
0x0012, 0x0138,
0x0013, 0x1700,
0x0029, 0x000d,
0x0020, 0x0000,
0x0021, 0x0000,
//********************************************Set gamma
0x0030, 0x0001,
0x0031, 0x0606,
0x0032, 0x0304,
0x0033, 0x0202,
0x0034, 0x0202,
0x0035, 0x0103,
0x0036, 0x011d,
0x0037, 0x0404,
0x0038, 0x0404,
0x0039, 0x0404,
0x003c, 0x0700,
0x003d, 0x0a1f,
//**********************************************Set Gram aera
0x0050, 0x0000,
0x0051, 0x00ef,
0x0052, 0x0000,
0x0053, 0x013f,
0x0060, 0x2700,
0x0061, 0x0001,
0x006a, 0x0000,
//*********************************************Paratial display
0x0090, 0x0010,
0x0092, 0x0000,
0x0093, 0x0003,
0x0095, 0x0101,
0x0097, 0x0000,
0x0098, 0x0000,
//******************************************** Plan Control
0x0007, 0x0021,
0x0007, 0x0031,
0x0007, 0x0173,
#endif
};

static inline void lcd_delay_us(uint8_t us) {
    ets_delay_us(us);
}

static inline void lcd_delay(uint8_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static inline void lcd_write8(const lcd_config_t *config, uint8_t data) {
    GPIO.out_w1ts = config->data_set_mask[data];
    GPIO.out_w1tc = config->data_clear_mask[data];
}

static inline void lcd_wr_strobe(const lcd_config_t *config) {
    GPIO.out_w1tc = config->wr_clear_mask;
    lcd_delay_us(config->wr_delay_us);
    GPIO.out_w1ts = config->wr_set_mask;
}

static inline void lcd_write_bus(const lcd_config_t *config, uint16_t data) {
    lcd_write8(config, HIGH_BYTE(data));
    lcd_wr_strobe(config);
    lcd_write8(config, LOW_BYTE(data));
    lcd_wr_strobe(config);
}

static inline void lcd_write_command(const lcd_config_t *config, uint16_t command) {
    LOG(TAG, "lcd cmd: 0x%02x", command);
    gpio_set_level(config->gpio_rs, LOW);
    lcd_write_bus(config, command);
}

static inline void lcd_write_data(const lcd_config_t *config, uint16_t data) {
    LOG(TAG, "lcd data: 0x%02x", data);
    gpio_set_level(config->gpio_rs, HIGH);
    lcd_write_bus(config, data);
}

static void lcd_write_register(const lcd_config_t *config, uint16_t command, uint16_t data) {
    lcd_write_command(config, command);
    lcd_write_data(config, data);
}

static void lcd_address_set(const lcd_config_t *config, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    lcd_write_register(config, 0x2a, x1);
    lcd_write_register(config, 0x2a, x2);
    lcd_write_register(config, 0x2b, y1);
    lcd_write_register(config, 0x2b, y2);
    lcd_write_command(config, 0x2c);
}

static esp_err_t lcd_init_gpios(lcd_config_t *config) {
    LOG(TAG, "lcd_init_gpios");

    gpio_num_t gpios[] = {
            config->gpio_rst,
            config->gpio_cs,
            config->gpio_rs,
            config->gpio_wr,
            config->gpio_rd,
            config->gpio_data[0], config->gpio_data[1], config->gpio_data[2], config->gpio_data[3],
            config->gpio_data[4], config->gpio_data[5], config->gpio_data[6], config->gpio_data[7],
    };
    gpio_config_t cfg = {
            .mode = GPIO_MODE_INPUT_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    for (int i = 0; i < sizeof(gpios); ++i) {
        if (gpios[i] == GPIO_NUM_NC) {
            continue;
        }
        gpio_pad_select_gpio(gpios[i]);
        cfg.pin_bit_mask |= BIT64(gpios[i]);
    }
    ESP_ERROR_CHECK(gpio_config(&cfg));

    for (int byte = 0; byte < 256; ++byte) {
        for (int bit = 0; bit < 8; ++bit) {
            if (byte & BIT(bit)) {
                config->data_set_mask[byte] |= BIT(bit);
            } else {
                config->data_clear_mask[byte] |= BIT(bit);
            }
        }
    }
    config->wr_set_mask = BIT(config->gpio_wr);
    config->wr_clear_mask = BIT(config->gpio_wr);
    return ESP_OK;
}


static void lcd_init_registers(const lcd_config_t *config) {
    gpio_set_level(config->gpio_cs, HIGH);
    gpio_set_level(config->gpio_wr, HIGH);
    gpio_set_level(config->gpio_cs, LOW);

    lcd_write_register(config, 0xe5, 0x8000);
    lcd_write_register(config, 0x00, 0x0001);

    lcd_write_register(config, 0x01, 0x0100);
    lcd_write_register(config, 0x02, 0x0700);
    lcd_write_register(config, 0x03, 0x1030);
    lcd_write_register(config, 0x04, 0x0000);
    lcd_write_register(config, 0x08, 0x0202);
    lcd_write_register(config, 0x09, 0x0000);
    lcd_write_register(config, 0x0a, 0x0000);
    lcd_write_register(config, 0x0c, 0x0000);
    lcd_write_register(config, 0x0d, 0x0000);
    lcd_write_register(config, 0x0f, 0x0000);
    //********************************************* Power On
    lcd_write_register(config, 0x10, 0x0000);
    lcd_write_register(config, 0x11, 0x0000);
    lcd_write_register(config, 0x12, 0x0000);
    lcd_write_register(config, 0x13, 0x0000);

    lcd_write_register(config, 0x10, 0x17b0);
    lcd_write_register(config, 0x11, 0x0037);

    lcd_write_register(config, 0x12, 0x0138);

    lcd_write_register(config, 0x13, 0x1700);
    lcd_write_register(config, 0x29, 0x000d);

    lcd_write_register(config, 0x20, 0x0000);
    lcd_write_register(config, 0x21, 0x0000);
    //******************************************** Set gamma
    lcd_write_register(config, 0x30, 0x0001);
    lcd_write_register(config, 0x31, 0x0606);
    lcd_write_register(config, 0x32, 0x0304);
    lcd_write_register(config, 0x33, 0x0202);
    lcd_write_register(config, 0x34, 0x0202);
    lcd_write_register(config, 0x35, 0x0103);
    lcd_write_register(config, 0x36, 0x011d);
    lcd_write_register(config, 0x37, 0x0404);
    lcd_write_register(config, 0x38, 0x0404);
    lcd_write_register(config, 0x39, 0x0404);
    lcd_write_register(config, 0x3c, 0x0700);
    lcd_write_register(config, 0x3d, 0x0a1f);
    //********************************************** Set Gram area
    lcd_write_register(config, 0x50, 0x0000);
    lcd_write_register(config, 0x51, 0x00ef);
    lcd_write_register(config, 0x52, 0x0000);
    lcd_write_register(config, 0x53, 0x013f);
    lcd_write_register(config, 0x60, 0x2700);
    lcd_write_register(config, 0x61, 0x0001);
    lcd_write_register(config, 0x6a, 0x0000);
    //********************************************* Partial display
    lcd_write_register(config, 0x90, 0x0010);
    lcd_write_register(config, 0x92, 0x0000);
    lcd_write_register(config, 0x93, 0x0003);
    lcd_write_register(config, 0x95, 0x0101);
    lcd_write_register(config, 0x97, 0x0000);
    lcd_write_register(config, 0x98, 0x0000);
    //******************************************** Plan Control
    lcd_write_register(config, 0x07, 0x0021);

    lcd_write_register(config, 0x07, 0x0031);
    lcd_write_register(config, 0x07, 0x0173);
}

static void lcd_h_line(const lcd_config_t *config, uint16_t x, uint16_t y, uint16_t l, uint16_t c) {
    lcd_write_command(config, 0x002c); // write_memory_start
    gpio_set_level(config->gpio_rs, HIGH);
    gpio_set_level(config->gpio_cs, LOW);
    l += x;
    lcd_address_set(config, x, y, l, y);
    for (int i = 1; i <= l * 2; i++) {
        lcd_write_data(config, c);
    }
    gpio_set_level(config->gpio_cs, HIGH);
}

static void lcd_v_line(const lcd_config_t *config, uint16_t x, uint16_t y, uint16_t l, uint16_t c) {
    lcd_write_command(config, 0x002c); // write_memory_start
    gpio_set_level(config->gpio_rs, HIGH);
    gpio_set_level(config->gpio_cs, LOW);
    l += y;
    lcd_address_set(config, x, y, x, l);
    for (int i = 1; i <= l * 2; i++) {
        lcd_write_data(config, c);
    }
    gpio_set_level(config->gpio_cs, HIGH);
}

static void lcd_rect(const lcd_config_t *config, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c) {
    lcd_h_line(config, x, y, w, c);
    lcd_h_line(config, x, y + h, w, c);
    lcd_v_line(config, x, y, h, c);
    lcd_v_line(config, x + w, y, h, c);
}

static void lcd_rectf(const lcd_config_t *config, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c) {
    for (int i = 0; i < h; i++) {
        lcd_h_line(config, x, y, w, c);
        lcd_h_line(config, x, y + i, w, c);
    }
}

static inline int lcd_rgb(int r, int g, int b) {
    return r << 16 | g << 8 | b;
}

static void lcd_clear(const lcd_config_t *config, uint16_t j) {
    lcd_address_set(config, 0, 0, 320, 240);
    lcd_write_command(config, 0x002c); //write_memory_start
    gpio_set_level(config->gpio_rs, HIGH);
    gpio_set_level(config->gpio_cs, LOW);

    for (int w = 0; w < 320; w++) {
        for (int h = 0; h < 240; h++) {
            lcd_write_data(config, w);
            lcd_write_data(config, h);
        }
    }
    gpio_set_level(config->gpio_cs, HIGH);
}

int random_int(int min, int max) {
    return min + (int) random() % (max + 1 - min);
}

void loop(const lcd_config_t *config) {
    for (int i = 0; i < 1000; i++) {
        lcd_rect(config, random_int(0, 300), random_int(0, 300), random_int(0, 300), random_int(0, 300),
                 random_int(0, 65535)); // rectangle at x, y, with, height, color
    }
    lcd_clear(config, 0xf800);
}


static esp_err_t lcd_controller_init(const lcd_config_t *config) {
    LOG(TAG, "lcd_controller_init");
    ESP_ERROR_CHECK(gpio_set_level(config->gpio_cs, HIGH));
    ESP_ERROR_CHECK(gpio_set_level(config->gpio_rs, HIGH));
    ESP_ERROR_CHECK(gpio_set_level(config->gpio_wr, HIGH));
    ESP_ERROR_CHECK(gpio_set_level(config->gpio_rd, HIGH));
    ESP_ERROR_CHECK(gpio_set_level(config->gpio_rst, HIGH));
    return ESP_OK;
}

static esp_err_t lcd_init_table16(const uint16_t *table, int16_t size) {
    while (size > 0) {
        uint16_t cmd = *table++;
        uint16_t d = *table++;
        if (cmd == TFTLCD_DELAY)
            lcd_delay(d);
        else {
            lcd_write_register(cmd, d);
        }
        size -= 2 * sizeof(int16_t);
    }
    return ESP_OK;
}

#if 0
static esp_err_t lcd_reset(void) {
    LOG(TAG, "lcd_reset");
    ESP_ERROR_CHECK(gpio_set_level(LCD_RST, LOW));
    // lcd_delay(2);
    lcd_delay(10);
    ESP_ERROR_CHECK(gpio_set_level(LCD_RST, HIGH));
    lcd_delay(50); //allow controller to re-start
    // lcd_delay(10); //allow controller to re-start
    return ESP_OK;
}

static uint16_t lcd_read8(void) {
    LOG(TAG, "lcd_read8");
    uint16_t result = 0;
    for (int i = 0; i < LCD_DATA_LEN; ++i) {
        int level = gpio_get_level(LCD_DATA[i]) << i;
        LOG(TAG, " [%d:%d]: 0x%04x", i, LCD_DATA[i], result);
        LOG(TAG, "   [%d:%d]: 0x%04x", i, LCD_DATA[i], result);
        result |= level;
    }
    return result;
}

static void lcd_push_command(uint8_t command, uint8_t *block, int8_t n) {
    LOG(TAG, "lcd_push_command");
    gpio_set_level(LCD_CS, LOW);
    gpio_set_level(LCD_RS, LOW);
    gpio_set_level(LCD_RD, HIGH);
    gpio_set_level(LCD_WR, HIGH);
    lcd_write8(command);
    gpio_set_level(LCD_WR, LOW);
    lcd_delay_us(10);
    gpio_set_level(LCD_WR, HIGH);
    gpio_set_level(LCD_RS, HIGH);
    while (n--) {
        lcd_write8(*block++);
        gpio_set_level(LCD_WR, LOW);
        lcd_delay_us(10);
        gpio_set_level(LCD_WR, HIGH);
    }
    gpio_set_level(LCD_CS, HIGH);
}

static void lcd_write_command(uint16_t command) {
    LOG(TAG, "lcd_write_command cmd: %04x", command);
    lcd_set_write_dir();
    gpio_set_level(LCD_CS, LOW);
    gpio_set_level(LCD_RS, LOW);
    gpio_set_level(LCD_RD, HIGH);
    gpio_set_level(LCD_WR, HIGH);
    lcd_write8(command >> 8);
    gpio_set_level(LCD_WR, LOW);
    lcd_delay_us(10);
    gpio_set_level(LCD_WR, HIGH);
    lcd_write8(command);
    gpio_set_level(LCD_WR, LOW);
    lcd_delay_us(10);
    gpio_set_level(LCD_WR, HIGH);
    //    gpio_set_level(LCD_CS, HIGH);
}

static uint8_t lcd_read_data8(void) {
    LOG(TAG, "lcd_read_data8");
    lcd_set_read_dir();
    gpio_set_level(LCD_CS, LOW);
    gpio_set_level(LCD_RS, HIGH);
    gpio_set_level(LCD_RD, HIGH);
    gpio_set_level(LCD_WR, HIGH);

    gpio_set_level(LCD_RD, LOW);
    lcd_delay_us(10);
    uint8_t result = lcd_read8();
    gpio_set_level(LCD_RD, HIGH);
    lcd_delay_us(10);

    LOG(TAG, " 0x%02d", result);
    return result;
}

static void lcd_read_reg(uint16_t reg, uint8_t n, const char *msg) {
    LOG(TAG, "lcd_read_reg reg: 0x%04x n: 0x%04x msg: %s", reg, n, msg);
    lcd_reset();
    lcd_set_write_dir();
    printf("reg(0x%04x)", reg);
    lcd_write_command(reg);
    lcd_set_read_dir();
    while (n--) {
        printf(" %02x", lcd_read_data8());
    }
    gpio_set_level(LCD_CS, HIGH);
    lcd_set_write_dir();

    printf("\t%s\n", msg);
}

static void lcd_write_data(uint16_t data) {
    LOG(TAG, "lcd_write_data data: 0x%04x", data);
    lcd_set_write_dir();
    gpio_set_level(LCD_CS, LOW);
    gpio_set_level(LCD_RS, HIGH);
    gpio_set_level(LCD_RD, HIGH);
    gpio_set_level(LCD_WR, HIGH);

    lcd_write8(data >> 8);

    gpio_set_level(LCD_WR, LOW);
    lcd_delay_us(10);
    gpio_set_level(LCD_WR, HIGH);

    lcd_write8(data);

    gpio_set_level(LCD_WR, LOW);
    lcd_delay_us(10);
    gpio_set_level(LCD_WR, HIGH);

    gpio_set_level(LCD_CS, HIGH);
}

static uint16_t lcd_read_data16() {
    uint16_t result = lcd_read_data8() << 8;
    result |= lcd_read_data8();
    return result;
}

static void lcd_write_register(uint16_t addr, uint16_t data) {
    lcd_write_command(addr);
    lcd_write_data(data);
}

static void lcd_read_regs(const char *title) {
    printf("%s\n", title);
    lcd_read_reg(0x00, 2, "ID: ILI9320, ILI9325, ILI9335, ...");
    lcd_read_reg(0x04, 4, "Manufacturer ID");
    lcd_read_reg(0x09, 5, "Status Register");
    lcd_read_reg(0x0A, 2, "Get Powsr Mode");
    lcd_read_reg(0x0C, 2, "Get Pixel Format");
    lcd_read_reg(0x30, 5, "PTLAR");
    lcd_read_reg(0x33, 7, "VSCRLDEF");
    lcd_read_reg(0x61, 2, "RDID1 HX8347-G");
    lcd_read_reg(0x62, 2, "RDID2 HX8347-G");
    lcd_read_reg(0x63, 2, "RDID3 HX8347-G");
    lcd_read_reg(0x64, 2, "RDID1 HX8347-A");
    lcd_read_reg(0x65, 2, "RDID2 HX8347-A");
    lcd_read_reg(0x66, 2, "RDID3 HX8347-A");
    lcd_read_reg(0x67, 2, "RDID Himax HX8347-A");
    lcd_read_reg(0x70, 2, "Panel Himax HX8347-A");
    lcd_read_reg(0xA1, 5, "RD_DDB SSD1963");
    lcd_read_reg(0xB0, 2, "lcd_rgb Interface Signal Control");
    lcd_read_reg(0xB3, 5, "Frame Memory");
    lcd_read_reg(0xB4, 2, "Frame Mode");
    lcd_read_reg(0xB6, 5, "Display Control");
    lcd_read_reg(0xB7, 2, "Entry Mode Set");
    lcd_read_reg(0xBF, 6, "ILI9481, HX8357-B");
    lcd_read_reg(0xC0, 9, "Panel Control");
    lcd_read_reg(0xC1, 4, "Display Timing");
    lcd_read_reg(0xC5, 2, "Frame Rate");
    lcd_read_reg(0xC8, 13, "GAMMA");
    lcd_read_reg(0xCC, 2, "Panel Control");
    lcd_read_reg(0xD0, 4, "Power Control");
    lcd_read_reg(0xD1, 4, "VCOM Control");
    lcd_read_reg(0xD2, 3, "Power Normal");
    lcd_read_reg(0xD3, 4, "ILI9341, ILI9488");
    lcd_read_reg(0xD4, 4, "Novatek");
    lcd_read_reg(0xDA, 2, "RDID1");
    lcd_read_reg(0xDB, 2, "RDID2");
    lcd_read_reg(0xDC, 2, "RDID3");
    lcd_read_reg(0xE0, 16, "GAMMA-P");
    lcd_read_reg(0xE1, 16, "GAMMA-N");
    lcd_read_reg(0xEF, 6, "ILI9327");
    lcd_read_reg(0xF2, 12, "Adjust Control 2");
    lcd_read_reg(0xF6, 4, "Interface Control");
}

static void lcd_address_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    lcd_write_register(0x2a, x1);
    lcd_write_register(0x2a, x2);
    lcd_write_register(0x2b, y1);
    lcd_write_register(0x2b, y2);
    lcd_write_command(0x2c);
}

static void lcd_clear(uint16_t color) {
    lcd_address_set(0, 0, 320, 240);
    lcd_write_command(0x002c); //write_memory_start
    for (int w = 0; w < 320; w++) {
        for (int h = 0; h < 240; h++) {
            lcd_write_data(color);
        }
    }
}
#endif

static esp_err_t lcd_reset(void) {
    gpio_set_level(LCD_RST, HIGH);
    lcd_delay(5);
    gpio_set_level(LCD_RST, LOW);
    lcd_delay(15);
    gpio_set_level(LCD_RST, HIGH);
    lcd_delay(15);

    return ESP_OK;
}

esp_err_t lcd_init(lcd_config_t *config) {
    ESP_ERROR_CHECK(lcd_init_gpios(config));
    ESP_ERROR_CHECK(lcd_controller_init(config));
    ESP_ERROR_CHECK(lcd_reset(config));
    lcd_init_registers(config);
    (void) RM68090_REG_VALUES;
    // lcd_init_table16(RM68090_REG_VALUES, sizeof(RM68090_REG_VALUES));

    // lcd_clear(RED);
    // lcd_read_regs("diagnose any controller");
    //    loop();
    return ESP_OK;
}
