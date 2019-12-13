#ifndef HYDROPONICS_DRIVERS_EZO_H
#define HYDROPONICS_DRIVERS_EZO_H

typedef const struct {
    const char *cmd;
    const char *cmd_response;
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
#define EZO_DELAY_MS_SHORT 300
#define EZO_DELAY_MS_SLOW 600
#define EZO_DELAY_MS_SLOWEST 900

extern const ezo_cmd_t ezo_cmd_read;
extern const ezo_cmd_t ezo_cmd_read_temperature;
extern const ezo_cmd_t ezo_cmd_device_info;
extern const ezo_cmd_t ezo_cmd_status;
extern const ezo_cmd_t ezo_cmd_export;
extern const ezo_cmd_t ezo_cmd_calibration;
extern const ezo_cmd_t ezo_cmd_calibration_low;
extern const ezo_cmd_t ezo_cmd_calibration_high;
extern const ezo_cmd_t ezo_cmd_calibration_mid;

typedef enum {
    EZO_CALIBRATION_UNKNOWN = 0,
    EZO_CALIBRATION_LOW = BIT0,
    EZO_CALIBRATION_MID = BIT1,
    EZO_CALIBRATION_HIGH = BIT2,
} ezo_calibration_t;

typedef struct {
    const char *type;
    const char *probe;
    const uint8_t address;
    const uint16_t delay_ms;
    const uint16_t delay_read_ms;
    const uint16_t delay_calibration_ms;
    const ezo_calibration_t calibration;
    ezo_cmd_t *cmd_read;
    ezo_cmd_t *cmd_read_temperature;
    ezo_cmd_t *cmd_device_info;
    ezo_cmd_t *cmd_status;
    ezo_cmd_t *cmd_export;
    ezo_cmd_t *cmd_calibration;
    char *version;
    char buf[EZO_MAX_BUFFER_LEN];
    size_t bytes_read;
    ezo_sensor_response_t status;
} ezo_sensor_t;

esp_err_t ezo_init(ezo_sensor_t *sensor);

esp_err_t ezo_free(ezo_sensor_t *sensor);

esp_err_t ezo_send_command(ezo_sensor_t *sensor, const ezo_cmd_t *cmd, uint16_t delay_ms, const char *fmt, ...);

esp_err_t ezo_send_command_float(ezo_sensor_t *sensor, const ezo_cmd_t *cmd, uint16_t delay_ms, float value);

esp_err_t ezo_read_command(ezo_sensor_t *sensor, float *value);

esp_err_t ezo_read_temperature_command(ezo_sensor_t *sensor, float *value, float temp);

esp_err_t ezo_export_calibration(ezo_sensor_t *sensor, char **buffer, size_t *size);

esp_err_t ezo_calibration_status(ezo_sensor_t *sensor, uint8_t *value);

#endif //HYDROPONICS_DRIVERS_EZO_H
