#ifndef HYDROPONICS_NETWORK_WIFI_H
#define HYDROPONICS_NETWORK_WIFI_H

#include "esp_err.h"

esp_err_t wifi_init(const char *ssid, const char *password);

#endif //HYDROPONICS_NETWORK_WIFI_H
