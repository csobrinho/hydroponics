#include "esp_log.h"

#include "driver/i2c.h"

#include "buses.h"
#include "utils.h"

static const char *TAG = "buses";

static void buses_reset(void) {
    gpio_config_t config = {
            .pin_bit_mask = BIT64(LCD_RST),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
    ESP_ERROR_CHECK(gpio_set_level(LCD_RST, true));
    safe_delay_ms(100);
    ESP_ERROR_CHECK(gpio_set_level(LCD_RST, false));
    safe_delay_ms(100);
    ESP_ERROR_CHECK(gpio_set_level(LCD_RST, true));
    safe_delay_ms(100);
}

static void buses_i2c_unstuck(void) {
    gpio_config_t config = {
            .pin_bit_mask = BIT64(I2C_MASTER_SDA) | BIT64(I2C_MASTER_SCL),
            .mode = GPIO_MODE_OUTPUT_OD,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
    ESP_ERROR_CHECK(gpio_set_level(I2C_MASTER_SDA, true));
    ESP_ERROR_CHECK(gpio_set_level(I2C_MASTER_SCL, true));
    for (int i = 0; i < 10; i++) { // 9nth cycle acts as NACK
        ESP_ERROR_CHECK(gpio_set_level(I2C_MASTER_SCL, true));
        safe_delay_us(5);
        ESP_ERROR_CHECK(gpio_set_level(I2C_MASTER_SCL, false));
        safe_delay_us(5);
    }

    // A STOP signal (SDA from low to high while CLK is high)
    ESP_ERROR_CHECK(gpio_set_level(I2C_MASTER_SDA, false));
    safe_delay_us(5);
    ESP_ERROR_CHECK(gpio_set_level(I2C_MASTER_SCL, true));
    safe_delay_us(2);
    ESP_ERROR_CHECK(gpio_set_level(I2C_MASTER_SDA, true));
    safe_delay_us(2);

    // Bus status is now FREE. Return to power up mode.
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&config));
    safe_delay_ms(250);
}

void buses_init(void) {
    buses_reset();
    buses_i2c_unstuck();

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    ESP_LOGI(TAG, "I2C clock: %d kHz", I2C_MASTER_FREQ_HZ / 1000);
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE,
                                       I2C_MASTER_TX_BUF_DISABLE, 0));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
}

void buses_scan(void) {
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    printf("00:         ");
    for (uint8_t i = 3; i < 0x78; i++) {
        if (i % 16 == 0) {
            printf("\n%.2x:", i);
        }
        i2c_cmd_handle_t handle = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(handle));
        ESP_ERROR_CHECK(i2c_master_write_byte(handle, (i << 1) | I2C_MASTER_WRITE, I2C_WRITE_ACK_CHECK));
        ESP_ERROR_CHECK(i2c_master_stop(handle));
        esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, handle, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
        i2c_cmd_link_delete(handle);

        if (err == ESP_OK) {
            printf(" %.2x", i);
        } else {
            printf(" --");
        }
    }
    printf("\n");
}
