#ifndef HYDROPONICS_DRIVER_LCD_LCD_H
#define HYDROPONICS_DRIVER_LCD_LCD_H

#include "driver/gpio.h"

#include "esp_log.h"

#define WR_IDLE(d)      gpio_set_level(d->config.ws_io_num, 1)
#define WR_ACTIVE(d)    gpio_set_level(d->config.ws_io_num, 0)
#define WR_STROBE(d)    WR_ACTIVE(d); WR_IDLE(d)
#define RD_IDLE(d)      gpio_set_level(d->config.rd_io_num, 1)
#define RD_ACTIVE(d)    gpio_set_level(d->config.rd_io_num, 0)
#define RD_STROBE(d)    RD_IDLE(d); RD_ACTIVE(d); RD_ACTIVE(d); RD_ACTIVE(d)
#define CD_DATA(d)      gpio_set_level(d->config.rs_io_num, 1)
#define CD_COMMAND(d)   gpio_set_level(d->config.rs_io_num, 0)
#define RESET_IDLE(d)   gpio_set_level(d->config.rst_io_num, 1)
#define RESET_ACTIVE(d) gpio_set_level(d->config.rst_io_num, 0)

// #define LLOG(args...) ESP_LOGD(args)
#define LLOG(args...)    do {} while (0)

#define LCD_BUFFER_SIZE 2000   // In uint32_t blocks.
#define LCD_CMD_DELAY   0xFFFF // Used to add a delay inside the 'lcd_init_registers'.

typedef enum {
    ROTATION_UNKNOWN = -1,
    ROTATION_PORTRAIT = 0,
    ROTATION_LANDSCAPE = 1,
    ROTATION_PORTRAIT_REV = 2,
    ROTATION_LANDSCAPE_REV = 3,
    ROTATION_LANDSCAPE_MAX,
} rotation_t;

typedef const struct {
    gpio_num_t data_width;      /*!< Parallel data width, 16bit or 8bit available */
    gpio_num_t data_io_num[16]; /*!< Parallel data output IO*/
    gpio_num_t ws_io_num;       /*!< write clk io */
    gpio_num_t rs_io_num;       /*!< rs io num */
    gpio_num_t rd_io_num;       /*!< (optional) rd io num */
    gpio_num_t rst_io_num;      /*!< (optional) reset io num */
    rotation_t rotation;        /*!< Initial rotation */
} lcd_config_t;

typedef enum {
    LCD_COLOR_BLACK = 0x0000,
    LCD_COLOR_BLUE = 0x001F,
    LCD_COLOR_RED = 0xF800,
    LCD_COLOR_GREEN = 0x07E0,
    LCD_COLOR_CYAN = 0x07FF,
    LCD_COLOR_MAGENTA = 0xF81F,
    LCD_COLOR_YELLOW = 0xFFE0,
    LCD_COLOR_WHITE = 0xFFFF,
} lcd_color_t;

typedef void *lcd_dev_handle_t;

typedef struct lcd_dev lcd_dev_t;
struct lcd_dev {
    const uint16_t id;
    const lcd_config_t config;
    uint16_t *buffer;
    size_t buffer_len;
    rotation_t rotation;
    lcd_dev_handle_t handle;
    struct {
        uint16_t mc;
        uint16_t mp;
        uint16_t mw;
        uint16_t sc;
        uint16_t ec;
        uint16_t sp;
        uint16_t ep;
        uint16_t reverse;
        uint16_t width;    /*!< Current width */
        uint16_t height;   /*!< Current height */
    } registers;

    const struct {
        esp_err_t (*init)(lcd_dev_t *dev);

        void (*draw_pixel)(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y);

        void (*fill)(lcd_dev_t *dev, uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

        void (*draw)(lcd_dev_t *dev, const uint16_t *buf, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

        void (*clear)(lcd_dev_t *dev, uint16_t color);

        esp_err_t (*set_rotation)(lcd_dev_t *dev, rotation_t rotation);

        esp_err_t (*vertical_scroll)(lcd_dev_t *dev, int16_t top, int16_t scroll_lines, int16_t offset);

        esp_err_t (*invert_display)(lcd_dev_t *dev, bool reverse);
    } device;
};

esp_err_t lcd_init(lcd_dev_t *dev);

esp_err_t lcd_reset(const lcd_dev_t *dev);

// Low level primitives.

esp_err_t lcd_init_registers(const lcd_dev_t *dev, const uint16_t *table, size_t size);

uint16_t lcd_read_reg(const lcd_dev_t *dev, uint16_t cmd);

void lcd_write_data16(const lcd_dev_t *dev, uint16_t data);

void lcd_write_datan(const lcd_dev_t *dev, const uint16_t *buf, size_t len);

void lcd_write_cmd(const lcd_dev_t *dev, uint16_t cmd);

void lcd_write_reg(const lcd_dev_t *dev, uint16_t cmd, uint16_t data);

// High level primitives.

void lcd_clear(lcd_dev_t *dev, uint16_t color);

void lcd_fill(lcd_dev_t *dev, uint16_t color, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void lcd_hline(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t width);

void lcd_vline(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t height);

void lcd_line(lcd_dev_t *dev, uint16_t color, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

void lcd_rect(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void lcd_draw(lcd_dev_t *dev, const uint16_t *img, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

void lcd_draw_pixel(lcd_dev_t *dev, uint16_t color, uint16_t x, uint16_t y);

esp_err_t lcd_set_rotation(lcd_dev_t *dev, rotation_t rotation);

esp_err_t lcd_vertical_scroll(lcd_dev_t *dev, int16_t top, int16_t scroll_lines, int16_t offset);

esp_err_t lcd_invert_display(lcd_dev_t *dev, bool reverse);

uint16_t lcd_width(const lcd_dev_t *dev);

uint16_t lcd_height(const lcd_dev_t *dev);

#endif //HYDROPONICS_DRIVER_LCD_LCD_H
