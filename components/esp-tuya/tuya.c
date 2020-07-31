#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "esp_err.h"

#include "buffer.h"
#include "error.h"
#include "tuya.h"
#include "utils.h"

static const char *TAG = "tuya";

#define PACKET_LOG ESP_LOG_DEBUG

static const uint32_t PACKET_HEADER = 0x000055aa;
static const uint32_t PACKET_FOOTER = 0x0000aa55;

static const size_t HEADERS_LEN = 4 + 4 + 4 + 4;
static const size_t CRC_FOOTER_LEN = 4 + 4;
static const size_t PACKET_MIN_LEN = HEADERS_LEN + CRC_FOOTER_LEN;
static const size_t VERSION_HEADER_LEN = 3 + 4 + 4 + 4; // 3.3 + three uint32_t

static const uint8_t UDP_KEY[AES128_BLOCK_SIZE] = "\x6c\x1e\xc8\xe2\xbb\x9b\xb5\x9a\xb5\x0b\x0d\xaf\x64\x9b\x41\x0a";
static const size_t SOCKET_BUF_LEN = 512;
static const uint16_t PORT_UDP = 6667;
static const uint16_t PORT_TCP = 6668;

static const uint32_t CRC_TABLE[] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
        0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
        0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
        0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
        0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
        0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
        0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
        0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
        0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
        0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
        0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
        0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
        0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
        0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
        0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
        0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
        0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
        0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
        0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
        0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
        0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
        0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static esp_err_t tuya_crc32(const uint8_t *buf, size_t len) {
    ARG_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_CHECK(len > 0, ERR_PARAM_LE_ZERO);
    uint32_t crc = 0xffffffff;
    for (int i = 0; i < len; ++i) {
        ESP_LOGV(TAG, "[%03d] 0x%02x -> 0x%08x", i, buf[i], crc);
        crc = (crc >> 8) ^ CRC_TABLE[(crc ^ buf[i]) & 0xff];
    }
    return crc ^ 0xffffffff;
}

static esp_err_t tuya_decrypt_payload(const tuya_connection_t *conn, const uint8_t *buf, size_t len, tuya_msg_t *msg) {
    ARG_CHECK(conn != NULL, ERR_PARAM_NULL);
    ARG_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_CHECK(msg != NULL, ERR_PARAM_NULL);

    msg->payload.data = NULL;
    msg->payload.len = 0;
    msg->payload.allocated = false;

    if (len == 0) {
        // This is a short message without any payload.
        return ESP_OK;
    }
    ssize_t to_decode_len = len;
    const uint8_t *start = buf;

    ESP_ERROR_CHECK(buffer_peek32_b(buf, &msg->ret_code));
    if ((msg->ret_code & 0xffffff00) == 0) {
        msg->has_ret_code = true;
        to_decode_len -= 4;
        start += 4;
    } else {
        msg->has_ret_code = false;
        msg->ret_code = 0;
    }
    if (to_decode_len <= 0) {
        // This is a short message without any extra payload.
        return ESP_OK;
    }
    if (strncmp((const char *) start, "3.3", 3) == 0) {
        start += VERSION_HEADER_LEN;
        to_decode_len -= VERSION_HEADER_LEN;
    }
    if (to_decode_len <= 0) {
        // This is a short message (besides the 3.3 header) without any extra payload.
        return ESP_OK;
    }

    msg->payload.len = to_decode_len;
    msg->payload.data = calloc(1, to_decode_len);
    msg->payload.allocated = true;

    const uint8_t *keys[16] = {conn->key, UDP_KEY};
    for (int i = 0; i < 2; ++i) {
        ESP_ERROR_CHECK(aes128_decrypt(start, to_decode_len, msg->payload.data, to_decode_len, keys[i]));
        if (msg->payload.data[0] != '{') {
            ESP_LOGD(TAG, "Failed to decrypt [%d/2]", i + 1);
            continue;
        }
        ESP_ERROR_CHECK(pkcs_7_strip_padding(msg->payload.data, &msg->payload.len));
        return ESP_OK;
    }
    return ESP_FAIL;
}

