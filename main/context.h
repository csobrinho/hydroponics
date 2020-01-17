#ifndef HYDROPONICS_CONTEXT_H
#define HYDROPONICS_CONTEXT_H

#include "freertos/event_groups.h"
#include "freertos/portmacro.h"

#include "esp_bit_defs.h"

#include "rotary_encoder.h"

#define CONTEXT_UNKNOWN_VALUE INT16_MIN
#define CONTEXT_VALUE_IS_VALID(x) (x != CONTEXT_UNKNOWN_VALUE)

typedef enum {
    CONTEXT_EVENT_TEMP_INDOOR = BIT0,    /*!< Updated indoor temperature from BME280 sensor. */
    CONTEXT_EVENT_TEMP_WATER = BIT1,     /*!< Updated water temperature from DS18B20 sensor. */
    CONTEXT_EVENT_HUMIDITY = BIT0,       /*!< Updated humidity value from BME280 sensor. */
    CONTEXT_EVENT_PRESSURE = BIT0,       /*!< Updated pressure value from BME280 sensor. */
    CONTEXT_EVENT_EC = BIT2,             /*!< Updated EC value or parameters. */
    CONTEXT_EVENT_PH = BIT3,             /*!< Updated PH value or parameters. */
    CONTEXT_EVENT_ROTARY = BIT4,         /*!< Updated rotary value or state. */
    CONTEXT_EVENT_PUMP_PH_UP = BIT5,     /*!< Updated PH up pump state. */
    CONTEXT_EVENT_PUMP_PH_DOWN = BIT6,   /*!< Updated PH down pump state. */
    CONTEXT_EVENT_PUMP_EC_A = BIT7,      /*!< Updated EC A nutrient solution pump state. */
    CONTEXT_EVENT_PUMP_EC_B = BIT8,      /*!< Updated EC B nutrient solution pump state. */
    CONTEXT_EVENT_PUMP_MAIN = BIT9,      /*!< Updated main pump state. */
    CONTEXT_EVENT_NETWORK = BIT10,       /*!< Updated network state. */
    CONTEXT_EVENT_TIME = BIT11,          /*!< Updated network time. */
    CONTEXT_EVENT_CONFIG = BIT12,        /*!< Updated config. */
} context_event_t;

typedef struct {
    portMUX_TYPE spinlock;
    EventGroupHandle_t event_group;

    struct {
        const char *device_id;
        const char *ssid;
        const char *password;
    } config;

    struct {
        struct {
            volatile float indoor;
            volatile float water;
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
        bool connected;
        bool time_updated;
    } network;
} context_t;

context_t *context_create(void);

esp_err_t context_set_temp_indoor_humidity_pressure(context_t *context, float temp, float humidity, float pressure);

esp_err_t context_set_temp_water(context_t *context, float temp);

esp_err_t context_set_ec(context_t *context, float value);

esp_err_t context_set_ec_target(context_t *context, float target_min, float target_max);

esp_err_t context_set_ph(context_t *context, float value);

esp_err_t context_set_ph_target(context_t *context, float target_min, float target_max);

esp_err_t context_set_rotary(context_t *context, rotary_encoder_state_t state);

esp_err_t context_set_rotary_pressed(context_t *context, bool pressed);

esp_err_t context_set_network_connected(context_t *context, bool connected);

esp_err_t context_set_time_updated(context_t *context);

esp_err_t context_set_config(context_t *context, const char *device_id, const char *ssid, const char *password);

#endif //HYDROPONICS_CONTEXT_H
