#ifndef HYDROPONICS_ERROR_H
#define HYDROPONICS_ERROR_H

#include "esp_log.h"

#define ERR_PARAM_NULL    "parameter == null"
#define ERR_PARAM_LE_ZERO "parameter <= 0"

#define ARG_CHECK(a, str)  if(!(a)) {                                           \
        ESP_LOGE(TAG, "%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str);  \
        return ESP_ERR_INVALID_ARG;                                             \
        }

#define ARG_ERROR_CHECK(a, str)  if(!(a)) {                                     \
        ESP_LOGE(TAG, "%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str);  \
        ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);                                 \
        }

#endif //HYDROPONICS_ERROR_H
