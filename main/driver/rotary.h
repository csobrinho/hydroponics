#ifndef HYDROPONICS_DRIVER_ROTARY_H
#define HYDROPONICS_DRIVER_ROTARY_H

#include "esp_err.h"

typedef struct {
    const gpio_num_t dt;            /*!< Data gpio pin. */
    const gpio_num_t clk;           /*!< Clock gpio pin. */
    const gpio_num_t sw;         /*!< Switch gpio pin. */
    const pcnt_unit_t unit;         /*!< Pulse Counter unit number. */
    xQueueHandle queue;             /*!< Event queue that will receive the rotary values. */
} rotary_config_t;

typedef enum {
    ROTARY_EVENT_TYPE_UNKNOWN = 0,  /*!< Rotary type unknown. */
    ROTARY_EVENT_TYPE_INC,          /*!< Rotary increased. */
    ROTARY_EVENT_TYPE_DEC,          /*!< Rotary decreased. */
    ROTARY_EVENT_TYPE_SWITCH,       /*!< Rotary switch pressed. */
} rotary_evt_type_t;

typedef struct {
    pcnt_unit_t unit;
    uint32_t status;

    rotary_evt_type_t type;
    int16_t value;
} rotary_evt_t;

esp_err_t rotary_init(const rotary_config_t *config);

esp_err_t rotary_pause(const rotary_config_t *config);

esp_err_t rotary_clear(const rotary_config_t *config);

esp_err_t rotary_resume(const rotary_config_t *config);

esp_err_t rotary_value(const rotary_config_t *config, int16_t *count);

#endif //HYDROPONICS_DRIVER_ROTARY_H
