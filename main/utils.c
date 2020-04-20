#include <stdlib.h>
#include <stdint.h>

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
