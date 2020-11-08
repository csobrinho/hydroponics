#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "esp_err.h"

#include "error.h"
#include "lcd.h"
#include "lcd_spi.h"
#include "utils.h"

static const char *const TAG = "lcd_spi";

typedef union {
    struct {
        gpio_num_t pin_num: 16;
        lcd_spi_dc_t dc: 16;
    } fields;
    uint32_t bits;
} lcd_callback_t;

#define HANDLE(d) ((spi_device_handle_t)(d->handle))
#define TO_VOID_P(dev, val) ((void *) (uintptr_t) ((lcd_callback_t){ \
        .fields = {.pin_num = (dev)->config.spi.dc_io_num, .dc = (val)}}.bits))
#define FROM_VOID_P(vp) ((lcd_callback_t){.bits = (uintptr_t)(vp)})

IRAM_ATTR static void spi_pre_transfer_callback(spi_transaction_t *t) {
    lcd_callback_t callback = FROM_VOID_P(t->user);
    gpio_set_level(callback.fields.pin_num, callback.fields.dc);
}

esp_err_t lcd_spi_init(lcd_dev_t *dev) {
    ARG_CHECK(dev != NULL, ERR_PARAM_NULL);
    ARG_CHECK(dev->config.type == LCD_TYPE_SPI, "type must be LCD_TYPE_SPI");
    ARG_CHECK(dev->config.spi.mosi_io_num != GPIO_NUM_NC, "mosi_io_num must be defined");
    ARG_CHECK(dev->config.spi.sclk_io_num != GPIO_NUM_NC, "sclk_io_num must be defined");
    ARG_CHECK(dev->config.spi.cs_io_num != GPIO_NUM_NC, "spics_io_num must be defined");
    ARG_CHECK(dev->config.spi.dc_io_num != GPIO_NUM_NC, "dc_io_num must be defined");

    size_t screen_size = dev->config.screen.width * dev->config.screen.height * dev->config.screen.bytes;
    dev->buffer_len = screen_size / dev->config.screen.divisor;
    ESP_LOGI(TAG, "Allocating %d bytes for the lcd screen memory.", dev->buffer_len);
    dev->buffer = heap_caps_calloc(1, dev->buffer_len, dev->config.screen.caps);
    CHECK_NO_MEM(dev->buffer);
    dev->handle = NULL;

    // Setup the GPIOs as general purpose outputs.
    gpio_pad_select_gpio(dev->config.spi.dc_io_num);
    const gpio_config_t conf = {
            .pin_bit_mask = BIT64(dev->config.spi.dc_io_num),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&conf));

    // Initialize the SPI bus.
    const spi_bus_config_t bus = {
            .mosi_io_num = dev->config.spi.mosi_io_num,
            .miso_io_num = dev->config.spi.miso_io_num,
            .sclk_io_num = dev->config.spi.sclk_io_num,
            .quadwp_io_num = GPIO_NUM_NC,
            .quadhd_io_num = GPIO_NUM_NC,
            .max_transfer_sz = (int) screen_size + 128,
            .flags = SPICOMMON_BUSFLAG_SCLK | SPICOMMON_BUSFLAG_MISO | SPICOMMON_BUSFLAG_MOSI,
            .intr_flags = ESP_INTR_FLAG_IRAM,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(dev->config.spi.host, &bus, dev->config.spi.dma_chan));

    // Attach the LCD to the SPI bus.
    const spi_device_interface_config_t interface = {
            .mode = dev->config.spi.mode,
            .clock_speed_hz = dev->config.spi.clock_speed_hz,
            .input_delay_ns = 0, // dev->config.spi.clock_speed_hz / 3000000,
            .spics_io_num = dev->config.spi.cs_io_num,
            .flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_NO_DUMMY,
            .queue_size = 48,
            .pre_cb=spi_pre_transfer_callback
    };
    spi_device_handle_t spi = NULL;
    ESP_ERROR_CHECK(spi_bus_add_device(dev->config.spi.host, &interface, &spi));
    CHECK_NO_MEM(spi);
    dev->handle = spi;

    return ESP_OK;
}

static void lcd_spi_write_cmd(const lcd_dev_t *dev, uint8_t cmd) {
    LLOG(TAG, "[%s] 0x%02x", __FUNCTION__, cmd);
    spi_transaction_t t = {0};
    t.length = 8;
    t.tx_data[0] = cmd;
    t.flags = SPI_TRANS_USE_TXDATA;
    t.user = TO_VOID_P(dev, LCD_SPI_COMMAND);
    ESP_ERROR_CHECK(spi_device_polling_transmit(HANDLE(dev), &t)); // Transmit!
}

static void lcd_spi_write_datan(const lcd_dev_t *dev, const uint8_t *data, size_t len) {
    ARG_ERROR_CHECK(data != NULL, ERR_PARAM_NULL);
    LLOG(TAG, "[%s] len: %d | 0x%02x 0x%02x 0x%02x 0x%02x", __FUNCTION__, len, data[0], data[1], data[2], data[3]);
    if (len == 0) {
        return;
    }
    spi_transaction_t t = {0};
    t.length = 8 * len;
    switch (len) {
        case 4:
            t.tx_data[3] = data[3];
            // fallthrough.
        case 3:
            t.tx_data[2] = data[2];
            // fallthrough.
        case 2:
            t.tx_data[1] = data[1];
            // fallthrough.
        case 1:
            t.tx_data[0] = data[0];
            t.flags = SPI_TRANS_USE_TXDATA;
            LLOG_HEX(TAG, t.tx_data, len);
            break;
        default:
            t.tx_buffer = data;
            LLOG_HEX(TAG, t.tx_buffer, 4);
            break;
    }
    t.user = TO_VOID_P(dev, LCD_SPI_DATA);
    ESP_ERROR_CHECK(spi_device_polling_transmit(HANDLE(dev), &t)); // Transmit!
}