static esp_err_t tuya_unpack(const tuya_connection_t *conn, const uint8_t *buf, size_t len, tuya_msg_t *msg) {
    ARG_CHECK(conn != NULL, ERR_PARAM_NULL);
    ARG_CHECK(buf != NULL, ERR_PARAM_NULL);
    ARG_CHECK(len >= PACKET_MIN_LEN, "buf must be at least 24 bytes");
    ARG_CHECK(msg != NULL, ERR_PARAM_NULL);

    uint32_t header = 0;
    ESP_ERROR_CHECK(buffer_peek32_b(buf, &header));
    ARG_CHECK(header == PACKET_HEADER, "msg header != 0x%08x", PACKET_HEADER);

    ESP_ERROR_CHECK(buffer_peek32_b(buf + 4, &msg->sequence));
    ESP_ERROR_CHECK(buffer_peek32_b(buf + 8, &msg->command));

    size_t payload_len = 0;
    ESP_ERROR_CHECK(buffer_peek32_b(buf + 12, &payload_len));
    ARG_CHECK(payload_len >= CRC_FOOTER_LEN, "msg payload_len - %d < 0", CRC_FOOTER_LEN);
    payload_len -= CRC_FOOTER_LEN;

    uint32_t crc = 0;
    ESP_ERROR_CHECK(buffer_peek32_b(buf + HEADERS_LEN + payload_len, &crc));
    uint32_t expected_crc = tuya_crc32(buf, HEADERS_LEN + payload_len);
    ARG_CHECK(crc == expected_crc, "msg crc do not match. got 0x%08x, expected 0x%08x", crc, expected_crc);

    uint32_t footer = 0;
    ESP_ERROR_CHECK(buffer_peek32_b(buf + HEADERS_LEN + payload_len + 4, &footer));
    ARG_CHECK(footer == PACKET_FOOTER, "msg footer != 0x%08x", PACKET_FOOTER);

    ESP_ERROR_CHECK(tuya_decrypt_payload(conn, buf + HEADERS_LEN, payload_len, msg));

    return ESP_OK;
}

static esp_err_t tuya_encrypt_payload(const tuya_connection_t *conn, const uint8_t *payload, size_t payload_len,
                                      uint8_t *buf, size_t buf_len) {
    ARG_CHECK(conn != NULL, ERR_PARAM_NULL);
    ARG_CHECK(buf != NULL, ERR_PARAM_NULL);
    ESP_ERROR_CHECK(aes128_encrypt(payload, payload_len, buf, buf_len, conn->key));
    return ESP_OK;
}

static esp_err_t tuya_socket(const tuya_connection_t *conn, int *fd, struct sockaddr_in *addr) {
    ARG_CHECK(conn != NULL, ERR_PARAM_NULL);
    ARG_CHECK(conn->ip != NULL, ERR_PARAM_NULL);
    ARG_CHECK(fd != NULL, ERR_PARAM_NULL);
    ARG_CHECK(addr != NULL, ERR_PARAM_NULL);

    addr->sin_family = AF_INET;
    addr->sin_port = htons(conn->type == TUYA_CONNECTION_UDP ? PORT_UDP : PORT_TCP);
    int err = inet_aton(conn->ip, &addr->sin_addr.s_addr);
    ARG_CHECK(err != 0, "inet_aton(%s), error: %d", conn->ip, err);

    *fd = socket(addr->sin_family, conn->type, 0);
    FAIL_IF(*fd < 0, "socket, error: %d", *fd);

    if (conn->type == SOCK_DGRAM) {
        int val = 1;
        err = setsockopt(*fd, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val));
        FAIL_IF(err < 0, "setsockopt(SO_BROADCAST), error: %d", err);
    }
    if (conn->timeout.tv_sec > 0 || conn->timeout.tv_usec > 0) {
        err = setsockopt(*fd, SOL_SOCKET, SO_RCVTIMEO, &conn->timeout, sizeof(conn->timeout));
        FAIL_IF(err < 0, "setsockopt(SO_RCVTIMEO), error: %d", err);
        err = setsockopt(*fd, SOL_SOCKET, SO_SNDTIMEO, &conn->timeout, sizeof(conn->timeout));
        FAIL_IF(err < 0, "setsockopt(SO_SNDTIMEO), error: %d", err);
    }
    return ESP_OK;

    fail:
    if (*fd >= 0) {
        close(*fd);
    }
    return ESP_FAIL;
}

