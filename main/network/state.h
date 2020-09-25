#ifndef HYDROPONICS_NETWORK_STATE_BUILDER_H
#define HYDROPONICS_NETWORK_STATE_BUILDER_H

#include "esp_err.h"

#include "state.pb-c.h"

#include "context.h"

esp_err_t state_push_memory(uint32_t min_free, uint32_t free);

esp_err_t state_push_tasks(const TaskStatus_t *task_status, size_t size, uint32_t total_runtime_percentage);

esp_err_t state_push_telemetry(size_t size, const Hydroponics__StateTelemetry__Type *types, const float *values);

esp_err_t state_push_output(size_t size, const size_t *buckets, const Hydroponics__Output *outputs,
                            const Hydroponics__OutputState *output_states);

#endif //HYDROPONICS_NETWORK_STATE_BUILDER_H
