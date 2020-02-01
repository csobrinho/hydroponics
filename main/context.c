#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"

#include "context.h"
#include "error.h"

#define context_set(c, v, f)   \
    if ((c) != (v)) {          \
      (c) = (v);               \
      bitsToSet |= (f);        \
    }                          \


static const char *TAG = "context";

context_t *context_create(void) {
    context_t *context = calloc(1, sizeof(context_t));

    portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
    context->spinlock = spinlock;
    context->event_group = xEventGroupCreate();

    context->sensors.temp.indoor = CONTEXT_UNKNOWN_VALUE;
    context->sensors.temp.water = CONTEXT_UNKNOWN_VALUE;
    context->sensors.humidity = CONTEXT_UNKNOWN_VALUE;
    context->sensors.pressure = CONTEXT_UNKNOWN_VALUE;

    context->sensors.ec.value = CONTEXT_UNKNOWN_VALUE;
    context->sensors.ec.target_min = CONTEXT_UNKNOWN_VALUE;
    context->sensors.ec.target_max = CONTEXT_UNKNOWN_VALUE;

    context->sensors.ph.value = CONTEXT_UNKNOWN_VALUE;
    context->sensors.ph.target_min = CONTEXT_UNKNOWN_VALUE;
    context->sensors.ph.target_max = CONTEXT_UNKNOWN_VALUE;

    return context;
}

esp_err_t context_set_temp_indoor_humidity_pressure(context_t *context, float temp, float humidity, float pressure) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    portENTER_CRITICAL(&context->spinlock);
    context_set(context->sensors.temp.indoor, temp, CONTEXT_EVENT_TEMP_INDOOR)
    context_set(context->sensors.humidity, humidity, CONTEXT_EVENT_HUMIDITY)
    context_set(context->sensors.pressure, pressure, CONTEXT_EVENT_PRESSURE)
    portEXIT_CRITICAL(&context->spinlock);

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_temp_water(context_t *context, float temp) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    context_set(context->sensors.temp.water, temp, CONTEXT_EVENT_TEMP_WATER)

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_temp_probe(context_t *context, float temp) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    context_set(context->sensors.temp.probe, temp, CONTEXT_EVENT_TEMP_PROBE)

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_ec(context_t *context, float value) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    context_set(context->sensors.ec.value, value, CONTEXT_EVENT_EC)

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_ec_target(context_t *context, float target_min, float target_max) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    portENTER_CRITICAL(&context->spinlock);
    context_set(context->sensors.ec.target_min, target_min, CONTEXT_EVENT_EC)
    context_set(context->sensors.ec.target_max, target_max, CONTEXT_EVENT_EC)
    portEXIT_CRITICAL(&context->spinlock);

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_ph(context_t *context, float value) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    context_set(context->sensors.ph.value, value, CONTEXT_EVENT_PH)

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_ph_target(context_t *context, float target_min, float target_max) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    portENTER_CRITICAL(&context->spinlock);
    context_set(context->sensors.ph.target_min, target_min, CONTEXT_EVENT_PH)
    context_set(context->sensors.ph.target_max, target_max, CONTEXT_EVENT_PH)
    portEXIT_CRITICAL(&context->spinlock);

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_rotary(context_t *context, rotary_encoder_state_t state) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    portENTER_CRITICAL(&context->spinlock);
    context_set(context->inputs.rotary.state.position, state.position, CONTEXT_EVENT_ROTARY)
    context_set(context->inputs.rotary.state.direction, state.direction, CONTEXT_EVENT_ROTARY)
    portEXIT_CRITICAL(&context->spinlock);

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_rotary_pressed(context_t *context, bool pressed) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    context_set(context->inputs.rotary.pressed, pressed, CONTEXT_EVENT_ROTARY)

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_network_connected(context_t *context, bool connected) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    context_set(context->network.connected, connected, CONTEXT_EVENT_NETWORK)

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_time_updated(context_t *context) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    context_set(context->network.time_updated, true, CONTEXT_EVENT_TIME)

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_config(context_t *context, const char *device_id, const char *ssid, const char *password) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    portENTER_CRITICAL(&context->spinlock);
    context_set(context->config.device_id, device_id, CONTEXT_EVENT_CONFIG)
    context_set(context->config.ssid, ssid, CONTEXT_EVENT_CONFIG)
    context_set(context->config.password, password, CONTEXT_EVENT_CONFIG)
    portEXIT_CRITICAL(&context->spinlock);

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}
