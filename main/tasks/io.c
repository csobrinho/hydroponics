#include "freertos/FreeRTOS.h"

#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "io.h"

static const char *TAG = "io";

static void io_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);
}

esp_err_t io_init(context_t *context) {
    xTaskCreatePinnedToCore(io_task, "io", 2048, context, tskIDLE_PRIORITY + XXX, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
