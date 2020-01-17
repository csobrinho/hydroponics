#ifndef HYDROPONICS_NETWORK_WIFI_H
#define HYDROPONICS_NETWORK_WIFI_H

#include "esp_err.h"

#include "context.h"

esp_err_t wifi_init(context_t *context, const char *ssid, const char *password);

esp_err_t wifi_disconnect(void);

#endif //HYDROPONICS_NETWORK_WIFI_H
