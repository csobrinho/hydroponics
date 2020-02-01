#include "driver/i2c.h"

#include "esp_err.h"

#include "buses.h"
#include "error.h"
#include "ext_gpio.h"

static const char *TAG = "ext_gpio";

typedef enum {
    EXT_GPIO_REG_BASE = 0x00,
    EXT_GPIO_REG_IODIRA = 0x00,
    EXT_GPIO_REG_IODIRB = 0x01,
    EXT_GPIO_REG_IPOLA = 0x02,
    EXT_GPIO_REG_IPOLB = 0x03,
    EXT_GPIO_REG_GPINTENA = 0x04,
    EXT_GPIO_REG_GPINTENB = 0x05,
    EXT_GPIO_REG_DEFVALA = 0x06,
    EXT_GPIO_REG_DEFVALB = 0x07,
    EXT_GPIO_REG_INTCONA = 0x08,
    EXT_GPIO_REG_INTCONB = 0x09,
    EXT_GPIO_REG_IOCONA = 0x0a,
    EXT_GPIO_REG_IOCONB = 0x0b,
    EXT_GPIO_REG_GPPUA = 0x0c,
    EXT_GPIO_REG_GPPUB = 0x0d,
    EXT_GPIO_REG_INTFA = 0x0e,
    EXT_GPIO_REG_INTFB = 0x0f,
    EXT_GPIO_REG_INTCAPA = 0x10,
    EXT_GPIO_REG_INTCAPB = 0x11,
    EXT_GPIO_REG_GPIOA = 0x12,
    EXT_GPIO_REG_GPIOB = 0x13,
    EXT_GPIO_REG_OLATA = 0x14,
    EXT_GPIO_REG_OLATB = 0x15,
} ext_gpio_reg_t;

typedef struct {
    union {
        struct {
            uint8_t _reserved1:1;
            uint8_t intpol:1;     /*!< This bit sets the polarity of the INT output pin.
                                   *    1: Active-high.
                                   *    0: Active-low.
                                   */
            uint8_t odr:1;        /*!< Configures the INT pin as an open-drain output.
                                   *    1: Open-drain output (overrides the INTPOL bit.)
                                   *    0: Active driver output (INTPOL bit sets the polarity.)
                                   */
            uint8_t _reserved2:1;
            uint8_t disslw:1;     /*!< Slew Rate control bit for SDA output.
                                   *    1: Slew rate disabled.
                                   *    0: Slew rate enabled.
                                   */
            uint8_t seqop:1;      /*!< Sequential Operation mode bit.
                                   *    1: Sequential operation disabled, address pointer does not increment.
                                   *    0: Sequential operation enabled, address pointer increments.
                                   */
            uint8_t mirror:1;     /*!< INT Pins Mirror bit.
                                   *    1: The INT pins are internally connected.
                                   *    0: The INT pins are not connected. INTA is associated with PORTA and INTB is
                                   *       associated with PORTB.
                                   */
            uint8_t bank:1;       /*!< Controls how the registers are addressed.
                                   *    1: The registers associated with each port are separated into different banks.
                                   *    0: The registers are in the same bank (addresses are sequential).
                                   */
        };
        uint8_t iocon;
    };
} iocon_t;

typedef struct regs {
    union {
        struct {
            uint8_t iodir_a;   /*!< I/O Direction Register. */
            uint8_t iodir_b;   /*!< I/O Direction Register. */
            uint8_t ipol_a;    /*!< Input Polarity Port A Register. */
            uint8_t ipol_b;    /*!< Input Polarity Port B Register. */
            uint8_t gpinten_a; /*!< Interrupt On Change Control Register. */
            uint8_t gpinten_b; /*!< Interrupt On Change Control Register. */
            uint8_t defval_a;  /*!< Default Compare Register For Interrupt On Change. */
            uint8_t defval_b;  /*!< Default Compare Register For Interrupt On Change. */
            uint8_t intcon_a;  /*!< Interrupt Control Register. */
            uint8_t intcon_b;  /*!< Interrupt Control Register. */
            iocon_t iocon_a;   /*!< I/O Expander Configuration Register. */
            iocon_t iocon_b;   /*!< I/O Expander Configuration Register. */
            uint8_t gppu_a;    /*!< Pull Up Resistor Configuration Register. */
            uint8_t gppu_b;    /*!< Pull Up Resistor Configuration Register. */
            uint8_t intf_a;    /*!< Interrupt Flag Register. */
            uint8_t intf_b;    /*!< Interrupt Flag Register. */
            uint8_t intcap_a;  /*!< Interrupt Captured Value For Port A Register. */
            uint8_t intcap_b;  /*!< Interrupt Captured Value For Port B Register. */
            uint8_t gpio_a;    /*!< General Purpose I/O Port A Register. */
            uint8_t gpio_b;    /*!< General Purpose I/O Port B Register. */
            uint8_t olat_a;    /*!< Output Latch Register 0. */
            uint8_t olat_b;    /*!< Output Latch Register 0. */
        };
        uint8_t value[22];
    };
} ext_gpio_status_t;

