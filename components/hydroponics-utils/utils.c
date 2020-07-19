#include <stdlib.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "mbedtls/sha256.h"

#include "error.h"
#include "utils.h"

static const char *TAG = "util";

int random_int(int min, int max) {
    return min + (int) random() % (max + 1 - min);
}

uint16_t clamp(uint16_t value, uint16_t min, uint16_t max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

esp_err_t sha256(const uint8_t *buf, const size_t len, unsigned char output[32]) {
    ARG_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_CHECK(len > 0, ERR_PARAM_LE_ZERO);
    ARG_CHECK(output != NULL, ERR_PARAM_NULL);

    output[0] = '\0';
    int err = mbedtls_sha256_ret(buf, len, output, 0 /* SHA-256 */);
    if (err == 0) {
        return ESP_OK;
    }
    ESP_LOGW(TAG, "mbedtls_sha256_ret, error: %d", err);
    return ESP_FAIL;
}

const char *enum_from_value(const ProtobufCEnumDescriptor *descriptor, int value) {
    ARG_ERROR_CHECK(descriptor != NULL, ERR_PARAM_NULL);
    for (int i = 0; i < descriptor->n_values; ++i) {
        if (descriptor->values[i].value == value) {
            return descriptor->values[i].name;
        }
    }
    return "???";
}

void safe_delay_us(uint32_t delay_us) {
    if (delay_us * 1000 < portTICK_PERIOD_MS) {
        ets_delay_us(delay_us);
        return;
    }
    safe_delay_ms(delay_us / 1000);
}

void safe_delay_ms(uint32_t delay_ms) {
    if (delay_ms < portTICK_PERIOD_MS) {
        ets_delay_us(delay_ms * 1000);
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
}

double lin_regression(const double coeff[], size_t coeff_size, double value) {
    double ret = 0;
    double x = 1;
    for (int i = 0; i < coeff_size; ++i) {
        ret += coeff[coeff_size - 1 - i] * x;
        if (i == 0) {
            x = value;
        } else {
            x *= value;
        }
    }
    return ret;
}