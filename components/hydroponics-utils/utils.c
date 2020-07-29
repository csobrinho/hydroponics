#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "mbedtls/aes.h"
#include "mbedtls/cmac.h"
#include "mbedtls/md5.h"
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

size_t round_up(size_t len, uint16_t block_size) {
    return len + (len % block_size);
}

static esp_err_t aes128(int mode, const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len,
                        const uint8_t key[AES128_BLOCK_SIZE]) {
    ARG_CHECK(src != NULL, ERR_PARAM_NULL);
    ARG_CHECK(dst != NULL, ERR_PARAM_NULL);
    ARG_CHECK(key != NULL, ERR_PARAM_NULL);
    ARG_CHECK(src_len > 0, ERR_PARAM_LE_ZERO);
    ARG_CHECK(dst_len >= src_len, "dst_len must be >= src_len");
    if (mode == MBEDTLS_AES_DECRYPT) {
        ARG_CHECK(src_len % AES128_BLOCK_SIZE == 0, "src_len must be a multiple of %d", AES128_BLOCK_SIZE);
    } else {
        ARG_CHECK(dst_len % AES128_BLOCK_SIZE == 0, "dst_len must be a multiple of %d", AES128_BLOCK_SIZE);
    }
    ESP_LOGI(TAG, "mode:    %s", mode == MBEDTLS_AES_DECRYPT ? "DECRYPT" : "ENCRYPT");
    ESP_LOGI(TAG, "src_len: %d", src_len);
    ESP_LOGI(TAG, "dst_len: %d", dst_len);

    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    int err = mbedtls_aes_setkey_enc(&ctx, key, AES128_BLOCK_SIZE * 8);
    if (err != 0) {
        ESP_LOGW(TAG, "mbedtls_aes_setkey_enc, error: %d", err);
        goto fail;
    }

    int len = src_len;
    const uint8_t *psrc = src;
    uint8_t *pdst = dst;
    while (len > 0) {
        err = mbedtls_aes_crypt_ecb(&ctx, mode, psrc, pdst);
        ESP_LOGI(TAG, "Encrypt %d / %d to go, error: %d", (int) (psrc - src), len, err);
        if (err != 0) {
            ESP_LOGW(TAG, "mbedtls_aes_crypt_ecb, error: %d", err);
            goto fail;
        }
        len -= AES128_BLOCK_SIZE;
        psrc += AES128_BLOCK_SIZE;
        pdst += AES128_BLOCK_SIZE;
    }
    ESP_LOGI(TAG, "Adding PKCS #7 padding. src_len: %d dstr_len: %d", src_len, dst_len);
    ESP_ERROR_CHECK(pkcs_7_add_padding(dst, &src_len, dst_len));
    mbedtls_aes_free(&ctx);
    return ESP_OK;
    fail:
    mbedtls_aes_free(&ctx);
    return ESP_FAIL;
}

esp_err_t aes128_decrypt(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len,
                         const uint8_t key[AES128_BLOCK_SIZE]) {
    return aes128(MBEDTLS_AES_DECRYPT, src, src_len, dst, dst_len, key);
}

esp_err_t aes128_encrypt(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len,
                         const uint8_t key[AES128_BLOCK_SIZE]) {
    return aes128(MBEDTLS_AES_ENCRYPT, src, src_len, dst, dst_len, key);
}

esp_err_t pkcs_7_strip_padding(uint8_t *buf, size_t *len) {
    ARG_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_CHECK(*len > 0, ERR_PARAM_LE_ZERO);
    const size_t max = *len;
    if (max < 16) {
        return ESP_OK;
    }
    const uint8_t last = buf[max - 1];
    ESP_LOGD(TAG, "Checking for PKCS #7 padding. [%d] -> %d", max - 1, last);
    if (last == 0 || last >= 16) {
        return ESP_OK;
    }
    for (size_t i = max - last; i < max - 1; i++) {
        if (buf[i] != last) {
            ESP_LOGV(TAG, "[%d] -> %d != %d", i, buf[i], last);
            return ESP_OK;
        }
    }
    ESP_LOGD(TAG, "Stripping from %d -> %d due to PKCS #7 padding", max - last, max - 1);
    memset(&buf[max - last], 0, last);
    *len -= last;
    return ESP_OK;
}

esp_err_t pkcs_7_add_padding(uint8_t *buf, size_t *len, size_t max_size) {
    ARG_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_CHECK(*len > 0, ERR_PARAM_LE_ZERO);
    ARG_CHECK(max_size >= round_up(*len, AES128_BLOCK_SIZE), "max_size < round_up(*len, AES128_BLOCK_SIZE)");
    size_t padding = *len % AES128_BLOCK_SIZE;
    if (padding == 0) {
        ESP_LOGI(TAG, "No padding necessary");
        return ESP_OK;
    }
    ESP_LOGI(TAG, "Adding %d bytes of padding", padding);
    buf += *len;
    for (int i = 0; i < padding; ++i) {
        *buf++ = padding;
    }
    *len += padding;
    return ESP_OK;
}

esp_err_t md5(const uint8_t *buf, size_t len, uint8_t output[16]) {
    ARG_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_CHECK(len > 0, ERR_PARAM_LE_ZERO);
    ARG_CHECK(output != NULL, ERR_PARAM_NULL);
    int err = mbedtls_md5_ret(buf, len, output);
    if (err == 0) {
        return ESP_OK;
    }
    ESP_LOGW(TAG, "mbedtls_md5_ret, error: %d", err);
    return ESP_FAIL;
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
