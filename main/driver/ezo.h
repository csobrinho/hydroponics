#ifndef HYDROPONICS_DRIVERS_EZO_H
#define HYDROPONICS_DRIVERS_EZO_H

#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define EZO_MAX_BUFFER_LEN 32
#define EZO_DELAY_MS_SHORT 300
#define EZO_DELAY_MS_SLOW 600
#define EZO_DELAY_MS_SLOWEST 900
#define EZO_MAX_RETRIES 4

typedef enum {
    EZO_SENSOR_RESPONSE_UNKNOWN = 0,
    EZO_SENSOR_RESPONSE_SUCCESS = 1,
    EZO_SENSOR_RESPONSE_SYNTAX_ERROR = 2,
    EZO_SENSOR_RESPONSE_PROCESSING = 254,
    EZO_SENSOR_RESPONSE_NO_DATA = 255,
} ezo_sensor_response_t;

typedef enum {
    EZO_CALIBRATION_STEP_DRY = 0,
    EZO_CALIBRATION_STEP_SINGLE = 1,
    EZO_CALIBRATION_STEP_LOW = 2,
    EZO_CALIBRATION_STEP_MID = 3,
    EZO_CALIBRATION_STEP_HIGH = 4,
    EZO_CALIBRATION_STEP_CLEAR = 5,
    EZO_CALIBRATION_STEP_MAX = 6,
} ezo_calibration_step_t;

typedef enum {
    EZO_CALIBRATION_MODE_NONE = 0,
    EZO_CALIBRATION_MODE_ONE_POINT = 1,
    EZO_CALIBRATION_MODE_TWO_POINTS = 2,
    EZO_CALIBRATION_MODE_THREE_POINTS = 3,
} ezo_calibration_mode_t;

typedef enum {
    EZO_STATUS_POWERED_OFF = 'P',
    EZO_STATUS_SOFTWARE_RESET = 'S',
    EZO_STATUS_BROWN_OUT = 'B',
    EZO_STATUS_WATCHDOG = 'W',
    EZO_STATUS_UNKNOWN = 'U',
} ezo_status_t;

typedef struct {
    const char *probe;
    const uint8_t address;
    const uint16_t delay_ms;
    const uint16_t delay_read_ms;
    const uint16_t delay_calibration_ms;
    const ezo_calibration_mode_t calibration;
    char type[8];
    char version[8];
    char buf[EZO_MAX_BUFFER_LEN];
    size_t bytes_read;
    ezo_sensor_response_t status;
    xSemaphoreHandle lock;
    bool pause;
#ifdef CONFIG_ESP_SIMULATE_SENSORS
    float simulate;
    float threshold;
#endif
} ezo_sensor_t;

esp_err_t ezo_init(ezo_sensor_t *sensor);

esp_err_t ezo_free(ezo_sensor_t *sensor);

ezo_sensor_t *ezo_find(const char *type);

esp_err_t ezo_send_command(ezo_sensor_t *sensor, uint16_t delay_ms, const char *cmd_fmt, ...)
__printflike(3, 4);

esp_err_t ezo_parse_response(ezo_sensor_t *sensor, uint8_t fields, const char *response_fmt, ...)
__scanflike(3, 4);

esp_err_t ezo_read(ezo_sensor_t *sensor, float *value);

esp_err_t ezo_read_temperature(ezo_sensor_t *sensor, float *value, float temp);

esp_err_t ezo_device_info(ezo_sensor_t *sensor);

esp_err_t ezo_status(ezo_sensor_t *sensor, ezo_status_t *status, float *voltage);

esp_err_t ezo_export_calibration(ezo_sensor_t *sensor, char **buffer, size_t *size);

esp_err_t ezo_calibration_mode(ezo_sensor_t *sensor, ezo_calibration_mode_t *mode);

esp_err_t ezo_calibration_step(ezo_sensor_t *sensor, ezo_calibration_step_t step, float value);

esp_err_t ezo_protocol_lock(ezo_sensor_t *sensor, bool lock);

#endif //HYDROPONICS_DRIVERS_EZO_H
