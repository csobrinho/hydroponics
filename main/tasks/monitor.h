#ifndef HYDROPONICS_TASKS_MONITOR_H
#define HYDROPONICS_TASKS_MONITOR_H

#include "cJSON.h"
#include "esp_err.h"

#include "context.h"

esp_err_t monitor_init(context_t *context);

#endif //HYDROPONICS_TASKS_MONITOR_H
