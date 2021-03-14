#include "esp_err.h"

#include "ucg.h"

#include "error.h"
#include "ext_display.h"
#include "lcd.h"
#include "screens/ext_main.h"
#include "lcd_ucg_hal.h"
#include "lcd_dev.h"

static const char *const TAG = "ext_display";
static ucg_t ucg = {0};

static void ext_display_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    ESP_ERROR_CHECK(lcd_init(&lcd_dev));
    ESP_ERROR_CHECK(lcd_ucg_hal_init(&lcd_dev, &ucg));
    ESP_ERROR_CHECK(ext_main_init(context, &lcd_dev, &ucg));

    while (true) {
        ESP_ERROR_CHECK(ext_main_draw(context, &lcd_dev, &ucg));
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

esp_err_t ext_display_init(context_t *context) {
    xTaskCreatePinnedToCore(ext_display_task, "ext_display", 4096, context, 15, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