esp_err_t tuya_free(tuya_msg_t *msg) {
    ARG_CHECK(msg != NULL, ERR_PARAM_NULL);
    if (msg->payload.allocated && (msg->payload.len > 0 || msg->payload.data != NULL)) {
        SAFE_FREE(msg->payload.data);
    }
    memset(msg, 0, sizeof(tuya_msg_t));
    return ESP_OK;
}

esp_err_t tuya_pack(const tuya_connection_t *conn, uint32_t sequence, tuya_command_t command, const uint8_t *payload,
                    size_t payload_len, uint8_t *buf, size_t buf_len, size_t *written) {
    ARG_CHECK(conn != NULL, ERR_PARAM_NULL);
    ARG_CHECK(buf != NULL, ERR_PARAM_NULL);

    size_t extra_payload = 0;
    size_t padded_payload = round_up(payload_len, AES128_BLOCK_SIZE);

    size_t total_len = HEADERS_LEN; // header + sequence + command + payload_len
    if (command != TUYA_COMMAND_DP_QUERY) {
        extra_payload = VERSION_HEADER_LEN;
    }
    total_len += extra_payload;
    total_len += padded_payload;
    total_len += CRC_FOOTER_LEN; // crc + footer
    ARG_CHECK(buf_len >= total_len, "buf must be at least %d bytes", total_len);

    memset(buf, 0, total_len);

    uint32_t *p = (uint32_t *) buf;
    *p++ = htonl(PACKET_HEADER);
    *p++ = htonl(sequence);
    *p++ = htonl(command);
    *p++ = htonl(extra_payload + padded_payload + CRC_FOOTER_LEN);
    if (command != TUYA_COMMAND_DP_QUERY) {
        uint8_t *version = (uint8_t *) p;
        *version++ = '3';
        *version++ = '.';
        *version++ = '3';
        p = (uint32_t *) (version + 12);
    }
    ESP_ERROR_CHECK(tuya_encrypt_payload(conn, payload, payload_len, (uint8_t *) p, padded_payload));
    uint32_t crc = tuya_crc32(buf, HEADERS_LEN + extra_payload + padded_payload);
    ESP_LOGD(TAG, "Calculated crc: 0x%08x", crc);
    p = (uint32_t *) (buf + HEADERS_LEN + extra_payload + padded_payload);
    *p++ = htonl(crc);
    *p++ = htonl(PACKET_FOOTER);
    ARG_CHECK((uint8_t *) p == (buf + total_len), "Expected %p, Got == %p", buf + total_len, p);
    *written = total_len;
    return ESP_OK;
}

esp_err_t tuya_recv(const tuya_connection_t *conn, tuya_msg_t *rx) {
    ARG_CHECK(conn != NULL, ERR_PARAM_NULL);
    ARG_CHECK(conn->type == TUYA_CONNECTION_UDP, "Only TUYA_CONNECTION_UDP is supported");
    ARG_CHECK(rx != NULL, ERR_PARAM_NULL);

    int fd = -1;
    uint8_t *buf = calloc(1, SOCKET_BUF_LEN);
    FAIL_IF(buf == NULL, "tuya_recv, failed to allocate %d bytes", SOCKET_BUF_LEN);

    struct sockaddr_in addr = {0};
    esp_err_t e = tuya_socket(conn, &fd, &addr);
    FAIL_IF(e != ESP_OK, "tuya_socket, error: %d", e);

    int err = bind(fd, (struct sockaddr *) &addr, sizeof(addr));
    FAIL_IF(err < 0, "bind, error: -1");

    struct sockaddr_in from = {0};
    socklen_t client_len = 0;

    ESP_LOGD(TAG, "UDP %s:%d receiving broadcast message", conn->ip, PORT_UDP);
    ssize_t len = recvfrom(fd, buf, SOCKET_BUF_LEN, 0, (struct sockaddr *) &from, &client_len);
    switch (len) {
        case -1:
            ESP_LOGW(TAG, "recvfrom, error reading socket");
            goto fail;
        case 0:
            ESP_LOGW(TAG, "recvfrom, socket closed");
            goto fail;
        case SOCKET_BUF_LEN:
            ESP_LOGI(TAG, "recvfrom, too large. Only decoding the first message");
            // fall-through.
        default:
            ESP_LOG_BUFFER_HEXDUMP("tuya.recv", buf, len, PACKET_LOG);
            ESP_ERROR_CHECK(tuya_unpack(conn, buf, len, rx));
            close(fd);
            SAFE_FREE(buf);
            return ESP_OK;
    }
    fail:
    if (fd >= 0) {
        close(fd);
    }
    SAFE_FREE(buf);
    return ESP_FAIL;
}

