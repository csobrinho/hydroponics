#include <errno.h>
#include <stdatomic.h>
#include <string.h>
#include <time.h>
#include <sys/queue.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "esp_err.h"

#include "ccronexpr.h"

#include "context.h"
#include "error.h"
#include "cron.h"
#include "utils.h"

#define INVALID_INSTANT ((time_t) -1) // Invalid time defined in time.h.
#define NS_TO_MS 1000000L
#define S_TO_MS 1000

typedef struct {
    cron_handle_t handle;
    const char name[32];
    const char expression[64];
    cron_callback_t callback;
    void *data;
    cron_expr expr;
    time_t next_execution;
} cron_job_t;

typedef enum {
    CRON_OP_ADD = 0,
    CRON_OP_REMOVE = 1,
    CRON_OP_DUMP = 2,
} cron_op_type_t;

typedef struct {
    cron_op_type_t type;
    union {
        struct {
            cron_job_t job;
        } add;
        struct {
            cron_handle_t handle;
        } remove;
    };
} cron_op_t;

typedef struct cron_job_entry {
    cron_job_t job;
    TAILQ_ENTRY(cron_job_entry) next;
} cron_job_entry_t;

typedef TAILQ_HEAD(cron_job_head, cron_job_entry) cron_job_head_t;

static const char *TAG = "cron";
static atomic_uint id = 1;
static QueueHandle_t queue;
static cron_job_head_t cron_job_head;

static struct timespec cron_now(void) {
    struct timespec now = {0};
    if (clock_gettime(CLOCK_REALTIME, &now) != 0) {
        ESP_LOGE(TAG, "clock_gettime, error: %d", errno);
        ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);
    }
    return now;
}

static TickType_t cron_next_delay(void) {
    if (TAILQ_EMPTY(&cron_job_head)) {
        return portMAX_DELAY;
    }
    cron_job_entry_t *first = TAILQ_FIRST(&cron_job_head);
    if (first == NULL || first->job.next_execution == INVALID_INSTANT) {
        return portMAX_DELAY;
    }
    struct timespec now = {0};
    if (clock_gettime(CLOCK_REALTIME, &now) != 0) {
        return portMAX_DELAY;
    }
    long delay_ms = (first->job.next_execution - now.tv_sec) * S_TO_MS - (now.tv_nsec / NS_TO_MS);
    if (delay_ms <= 0) {
        // We passed the deadline so run as many as possible.
        return 0;
    }
    // Round up the delay to the nearest 'tick' to avoid undershooting.
    long tick_ms = portTICK_PERIOD_MS;
    long round_up_delay_ms = ((delay_ms + tick_ms - 1) / tick_ms) * tick_ms;
    ESP_LOGD(TAG, "cron_next_delay will wait %ld ms (next: %ld, tv_sec: %ld, tv_msec: %ld)", round_up_delay_ms,
             first->job.next_execution, now.tv_sec, now.tv_nsec / NS_TO_MS);
    return pdMS_TO_TICKS(round_up_delay_ms);
}

static esp_err_t cron_calculate_next(cron_job_t *job) {
    job->next_execution = cron_next(&job->expr, cron_now().tv_sec);
    return job->next_execution != INVALID_INSTANT ? ESP_OK : ESP_FAIL;
}

static esp_err_t cron_unschedule_job(cron_handle_t handle) {
    ARG_CHECK(handle > INVALID_CRON_HANDLE, ERR_PARAM_LE_ZERO);

    cron_job_entry_t *e = NULL, *tmp = NULL;
    TAILQ_FOREACH_SAFE(e, &cron_job_head, next, tmp) {
        if (e->job.handle == handle) {
            TAILQ_REMOVE(&cron_job_head, e, next);
            break;
        }
    }
    return ESP_OK;
}

static esp_err_t cron_schedule_job(cron_job_entry_t *entry) {
    ESP_ERROR_CHECK(cron_calculate_next(&entry->job));
    ESP_ERROR_CHECK(cron_unschedule_job(entry->job.handle));

    if (entry->job.next_execution == INVALID_INSTANT) {
        TAILQ_INSERT_TAIL(&cron_job_head, entry, next);
        return ESP_OK;
    }
    if (TAILQ_EMPTY(&cron_job_head)) {
        TAILQ_INSERT_HEAD(&cron_job_head, entry, next);
        return ESP_OK;
    }
    cron_job_entry_t *e = NULL;
    TAILQ_FOREACH(e, &cron_job_head, next) {
        if (entry->job.next_execution < e->job.next_execution) {
            TAILQ_INSERT_BEFORE(e, entry, next);
            return ESP_OK;
        }
    }
    TAILQ_INSERT_TAIL(&cron_job_head, entry, next);
    return ESP_OK;
}

