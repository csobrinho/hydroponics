#ifndef HYDROPONICS_CONTEXT_H
#define HYDROPONICS_CONTEXT_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"

#include "esp_bit_defs.h"

#include "config/config.pb-c.h"
#include "rotary_encoder.h"

#define CONTEXT_UNKNOWN_VALUE INT16_MIN
#define CONTEXT_VALUE_IS_VALID(x) ((x) != CONTEXT_UNKNOWN_VALUE)

typedef enum {
    CONTEXT_EVENT_TEMP_INDOOR = BIT0,    /*!< Updated indoor temperature from BME280 sensor. */
    CONTEXT_EVENT_TEMP_WATER = BIT1,     /*!< Updated water temperature from DS18B20 sensor. */
    CONTEXT_EVENT_TEMP_PROBE = BIT2,     /*!< Updated water temperature from EC probe. */
    CONTEXT_EVENT_HUMIDITY = BIT0,       /*!< Updated humidity value from BME280 sensor. */
    CONTEXT_EVENT_PRESSURE = BIT0,       /*!< Updated pressure value from BME280 sensor. */
    CONTEXT_EVENT_EC = BIT3,             /*!< Updated EC value or parameters. */
    CONTEXT_EVENT_PH = BIT4,             /*!< Updated PH value or parameters. */
    CONTEXT_EVENT_ROTARY = BIT5,         /*!< Updated rotary value or state. */
    CONTEXT_EVENT_PUMP_PH_UP = BIT6,     /*!< Updated PH up pump state. */
    CONTEXT_EVENT_PUMP_PH_DOWN = BIT7,   /*!< Updated PH down pump state. */
    CONTEXT_EVENT_PUMP_EC_A = BIT8,      /*!< Updated EC A nutrient solution pump state. */
    CONTEXT_EVENT_PUMP_EC_B = BIT9,      /*!< Updated EC B nutrient solution pump state. */
    CONTEXT_EVENT_PUMP_MAIN = BIT10,     /*!< Updated main pump state. */
    CONTEXT_EVENT_NETWORK = BIT11,       /*!< Updated network state. */
    CONTEXT_EVENT_TIME = BIT12,          /*!< Updated network time. */
    CONTEXT_EVENT_BASE_CONFIG = BIT13,   /*!< Updated base config. */
    CONTEXT_EVENT_CONFIG = BIT14,        /*!< Updated config. */
    CONTEXT_EVENT_IOT = BIT15,           /*!< Updated iot state. */
    CONTEXT_EVENT_STATE = BIT16,         /*!< Updated state state. */
    CONTEXT_EVENT_NETWORK_ERROR = BIT17, /*!< Updated network error state. */
} context_event_t;

typedef struct {
    portMUX_TYPE spinlock;
    EventGroupHandle_t event_group;

    struct {
        const char *device_id;
        const char *ssid;
        const char *password;
        const char *syslog_hostname;
        uint16_t syslog_port;
        const Hydroponics__Config *config;
        uint32_t config_version;
    } config;

    struct {
        struct {
            volatile float indoor;
            volatile float water;
            volatile float probe;
        } temp;
        volatile float humidity;
        volatile float pressure;
        struct {
            volatile float value;
            volatile float target_min;
            volatile float target_max;
        } ec;
        struct {
            volatile float value;
            volatile float target_min;
            volatile float target_max;
        } ph;
    } sensors;

    struct {
        struct {
            volatile rotary_encoder_state_t state;
            volatile bool pressed;
        } rotary;
    } inputs;

    struct {
        struct {
            struct {
            } ph_up;
            struct {
            } ph_down;
            struct {
            } ec_a;
            struct {
            } ec_b;
            struct {
            } main;
        } pumps;
    } outputs;

    struct {
    } network;

    struct {
        char *state_message;
    } status;
} context_t;

context_t *context_create(void);

void context_lock(context_t *context);

void context_unlock(context_t *context);

esp_err_t context_set_temp_indoor_humidity_pressure(context_t *context, float temp, float humidity, float pressure);

esp_err_t context_set_temp_water(context_t *context, float temp);

esp_err_t context_set_temp_probe(context_t *context, float temp);

esp_err_t context_set_ec(context_t *context, float value);

esp_err_t context_set_ec_target(context_t *context, float target_min, float target_max);

esp_err_t context_set_ph(context_t *context, float value);

esp_err_t context_set_ph_target(context_t *context, float target_min, float target_max);

esp_err_t context_set_rotary(context_t *context, rotary_encoder_state_t state);

esp_err_t context_set_rotary_pressed(context_t *context, bool pressed);

esp_err_t context_set_network_connected(context_t *context, bool connected);

esp_err_t context_set_network_error(context_t *context, bool error);

esp_err_t context_set_time_updated(context_t *context);

esp_err_t context_set_iot_connected(context_t *context, bool connected);

esp_err_t context_set_base_config(context_t *context, const char *device_id, const char *ssid, const char *password);

esp_err_t context_set_config(context_t *context, const Hydroponics__Config *config);

esp_err_t context_set_state_message(context_t *context, char *message);

esp_err_t context_get_state_message(context_t *context, char **message);

#endif //HYDROPONICS_CONTEXT_H