static ext_gpio_status_t status = {0};

#define A_OR_B(num, a, b)  ((num) < 8 ? (a) : (b))
#define PIN_MASK(gpio_num) (BIT(((gpio_num) % 8)))

static esp_err_t ext_gpio_read(ext_gpio_reg_t reg_addr, uint8_t *data, size_t data_len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (EXT_GPIO_ADDRESS << 1) | I2C_MASTER_WRITE, I2C_WRITE_ACK_CHECK);
    i2c_master_write_byte(cmd, reg_addr, I2C_WRITE_ACK_CHECK);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (EXT_GPIO_ADDRESS << 1) | I2C_MASTER_READ, I2C_WRITE_ACK_CHECK);
    i2c_master_read(cmd, data, data_len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    return err;
}

static esp_err_t ext_gpio_write(ext_gpio_reg_t reg_addr, uint8_t *data, size_t data_len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (EXT_GPIO_ADDRESS << 1) | I2C_MASTER_WRITE, I2C_WRITE_ACK_CHECK);
    i2c_master_write_byte(cmd, reg_addr, I2C_WRITE_ACK_CHECK);
    i2c_master_write(cmd, data, data_len, true);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    return err;
}

static esp_err_t ext_gpio_set_bits(ext_gpio_reg_t reg_addr, uint8_t *data, uint8_t mask) {
    *data |= mask;
    return ext_gpio_write(reg_addr, data, 1);
}

static esp_err_t ext_gpio_clear_bits(ext_gpio_reg_t reg_addr, uint8_t *data, uint8_t mask) {
    *data &= ~mask;
    return ext_gpio_write(reg_addr, data, 1);
}

esp_err_t ext_gpio_init(void) {
    ESP_ERROR_CHECK(ext_gpio_read(EXT_GPIO_REG_BASE, (uint8_t *) &status.value, sizeof(status)));
    ESP_ERROR_CHECK(ext_gpio_set_bits(EXT_GPIO_REG_IODIRA, &status.iodir_a, 0xff));
    ESP_ERROR_CHECK(ext_gpio_set_bits(EXT_GPIO_REG_IODIRB, &status.iodir_a, 0xff));
    return ESP_OK;
}

esp_err_t ext_gpio_config_intr(bool mirroring, bool openDrain, bool polarity) {
    status.iocon_a.mirror = mirroring;
    status.iocon_a.odr = openDrain;
    status.iocon_a.intpol = polarity;
    status.iocon_b.mirror = mirroring;
    status.iocon_b.odr = openDrain;
    status.iocon_b.intpol = polarity;
    ESP_ERROR_CHECK(ext_gpio_write(EXT_GPIO_REG_IOCONA, (uint8_t *) &status.iocon_a, 2));
    return ESP_OK;
}

esp_err_t ext_gpio_set_intr_type(ext_gpio_num_t gpio_num, gpio_int_type_t intr_type) {
    ARG_CHECK(intr_type == GPIO_INTR_ANYEDGE || intr_type == GPIO_INTR_NEGEDGE || intr_type == GPIO_INTR_POSEDGE,
              "intr_type is invalid")

    uint8_t mask = PIN_MASK(gpio_num);

    ext_gpio_reg_t reg = A_OR_B(gpio_num, EXT_GPIO_REG_INTCONA, EXT_GPIO_REG_INTCONB);
    uint8_t *r = A_OR_B(gpio_num, &status.intcon_a, &status.intcon_b);
    // Set the pin interrupt control (0 means change, 1 means compare against given value).
    intr_type != GPIO_INTR_ANYEDGE ? ext_gpio_set_bits(reg, r, mask) : ext_gpio_clear_bits(reg, r, mask);

    // If the mode is not ANYEDGE, we need to set up a default value, different value triggers interrupt.

    // In a RISING interrupt the default value is 0, interrupt is triggered when the pin goes to 1.
    // In a FALLING interrupt the default value is 1, interrupt is triggered when pin goes to 0.
    reg = A_OR_B(gpio_num, EXT_GPIO_REG_DEFVALA, EXT_GPIO_REG_DEFVALB);
    r = A_OR_B(gpio_num, &status.defval_a, &status.defval_b);
    intr_type == GPIO_INTR_NEGEDGE ? ext_gpio_set_bits(reg, r, mask) : ext_gpio_clear_bits(reg, r, mask);

    // enable the pin for interrupt
    reg = A_OR_B(gpio_num, EXT_GPIO_REG_GPINTENA, EXT_GPIO_REG_GPINTENB);
    r = A_OR_B(gpio_num, &status.gpinten_a, &status.gpinten_b);
    ext_gpio_set_bits(reg, r, mask);

    return ESP_OK;
}

esp_err_t ext_gpio_invert_input(ext_gpio_num_t gpio_num, bool invert) {
    ext_gpio_reg_t reg = A_OR_B(gpio_num, EXT_GPIO_REG_IPOLA, EXT_GPIO_REG_IPOLB);
    uint8_t *r = A_OR_B(gpio_num, &status.ipol_a, &status.ipol_b);
    uint8_t mask = PIN_MASK(gpio_num);
    return invert ? ext_gpio_set_bits(reg, r, mask) : ext_gpio_clear_bits(reg, r, mask);
}

