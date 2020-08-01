#ifndef HYDROPONICS_ERROR_H
#define HYDROPONICS_ERROR_H

#include <sys/cdefs.h>

#include "esp_log.h"

#define ERR_PARAM_NULL    "parameter == null"
#define ERR_PARAM_LE_ZERO "parameter <= 0"

#define ARG_UNUSED(x) (void)(x)

#define ARG_CHECK(a, str, ...) do {                                                         \
      if (!(a)) {                                                                           \
        arg_loge(TAG, "%s:%d (%s): " str, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
        return ESP_ERR_INVALID_ARG;                                                         \
      }                                                                                     \
    } while(0)

#define CHECK_NO_MEM(ptr) do {                                                              \
      if ((ptr) == NULL) {                                                                  \
        arg_loge(TAG, "%s:%d (%s): out of memory", __FILE__, __LINE__, __FUNCTION__);       \
        return ESP_ERR_NO_MEM;                                                              \
      }                                                                                     \
    } while(0)

#define ARG_ERROR_CHECK(a, str, ...) do {                                                   \
      if (!(a)) {                                                                           \
        arg_loge(TAG, "%s:%d (%s): " str, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
        ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);                                             \
      }                                                                                     \
    } while(0)

#define FAIL_IF(a, str, ...) do {                                                           \
      if ((a)) {                                                                            \
        arg_loge(TAG, "%s:%d (%s): " str, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
        goto fail;                                                                          \
      }                                                                                     \
    } while(0)

#define FAIL_IF_NO_MEM(ptr) do {                                                            \
      if ((ptr) == NULL) {                                                                  \
        arg_loge(TAG, "%s:%d (%s): out of memory", __FILE__, __LINE__, __FUNCTION__);       \
        goto fail;                                                                          \
      }                                                                                     \
    } while(0)

void arg_loge(const char *tag, const char *fmt, ...) __printflike(2, 3);

#endif //HYDROPONICS_ERROR_H
