#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "syslog.h"

#define SYSLOG_NILVALUE "-"

#define SYSLOG_PRIMASK 0x07  /* mask to extract priority part (internal) */
/* extract priority */
#define SYSLOG_PRI(p)  ((p) & SYSLOG_PRIMASK)
#define SYSLOG_MAKEPRI(fac, pri) (((fac) << 3) | (pri))

#define SYSLOG_NFACILITIES 24  /* current number of facilities */
#define SYSLOG_FACMASK 0x03f8  /* mask to extract facility part */
/* facility of pri */
#define SYSLOG_FAC(p)  (((p) & SYSLOG_FACMASK) >> 3)

#define SYSLOG_MASK(pri) (1 << (pri))            /* mask for one priority */
#define SYSLOG_UPTO(pri) ((1 << ((pri)+1)) - 1)  /* all priorities through pri */

#define SYSLOG_QUEUE_SIZE 128

static const char *TAG = "syslog";
static QueueHandle_t queue;
static struct sockaddr_in dest_addr = {0};
static int socket_fd = INT32_MIN;

#define ESP_LOGDE(tag, format, ...) do {                                        \
        printf(LOG_FORMAT(E, format), esp_log_timestamp(), tag, ##__VA_ARGS__); \
    } while(0)

static esp_err_t syslog_connect() {
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (socket_fd != -1) {
        struct timeval send_timeout = {.tv_sec=1, .tv_usec=0};
        if (setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *) &send_timeout, sizeof(send_timeout)) < 0) {
            ESP_LOGDE(TAG, "Failed to set SO_SNDTIMEO: %s", strerror(errno));
        }
        return ESP_OK;
    }
    ESP_LOGDE(TAG, "Unable to create socket: %s", strerror(errno));
    return ESP_ERR_INVALID_STATE;
}

static esp_err_t syslog_send(syslog_entry_t *msg) {
    if (sendto(socket_fd, msg->msg, msg->msg_len, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr)) >= 0) {
        return ESP_OK;
    }
    ESP_LOGDE(TAG, "Error occurred during sending: %s", strerror(errno));
    return ESP_ERR_INVALID_STATE;
}

static inline void syslog_free(syslog_entry_t *msg) {
    if (msg != NULL && msg->msg != NULL) {
        free((void *) msg->msg);
        msg->msg = NULL;
    }
}

static int syslog_printf(const char *fmt, va_list va) {
    time_t now = 0;
    time(&now);
    char *buf;
    size_t len = vasprintf(&buf, fmt, va);
    const syslog_entry_t msg = {
            .priority = SYSLOG_PRIORITY_INFO,
            .timestamp = now,
            .msg = buf,
            .msg_len = len,
    };
    while (xQueueSend(queue, &msg, 0) != pdTRUE) {
        // If we fail, just pop one from the queue to get space.
        syslog_entry_t tmp = {0};
        if (xQueueReceive(queue, &tmp, 0) == pdTRUE) {
            syslog_free(&tmp);
        }
    }
    printf("%s", buf);
    return len;
}

static void syslog_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    syslog_entry_t msg;
    syslog_connect();
    while (true) {
        xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_CONFIG | CONTEXT_EVENT_NETWORK, pdFALSE, pdTRUE,
                            portMAX_DELAY);
        while (true) {
            if (xQueueReceive(queue, &msg, portMAX_DELAY) == pdTRUE) {
                esp_err_t err = syslog_send(&msg);
                syslog_free(&msg);
                if (err != ESP_OK) {
                    // Something went wrong so just wait a few seconds and wait for config/network to be ready again
                    vTaskDelay(pdMS_TO_TICKS(5000));
                    break;
                }
            }
        }
    }
}

esp_err_t syslog_init(context_t *context) {
    queue = xQueueCreate(SYSLOG_QUEUE_SIZE, sizeof(syslog_entry_t));
    if (queue == NULL) {
        return ESP_ERR_NO_MEM;
    }
    // Setup the new remote logging.
    esp_log_set_vprintf(syslog_printf);

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(context->config.syslog_port);
    dest_addr.sin_addr.s_addr = inet_addr(context->config.syslog_hostname);

    xTaskCreatePinnedToCore(syslog_task, "syslog", 4096, context, 5, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
