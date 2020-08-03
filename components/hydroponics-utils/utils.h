#ifndef HYDROPONICS_UTILS_H
#define HYDROPONICS_UTILS_H

#include "mbedtls/cmac.h"
#include "protobuf-c/protobuf-c.h"

#include "esp_err.h"

#define SAFE_FREE(f) do {    \
        if ((f) != NULL) {   \
            free(f);         \
            (f) = NULL;      \
        }                    \
    } while(0)

#define AES128_BLOCK_SIZE MBEDTLS_AES_BLOCK_SIZE

int random_int(int min, int max);

uint16_t clamp(uint16_t value, uint16_t min, uint16_t max);

size_t round_up(size_t len, uint16_t block_size);

esp_err_t aes128_decrypt(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len,
                         const uint8_t key[AES128_BLOCK_SIZE]);

esp_err_t aes128_encrypt(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len,
                         const uint8_t key[AES128_BLOCK_SIZE]);

esp_err_t pkcs_7_strip_padding(uint8_t *buf, size_t *len);

esp_err_t pkcs_7_add_padding(uint8_t *buf, size_t *len, size_t max_size);

esp_err_t md5(const uint8_t *buf, size_t len, uint8_t output[16]);

esp_err_t sha256(const uint8_t *buf, size_t len, unsigned char output[32]);

const char *enum_from_value(const ProtobufCEnumDescriptor *descriptor, int value);

void safe_delay_us(uint32_t delay_us);

void safe_delay_ms(uint32_t delay_ms);

double lin_regression(const double coeff[], size_t size, double value);

#endif //HYDROPONICS_UTILS_H
