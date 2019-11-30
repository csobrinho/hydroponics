#ifndef HYDROPONICS_DRIVERS_EZO_H
#define HYDROPONICS_DRIVERS_EZO_H

typedef const struct {
    char *cmd;
    uint16_t delay_ms;
    bool has_read;
} ezo_cmd_t;

typedef enum {
    EZO_SENSOR_RESPONSE_UNKNOWN = 0,
    EZO_SENSOR_RESPONSE_SUCCESS = 1,
    EZO_SENSOR_RESPONSE_SYNTAX_ERROR = 2,
    EZO_SENSOR_RESPONSE_PROCESSING = 254,
    EZO_SENSOR_RESPONSE_NO_DATA = 255,
} ezo_sensor_response_t;

#define EZO_MAX_BUFFER_LEN 32
#define EZO_MAX_RETRIES 5

typedef struct {
    const char *type;
    const char *probe;
    const uint8_t address;
    const ezo_cmd_t cmd_device_info;
    const ezo_cmd_t cmd_read;
    char *version;
    char buf[EZO_MAX_BUFFER_LEN];
    size_t bytes_read;
    ezo_sensor_response_t status;
} ezo_sensor_t;

esp_err_t ezo_init(ezo_sensor_t *sensor);

esp_err_t ezo_free(ezo_sensor_t *sensor);

esp_err_t ezo_send_command(ezo_sensor_t *sensor, ezo_cmd_t cmd, const char *args);

esp_err_t ezo_read_command(ezo_sensor_t *sensor, float *value);

#endif //HYDROPONICS_DRIVERS_EZO_H
