#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "lwip/def.h"

#include "esp_err.h"

#include "error.h"
#include "buffer.h"

static const char *TAG = "buffer";

esp_err_t buffer_init(FILE **f, const uint8_t *buf, size_t len) {
    ARG_CHECK(f != NULL, ERR_PARAM_NULL);
    ARG_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_CHECK(len > 0, ERR_PARAM_LE_ZERO);

    *f = fmemopen((void *) buf, len, "rb");
    ARG_CHECK(*f != NULL, "Could not open memory stream");
    return ESP_OK;
}

esp_err_t buffer_free(FILE *f) {
    ARG_CHECK(f != NULL, ERR_PARAM_NULL);
    int err = fclose(f);
    if (err != 0) {
        ESP_LOGW(TAG, "memory stream fclose, ret: %d error: %s", err, strerror(errno));
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

esp_err_t buffer_read8(FILE *f, uint8_t *val) {
    ARG_CHECK(f != NULL, ERR_PARAM_NULL);
    ARG_CHECK(val != NULL, ERR_PARAM_NULL);
    int ret = fread(val, 1, sizeof(uint8_t), f);
    ESP_LOGV(TAG, "buffer_read8 val: 0x%02x", *val);
    if (ret != sizeof(uint8_t)) {
        ESP_LOGW(TAG, "memory stream fread, ret: %d error: %s", ret, strerror(errno));
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

esp_err_t buffer_read16(FILE *f, uint16_t *val) {
    ARG_CHECK(f != NULL, ERR_PARAM_NULL);
    ARG_CHECK(val != NULL, ERR_PARAM_NULL);
    int ret = fread(val, 1, sizeof(uint16_t), f);
    *val = ntohs(*val); // Fix endianness.
    ESP_LOGV(TAG, "buffer_read16 val: 0x%04x", *val);
    if (ret != sizeof(uint16_t)) {
        ESP_LOGW(TAG, "memory stream fread, ret: %d error: %s", ret, strerror(errno));
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

esp_err_t buffer_read32(FILE *f, uint32_t *val) {
    ARG_CHECK(f != NULL, ERR_PARAM_NULL);
    ARG_CHECK(val != NULL, ERR_PARAM_NULL);
    int ret = fread(val, 1, sizeof(uint32_t), f);
    *val = ntohl(*val); // Fix endianness.
    ESP_LOGV(TAG, "buffer_read32 val: 0x%08x", *val);
    if (ret != sizeof(uint32_t)) {
        ESP_LOGW(TAG, "memory stream fread, ret: %d error: %s", ret, strerror(errno));
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

esp_err_t buffer_peek32_b(const uint8_t *buf, uint32_t *val) {
    ARG_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_CHECK(val != NULL, ERR_PARAM_NULL);
    const uint32_t *p = (const uint32_t *) buf;
    *val = ntohl(*p);
    return ESP_OK;
}

esp_err_t buffer_read8s(FILE *f, uint8_t *val, size_t len) {
    ARG_CHECK(f != NULL, ERR_PARAM_NULL);
    ARG_CHECK(val != NULL, ERR_PARAM_NULL);
    ESP_LOGV(TAG, "buffer_read32 len: %d", len);
    int ret = fread(val, 1, len, f);
    if (ret != len) {
        ESP_LOGW(TAG, "memory stream fread, ret: %d error: %s", ret, strerror(errno));
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}
