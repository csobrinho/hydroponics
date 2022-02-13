#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "cron.h"
#include "error.h"
#include "monitor.h"
#include "network/state.h"
#include "utils.h"

#define MONITOR_CRON_MEMORY "0 * * * * *"    // Once every minute.
#define MONITOR_CRON_WIFI   "*/30 * * * * *" // Once every 30s.
#define MONITOR_CRON_TASKS  "0 */2 * * * *"  // Once every 2 minutes.

static const char *const TAG = "monitor";
static const uint8_t STATES[] = {'R', '*', 'B', 'S', 'D', '?'};

static int by_task_name(const void *a, const void *b) {
    TaskStatus_t *ia = (TaskStatus_t *) a;
    TaskStatus_t *ib = (TaskStatus_t *) b;
    return strcmp(ia->pcTaskName, ib->pcTaskName);
}

static esp_err_t monitor_dump_stdout(const TaskStatus_t *tasks, size_t size, uint32_t total_runtime_percentage) {
    ESP_LOGI(TAG, "==========================================================");
    ESP_LOGI(TAG, " St C  P  Name                    Runtime      %%    Stack");
    ESP_LOGI(TAG, "==========================================================");

    // For each populated position in the taskStatus array, format the raw data as human readable.
    for (UBaseType_t i = 0; i < size; i++) {
        // What percentage of the total run time has the task used?
        // This will always be rounded down to the nearest integer.
        uint32_t stats_percentage = tasks[i].ulRunTimeCounter / total_runtime_percentage;

        const uint8_t state = tasks[i].eCurrentState < sizeof(STATES)
                              ? STATES[tasks[i].eCurrentState]
                              : STATES[sizeof(STATES) - 1];

        const unsigned char cpu = tasks[i].xCoreID == tskNO_AFFINITY ? '?' : '0' + tasks[i].xCoreID;
        if (stats_percentage > 0) {
            ESP_LOGI(TAG, " [%c:%c:%2d] %-16s    %11u    %2u%%    %5u", state, cpu, tasks[i].uxCurrentPriority,
                     tasks[i].pcTaskName, tasks[i].ulRunTimeCounter, stats_percentage,
                     tasks[i].usStackHighWaterMark);
        } else {
            // If the percentage is zero here then the task has consumed less than 1% of the total run time.
            ESP_LOGI(TAG, " [%c:%c:%2d] %-16s    %11u    <1%%    %5u", state, cpu, tasks[i].uxCurrentPriority,
                     tasks[i].pcTaskName, tasks[i].ulRunTimeCounter, tasks[i].usStackHighWaterMark);
        }
    }
    ESP_LOGI(TAG, "==========================================================");
    return ESP_OK;
}

static esp_err_t monitor_post_state(const TaskStatus_t *tasks, size_t size, uint32_t total_runtime_percentage) {
    ESP_ERROR_CHECK(state_push_tasks(tasks, size, total_runtime_percentage));
    return ESP_OK;
}

static void monitor_tasks_callback(cron_handle_t handle, const char *name, void *data) {
    ARG_UNUSED(handle);
    ARG_UNUSED(name);
    ARG_UNUSED(data);

    // Take a snapshot of the number of tasks in case it changes while this function is executing.
    size_t size = uxTaskGetNumberOfTasks();

    // Allocate a TaskStatus_t structure for each task. An array could be allocated statically at compile time.
    TaskStatus_t *tasks = calloc(size, sizeof(TaskStatus_t));

    if (tasks == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for the monitor");
        ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    }

    // Generate raw status information about each task.
    uint32_t total_runtime_percentage = 0;
    size = uxTaskGetSystemState(tasks, size, &total_runtime_percentage);

    // For percentage calculations.
    total_runtime_percentage /= 100UL;

    // Avoid divide by zero errors.
    if (total_runtime_percentage > 0) {
        // Sort the TaskStatus_t array by the task name.
        qsort(tasks, size, sizeof(TaskStatus_t), by_task_name);
        ESP_ERROR_CHECK(monitor_dump_stdout(tasks, size, total_runtime_percentage));
        // FIXME: disabled ESP_ERROR_CHECK(monitor_post_state(tasks, size, total_runtime_percentage));
    }
    SAFE_FREE(tasks);
}

static void monitor_memory_callback(cron_handle_t handle, const char *name, void *data) {
    ARG_UNUSED(handle);
    ARG_UNUSED(name);
    ARG_UNUSED(data);

    uint32_t min_free = esp_get_minimum_free_heap_size();
    uint32_t free = esp_get_free_heap_size();
    ESP_LOGI(TAG, "Minimum free heap: %d    free heap: %d", min_free, free);

    // FIXME: ESP_ERROR_CHECK(state_push_memory(min_free, free));
}

static void monitor_wifi_callback(cron_handle_t handle, const char *name, void *data) {
    ARG_UNUSED(handle);
    ARG_UNUSED(name);
    ARG_UNUSED(data);

    wifi_ap_record_t record = {0};
    if (esp_wifi_sta_get_ap_info(&record) != ESP_OK) {
        return;
    }
    ESP_LOGI(TAG, "Wifi rssi: %d", record.rssi);
}

esp_err_t monitor_init(context_t *context) {
    ARG_UNUSED(context);
    ESP_ERROR_CHECK(cron_create("monitor_wifi", MONITOR_CRON_WIFI, monitor_wifi_callback, NULL, NULL));
    ESP_ERROR_CHECK(cron_create("monitor_memory", MONITOR_CRON_MEMORY, monitor_memory_callback, NULL, NULL));
    ESP_ERROR_CHECK(cron_create("monitor_tasks", MONITOR_CRON_TASKS, monitor_tasks_callback, NULL, NULL));
    return ESP_OK;
}
