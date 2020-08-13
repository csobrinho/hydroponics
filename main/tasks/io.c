#include <string.h>
#include <sys/queue.h>

#include "freertos/FreeRTOS.h"

#include "esp_err.h"
#include "driver/gpio.h"

#include "buses.h"
#include "config.h"
#include "context.h"
#include "cron.h"
#include "driver/ext_gpio.h"
#include "error.h"
#include "io.h"
#include "tasks/tuya_io.h"
#include "utils.h"

typedef struct {
    Hydroponics__OutputState state;
    size_t n_output;
    Hydroponics__Output *output;
    bool single_shot;
} io_cron_args_t;

typedef struct entry {
    cron_handle_t handle;
    io_cron_args_t *cron_args;
    TAILQ_ENTRY(entry) next;
} entry_t;

typedef TAILQ_HEAD(head, entry) head_t;

typedef enum {
    OP_CONFIG = 1,
    OP_SET = 2,
} op_type_t;

typedef struct {
    op_type_t type;
    union {
        struct {
            const Hydroponics__Config *config;
        } config;
        struct {
            const Hydroponics__Output output;
            bool value;
            uint16_t delay_ms;
        } set;
    };
} op_t;

static const char *TAG = "io";
static QueueHandle_t queue;
static head_t head;

static esp_err_t io_cron_args_create(io_cron_args_t **cron_args, const size_t n_output,
                                     const Hydroponics__Output *output, const Hydroponics__OutputState state) {
    ARG_CHECK(cron_args != NULL, ERR_PARAM_NULL);

    *cron_args = calloc(1, sizeof(io_cron_args_t));
    if (*cron_args == NULL) {
        return ESP_ERR_NO_MEM;
    }
    (*cron_args)->state = state;
    (*cron_args)->n_output = n_output;
    (*cron_args)->output = calloc(1, n_output * sizeof(Hydroponics__Output));
    if ((*cron_args)->output == NULL) {
        SAFE_FREE(*cron_args);
        return ESP_ERR_NO_MEM;
    }
    memcpy((*cron_args)->output, output, n_output * sizeof(Hydroponics__Output));
    return ESP_OK;
}

static esp_err_t io_cron_args_destroy(io_cron_args_t *cron_args) {
    if (cron_args != NULL) {
        SAFE_FREE(cron_args->output);
        SAFE_FREE(cron_args);
    }
    return ESP_OK;
}

static void io_generic_set(Hydroponics__Output output, Hydroponics__OutputState state) {
    if (IS_EXT_GPIO(output)) {
        uint32_t value = state == HYDROPONICS__OUTPUT_STATE__ON ? true : false;
        ESP_ERROR_CHECK(ext_gpio_set_level((ext_gpio_num_t) output, value));
    } else if (IS_EXT_TUYA(output)) {
        ESP_ERROR_CHECK(tuya_io_set(output, state));
    } else {
        ESP_LOGE(TAG, "Unknown output: %d", output);
    }
}

static void io_cron_callback(cron_handle_t handle, const char *name, void *data) {
    ARG_UNUSED(handle);
    io_cron_args_t *args = (io_cron_args_t *) data;
    for (int i = 0; i < args->n_output; ++i) {
        ESP_LOGI(TAG, "io_cron_callback name: %s action: %3s output: %s", name,
                 enum_from_value(&hydroponics__output_state__descriptor, args->state),
                 enum_from_value(&hydroponics__output__descriptor, args->output[i]));
        io_generic_set(args->output[i], args->state);
    }
    if (args->single_shot) {
        entry_t *e = NULL, *tmp = NULL;
        TAILQ_FOREACH_SAFE(e, &head, next, tmp) {
            if (e->handle == handle) {
                ESP_ERROR_CHECK(cron_delete(e->handle));
                ESP_ERROR_CHECK(io_cron_args_destroy(e->cron_args));
                TAILQ_REMOVE(&head, e, next);
            }
        }
    }
    if (args->single_shot) {
        ESP_ERROR_CHECK(cron_delete(handle));
        ESP_ERROR_CHECK(io_cron_args_destroy(args));
    }
}

static void io_config_callback(const Hydroponics__Config *config) {
    const op_t cmd = {.type = OP_CONFIG, .config = {.config = config}};
    xQueueSend(queue, &cmd, portMAX_DELAY);
}

static void io_set_default_state(const Hydroponics__Config *config) {
    if (config == NULL || config->n_startup_state <= 0) {
        return;
    }
    // Remove all upcoming messages to avoid getting stuck if the queue is not big enough.
    xQueueReset(queue);
    for (int i = 0; i < config->n_startup_state; ++i) {
        Hydroponics__StartupState *s = config->startup_state[i];
        for (int j = 0; j < s->n_output; ++j) {
            io_generic_set(s->output[j], s->state);
        }
    }
}

