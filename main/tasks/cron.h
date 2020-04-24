#ifndef HYDROPONICS_TASKS_CRON_H
#define HYDROPONICS_TASKS_CRON_H

#include "esp_err.h"

#include "context.h"

typedef unsigned int cron_handle_t;

typedef void (*cron_callback_t)(cron_handle_t handle, const char *name, void *data);

esp_err_t cron_init(context_t *context);

/*
 *  expression: Cron syntax for the schedule
 *            ┌────────────── second (0 - 59)
 *            | ┌───────────── minute (0 - 59)
 *            | │ ┌──────────── hour (0 - 23)
 *            | │ │ ┌─────────── day of month (1 - 31)
 *            | │ │ │ ┌────────── month (1 - 12)
 *            | │ │ │ │ ┌───────── day of week (0 - 6) (Sunday to Saturday; 7 is also Sunday on some systems)
 *            | │ │ │ │ │
 *            | │ │ │ │ │
 *            | │ │ │ │ │
 *            * * * * * *
*/
esp_err_t cron_create(const char *name, const char *expression, cron_callback_t callback, void *data,
                      cron_handle_t *handle);

esp_err_t cron_delete(cron_handle_t handle);

#endif //HYDROPONICS_TASKS_CRON_H
