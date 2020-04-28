#include <string.h>
#include <sys/queue.h>

#include "freertos/FreeRTOS.h"

#include "esp_err.h"
#include "driver/gpio.h"

#include "context.h"
#include "cron.h"
#include "driver/ext_gpio.h"
#include "error.h"
#include "io.h"
#include "utils.h"

typedef struct {
    Hydroponics__Task__OutputAction action;
    size_t n_output;
    Hydroponics__Task__Output *output;
} io_cron_args_t;

typedef struct entry {
    cron_handle_t handle;
    io_cron_args_t *cron_args;
    TAILQ_ENTRY(entry) next;
} entry_t;

typedef TAILQ_HEAD(head, entry) head_t;

static const char *TAG = "io";
static head_t head;

static esp_err_t io_cron_args_create(io_cron_args_t **cron_args, const size_t n_output,
                                     const Hydroponics__Task__Output *output,
                                     const Hydroponics__Task__OutputAction action) {
    ARG_CHECK(cron_args != NULL, ERR_PARAM_NULL);

    *cron_args = malloc(sizeof(io_cron_args_t));
    if (*cron_args == NULL) {
        return ESP_ERR_NO_MEM;
    }
    (*cron_args)->action = action;
    (*cron_args)->n_output = n_output;
    (*cron_args)->output = malloc(n_output * sizeof(Hydroponics__Task__Output));
    if ((*cron_args)->output == NULL) {
        SAFE_FREE(*cron_args);
        return ESP_ERR_NO_MEM;
    }
    memcpy((*cron_args)->output, output, n_output * sizeof(Hydroponics__Task__Output));
    return ESP_OK;
}

static esp_err_t io_cron_args_destroy(io_cron_args_t *cron_args) {
    if (cron_args != NULL) {
        SAFE_FREE(cron_args->output);
        SAFE_FREE(cron_args);
    }
    return ESP_OK;
}

static void io_cron_callback(cron_handle_t handle, const char *name, void *data) {
    ARG_UNUSED(handle);
    io_cron_args_t *args = (io_cron_args_t *) data;
    for (int i = 0; i < args->n_output; ++i) {
        ESP_LOGI(TAG, "io_cron_callback name: %s action: %3s output: %s", name,
                 enum_from_value(&hydroponics__task__output_action__descriptor, args->action),
                 enum_from_value(&hydroponics__task__output__descriptor, args->output[i]));
        ESP_ERROR_CHECK(ext_gpio_set_level((ext_gpio_num_t) args->output[i],
                                           args->action == HYDROPONICS__TASK__OUTPUT_ACTION__ON ? true : false));
    }
}

static void io_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    while (true) {
        xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_CONFIG, pdTRUE, pdTRUE, portMAX_DELAY);
        ESP_LOGI(TAG, "Applying new config...");

        // Clear previous registrations.
        entry_t *e = NULL, *tmp = NULL;
        TAILQ_FOREACH_SAFE(e, &head, next, tmp) {
            cron_delete(e->handle);
            ESP_ERROR_CHECK(io_cron_args_destroy(e->cron_args));
            TAILQ_REMOVE(&head, e, next);
        }

        // Fetch a copy of the new config.
        Hydroponics__Config *config = NULL;
        ESP_ERROR_CHECK(context_get_config(context, &config));
        if (config != NULL && config->n_task > 0) {
            // Register the new tasks.
            for (int i = 0; i < config->n_task; ++i) {
                Hydroponics__Task *task = config->task[i];
                for (int j = 0; j < task->n_cron; ++j) {
                    Hydroponics__Task__Cron *cron = task->cron[j];
                    for (int k = 0; k < cron->n_expression; ++k) {
                        io_cron_args_t *cron_args = NULL;
                        cron_handle_t handle = INVALID_CRON_HANDLE;

                        for (int o = 0; o < task->n_output; ++o) {
                            Hydroponics__Task__Output output = task->output[o];
                            gpio_config_t cfg = {
                                    .pin_bit_mask = BIT64(output),
                                    .mode = GPIO_MODE_OUTPUT,
                                    .pull_up_en = GPIO_PULLUP_ENABLE,
                            };
                            ESP_ERROR_CHECK(ext_gpio_config(&cfg));
                        }

                        ESP_ERROR_CHECK(io_cron_args_create(&cron_args, task->n_output, task->output, cron->action));
                        ESP_ERROR_CHECK(cron_create(task->name, cron->expression[k], io_cron_callback, cron_args,
                                                    &handle));
                        e = calloc(1, sizeof(entry_t));
                        if (e == NULL) {
                            ESP_LOGE(TAG, "Error allocating entry_t for task %s", task->name);
                            ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
                        }
                        e->handle = handle;
                        e->cron_args = cron_args;
                        TAILQ_INSERT_HEAD(&head, e, next);
                    }
                }
            }
        }
        // Free the copied config proto.
        if (config != NULL) {
            hydroponics__config__free_unpacked(config, NULL);
            config = NULL;
        }
    }
}

esp_err_t io_init(context_t *context) {
    xTaskCreatePinnedToCore(io_task, "io", 4096, context, tskIDLE_PRIORITY + 10, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
