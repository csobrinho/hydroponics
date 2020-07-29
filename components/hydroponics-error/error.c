#include <stdio.h>

#include "esp_log.h"

#include "error.h"

void arg_loge(const char *tag, const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    char *buf = NULL;
    vasprintf(&buf, fmt, va);
    if (buf != NULL) {
        ESP_LOGE(tag, "%s", buf);
        free(buf);
    }
    va_end(va);
}
