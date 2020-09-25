#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "context.h"
#include "error.h"
#include "utils.h"

#define context_set(p, v, f) do {                    \
      if ((p) != (v)) {                              \
        (p) = (v);                                   \
        bitsToSet |= (f);                            \
      }                                              \
    } while (0)

#define context_set_single(c, p, v, f) do {          \
      if ((p) != (v)) {                              \
        (p) = (v);                                   \
        xEventGroupSetBits((c)->event_group, (f));   \
      }                                              \
    } while (0)

#define context_set_flags(c, v, f) do {              \
      if (v) {                                       \
        xEventGroupSetBits((c)->event_group, (f));   \
      } else {                                       \
        xEventGroupClearBits((c)->event_group, (f)); \
      }                                              \
    } while (0)

static const char *TAG = "context";

context_t *context_create(void) {
    context_t *context = calloc(1, sizeof(context_t));

    portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
    context->spinlock = spinlock;
    context->event_group = xEventGroupCreate();

    context->sensors.temp.indoor = CONTEXT_UNKNOWN_VALUE;
    context->sensors.temp.water = CONTEXT_UNKNOWN_VALUE;
    context->sensors.temp.probe = CONTEXT_UNKNOWN_VALUE;

    context->sensors.humidity = CONTEXT_UNKNOWN_VALUE;
    context->sensors.pressure = CONTEXT_UNKNOWN_VALUE;

    for (int tank = 0; tank < CONFIG_ESP_SENSOR_TANKS; ++tank) {
        context->sensors.ec[tank].value = CONTEXT_UNKNOWN_VALUE;
        context->sensors.ec[tank].target_min = CONTEXT_UNKNOWN_VALUE;
        context->sensors.ec[tank].target_max = CONTEXT_UNKNOWN_VALUE;

        context->sensors.ph[tank].value = CONTEXT_UNKNOWN_VALUE;
        context->sensors.ph[tank].target_min = CONTEXT_UNKNOWN_VALUE;
        context->sensors.ph[tank].target_max = CONTEXT_UNKNOWN_VALUE;

        context->sensors.tank[tank].value = CONTEXT_UNKNOWN_VALUE;
    }

    return context;
}

inline void context_lock(context_t *context) {
    portENTER_CRITICAL(&context->spinlock);
}

inline void context_unlock(context_t *context) {
    portEXIT_CRITICAL(&context->spinlock);
}

esp_err_t context_set_temp_indoor_humidity_pressure(context_t *context, float temp, float humidity, float pressure) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    context_lock(context);
    context_set(context->sensors.temp.indoor, temp, CONTEXT_EVENT_TEMP_INDOOR);
    context_set(context->sensors.humidity, humidity, CONTEXT_EVENT_HUMIDITY);
    context_set(context->sensors.pressure, pressure, CONTEXT_EVENT_PRESSURE);
    context_unlock(context);

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_temp_water(context_t *context, float temp) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    context_set_single(context, context->sensors.temp.water, temp, CONTEXT_EVENT_TEMP_WATER);
    return ESP_OK;
}

esp_err_t context_set_temp_probe(context_t *context, float temp) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    context_set_single(context, context->sensors.temp.probe, temp, CONTEXT_EVENT_TEMP_PROBE);
    return ESP_OK;
}

esp_err_t context_set_ec(context_t *context, int tank, float value) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    ARG_CHECK(tank < CONFIG_ESP_SENSOR_TANKS, "tank >= CONFIG_ESP_SENSOR_TANKS");
    context_set_single(context, context->sensors.ec[tank].value, value, CONTEXT_EVENT_EC);
    return ESP_OK;
}

esp_err_t context_set_ec_target(context_t *context, int tank, float target_min, float target_max) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    ARG_CHECK(tank < CONFIG_ESP_SENSOR_TANKS, "tank >= CONFIG_ESP_SENSOR_TANKS");

    EventBits_t bitsToSet = 0U;
    context_lock(context);
    context_set(context->sensors.ec[tank].target_min, target_min, CONTEXT_EVENT_EC);
    context_set(context->sensors.ec[tank].target_max, target_max, CONTEXT_EVENT_EC);
    context_unlock(context);

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_ph(context_t *context, int tank, float value) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    ARG_CHECK(tank < CONFIG_ESP_SENSOR_TANKS, "tank >= CONFIG_ESP_SENSOR_TANKS");
    context_set_single(context, context->sensors.ph[tank].value, value, CONTEXT_EVENT_PH);
    return ESP_OK;
}

