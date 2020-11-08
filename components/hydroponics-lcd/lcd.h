#ifndef HYDROPONICS_DRIVER_LCD_LCD_H
#define HYDROPONICS_DRIVER_LCD_LCD_H

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "esp_log.h"

// #define LLOG(args...)       ESP_LOGI(args)
// #define LLOG_HEX(tag, b, l) ESP_LOG_BUFFER_HEX(tag, b, l)
#define LLOG(args...)          do {} while (0)
#define LLOG_HEX(tag, b, len)  do {} while (0)

#define LCD_CMD_DELAY   0xFFFF // Used to add a delay inside the 'lcd_init_registers'.
#define LCD_CMD8_DELAY    0xFF // Used to add a delay inside the 'lcd_init_registers'.

#define U88_TO_U16(l, h) (((uint16_t) (h) << 8) | (l))
#define U8_HI(b)        ((uint8_t) ((b) >> 8))
#define U8_LO(b)        ((uint8_t) (b))

typedef enum {
    ROTATION_UNKNOWN = -1,
    ROTATION_PORTRAIT = 0,
    ROTATION_LANDSCAPE = 1,
    ROTATION_PORTRAIT_REV = 2,
    ROTATION_LANDSCAPE_REV = 3,
    ROTATION_MAX,
} rotation_t;

typedef enum {
    LCD_TYPE_PARALLEL = 0,
    LCD_TYPE_SPI = 1,
} lcd_type_t;

typedef struct {
    gpio_num_t data_width;      /*!< Parallel data width, 16bit or 8bit available */
    gpio_num_t data_io_num[16]; /*!< Parallel data output IO*/
    gpio_num_t ws_io_num;       /*!< write clk io */
    gpio_num_t rs_io_num;       /*!< rs io num */
    gpio_num_t rd_io_num;       /*!< (optional) rd io num */
} lcd_parallel_config_t;

typedef struct {
    gpio_num_t mosi_io_num;                /*!< GPIO pin for Master Out Slave In (=spi_d) signal. */
    gpio_num_t miso_io_num;                /*!< GPIO pin for Master In Slave Out (=spi_q) signal, or -1 if not used. */
    gpio_num_t sclk_io_num;                /*!< GPIO pin for Spi CLocK signal. */
    gpio_num_t cs_io_num;                  /*!< CS GPIO pin for this device, or -1 if not used. */
    gpio_num_t dc_io_num;                  /*!< D/C GPIO pin for this device. */
    uint8_t mode;                          /*!< SPI mode (0-3) */
    int clock_speed_hz;                    /*!< Clock speed, divisors of 80MHz, in Hz. See ``SPI_MASTER_FREQ_*``. */
    spi_host_device_t host;
    int dma_chan;
} lcd_spi_config_t;

typedef const struct {
    lcd_type_t type;            /*!< Screen type: parallel or spi. */
    struct {
        size_t width;           /*!< Screen width. */
        size_t height;          /*!< Screen height. */
        size_t bytes;           /*!< Screen number of bytes per pixel. */
        size_t divisor;         /*!< Screen divisor defines the max screen size that is pre-allocated for full frame. */
        uint32_t caps;          /*!< Bitwise OR of MALLOC_CAP_* flags indicating the type of memory to be allocated. */
    } screen;
    union {
        lcd_parallel_config_t parallel;
        lcd_spi_config_t spi;
    };
    gpio_num_t rst_io_num;      /*!< (optional) reset io num. */
    gpio_num_t led_io_num;      /*!< (optional) led io num. */
    rotation_t rotation;        /*!< Initial rotation. */
} lcd_config_t;

typedef enum {
    LCD_COLOR_BLUE = 0x001F,
    LCD_COLOR_TEAL = 0x0438,
    LCD_COLOR_GREEN = 0x07E0,
    LCD_COLOR_CYAN = 0x07FF,
    LCD_COLOR_RED = 0xF800,
    LCD_COLOR_MAGENTA = 0xF81F,
    LCD_COLOR_YELLOW = 0xFFE0,
    LCD_COLOR_ORANGE = 0xFC00,
    LCD_COLOR_PINK = 0xF81F,
    LCD_COLOR_PURPLE = 0x8010,
    LCD_COLOR_GREY = 0xC618,
    LCD_COLOR_WHITE = 0xFFFF,
    LCD_COLOR_BLACK = 0x0000,

    LCD_COLOR_LTBLUE = 0xB6DF,
    LCD_COLOR_LTTEAL = 0xBF5F,
    LCD_COLOR_LTGREEN = 0xBFF7,
    LCD_COLOR_LTCYAN = 0xC7FF,
    LCD_COLOR_LTRED = 0xFD34,
    LCD_COLOR_LTMAGENTA = 0xFD5F,
    LCD_COLOR_LTYELLOW = 0xFFF8,
    LCD_COLOR_LTORANGE = 0xFE73,
    LCD_COLOR_LTPINK = 0xFDDF,
    LCD_COLOR_LTPURPLE = 0xCCFF,
    LCD_COLOR_LTGREY = 0xE71C,

    LCD_COLOR_DKBLUE = 0x000D,
    LCD_COLOR_DKTEAL = 0x020C,
    LCD_COLOR_DKGREEN = 0x03E0,
    LCD_COLOR_DKCYAN = 0x03EF,
    LCD_COLOR_DKRED = 0x6000,
    LCD_COLOR_DKMAGENTA = 0x8008,
    LCD_COLOR_DKYELLOW = 0x8400,
    LCD_COLOR_DKORANGE = 0x8200,
    LCD_COLOR_DKPINK = 0x9009,
    LCD_COLOR_DKPURPLE = 0x4010,
    LCD_COLOR_DKGREY = 0x4A49,
} lcd_color_t;

