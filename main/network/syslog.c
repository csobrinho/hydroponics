#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_err.h"

#include "context.h"
#include "error.h"
#include "syslog.h"

#define SYSLOG_BUF_MAX_SIZE    (4 * 1024)
#define SYSLOG_SOCKET_MAX_SIZE 1024
#define SYSLOG_WAIT_TIME_MS    2000

static const char *TAG = "syslog";
static SemaphoreHandle_t lock;
static FILE *stream;
static char *stream_buf = NULL;
static size_t stream_len = 0;

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

static void syslog_remove_start(size_t to_remove) {
    off_t eob = ftello(stream);
    fseek(stream, -to_remove, SEEK_CUR);
    memmove(stream_buf, &stream_buf[to_remove], eob - to_remove);
    fflush(stream);
}

static int syslog_printf(const char *fmt, va_list va) {
    time_t now = 0;
    time(&now);

    TickType_t start = xTaskGetTickCount();
    xSemaphoreTake(lock, portMAX_DELAY);
    TickType_t end = xTaskGetTickCount();
    if (pdTICKS_TO_MS(end - start) > 50) {
        LOCAL_LOG(W, "BLOCKED for %d ms", pdTICKS_TO_MS(end - start));
    }

    off_t eob = ftello(stream);
    size_t written = vfprintf(stream, fmt, va);
    fflush(stream);
    printf("%.*s", written, &stream_buf[eob]);
    eob = ftello(stream);
    if (eob > SYSLOG_BUF_MAX_SIZE) {
        syslog_remove_start(eob - SYSLOG_BUF_MAX_SIZE);
    }
    xSemaphoreGive(lock);
    return written;
}

static void syslog_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    // Wait until the base config is loaded to get the syslog hostname and port.
    xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_BASE_CONFIG, pdFALSE, pdTRUE, portMAX_DELAY);

    while (true) {
        // Wait for the network to be up.
        xEventGroupWaitBits(context->event_group, CONTEXT_EVENT_NETWORK, pdFALSE, pdTRUE, portMAX_DELAY);

        TickType_t last_wake_time = xTaskGetTickCount();

        // If needed, connect to the socket,
        ESP_ERROR_CHECK(syslog_connect());

        while (true) {
            // Lock and fetch the current stream bytes.
            xSemaphoreTake(lock, portMAX_DELAY);
            fflush(stream);
            off_t eob = ftello(stream);
            if (eob <= 0) {
                // Nothing in the stream, try again in a few seconds.
                xSemaphoreGive(lock);
                break;
            }
            size_t to_send = eob > SYSLOG_SOCKET_MAX_SIZE ? SYSLOG_SOCKET_MAX_SIZE : eob;
            esp_err_t err = syslog_send(stream_buf, to_send);
            if (err != ESP_OK) {
                // Disconnect and try again in a few seconds.
                syslog_disconnect();
                xSemaphoreGive(lock);
                break;
            }
            if (eob <= SYSLOG_SOCKET_MAX_SIZE) {
                // Send was successful and the whole buffer should be flushed, so rewind the stream to 0.
                fseeko(stream, 0, SEEK_SET);
                fflush(stream);
            } else {
                // Truncate the start.
                syslog_remove_start(SYSLOG_SOCKET_MAX_SIZE);
            }
            xSemaphoreGive(lock);
        }
        // Try again in a few seconds.
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SYSLOG_WAIT_TIME_MS));
    }
}

esp_err_t syslog_init(context_t *context) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    lock = xSemaphoreCreateMutex();
    CHECK_NO_MEM(lock);

    stream = open_memstream(&stream_buf, &stream_len);
    CHECK_NO_MEM(stream);

    // Setup the new remote logging.
    esp_log_set_vprintf(syslog_printf);

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(context->config.syslog_port);
    dest_addr.sin_addr.s_addr = inet_addr(context->config.syslog_hostname);

    xTaskCreatePinnedToCore(syslog_task, "syslog", 2048, context, 5, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
