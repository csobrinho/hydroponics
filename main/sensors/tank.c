#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "ads1115.h"

#include "buses.h"
#include "context.h"
#include "error.h"
#include "tank.h"
#include "utils.h"

#define NO_OF_SAMPLES 16UL
#define COEFFICIENTS_MAX 4

static const char *const TAG = "tank";

typedef enum {
    TANK_I2C_ADDRESS_GND = UINT8_C(0x48),
    TANK_I2C_ADDRESS_VDD = UINT8_C(0x49),
    TANK_I2C_ADDRESS_SDA = UINT8_C(0x4b),
    TANK_I2C_ADDRESS_SCL = UINT8_C(0x4b),
    TANK_I2C_ADDRESS_NONE = UINT8_C(I2C_NO_DEVICE),
} tank_i2c_address_t;

typedef const struct {
    const char *name;                    /*!< Tank description, used for logging. */
    ads1115_mux_t device_mux;            /*!< ADC channel mux. Should be a differential channel. */
    double regression[COEFFICIENTS_MAX]; /*!< Linear regression coefficients. */
} tank_t;

static struct {
    const tank_i2c_address_t address;
    tank_t tanks;
    ads1115_t handle;
} config = {
        .address = TANK_I2C_ADDRESS_GND,
        .tanks = {
                .name = "Tank",
                .device_mux = ADS1115_MUX_0_1,
                // From https://docs.google.com/spreadsheets/d/1LZo2zjm7wT2C7UeA40zMUXK-daBry_TDRjhAEd-a5a8/view
                //                      x^3                   x^2                    x                    b
                .regression = {-0.00000000000000185, -0.00000000021752058, -0.00002203925836691, 0.52318676617797200},
        },
};

static void tank_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    while (true) {
        TickType_t last_wake_time = xTaskGetTickCount();
        if (config.tanks.name == NULL) {
            continue;
        }
        int64_t scratch = 0;
        ads1115_set_mux(&config.handle, config.tanks.device_mux);
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            scratch += ads1115_get_raw(&config.handle);
        }
        // Average the value and run linear regression.
        int32_t raw = (int32_t) (scratch / NO_OF_SAMPLES);
        double average = lin_regression(config.tanks.regression, COEFFICIENTS_MAX, raw);
        ESP_ERROR_CHECK(context_set_tank(context, CONFIG_TANK_A, average));
        ESP_LOGD(TAG, "%s: %d / %.1f %%", config.tanks.name, raw, average * 100);
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CONFIG_ESP_SAMPLING_TANK_MS));
    }
}

esp_err_t tank_init(context_t *context) {
    // Setup the ADCs.
    if (config.address != TANK_I2C_ADDRESS_NONE) {
        config.handle = ads1115_config(I2C_MASTER_NUM, config.address);
        ads1115_set_mode(&config.handle, ADS1115_MODE_SINGLE);
        ads1115_set_sps(&config.handle, ADS1115_SPS_64);
        ads1115_set_pga(&config.handle, ADS1115_FSR_0_512);
        ads1115_set_max_ticks(&config.handle, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
    }
    xTaskCreatePinnedToCore(tank_task, "tank", 2560, context, 15, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
