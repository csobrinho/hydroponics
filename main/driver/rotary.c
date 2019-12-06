#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "driver/periph_ctrl.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"

#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"

#include "error.h"
#include "rotary.h"

static const char *TAG = "rotary";

pcnt_isr_handle_t user_isr_handle = NULL; // User's ISR service handle

/* Decode the PCNT's current value and pass it to the queue. */
static void IRAM_ATTR pcnt_intr_handler(void *arg) {
    const rotary_config_t *config = (const rotary_config_t *) arg;
    rotary_evt_t evt;
    uint32_t intr_status = PCNT.int_st.val;
    if (intr_status & (BIT(config->unit))) {
        pcnt_get_counter_value(config->unit, &evt.value);
        if (PCNT.status_unit[config->unit].thres1_lat) {
            evt.type = ROTARY_EVENT_TYPE_INC;
        } else if (PCNT.status_unit[config->unit].thres0_lat) {
            evt.type = ROTARY_EVENT_TYPE_DEC;
        } else {
            evt.type = ROTARY_EVENT_TYPE_UNKNOWN;
        }
        PCNT.int_clr.val = BIT(config->unit);

        portBASE_TYPE HPTaskAwoken = pdFALSE;
        xQueueSendFromISR(config->queue, &evt, &HPTaskAwoken);
        if (HPTaskAwoken == pdTRUE) {
            portYIELD_FROM_ISR();
        }
    }
}

/* Decode the PCNT's current value and pass it to the queue. */
static void IRAM_ATTR pcnt_intr_handler2(void *arg) {
    const rotary_config_t *config = (const rotary_config_t *) arg;
    uint32_t intr_status = PCNT.int_st.val;
    rotary_evt_t evt;
    portBASE_TYPE HPTaskAwoken = pdFALSE;

    for (int i = 0; i < PCNT_UNIT_MAX; i++) {
        if (intr_status & (BIT(i))) {
            evt.unit = i;
            /* Save the PCNT event type that caused an interrupt
               to pass it to the main program */
            evt.status = PCNT.status_unit[i].val;
            PCNT.int_clr.val = BIT(i);
            xQueueSendFromISR(config->queue, &evt, &HPTaskAwoken);
            if (HPTaskAwoken == pdTRUE) {
                portYIELD_FROM_ISR();
            }
        }
    }
}

esp_err_t rotary_init(const rotary_config_t *config) {
    ARG_CHECK(config != NULL, ERR_PARAM_NULL);
    ARG_CHECK(config->queue != NULL, ERR_PARAM_NULL);

    gpio_config_t dt = {
            .pin_bit_mask = BIT(config->dt),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&dt);
    gpio_config_t clk = {
            .pin_bit_mask = BIT(config->clk),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&clk);

    pcnt_config_t pcnt_config = {
            .pulse_gpio_num = config->dt,
            .ctrl_gpio_num = config->clk,
            .channel = PCNT_CHANNEL_0,
            .unit = config->unit,
            // What to do on the positive / negative edge of pulse input?
            .pos_mode = PCNT_COUNT_INC,      // Count up on the positive edge.
            .neg_mode = PCNT_COUNT_DIS,      // Keep the counter value on the negative edge.
            // What to do when control input is low or high?
            .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
            .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
            // Set the maximum and minimum limit values to watch.
            .counter_h_lim = INT16_MAX,
            .counter_l_lim = INT16_MIN,
    };
    ESP_ERROR_CHECK(pcnt_unit_config(&pcnt_config));
    ESP_ERROR_CHECK(pcnt_set_filter_value(pcnt_config.unit, 100));

    /* Set threshold 0 and 1 values and enable events to watch */
    ESP_ERROR_CHECK(pcnt_set_event_value(pcnt_config.unit, PCNT_EVT_THRES_1, 1));
    ESP_ERROR_CHECK(pcnt_event_enable(pcnt_config.unit, PCNT_EVT_THRES_1));
    ESP_ERROR_CHECK(pcnt_set_event_value(pcnt_config.unit, PCNT_EVT_THRES_0, -1));
    ESP_ERROR_CHECK(pcnt_event_enable(pcnt_config.unit, PCNT_EVT_THRES_0));

    /* Enable events on zero, maximum and minimum limit values */
    ESP_ERROR_CHECK(pcnt_event_enable(pcnt_config.unit, PCNT_EVT_ZERO));
    ESP_ERROR_CHECK(pcnt_event_enable(pcnt_config.unit, PCNT_EVT_H_LIM));
    ESP_ERROR_CHECK(pcnt_event_enable(pcnt_config.unit, PCNT_EVT_L_LIM));

    /* Initialize PCNT's counter */
    ESP_ERROR_CHECK(rotary_pause(config));
    ESP_ERROR_CHECK(rotary_clear(config));

    /* Register ISR handler and enable interrupts for PCNT unit */
    // ESP_ERROR_CHECK(pcnt_isr_service_install(0));
    // ESP_ERROR_CHECK(pcnt_isr_handler_add(pcnt_config.unit, pcnt_intr_handler2, (void *) pcnt_config.unit));
    ESP_ERROR_CHECK(pcnt_isr_register(pcnt_intr_handler2, (void *) config, 0, &user_isr_handle));
    ESP_ERROR_CHECK(pcnt_intr_enable(pcnt_config.unit));

    /* Everything is set up, now go to counting */
    ESP_ERROR_CHECK(rotary_resume(config));
    return ESP_OK;
}

esp_err_t rotary_pause(const rotary_config_t *config) {
    ARG_CHECK(config != NULL, ERR_PARAM_NULL);
    ESP_ERROR_CHECK(pcnt_counter_pause(config->unit));
    return ESP_OK;
}

esp_err_t rotary_clear(const rotary_config_t *config) {
    ESP_ERROR_CHECK(pcnt_counter_clear(config->unit));
    return ESP_OK;
}

esp_err_t rotary_resume(const rotary_config_t *config) {
    ESP_ERROR_CHECK(pcnt_counter_resume(config->unit));
    return ESP_OK;
}

esp_err_t rotary_value(const rotary_config_t *config, int16_t *count) {
    ESP_ERROR_CHECK(pcnt_get_counter_value(config->unit, count));
    return ESP_OK;
}