#include "esp_log.h"

#include "driver/i2c.h"

#include "buses.h"

static const char *TAG = "buses";

void buses_init(void) {
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
