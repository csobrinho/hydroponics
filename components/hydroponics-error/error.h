#ifndef HYDROPONICS_ERROR_H
#define HYDROPONICS_ERROR_H

#include "esp_log.h"

#define ERR_PARAM_NULL    "parameter == null"
#define ERR_PARAM_LE_ZERO "parameter <= 0"

#define ARG_UNUSED(x) (void)(x)

#define ARG_CHECK(a, str) do {                                                  \
      if(!(a)) {                                                                \
        arg_loge(TAG, __FILE__, __LINE__, __FUNCTION__, str);  \
        return ESP_ERR_INVALID_ARG;                                             \
      }                                                                         \
    } while(0)

#define ARG_ERROR_CHECK(a, str) do {                                            \
      if(!(a)) {                                                                \
        arg_loge(TAG, __FILE__, __LINE__, __FUNCTION__, str);  \
        ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);                                 \
      }                                                                         \
    } while(0)

void arg_loge(const char* tag, const char* file, int line, const char* function, const char* str);

#endif //HYDROPONICS_ERROR_H
