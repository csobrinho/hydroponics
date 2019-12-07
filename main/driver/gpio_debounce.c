#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"

#include "esp_freertos_hooks.h"
#include "esp_log.h"

#include "error.h"
#include "gpio_debounce.h"

#define ALL_OFF        0b00000000u
#define ALL_ON         0b11111111u
#define MASK           0b11000111u
#define STATE_PRESSED  0b00000111u
#define STATE_RELEASED 0b11000000u

#define DEBOUNCE_MAX 5
#define DEBOUNCE_CPU_ID APP_CPU_NUM
#define DEBOUNCE_EVENT_QUEUE_LENGTH 1
// #define DEBOUNCE_DEBUG

typedef struct {
    gpio_num_t gpio;
    uint8_t history;
    QueueHandle_t queue;
} gpio_debounce_gpio_t;

typedef struct {
    TickType_t debounce_ticks;
    bool registered;                           /*!< If true, then `tick_hook` is already registered. */
    gpio_debounce_gpio_t *gpios[DEBOUNCE_MAX]; /*!< List of registered gpio pins. */
    TickType_t next;
} gpio_debounce_t;

static const char *TAG = "gpio_debounce";
static portMUX_TYPE config_spinlock = portMUX_INITIALIZER_UNLOCKED;
static gpio_debounce_t config = {0};

static void IRAM_ATTR tick_hook(void) {
    TickType_t now = xTaskGetTickCountFromISR();
    if (now < config.next) {
        return;
    }
    config.next = now + config.debounce_ticks;
    gpio_debounce_evt_t evt;
    for (int i = 0; i < DEBOUNCE_MAX; i++) {
        if (config.gpios[i] == NULL) {
            continue;
        }
        gpio_debounce_gpio_t cfg = *config.gpios[i];
        cfg.history <<= 1U;
        cfg.history |= (uint8_t) gpio_get_level(cfg.gpio);
#ifdef DEBOUNCE_DEBUG
        ESP_EARLY_LOGI(TAG, "%d -> 0x02x", cfg.gpio, cfg.history);
#endif
        gpio_debounce_type_t type = GPIO_DEBOUNCE_TYPE_UNKNOWN;
        // Is button pressed?
        if ((cfg.history & MASK) == STATE_PRESSED) {
            type = GPIO_DEBOUNCE_TYPE_ON;
            cfg.history = ALL_ON;
#ifdef DEBOUNCE_DEBUG
            ESP_EARLY_LOGI(TAG, "%d -> ON", cfg.gpio);
#endif
        } else if ((cfg.history & MASK) == STATE_RELEASED) {
            type = GPIO_DEBOUNCE_TYPE_OFF;
            cfg.history = ALL_OFF;
#ifdef DEBOUNCE_DEBUG
            ESP_EARLY_LOGI(TAG, "%d -> OFF", cfg.gpio);
#endif
        }
        if (type != GPIO_DEBOUNCE_TYPE_UNKNOWN) {
            evt.gpio = cfg.gpio;
            evt.type = type;
            xQueueOverwriteFromISR(cfg.queue, &evt, NULL);
        }
    }
}

static void gpio_debounce_register_tick_hook(void) {
    if (!config.registered) {
        config.next = xTaskGetTickCount() + config.debounce_ticks;
        esp_register_freertos_tick_hook_for_cpu(tick_hook, DEBOUNCE_CPU_ID);
        config.registered = true;
    }
}

static void gpio_debounce_deregister_tick_hook(void) {
    if (config.registered) {
        esp_deregister_freertos_tick_hook_for_cpu(tick_hook, DEBOUNCE_CPU_ID);
        config.registered = false;
    }
}

esp_err_t gpio_debounce_init(TickType_t debounce_ticks) {
    ARG_CHECK(debounce_ticks > 0, "debounce_ticks <= 0");
    config.debounce_ticks = debounce_ticks;
    return ESP_OK;
}

esp_err_t gpio_debounce_register(gpio_num_t gpio, QueueHandle_t queue) {
    ARG_CHECK(GPIO_IS_VALID_GPIO(gpio), "gpio is invalid");
    ARG_CHECK(queue != NULL, ERR_PARAM_NULL);

    portENTER_CRITICAL(&config_spinlock);

    for (int i = 0; i < DEBOUNCE_MAX; i++) {
        if (config.gpios[i] != NULL) {
            continue;
        }
        config.gpios[i] = calloc(1, sizeof(gpio_debounce_gpio_t));
        config.gpios[i]->gpio = gpio;
        config.gpios[i]->queue = queue;
        config.gpios[i]->history = gpio_get_level(gpio) ? ALL_ON : ALL_OFF;

        gpio_pad_select_gpio(gpio);
        gpio_set_pull_mode(gpio, GPIO_PULLUP_ONLY);
        gpio_set_direction(gpio, GPIO_MODE_INPUT);
        gpio_set_intr_type(gpio, GPIO_INTR_DISABLE);

        gpio_debounce_register_tick_hook();
        portEXIT_CRITICAL(&config_spinlock);
        return ESP_OK;
    }

    portEXIT_CRITICAL(&config_spinlock);
    return ESP_ERR_NO_MEM;
}

esp_err_t gpio_debounce_get(gpio_num_t gpio, gpio_debounce_type_t *value) {
    ARG_CHECK(GPIO_IS_VALID_GPIO(gpio), "gpio is invalid");
    ARG_CHECK(value != NULL, ERR_PARAM_NULL);
    if (!config.registered) {
        return GPIO_DEBOUNCE_TYPE_UNKNOWN;
    }

    for (int i = 0; i < DEBOUNCE_MAX; i++) {
        if (config.gpios[i] == NULL || config.gpios[i]->gpio != gpio) {
            continue;
        }
        gpio_debounce_gpio_t cfg = *config.gpios[i];
        if (cfg.history == ALL_ON) {
            *value = GPIO_DEBOUNCE_TYPE_ON;
        } else if (cfg.history == ALL_OFF) {
            *value = GPIO_DEBOUNCE_TYPE_OFF;
        } else {
            *value = GPIO_DEBOUNCE_TYPE_UNKNOWN;
        }
        return ESP_OK;
    }
    return ESP_ERR_NOT_FOUND;
}

QueueHandle_t gpio_debounce_create_queue(void) {
    return xQueueCreate(DEBOUNCE_EVENT_QUEUE_LENGTH, sizeof(gpio_debounce_evt_t));
}

esp_err_t gpio_debounce_deregister(gpio_num_t gpio) {
    ARG_CHECK(GPIO_IS_VALID_GPIO(gpio), "gpio is invalid");
    if (!config.registered) {
        return ESP_OK;
    }
    portENTER_CRITICAL(&config_spinlock);

    for (int i = 0; i < DEBOUNCE_MAX; i++) {
        if (config.gpios[i] == NULL || config.gpios[i]->gpio != gpio) {
            continue;
        }
        free(config.gpios[i]);
        config.gpios[i] = NULL;

        portEXIT_CRITICAL(&config_spinlock);
        return ESP_OK;
    }
    gpio_debounce_deregister_tick_hook();
    portEXIT_CRITICAL(&config_spinlock);
    return ESP_OK;
}
