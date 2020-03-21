#include <stdlib.h>
#include <time.h>
#include "lwip/apps/sntp.h"

#include "esp_log.h"

#include "error.h"
#include "ntp.h"

static const char *TAG = "ntp";

static void ntp_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_NETWORK, pdFALSE, pdFALSE, portMAX_DELAY);

    ESP_LOGI(TAG, "Initializing SNTP...");
    // Set the timezone to Pacific time.
    setenv("TZ", "PST8PDT", 1);
    tzset();
    ESP_LOGI(TAG, "Set timezone to Pacific Time...");

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "time.google.com");
    sntp_init();

    // Wait for time to be set.
    time_t now = 0;
    struct tm t = {0};
    while (t.tm_year < (2019 - 1900)) {
        ESP_LOGI(TAG, "Waiting for system time to be set...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        time(&now);
        localtime_r(&now, &t);
    }
    char buf[64] = {0};
    strftime(buf, sizeof(buf), "%F %R", &t);
    ESP_LOGI(TAG, "Time set to %s...", buf);
    context_set_time_updated(context);

    // We are done with this task.
    vTaskDelete(NULL);
}

esp_err_t ntp_init(context_t *context) {
    xTaskCreatePinnedToCore(ntp_task, "ntp", 3072, context, 5, NULL, tskNO_AFFINITY);
    return ESP_OK;
}