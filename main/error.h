#ifndef HYDROPONICS_ERROR_H
#define HYDROPONICS_ERROR_H

#define ERR_PARAM_NULL "parameter == null"

#define ARG_CHECK(a, str)  if(!(a)) {                                           \
        ESP_LOGE(TAG, "%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str);  \
        return ESP_ERR_INVALID_ARG;                                             \
        }

#endif //HYDROPONICS_ERROR_H
