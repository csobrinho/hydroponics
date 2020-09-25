#ifndef HYDROPONICS_NETWORK_IOT_H
#define HYDROPONICS_NETWORK_IOT_H

#include "esp_err.h"

#include "state.pb-c.h"

#include "context.h"

esp_err_t iot_init(context_t *context);

esp_err_t iot_publish_telemetry(Hydroponics__States *states);

esp_err_t iot_publish_state(Hydroponics__States *states);

#endif //HYDROPONICS_NETWORK_IOT_H
