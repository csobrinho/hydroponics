#ifndef ESP_TUYA_H
#define ESP_TUYA_H

#include <netdb.h>
#include <sys/time.h>

#include "esp_err.h"

#include "utils.h"

typedef enum {
    TUYA_COMMAND_CONTROL = 7,
    TUYA_COMMAND_STATUS = 8,
    TUYA_COMMAND_HEART_BEAT = 9,
    TUYA_COMMAND_DP_QUERY = 10,
    TUYA_COMMAND_UDP_NEW = 19,
} tuya_command_t;

typedef enum {
    TUYA_CONNECTION_UDP = SOCK_DGRAM,
    TUYA_CONNECTION_TCP = SOCK_STREAM,
} tuya_connection_type_t;

typedef const struct {
    tuya_connection_type_t type;
    char ip[16];
    uint8_t key[AES128_BLOCK_SIZE];
    struct timeval timeout;
} tuya_connection_t;

typedef struct {
    uint8_t *data;
    size_t len;
    bool allocated;
} tuya_payload_t;

typedef struct {
    uint32_t sequence;
    tuya_command_t command;
    tuya_payload_t payload;
    bool has_ret_code;
    uint32_t ret_code;
} tuya_msg_t;

esp_err_t tuya_recv(const tuya_connection_t *conn, tuya_msg_t *rx);

esp_err_t tuya_send(const tuya_connection_t *conn, uint32_t sequence, tuya_command_t command, const uint8_t *payload,
                    size_t payload_len, tuya_msg_t *rx);

esp_err_t tuya_free(tuya_msg_t *msg);

void tuya_dump(const tuya_msg_t *msg);

#endif //ESP_TUYA_H