static esp_err_t io_cron_add(const char *name, const char *expression, const size_t n_output,
                             const Hydroponics__Output *output, const Hydroponics__OutputState state,
                             uint16_t delay_ms) {
    io_cron_args_t *cron_args = NULL;
    cron_handle_t handle = INVALID_CRON_HANDLE;
    ESP_ERROR_CHECK(io_cron_args_create(&cron_args, n_output, output, state));
    if (delay_ms > 0) {
        ESP_ERROR_CHECK(cron_schedule_in(name, delay_ms, io_cron_callback, cron_args, &handle));
    } else {
        ESP_ERROR_CHECK(cron_create(name, expression, io_cron_callback, cron_args, &handle));
    }
    entry_t *e = calloc(1, sizeof(entry_t));
    if (e == NULL) {
        ESP_LOGE(TAG, "Error allocating entry_t for task %s", name);
        ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    }
    e->handle = handle;
    e->cron_args = cron_args;
    TAILQ_INSERT_HEAD(&head, e, next);

    return ESP_OK;
}

static void io_apply_config(const Hydroponics__Config *config) {
    ESP_LOGI(TAG, "Applying config...");

    // Clear previous registrations.
    entry_t *e = NULL, *tmp = NULL;
    TAILQ_FOREACH_SAFE(e, &head, next, tmp) {
        ESP_ERROR_CHECK(cron_delete(e->handle));
        ESP_ERROR_CHECK(io_cron_args_destroy(e->cron_args));
        TAILQ_REMOVE(&head, e, next);
    }
    io_set_default_state(config);
    if (config != NULL && config->n_task > 0) {
        // Register the new tasks.
        for (int i = 0; i < config->n_task; ++i) {
            const Hydroponics__Task *task = config->task[i];
            for (int j = 0; j < task->n_cron; ++j) {
                const Hydroponics__Task__Cron *cron = task->cron[j];
                for (int k = 0; k < cron->n_expression; ++k) {
                    for (int o = 0; o < task->n_output; ++o) {
                        const Hydroponics__Output output = task->output[o];
                        gpio_config_t cfg = {
                                .pin_bit_mask = BIT64(output),
                                .mode = GPIO_MODE_OUTPUT,
                                .pull_up_en = GPIO_PULLUP_ENABLE,
                        };
                        ESP_ERROR_CHECK(ext_gpio_config(&cfg));
                    }
                    ESP_ERROR_CHECK(io_cron_add(task->name, cron->expression[k], task->n_output, task->output,
                                                cron->state, 0));
                }
            }
        }
    }
    // Free the copied config proto.
    if (config != NULL) {
        hydroponics__config__free_unpacked((Hydroponics__Config *) config, NULL);
        config = NULL;
    }
}

static void io_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    while (true) {
        xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_CONFIG, pdFALSE, pdTRUE, portMAX_DELAY);
        const Hydroponics__Config *config = NULL;
        ESP_ERROR_CHECK(context_get_config(context, &config));
        io_apply_config(config);

        while (true) {
            op_t op = {0};
            if (xQueueReceive(queue, &op, portMAX_DELAY) == pdTRUE) {
                switch (op.type) {
                    case OP_CONFIG: { // Re-load config.
                        io_apply_config(op.config.config);
                        break;
                    }
                    case OP_SET: {
                        Hydroponics__OutputState state = op.set.value ? HYDROPONICS__OUTPUT_STATE__ON
                                                                      : HYDROPONICS__OUTPUT_STATE__OFF;
                        io_generic_set(op.set.output, state);
                        if (op.set.delay_ms == 0) {
                            break;
                        }
                        ESP_ERROR_CHECK(io_cron_add("STEP", NULL, 1, &op.set.output, !op.set.value, op.set.delay_ms));
                    }
                }
            }
        }
    }
}

esp_err_t io_init(context_t *context) {
    TAILQ_INIT(&head);

    queue = xQueueCreate(32, sizeof(op_t));
    CHECK_NO_MEM(queue);

    config_register(io_config_callback);
    xTaskCreatePinnedToCore(io_task, "io", 4096, context, tskIDLE_PRIORITY + 10, NULL, tskNO_AFFINITY);
    return ESP_OK;
}

esp_err_t io_set_level(const Hydroponics__Output output, bool value, uint16_t delay_ms) {
    const op_t cmd = {.type = OP_SET, .set = {.output = output, .value = value, .delay_ms = delay_ms}};
    xQueueSend(queue, &cmd, portMAX_DELAY);
    return ESP_OK;
}