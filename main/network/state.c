#include <string.h>
#include <time.h>

#include "timespec.h"

#include "context.h"
#include "error.h"
#include "iot.h"
#include "state.h"

static const char *const TAG = "state";

static uint64_t state_timestamp(void) {
    struct timespec now = {0};
    if (clock_gettime(CLOCK_REALTIME, &now) != 0) {
        return 0;
    }
    return timespec_to_ms(now);
}

esp_err_t state_push_memory(uint32_t min_free, uint32_t free) {
    Hydroponics__StateMemory memory = HYDROPONICS__STATE_MEMORY__INIT;
    memory.min_free = min_free;
    memory.free = free;

    Hydroponics__State state = HYDROPONICS__STATE__INIT;
    Hydroponics__State *pstate = &state;
    state.timestamp = state_timestamp();
    state.state_case = HYDROPONICS__STATE__STATE_MEMORY;
    state.memory = &memory;

    Hydroponics__States msg = HYDROPONICS__STATES__INIT;
    msg.n_state = 1;
    msg.state = &pstate;

    ESP_LOGW(TAG, "Created memory state: 0x%p", &msg);
    ESP_ERROR_CHECK(iot_publish_state(&msg));
    return ESP_OK;
}

esp_err_t state_push_tasks(const TaskStatus_t *task_status, size_t size, uint32_t total_runtime_percentage) {
    ARG_CHECK(task_status != NULL, ERR_PARAM_NULL);
    if (size == 0) {
        return ESP_OK;
    }

    Hydroponics__StateTask task[size];
    Hydroponics__StateTask *ptask[size];

    for (int i = 0; i < size; ++i) {
        hydroponics__state_task__init(&task[i]);

        const TaskStatus_t *status = &task_status[i];
        task[i].name = (char *) status->pcTaskName;
        task[i].state = (Hydroponics__StateTask__State) status->eCurrentState;
        task[i].priority = status->uxCurrentPriority;
        task[i].runtime = status->ulRunTimeCounter;
        task[i].stats = status->ulRunTimeCounter / total_runtime_percentage;
        task[i].highwater = status->usStackHighWaterMark;

        ptask[i] = &task[i];
    }

    Hydroponics__StateTasks tasks = HYDROPONICS__STATE_TASKS__INIT;
    tasks.n_task = size;
    tasks.task = ptask;

    Hydroponics__State state = HYDROPONICS__STATE__INIT;
    Hydroponics__State *pstate = &state;
    state.timestamp = state_timestamp();
    state.state_case = HYDROPONICS__STATE__STATE_TASKS;
    state.tasks = &tasks;

    Hydroponics__States msg = HYDROPONICS__STATES__INIT;
    msg.n_state = 1;
    msg.state = &pstate;

    ESP_LOGW(TAG, "Created tasks state: 0x%p", &msg);
    ESP_ERROR_CHECK(iot_publish_state(&msg));
    return ESP_OK;
}

esp_err_t state_push_telemetry(size_t size, const Hydroponics__StateTelemetry__Type *types, const float *values) {
    ARG_CHECK(values != NULL, ERR_PARAM_NULL);
    if (size == 0) {
        return ESP_OK;
    }
    Hydroponics__StateTelemetry telemetry = HYDROPONICS__STATE_TELEMETRY__INIT;
    for (int i = 0; i < size; ++i) {
        switch (types[i]) {
            case HYDROPONICS__STATE_TELEMETRY__TYPE__UNKNOWN:
                break;
            case HYDROPONICS__STATE_TELEMETRY__TYPE__TEMP_INDOOR:
                telemetry.temp_indoor = values[i];
                break;
            case HYDROPONICS__STATE_TELEMETRY__TYPE__TEMP_PROBE:
                telemetry.temp_probe = values[i];
                break;
            case HYDROPONICS__STATE_TELEMETRY__TYPE__HUMIDITY:
                telemetry.humidity = values[i];
                break;
            case HYDROPONICS__STATE_TELEMETRY__TYPE__PRESSURE:
                telemetry.pressure = values[i];
                break;
            case HYDROPONICS__STATE_TELEMETRY__TYPE__EC_A:
                telemetry.ec_a = values[i];
                break;
            case HYDROPONICS__STATE_TELEMETRY__TYPE__EC_B:
                telemetry.ec_b = values[i];
                break;
            case HYDROPONICS__STATE_TELEMETRY__TYPE__PH_A:
                telemetry.ph_a = values[i];
                break;
            case HYDROPONICS__STATE_TELEMETRY__TYPE__PH_B:
                telemetry.ph_b = values[i];
                break;
            case HYDROPONICS__STATE_TELEMETRY__TYPE__TANK_A:
                telemetry.tank_a = values[i];
                break;
            case HYDROPONICS__STATE_TELEMETRY__TYPE__TANK_B:
                telemetry.tank_b = values[i];
                break;
            default:
                ARG_CHECK(false, "Telemetry Type: %d is not supported", types[i]);
        }
    }
    Hydroponics__State state = HYDROPONICS__STATE__INIT;
    Hydroponics__State *pstate = &state;
    state.timestamp = state_timestamp();
    state.state_case = HYDROPONICS__STATE__STATE_TELEMETRY;
    state.telemetry = &telemetry;

    Hydroponics__States msg = HYDROPONICS__STATES__INIT;
    msg.n_state = 1;
    msg.state = &pstate;

    ESP_LOGW(TAG, "Created telemetry state: 0x%p", &msg);
    ESP_ERROR_CHECK(iot_publish_telemetry(&msg));

    return ESP_OK;
}

esp_err_t state_push_output(size_t size, const size_t *buckets, const Hydroponics__Output *outputs,
                            const Hydroponics__OutputState *output_states) {
    if (size == 0) {
        return ESP_OK;
    }
    ARG_CHECK(buckets != NULL, ERR_PARAM_NULL);
    ARG_CHECK(outputs != NULL, ERR_PARAM_NULL);
    ARG_CHECK(output_states != NULL, ERR_PARAM_NULL);

    Hydroponics__StateOutput state_output[size];
    Hydroponics__StateOutput *pstate_output[size];
    for (int i = 0; i < size; ++i) {
        hydroponics__state_output__init(&state_output[i]);

        state_output[i].n_output = buckets[i];
        state_output[i].output = (Hydroponics__Output *) outputs;
        state_output[i].state = output_states[i];
        outputs += buckets[i];

        pstate_output[i] = &state_output[i];
    }

    Hydroponics__StateOutputs state_outputs = HYDROPONICS__STATE_OUTPUTS__INIT;
    state_outputs.n_output = size;
    state_outputs.output = pstate_output;

    Hydroponics__State state = HYDROPONICS__STATE__INIT;
    Hydroponics__State *pstate = &state;
    state.timestamp = state_timestamp();
    state.state_case = HYDROPONICS__STATE__STATE_OUTPUTS;
    state.outputs = &state_outputs;

    Hydroponics__States msg = HYDROPONICS__STATES__INIT;
    msg.n_state = 1;
    msg.state = &pstate;

    ESP_LOGW(TAG, "Created output state: 0x%p", &msg);
    ESP_ERROR_CHECK(iot_publish_state(&msg));

    return ESP_OK;
}
