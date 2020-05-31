#include "esp_log.h"

#include "error.h"

void arg_loge(const char *tag, const char *file, int line, const char *function, const char *str) {
    ESP_LOGE(tag, "%s:%d (%s):%s", file, line, function, str);
}
