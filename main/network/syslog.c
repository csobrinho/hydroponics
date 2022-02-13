#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "freertos/semphr.h"

#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "syslog.h"
#include "utils.h"

#define SYSLOG_BUF_MAX_SIZE     (8 * 1024)
#define SYSLOG_SOCKET_MAX_SIZE  1400
#define SYSLOG_RETRY_TIME_TICKS (pdMS_TO_TICKS(3000))
#define SYSLOG_DONT_WAIT        0

static const char *const TAG = "syslog";
static RingbufHandle_t ring;

static struct sockaddr_in dest_addr = {0};
static int socket_fd = -1;

#define LOCAL_LOG(level, format, ...) do {                                          \
        printf(LOG_FORMAT(level, format), esp_log_timestamp(), TAG, ##__VA_ARGS__); \
    } while(0)

static esp_err_t syslog_disconnect() {
    if (socket_fd < 0) {
        return ESP_OK;
    }
    LOCAL_LOG(E, "Shutting down socket and restarting...");
    shutdown(socket_fd, 0);
    close(socket_fd);
    socket_fd = -1;
    return ESP_OK;
}

static esp_err_t syslog_connect() {
    if (socket_fd >= 0) {
        return ESP_OK;
    }
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (socket_fd >= 0) {
        const struct timeval send_timeout = {.tv_sec=2, .tv_usec=0};
        if (setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *) &send_timeout, sizeof(send_timeout)) < 0) {
            LOCAL_LOG(E, "Failed to set SO_SNDTIMEO: %s", strerror(errno));
        }
        int val = 1;
        if (setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val)) < 0) {
            LOCAL_LOG(E, "Failed to set SO_BROADCAST: %s", strerror(errno));
        }
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
            LOCAL_LOG(E, "Failed to set SO_REUSEADDR: %s", strerror(errno));
        }
        return ESP_OK;
    }
    LOCAL_LOG(E, "Unable to create socket: %s", strerror(errno));
    return ESP_ERR_INVALID_STATE;
}

static esp_err_t syslog_send(const char *buf, size_t len) {
    if (sendto(socket_fd, buf, len, 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr)) >= 0) {
        return ESP_OK;
    }
    LOCAL_LOG(E, "Error occurred during sending: %s", strerror(errno));
    return ESP_ERR_INVALID_STATE;
}

static int syslog_printf(const char *fmt, va_list va) {
    char *buffer = NULL;
    int written = vasprintf(&buffer, fmt, va);
    if (written == -1) {
        LOCAL_LOG(E, "Failed to log '%s'", fmt);
        goto fail;
    }
    if (written == 0) {
        goto fail;
    }
    if (written >= SYSLOG_BUF_MAX_SIZE) {
        LOCAL_LOG(E, "Failed to log '%s'", fmt);
        printf("%.*s", written, buffer);
        goto fail;
    }
    printf("%.*s", written, buffer);
    if (xRingbufferSend(ring, buffer, written, SYSLOG_DONT_WAIT) != pdTRUE) {
        LOCAL_LOG(E, "Failed append to log");
        goto fail;
    }
    fail:
    SAFE_FREE(buffer);
    return written;
}

static void syslog_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    // Wait until the base config is loaded to get the syslog hostname and port.
    xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_BASE_CONFIG, pdFALSE, pdTRUE, portMAX_DELAY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(context->config.syslog_port);
    dest_addr.sin_addr.s_addr = inet_addr(context->config.syslog_hostname);

    void *item = NULL;
    size_t to_send = 0;
    while (true) {
        // Wait for the network to be up.
        xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_NETWORK, pdFALSE, pdTRUE, portMAX_DELAY);

        TickType_t last_wake_time = xTaskGetTickCount();

        // If needed, connect to the socket,
        ESP_ERROR_CHECK(syslog_connect());

        while (true) {
            if (item == NULL) {
                item = xRingbufferReceiveUpTo(ring, &to_send, portMAX_DELAY, SYSLOG_SOCKET_MAX_SIZE);
            }
            if (item == NULL || to_send == 0) {
                // Nothing in the stream, try again in a few seconds.
                break;
            }
            esp_err_t err = syslog_send(item, to_send);
            if (err != ESP_OK) {
                // Disconnect and try again in a few seconds.
                syslog_disconnect();
                break;
            }
            vRingbufferReturnItem(ring, item);
            item = NULL;
            to_send = 0;
        }
        // Try again in a few seconds.
        vTaskDelayUntil(&last_wake_time, SYSLOG_RETRY_TIME_TICKS);
    }
}

esp_err_t syslog_init(context_t *context) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    ring = xRingbufferCreate(SYSLOG_BUF_MAX_SIZE, RINGBUF_TYPE_BYTEBUF);
    CHECK_NO_MEM(ring);

    // Setup the new remote logging.
    esp_log_set_vprintf(syslog_printf);

    xTaskCreatePinnedToCore(syslog_task, "syslog", 2048, context, 5, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