esp_err_t context_set_ph_target(context_t *context, int tank, float target_min, float target_max) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    ARG_CHECK(tank < CONFIG_ESP_SENSOR_TANKS, "tank >= CONFIG_ESP_SENSOR_TANKS");

    EventBits_t bitsToSet = 0U;
    context_lock(context);
    context_set(context->sensors.ph[tank].target_min, target_min, CONTEXT_EVENT_PH);
    context_set(context->sensors.ph[tank].target_max, target_max, CONTEXT_EVENT_PH);
    context_unlock(context);

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_tank(context_t *context, int tank, float value) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    ARG_CHECK(tank < CONFIG_ESP_SENSOR_TANKS, "tank >= CONFIG_ESP_SENSOR_TANKS");
    context_set_single(context, context->sensors.tank[tank].value, value, CONTEXT_EVENT_TANK);
    return ESP_OK;
}

esp_err_t context_set_rotary(context_t *context, rotary_encoder_state_t state) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    context_lock(context);
    context_set(context->inputs.rotary.state.position, state.position, CONTEXT_EVENT_ROTARY);
    context_set(context->inputs.rotary.state.direction, state.direction, CONTEXT_EVENT_ROTARY);
    context_unlock(context);

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_rotary_pressed(context_t *context, bool pressed) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    context_set_single(context, context->inputs.rotary.pressed, pressed, CONTEXT_EVENT_ROTARY);
    return ESP_OK;
}

esp_err_t context_set_network_connected(context_t *context, bool connected) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    context_set_flags(context, connected, CONTEXT_EVENT_NETWORK);
    return ESP_OK;
}

esp_err_t context_set_network_error(context_t *context, bool error) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    ESP_LOGW(TAG, "context_set_network_error val: %s is disabled for now", error ? "true" : "false");
    // context_set_flags(context, error, CONTEXT_EVENT_NETWORK_ERROR);
    return ESP_OK;
}

esp_err_t context_set_time_updated(context_t *context) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    xEventGroupSetBits(context->event_group, CONTEXT_EVENT_TIME);
    return ESP_OK;
}

esp_err_t context_set_iot_connected(context_t *context, bool connected) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    context_set_flags(context, connected, CONTEXT_EVENT_IOT);
    return ESP_OK;
}

esp_err_t context_set_base_config(context_t *context, const char *device_id, const char *ssid, const char *password) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    EventBits_t bitsToSet = 0U;
    context_lock(context);
    context_set(context->config.device_id, device_id, CONTEXT_EVENT_BASE_CONFIG);
    context_set(context->config.ssid, ssid, CONTEXT_EVENT_BASE_CONFIG);
    context_set(context->config.password, password, CONTEXT_EVENT_BASE_CONFIG);
    context_set(context->config.syslog_hostname, CONFIG_ESP_SYSLOG_IPV4_ADDR, CONTEXT_EVENT_BASE_CONFIG);
    context_set(context->config.syslog_port, CONFIG_ESP_SYSLOG_PORT, CONTEXT_EVENT_BASE_CONFIG);
    context_unlock(context);

    if (bitsToSet) xEventGroupSetBits(context->event_group, bitsToSet);
    return ESP_OK;
}

esp_err_t context_set_config(context_t *context, const Hydroponics__Config *config) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    context_lock(context);
    if (context->config.config == config) {
        context_unlock(context);
        return ESP_OK;
    }
    if (context->config.config != NULL) {
        hydroponics__config__free_unpacked((Hydroponics__Config *) context->config.config, NULL);
        context->config.config = NULL;
    }
    context->config.config = config;
    context->config.config_version++;
    bool clear = context->config.config == NULL;
    context_unlock(context);

    if (clear) {
        xEventGroupClearBits(context->event_group, CONTEXT_EVENT_CONFIG);
    } else {
        xEventGroupSetBits(context->event_group, CONTEXT_EVENT_CONFIG);
    }
    return ESP_OK;
}

esp_err_t context_get_config(context_t *context, const Hydroponics__Config **config) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);
    ARG_CHECK(config != NULL, ERR_PARAM_NULL);

    context_lock(context);
    esp_err_t ret = ESP_OK;
    uint8_t *data = NULL;
    *config = NULL;
    if (context->config.config == NULL) {
        goto fail;
    }
    size_t size = hydroponics__config__get_packed_size(context->config.config);
    if (size <= 0) {
        goto fail;
    }
    data = malloc(size);
    FAIL_IF_NO_MEM(data);

    hydroponics__config__pack(context->config.config, data);
    *config = hydroponics__config__unpack(NULL, size, data);

    fail:
    SAFE_FREE(data);
    context_unlock(context);
    return ret;
}