static esp_err_t cron_create_job(cron_job_t *job) {
    ARG_CHECK(job != NULL, ERR_PARAM_NULL);
    cron_job_entry_t *entry = calloc(1, sizeof(cron_job_entry_t));
    if (entry == NULL) {
        return ESP_ERR_NO_MEM;
    }
    memcpy(&entry->job, job, sizeof(cron_job_t));
    ESP_ERROR_CHECK(cron_schedule_job(entry));
    return ESP_OK;
}

static esp_err_t cron_destroy_job(cron_handle_t handle) {
    ARG_CHECK(handle > INVALID_CRON_HANDLE, ERR_PARAM_LE_ZERO);

    cron_job_entry_t *e = NULL, *tmp = NULL;
    TAILQ_FOREACH_SAFE(e, &cron_job_head, next, tmp) {
        if (e->job.handle == handle) {
            TAILQ_REMOVE(&cron_job_head, e, next);
            SAFE_FREE(e);
            break;
        }
    }
    return ESP_OK;
}

static void cron_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    // Wait for the time to be accurate.
    xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_TIME, pdFALSE, pdTRUE, portMAX_DELAY);
    while (true) {
        TickType_t ticks = cron_next_delay();
        cron_op_t op = {0};
        if (xQueueReceive(queue, &op, ticks) == pdTRUE) {
            switch (op.type) {
                case CRON_OP_ADD:
                    ESP_ERROR_CHECK(cron_create_job(&op.add.job));
                    break;
                case CRON_OP_REMOVE:
                    ESP_ERROR_CHECK(cron_destroy_job(op.remove.handle));
                    break;
                case CRON_OP_DUMP:
                    // TODO
                    break;
            }
        }
        if (TAILQ_EMPTY(&cron_job_head)) {
            continue;
        }
        cron_job_entry_t *e = NULL, *tmp = NULL;
        TAILQ_FOREACH_SAFE(e, &cron_job_head, next, tmp) {
            if (e->job.next_execution == INVALID_INSTANT) {
                break;
            }
            struct timespec now = cron_now();
            if (e->job.next_execution > now.tv_sec) {
                break;
            }
            e->job.callback(e->job.handle, e->job.name, e->job.data);
            ESP_ERROR_CHECK(cron_schedule_job(e));
        }
    }
}

esp_err_t cron_init(context_t *context) {
    queue = xQueueCreate(10, sizeof(cron_op_t));
    if (queue == NULL) {
        return ESP_ERR_NO_MEM;
    }
    TAILQ_INIT(&cron_job_head);

    xTaskCreatePinnedToCore(cron_task, "cron", 4096, context, tskIDLE_PRIORITY + 3, NULL, tskNO_AFFINITY);
    return ESP_OK;
}

esp_err_t cron_create(const char *name, const char *expression, cron_callback_t callback, void *data,
                      cron_handle_t *handle) {
    ARG_CHECK(name != NULL, ERR_PARAM_NULL);
    ARG_CHECK(expression != NULL, ERR_PARAM_NULL);
    ARG_CHECK(callback != NULL, ERR_PARAM_NULL);

    cron_handle_t new_handle = (cron_handle_t) id++;
    cron_job_t job = {0};
    job.handle = new_handle;
    strlcpy((char *) job.name, name, sizeof(job.name));
    strlcpy((char *) job.expression, expression, sizeof(job.expression));
    job.callback = callback;
    job.data = data;
    job.next_execution = INVALID_INSTANT;

    const char *error = NULL;
    cron_parse_expr(job.expression, &job.expr, &error);
    if (error != NULL) {
        ESP_LOGE(TAG, "Could not parse %s, error: %s", job.expression, error);
        return ESP_ERR_INVALID_ARG;
    }
    cron_op_t arg = {.type = CRON_OP_ADD, .add = {.job = job}};
    if (xQueueSend(queue, &arg, portMAX_DELAY) != pdPASS) {
        return ESP_FAIL;
    }
    if (handle != NULL) {
        *handle = arg.add.job.handle;
    }
    return ESP_OK;
}

esp_err_t cron_delete(cron_handle_t handle) {
    ARG_CHECK(handle > INVALID_CRON_HANDLE, ERR_PARAM_LE_ZERO);

    cron_op_t arg = {.type = CRON_OP_REMOVE, .remove = {.handle = handle}};
    return xQueueSend(queue, &arg, portMAX_DELAY) == pdPASS ? ESP_OK : ESP_FAIL;
}
