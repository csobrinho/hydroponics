#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "cron.h"
#include "error.h"
#include "monitor.h"
#include "utils.h"

#define MONITOR_CRON           "0 * * * * *"  // Once every minute, at 0s.
#define MONITOR_STATE_SAMPLING 2              // Once every two calls.

static const char *TAG = "monitor";
static const char *STATES[] = {"R", "*", "B", "S", "D", "?"};
static const size_t STATES_SIZE = 6;

static int by_task_name(const void *a, const void *b) {
    TaskStatus_t *ia = (TaskStatus_t *) a;
    TaskStatus_t *ib = (TaskStatus_t *) b;
    return strcmp(ia->pcTaskName, ib->pcTaskName);
}

static esp_err_t monitor_dump_stdin(context_t *context, TaskStatus_t *taskStatus, UBaseType_t size,
                                    uint32_t totalRunTimePercentage) {
    ARG_UNUSED(context);
    ESP_LOGI(TAG, "========================================================");
    ESP_LOGI(TAG, " St P  Name                     Runtime      %%    Stack");
    ESP_LOGI(TAG, "========================================================");

    // For each populated position in the taskStatus array, format the raw data as human readable.
    for (UBaseType_t i = 0; i < size; i++) {
        // What percentage of the total run time has the task used?
        // This will always be rounded down to the nearest integer.
        // ulTotalRunTimeDiv100 has already been divided by 100.
        uint32_t ulStatsAsPercentage = taskStatus[i].ulRunTimeCounter / totalRunTimePercentage;

        const char *state = taskStatus[i].eCurrentState < STATES_SIZE
                            ? STATES[taskStatus[i].eCurrentState]
                            : STATES[STATES_SIZE - 1];

        if (ulStatsAsPercentage > 0UL) {
            ESP_LOGI(TAG, " [%s:%2d] %-16s    %11u    %2u%%    %5u", state, taskStatus[i].uxCurrentPriority,
                     taskStatus[i].pcTaskName, taskStatus[i].ulRunTimeCounter, ulStatsAsPercentage,
                     taskStatus[i].usStackHighWaterMark);
        } else {
            // If the percentage is zero here then the task has
            // consumed less than 1% of the total run time.
            ESP_LOGI(TAG, " [%s:%2d] %-16s    %11u    <1%%    %5u", state, taskStatus[i].uxCurrentPriority,
                     taskStatus[i].pcTaskName, taskStatus[i].ulRunTimeCounter,
                     taskStatus[i].usStackHighWaterMark);
        }
    }
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Minimum free heap: %d    free heap: %d", esp_get_minimum_free_heap_size(), esp_get_free_heap_size());
    ESP_LOGI(TAG, "========================================================");

    return ESP_OK;
}

static esp_err_t monitor_dump_state_message(context_t *context, TaskStatus_t *taskStatus, UBaseType_t size,
                                            uint32_t totalRunTimePercentage) {
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for the root");
        goto error;
    }
    cJSON *memory = cJSON_AddObjectToObject(root, "memory");
    if (memory == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for the memory");
        goto error;
    }
    cJSON_AddNumberToObject(memory, "min_free", esp_get_minimum_free_heap_size());
    cJSON_AddNumberToObject(memory, "free", esp_get_free_heap_size());

    cJSON *tasks = cJSON_AddArrayToObject(root, "tasks");
    if (tasks == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for the tasks");
        goto error;
    }

    // For each populated position in the taskStatus array, format the raw data as human readable.
    for (UBaseType_t i = 0; i < size; i++) {
        cJSON *task = cJSON_CreateObject();
        if (task == NULL) {
            goto error;
        }
        cJSON_AddItemToArray(tasks, task);

        // What percentage of the total run time has the task used? This will always be rounded down to the nearest
        // integer. ulTotalRunTimeDiv100 has already been divided by 100.
        uint32_t ulStatsAsPercentage = taskStatus[i].ulRunTimeCounter / totalRunTimePercentage;

        const char *state = taskStatus[i].eCurrentState < STATES_SIZE
                            ? STATES[taskStatus[i].eCurrentState]
                            : STATES[STATES_SIZE - 1];

        cJSON_AddStringToObject(task, "name", taskStatus[i].pcTaskName);
        cJSON_AddStringToObject(task, "state", state);
        cJSON_AddNumberToObject(task, "priority", taskStatus[i].uxCurrentPriority);
        cJSON_AddNumberToObject(task, "runtime", taskStatus[i].ulRunTimeCounter);
        cJSON_AddNumberToObject(task, "stats", ulStatsAsPercentage);
        cJSON_AddNumberToObject(task, "highwater", taskStatus[i].usStackHighWaterMark);
    }

    char *message = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (message == NULL) {
        ESP_LOGE(TAG, "Failed to print the json state string");
        return ESP_FAIL;
    }
    ESP_ERROR_CHECK(context_set_state_message(context, message));
    return ESP_OK;

    error:
    vTaskDelay(pdMS_TO_TICKS(200));
    if (root != NULL) {
        cJSON_Delete(root);
    }
    return ESP_ERR_NO_MEM;
}

static void monitor_dump(context_t *context) {
    // Take a snapshot of the number of tasks in case it changes while this function is executing.
    UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();

    // Allocate a TaskStatus_t structure for each task.  An array could be allocated statically at compile time.
    TaskStatus_t *pxTaskStatusArray = malloc(uxArraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for the monitor");
        ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    }

    // Generate raw status information about each task.
    uint32_t ulTotalRunTime = 0;
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

    // For percentage calculations.
    ulTotalRunTime /= 100UL;

    // Avoid divide by zero errors.
    if (ulTotalRunTime > 0) {
        // Sort the TaskStatus_t array by the task name.
        qsort(pxTaskStatusArray, uxArraySize, sizeof(TaskStatus_t), by_task_name);
        ESP_ERROR_CHECK(monitor_dump_stdin(context, pxTaskStatusArray, uxArraySize, ulTotalRunTime));

        static int counter = 0;
        if ((++counter % MONITOR_STATE_SAMPLING) == 0) {
            ESP_ERROR_CHECK(monitor_dump_state_message(context, pxTaskStatusArray, uxArraySize, ulTotalRunTime));
        }
    }
    SAFE_FREE(pxTaskStatusArray);
}

static void monitor_cron_callback(cron_handle_t handle, const char *name, void *data) {
    ARG_UNUSED(handle);
    ESP_LOGI(TAG, "monitor_cron_callback name: %s", name);
    monitor_dump((context_t *) data);
}

esp_err_t monitor_init(context_t *context) {
    ESP_ERROR_CHECK(cron_create("monitor", MONITOR_CRON, monitor_cron_callback, context, NULL));
    return ESP_OK;
}