static void lcd_spi_read_datan(const lcd_dev_t *dev, uint8_t *data, size_t len) {
    ARG_ERROR_CHECK(data != NULL, ERR_PARAM_NULL);
    ARG_ERROR_CHECK(len > 0, ERR_PARAM_LE_ZERO);
    spi_transaction_t t = {0};
    t.rxlength = 8 * len;
    if (len <= 4) {
        t.flags = SPI_TRANS_USE_RXDATA;
    } else {
        t.rx_buffer = data;
    }
    t.user = TO_VOID_P(dev, LCD_SPI_DATA);
    ESP_ERROR_CHECK(spi_device_polling_transmit(HANDLE(dev), &t)); // Transmit and read.
    switch (len) {
        case 4:
            data[3] = t.rx_data[3];
            // fallthrough.
        case 3:
            data[2] = t.rx_data[2];
            // fallthrough.
        case 2:
            data[1] = t.rx_data[1];
            // fallthrough.
        case 1:
            data[0] = t.rx_data[0];
            LLOG_HEX(TAG, t.rx_data, 4);
            break;
        default:
            LLOG_HEX(TAG, t.rx_buffer, 4);
            break;
    }
}

void lcd_spi_read_regn(const lcd_dev_t *dev, uint16_t cmd, uint8_t *buf, size_t len) {
    lcd_spi_write_cmd(dev, cmd);
    safe_delay_us(100);
    lcd_spi_read_datan(dev, buf, len);
}

uint8_t lcd_spi_read_reg8(const lcd_dev_t *dev, uint16_t cmd) {
    uint8_t ret = 0;
    lcd_spi_read_regn(dev, cmd, &ret, 1);
    return ret;
}

uint16_t lcd_spi_read_reg16(const lcd_dev_t *dev, uint16_t cmd) {
    uint8_t ret[2] = {0};
    lcd_spi_read_regn(dev, cmd, ret, sizeof(ret));
    return U88_TO_U16(ret[0], ret[1]);
}

void lcd_spi_write_regn(const lcd_dev_t *dev, uint16_t cmd, const uint8_t *buf, size_t len) {
    lcd_spi_write_cmd(dev, cmd);
    if (len > 0) {
        ARG_ERROR_CHECK(buf != NULL, "buf == NULL");
        lcd_spi_write_datan(dev, buf, len);
    }
}

void lcd_spi_write_reg8(const lcd_dev_t *dev, uint16_t cmd, uint8_t data) {
    lcd_spi_write_regn(dev, cmd, &data, 1);
}

void lcd_spi_write_reg16(const lcd_dev_t *dev, uint16_t cmd, uint16_t data) {
    const uint8_t buf[2] = {U8_HI(data), U8_LO(data)};
    lcd_spi_write_regn(dev, cmd, buf, sizeof(buf));
}

void lcd_spi_write_data8(const lcd_dev_t *dev, uint8_t data) {
    lcd_spi_write_datan(dev, &data, 1);
}

void lcd_spi_write_data16(const lcd_dev_t *dev, uint16_t data) {
    const uint8_t buf[2] = {U8_HI(data), U8_LO(data)};
    lcd_spi_write_datan(dev, buf, sizeof(buf));
}

void lcd_spi_write_data16n(const lcd_dev_t *dev, uint16_t data, size_t len) {
    LLOG(TAG, "[%s] len: %d data: 0x%02x 0x%02x", __FUNCTION__, len, U8_HI(data), U8_LO(data));
    if (len == 0) {
        return;
    }
    size_t to_fill = len > dev->buffer_len ? dev->buffer_len : len;
    lcd_buf_fill(dev->buffer, data, to_fill);

    size_t nt = round_divide(len, dev->buffer_len);
    spi_transaction_t t[nt];
    memset(t, 0, nt * sizeof(spi_transaction_t));

    int idx = 0;
    while (len) {
        size_t trans_len = len > dev->buffer_len ? dev->buffer_len : len;
        t[idx].length = 8 * trans_len;
        t[idx].tx_buffer = dev->buffer;
        t[idx].user = TO_VOID_P(dev, LCD_SPI_DATA);
        LLOG_HEX(TAG, t[idx].tx_buffer, 4);
        LLOG(TAG, "[%s] nt: %d/%d | len: %d", __FUNCTION__, idx + 1, nt, len);
        ESP_ERROR_CHECK(spi_device_queue_trans(HANDLE(dev), &t[idx], portMAX_DELAY));
        len -= trans_len;
        idx++;
    }
    spi_transaction_t *pt = NULL;
    for (int x = 0; x < nt; x++) {
        // Wait for all transactions to finish.
        ESP_ERROR_CHECK(spi_device_get_trans_result(HANDLE(dev), &pt, portMAX_DELAY));
    }
}

const lcd_com_t lcd_com_spi = {
        .init = lcd_spi_init,
        .read_reg8 = lcd_spi_read_reg8,
        .read_reg16 = lcd_spi_read_reg16,
        .read_regn = lcd_spi_read_regn,
        .write_reg8 = lcd_spi_write_reg8,
        .write_reg16 = lcd_spi_write_reg16,
        .write_regn = lcd_spi_write_regn,
        .write_data8 = lcd_spi_write_data8,
        .write_data16  = lcd_spi_write_data16,
        .write_data16n  = lcd_spi_write_data16n,
};
