#ifndef HYDROPONICS_NETWORK_SYSLOG_H
#define HYDROPONICS_NETWORK_SYSLOG_H

#include <sys/cdefs.h>

#include "esp_err.h"

#include "context.h"

esp_err_t syslog_init(context_t *context);

#endif //HYDROPONICS_NETWORK_SYSLOG_H
