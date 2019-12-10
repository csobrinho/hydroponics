#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"

#include "context.h"

context_t *context_create(void) {
    context_t *context = calloc(1, sizeof(context_t));

    portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
    context->spinlock = spinlock;
    context->event_group = xEventGroupCreate();

    context->sensors.temp.indoor = CONTEXT_UNKNOWN_VALUE;
    context->sensors.temp.water = CONTEXT_UNKNOWN_VALUE;
    context->sensors.humidity = CONTEXT_UNKNOWN_VALUE;
    context->sensors.pressure = CONTEXT_UNKNOWN_VALUE;

    context->sensors.ec.value = CONTEXT_UNKNOWN_VALUE;
    context->sensors.ec.target_min = CONTEXT_UNKNOWN_VALUE;
    context->sensors.ec.target_max = CONTEXT_UNKNOWN_VALUE;

    context->sensors.ph.value = CONTEXT_UNKNOWN_VALUE;
    context->sensors.ph.target_min = CONTEXT_UNKNOWN_VALUE;
    context->sensors.ph.target_max = CONTEXT_UNKNOWN_VALUE;

    return context;
}