#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

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
static const char *TAG = "lcd";
static const gpio_num_t LCD_RST = GPIO_NUM_16;
static const gpio_num_t LCD_CS = GPIO_NUM_15;
static const gpio_num_t LCD_RS = GPIO_NUM_2;
static const gpio_num_t LCD_WR = GPIO_NUM_0;
static const gpio_num_t LCD_RD = GPIO_NUM_17;

static const gpio_num_t LCD_CONTROL[] = {LCD_RST, LCD_CS, LCD_RS, LCD_WR, LCD_RD};
static const uint8_t LCD_CONTROL_LEN = sizeof(LCD_CONTROL) / sizeof(LCD_CONTROL[0]);
static const gpio_num_t LCD_DATA[] = {GPIO_NUM_23, GPIO_NUM_18, GPIO_NUM_14, GPIO_NUM_27,
                                      GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_22, GPIO_NUM_19};
static const uint8_t LCD_DATA_LEN = sizeof(LCD_DATA) / sizeof(LCD_DATA[0]);

static const uint16_t RM68090_REG_VALUES[] = {
#if 0
0x00E5, 0x78F0,     // set SRAM internal timing
0x0001, 0x0100,     // set Driver Output Control
0x0002, 0x0200,     // set 1 line inversion
0x0003, 0x1030,     // set GRAM write direction and BGR=1.
0x0004, 0x0000,     // Resize register
0x0005, 0x0000,     // .kbv 16bits Data Format Selection
0x0008, 0x0207,     // set the back porch and front porch
0x0009, 0x0000,     // set non-display area refresh cycle ISC[3:0]
0x000A, 0x0000,     // FMARK function
0x000C, 0x0000,     // RGB interface setting
0x000D, 0x0000,     // Frame marker Position
0x000F, 0x0000,     // RGB interface polarity
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

uint8_t unlock_1520[] = {0xB0, 2, 0x00, 0x00};
uint8_t unlock_1526[] = {0xB0, 2, 0x3F, 0x3F};
uint8_t unlock_8357[] = {0xB9, 3, 0xFF, 0x83, 0x57};
uint8_t unlock_5310[] = {0xED, 2, 0x01, 0xFE};
uint8_t d5310_0_in[] = {0xEE, 2, 0xDE, 0x21}; //enter CMD3 8bit args
uint8_t d5310_1_in[] = {0xBF, 1, 0xAA}; //enter page#1 16bit args
uint8_t d5310_1_out[] = {0x00, 1, 0xAA}; //leave page#1 16bit args
uint8_t d1526nvm[] = {0xE2, 1, 0x3F};
uint8_t *unlock = NULL;
uint8_t *page_N = NULL;

static void lcd_delay_us(uint8_t us) {
    ets_delay_us(us);
}

static void lcd_delay(uint8_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void lcd_set_write_dir(void) {
    for (int i = 0; i < LCD_DATA_LEN; ++i) {
        gpio_set_direction(LCD_DATA[i], GPIO_MODE_OUTPUT);
    }
}

static void lcd_set_read_dir(void) {
    for (int i = 0; i < LCD_DATA_LEN; ++i) {
        gpio_set_direction(LCD_DATA[i], GPIO_MODE_INPUT);
    }
}

static esp_err_t lcd_gpios(void) {
    LOG(TAG, "lcd_gpios");
    gpio_config_t cfg = {
            .mode = GPIO_MODE_INPUT_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    for (int i = 0; i < LCD_CONTROL_LEN; ++i) {
        cfg.pin_bit_mask |= BIT64(LCD_CONTROL[i]);
    }
    for (int i = 0; i < LCD_DATA_LEN; ++i) {
        cfg.pin_bit_mask |= BIT64(LCD_DATA[i]);
    }
    ESP_ERROR_CHECK(gpio_config(&cfg));

    return ESP_OK;
}

static esp_err_t lcd_controller_init(void) {
    LOG(TAG, "lcd_controller_init");
    ESP_ERROR_CHECK(gpio_set_level(LCD_CS, HIGH));
    ESP_ERROR_CHECK(gpio_set_level(LCD_RS, HIGH));
    ESP_ERROR_CHECK(gpio_set_level(LCD_WR, HIGH));
    ESP_ERROR_CHECK(gpio_set_level(LCD_RD, HIGH));
    ESP_ERROR_CHECK(gpio_set_level(LCD_RST, HIGH));
    return ESP_OK;
}

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

static void lcd_write8(uint16_t data) {
    LOG(TAG, "lcd_write8");
    for (int i = 0; i < LCD_DATA_LEN; ++i) {
        LOG(TAG, " [%d:%d]: %01x", i, LCD_DATA[i], (uint16_t) (data & BIT(i)));
        gpio_set_level(LCD_DATA[i], (data & BIT(i)) ? 1 : 0);
    }
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
    if (unlock) lcd_push_command(unlock[0], unlock + 2, unlock[1]);
    if (page_N) lcd_push_command(page_N[0], page_N + 2, page_N[1]);
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
    lcd_read_reg(0xB0, 2, "RGB Interface Signal Control");
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

esp_err_t lcd_init(void) {
    ESP_ERROR_CHECK(lcd_gpios());
    ESP_ERROR_CHECK(lcd_controller_init());
    ESP_ERROR_CHECK(lcd_reset());
    ESP_ERROR_CHECK(lcd_init_table16(RM68090_REG_VALUES, sizeof(RM68090_REG_VALUES)));

    lcd_clear(0xf800);
    // lcd_read_regs("diagnose any controller");

    return ESP_OK;
}