esp_err_t tuya_send(const tuya_connection_t *conn, uint32_t sequence, tuya_command_t command, const uint8_t *payload,
                    size_t payload_len, tuya_msg_t *rx) {
    ARG_CHECK(conn != NULL, ERR_PARAM_NULL);
    ARG_CHECK(conn->type == TUYA_CONNECTION_TCP, "Only TUYA_CONNECTION_TCP is supported");
    ARG_CHECK(rx != NULL, ERR_PARAM_NULL);

    int fd = -1;
    tuya_msg_t tx = {0};
    uint8_t *buf = calloc(1, SOCKET_BUF_LEN);
    FAIL_IF(buf == NULL, "tuya_send, failed to allocate %d bytes", SOCKET_BUF_LEN);

    size_t to_write = 0;
    ESP_ERROR_CHECK(tuya_pack(conn, sequence, command, payload, payload_len, buf, SOCKET_BUF_LEN, &to_write));

    struct sockaddr_in addr = {0};
    esp_err_t e = tuya_socket(conn, &fd, &addr);
    FAIL_IF(e != ESP_OK, "tuya_socket, error: %d", e);

    int err = connect(fd, (struct sockaddr *) &addr, sizeof(addr));
    FAIL_IF(err != 0, "connect, error: %d", err);

    ESP_LOGD(TAG, "TCP %s:%d sending message", conn->ip, PORT_TCP);
    ESP_LOG_BUFFER_HEXDUMP("tuya.write", buf, to_write, PACKET_LOG);
    ssize_t written = write(fd, buf, to_write);
    FAIL_IF(to_write != written, "sendto, error: %d", written);

    memset(buf, 0, SOCKET_BUF_LEN);
    ESP_LOGD(TAG, "TCP %s:%d receiving message", conn->ip, PORT_TCP);
    ssize_t len = read(fd, buf, SOCKET_BUF_LEN);

    switch (len) {
        case -1:
            ESP_LOGW(TAG, "read, error reading socket");
            goto fail;
        case 0:
            ESP_LOGW(TAG, "read, socket closed");
            goto fail;
        case SOCKET_BUF_LEN:
            ESP_LOGI(TAG, "read, too large. Only decoding the first message");
            // fall-through.
        default:
            ESP_LOGD(TAG, "read %d bytes", len);
            ESP_LOG_BUFFER_HEXDUMP("tuya.read", buf, len, PACKET_LOG);
            ESP_ERROR_CHECK(tuya_unpack(conn, buf, len, rx));
            close(fd);
            ESP_ERROR_CHECK(tuya_free(&tx));
            SAFE_FREE(buf);
            return ESP_OK;
    }
    fail:
    if (fd >= 0) {
        close(fd);
    }
    ESP_ERROR_CHECK(tuya_free(&tx));
    SAFE_FREE(buf);
    return ESP_FAIL;
}

void tuya_dump(const tuya_msg_t *msg) {
    ESP_LOGI(TAG, "msg.sequence:     0x%08x", msg->sequence);
    ESP_LOGI(TAG, "msg.command:      0x%08x", msg->command);
    ESP_LOGI(TAG, "msg.payload.len:  0x%08x", msg->payload.len);
    ESP_LOGI(TAG, "msg.payload.mem:  %d", msg->payload.allocated);
    ESP_LOG_BUFFER_HEXDUMP(TAG, msg->payload.data, msg->payload.len, ESP_LOG_INFO);
    ESP_LOGI(TAG, "msg.has_ret_code: %d", msg->has_ret_code);
    ESP_LOGI(TAG, "msg.ret_code:     0x%08x", msg->ret_code);
}