typedef struct {
    uint8_t r, g, b;
} lcd_rgb_t;

typedef void *lcd_dev_handle_t;

typedef struct lcd_dev lcd_dev_t;
typedef struct lcd_device lcd_device_t;
typedef struct lcd_com lcd_com_t;

struct lcd_dev {
    const uint16_t id;
    const lcd_config_t config;
    uint16_t *buffer;
    size_t buffer_len;
    rotation_t rotation;
    lcd_dev_handle_t handle;
    struct {
        uint16_t mw;       /*!< Memory write reg. */
        uint16_t mc;       /*!< Memory x1 address reg. */
        uint16_t mp;       /*!< Memory y1 address reg. */
        uint16_t ec;       /*!< Memory x2 address reg. */
        uint16_t ep;       /*!< Memory y2 address reg. */
        uint16_t sc;       /*!< Memory scroll x address reg. */
        uint16_t sp;       /*!< Memory scroll y address reg. */
        uint16_t reverse;
        uint16_t width;    /*!< Current width. */
        uint16_t height;   /*!< Current height. */
    } registers;
    const lcd_device_t *device;
};

struct lcd_device {
    const lcd_com_t *comm;

    esp_err_t (*init)(lcd_dev_t *dev);

    void (*draw_pixel)(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y);

    void (*fill)(lcd_dev_t *dev, uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void (*draw)(lcd_dev_t *dev, const uint16_t *buf, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

    void (*clear)(lcd_dev_t *dev, uint16_t color);

    esp_err_t (*set_rotation)(lcd_dev_t *dev, rotation_t rotation);

    esp_err_t (*vertical_scroll)(lcd_dev_t *dev, int16_t top, int16_t scroll_lines, int16_t offset);

    esp_err_t (*invert_display)(lcd_dev_t *dev, bool reverse);
};

struct lcd_com {
    esp_err_t (*init)(lcd_dev_t *dev);

    uint8_t (*read_reg8)(const lcd_dev_t *dev, uint16_t cmd);

    uint16_t (*read_reg16)(const lcd_dev_t *dev, uint16_t cmd);

    void (*read_regn)(const lcd_dev_t *dev, uint16_t cmd, uint8_t *buf, size_t len);

    void (*write_reg8)(const lcd_dev_t *dev, uint16_t cmd, uint8_t data);

    void (*write_reg16)(const lcd_dev_t *dev, uint16_t cmd, uint16_t data);

    void (*write_regn)(const lcd_dev_t *dev, uint16_t cmd, const uint8_t *buf, size_t len);

    void (*write_data8)(const lcd_dev_t *dev, uint8_t data);

    void (*write_data16)(const lcd_dev_t *dev, uint16_t data);

    void (*write_data16n)(const lcd_dev_t *dev, uint16_t data, size_t len);
};

esp_err_t lcd_init(lcd_dev_t *dev);

// Low level primitives.

esp_err_t lcd_init_registers(const lcd_dev_t *dev, const uint16_t *table, size_t size);

uint16_t lcd_read_reg(const lcd_dev_t *dev, uint16_t cmd);

void lcd_write_data16(const lcd_dev_t *dev, uint16_t data);

void lcd_write_data16n(const lcd_dev_t *dev, uint16_t data, size_t len);

void lcd_write_datan(const lcd_dev_t *dev, const uint16_t *buf, size_t len);

void lcd_write_cmd(const lcd_dev_t *dev, uint16_t cmd);

// High level primitives.

void lcd_clear(lcd_dev_t *dev, uint16_t color);

void lcd_fill(lcd_dev_t *dev, uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void lcd_fill_wh(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void lcd_hline(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t width);

void lcd_vline(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t height);

void lcd_line(lcd_dev_t *dev, uint16_t color, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

void lcd_rect(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void lcd_draw(lcd_dev_t *dev, const uint16_t *img, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void lcd_draw_pixel(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y);

esp_err_t lcd_set_rotation(lcd_dev_t *dev, rotation_t rotation);

esp_err_t lcd_vertical_scroll(lcd_dev_t *dev, int16_t top, int16_t scroll_lines, int16_t offset);

esp_err_t lcd_invert_display(lcd_dev_t *dev, bool reverse);

esp_err_t lcd_set_backlight(lcd_dev_t *dev, bool on);

uint16_t lcd_width(const lcd_dev_t *dev);

uint16_t lcd_height(const lcd_dev_t *dev);

uint16_t lcd_rgb565(uint8_t r, uint8_t g, uint8_t b);

uint16_t lcd_rgb565s(lcd_rgb_t color);

lcd_rgb_t lcd_rgb888(uint16_t color);

void lcd_buf_fill(uint16_t *buf, uint16_t data, size_t len);

#endif //HYDROPONICS_DRIVER_LCD_LCD_H
