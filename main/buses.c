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

    ESP_LOGI(TAG, "sda_io_num %d", I2C_MASTER_SDA);
    ESP_LOGI(TAG, "scl_io_num %d", I2C_MASTER_SCL);
    ESP_LOGI(TAG, "clk_speed %d", I2C_MASTER_FREQ_HZ);
    ESP_LOGI(TAG, "i2c_param_config %d", conf.mode);
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_LOGI(TAG, "i2c_driver_install %d", I2C_MASTER_NUM);
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE,
                                       I2C_MASTER_TX_BUF_DISABLE, 0));
}

void buses_scan(void) {
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    printf("00:         ");
    for (uint8_t i = 3; i < 0x78; i++) {
        esp_err_t ret = buses_i2c_write(i, NULL, 0);
        if (i % 16 == 0) {
            printf("\n%.2x:", i);
        }
        if (ret == ESP_OK) {
            printf(" %.2x", i);
        } else {
            printf(" --");
        }
    }
    printf("\n");
}

esp_err_t buses_i2c_write(uint8_t address, uint8_t *buf, size_t len) {
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(handle);
    err += i2c_master_write_byte(handle, (address << 1) | I2C_MASTER_WRITE, I2C_WRITE_ACK_CHECK);
    if (len > 0) {
        err += i2c_master_write(handle, buf, len, I2C_WRITE_ACK_CHECK);
    }
    err += i2c_master_stop(handle);
    err += i2c_master_cmd_begin(I2C_MASTER_NUM, handle, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(handle);
    return err;
}

esp_err_t buses_i2c_read(uint8_t address, uint8_t *buf, size_t len, buses_i2c_stop_t stop) {
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(handle);
    err += i2c_master_write_byte(handle, (address << 1) | I2C_MASTER_READ, I2C_WRITE_ACK_CHECK);
    if (len > 0) {
        err += i2c_master_read(handle, buf, len, I2C_MASTER_ACK);
    }
    if (stop == BUSES_I2C_STOP) {
        err += i2c_master_stop(handle);
    }
    err += i2c_master_cmd_begin(I2C_MASTER_NUM, handle, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(handle);
    return err;
}

esp_err_t buses_i2c_continue_read(uint8_t *buf, size_t len, i2c_ack_type_t ack) {
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    esp_err_t err = i2c_master_read(handle, buf, len, ack);
    err += i2c_master_stop(handle);
    err += i2c_master_cmd_begin(I2C_MASTER_NUM, handle, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(handle);
    return err;
}

esp_err_t buses_i2c_stop(void) {
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    esp_err_t err = i2c_master_stop(handle);
    err += i2c_master_cmd_begin(I2C_MASTER_NUM, handle, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(handle);
    return err;
}