int ext_gpio_get_level(ext_gpio_num_t gpio_num) {
    uint8_t *r = A_OR_B(gpio_num, &status.gpio_a, &status.gpio_b);
    ESP_ERROR_CHECK(ext_gpio_read(A_OR_B(gpio_num, EXT_GPIO_REG_GPIOA, EXT_GPIO_REG_GPIOB), r, 1));
    uint8_t mask = PIN_MASK(gpio_num);
    return *r & mask ? 1 : 0;
}

uint16_t ext_gpio_get(void) {
    ESP_ERROR_CHECK(ext_gpio_read(EXT_GPIO_REG_GPIOA, &status.gpio_a, 2)); // Read A and B.
    return status.gpio_b << 8 | status.gpio_a;
}

esp_err_t ext_gpio_set_level(ext_gpio_num_t gpio_num, uint32_t level) {
    ext_gpio_reg_t reg = A_OR_B(gpio_num, EXT_GPIO_REG_GPIOA, EXT_GPIO_REG_GPIOB);
    uint8_t *r = A_OR_B(gpio_num, &status.gpio_a, &status.gpio_b);
    uint8_t mask = PIN_MASK(gpio_num);
    return level ? ext_gpio_set_bits(reg, r, mask) : ext_gpio_clear_bits(reg, r, mask);
}

esp_err_t ext_gpio_set(uint16_t value) {
    status.gpio_a = value & 0xff;
    status.gpio_b = value >> 8;
    ESP_ERROR_CHECK(ext_gpio_write(EXT_GPIO_REG_GPIOA, &status.gpio_a, 2));
    return ESP_OK;
}

esp_err_t ext_gpio_set_direction(ext_gpio_num_t gpio_num, gpio_mode_t mode) {
    ARG_CHECK(mode == GPIO_MODE_INPUT || mode == GPIO_MODE_OUTPUT, "mode is not supported")
    ext_gpio_reg_t reg = A_OR_B(gpio_num, EXT_GPIO_REG_IODIRA, EXT_GPIO_REG_IODIRB);
    uint8_t *r = A_OR_B(gpio_num, &status.iodir_a, &status.iodir_b);
    uint8_t mask = PIN_MASK(gpio_num);
    return mode == GPIO_MODE_INPUT ? ext_gpio_set_bits(reg, r, mask) : ext_gpio_clear_bits(reg, r, mask);
}

esp_err_t ext_gpio_set_pull_mode(ext_gpio_num_t gpio_num, gpio_pull_mode_t pull) {
    ARG_CHECK(pull == GPIO_PULLUP_ONLY || pull == GPIO_FLOATING, "pull is not supported")
    ext_gpio_reg_t reg = A_OR_B(gpio_num, EXT_GPIO_REG_GPPUA, EXT_GPIO_REG_GPPUB);
    uint8_t *r = A_OR_B(gpio_num, &status.gppu_a, &status.gppu_b);
    uint8_t mask = PIN_MASK(gpio_num);
    return pull == GPIO_PULLUP_ONLY ? ext_gpio_set_bits(reg, r, mask) : ext_gpio_clear_bits(reg, r, mask);
}

esp_err_t ext_gpio_config(const gpio_config_t *config) {
    ARG_CHECK(config != NULL, ERR_PARAM_NULL)
    ARG_CHECK(config->pin_bit_mask < BIT(EXT_GPIO_MAX), "pin_bit_mask can only contain 0-15")

    uint32_t io_num = 0;
    uint8_t input_en = 0;
    uint8_t output_en = 0;
    uint8_t pu_en = 0;

    do {
        if (((config->pin_bit_mask >> io_num) & BIT(0))) {
            if (config->mode == GPIO_MODE_INPUT) {
                input_en = 1;
                ext_gpio_set_direction(io_num, config->mode);
            } else if (config->mode == GPIO_MODE_OUTPUT) {
                output_en = 1;
                ext_gpio_set_direction(io_num, config->mode);
            } else {
                ESP_LOGE(TAG, "config->mode = %d is not supported", config->mode);
            }
            if (config->pull_up_en) {
                pu_en = 1;
                ext_gpio_set_pull_mode(io_num, GPIO_PULLUP_ONLY);
            } else {
                ext_gpio_set_pull_mode(io_num, GPIO_FLOATING);
            }
            if (config->pull_down_en) {
                ESP_LOGE(TAG, "config->pull_down_en = %d is not supported", config->pull_down_en);
            }

            ESP_LOGI(TAG, "EXT_GPIO[%d]| InputEn: %d| OutputEn: %d| Pullup: %d| Intr:%d ",
                     io_num, input_en, output_en, pu_en, config->intr_type);
            ext_gpio_set_intr_type(io_num, config->intr_type);
        }
        io_num++;
    } while (io_num < GPIO_PIN_COUNT);
    return ESP_OK;
}
