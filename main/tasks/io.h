#ifndef HYDROPONICS_TASKS_IO_H
#define HYDROPONICS_TASKS_IO_H

#include "esp_err.h"

#include "context.h"

esp_err_t io_init(context_t *context);

esp_err_t io_set_level(const Hydroponics__Output output, bool value, uint16_t delay_ms);

#endif //HYDROPONICS_TASKS_IO_H
