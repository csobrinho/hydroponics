#ifndef HYDROPONICS_TASKS_TUYA_IO_H
#define HYDROPONICS_TASKS_TUYA_IO_H

#include "esp_err.h"

#include "context.h"

esp_err_t tuya_io_init(context_t *context);

esp_err_t tuya_io_set(Hydroponics__Output output, bool value);

#endif //HYDROPONICS_TASKS_TUYA_IO_H
