#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "u8g2.h"

#include "buses.h"
#include "sdkconfig.h"
#include "context.h"
#include "driver/lcd/u8g2_esp32_hal.h"
#include "error.h"

#define I2C_ADDRESS_OLED 0x78  /*!< Slave address for OLED display. */

static const char *TAG = "display";
static const EventBits_t clear_bits = CONTEXT_EVENT_TEMP_INDOOR | CONTEXT_EVENT_TEMP_PROBE | CONTEXT_EVENT_PRESSURE
                                      | CONTEXT_EVENT_HUMIDITY | CONTEXT_EVENT_EC | CONTEXT_EVENT_PH;
static const EventBits_t wait_bits = clear_bits | CONTEXT_EVENT_NETWORK | CONTEXT_EVENT_TIME | CONTEXT_EVENT_IOT;
static u8g2_t u8g2;

static size_t snprintf_append(char *buf, size_t len, size_t max_size, const char *format, float value) {
    if (CONTEXT_VALUE_IS_VALID(value)) {
        return snprintf(buf + len, max_size - len, format, value);
    }
    return snprintf(buf + len, max_size - len, " ??");
}

static void snprintf_value(char *buf, size_t max_size, const char *format, const char *format_off, float value) {
    if (CONTEXT_VALUE_IS_VALID(value)) {
        snprintf(buf, max_size, format, value);
    } else {
        strncpy(buf, format_off, max_size);
    }
}

static esp_err_t display_draw(context_t *context, bool connected, bool time_updated, bool iot_connected) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    char buf[128] = {0};

    context_lock(context);
    float indoor = context->sensors.temp.indoor;
    float probe = context->sensors.temp.probe;
    float humidity = context->sensors.humidity;
    float eca = context->sensors.ec[CONFIG_TANK_A].value;
    float pha = context->sensors.ph[CONFIG_TANK_A].value;
    context_unlock(context);

    size_t len = strlcpy(buf, "Tmp:", sizeof(buf));
    len += snprintf_append(buf, len, sizeof(buf), " %.1f", indoor);
    len += snprintf(buf + len, sizeof(buf) - len, " |");
    len += snprintf_append(buf, len, sizeof(buf), " %.1f", probe);
    snprintf(buf + len, sizeof(buf) - len, " \260C");
    u8g2_DrawStr(&u8g2, 0, 7, buf);

    snprintf_value(buf, sizeof(buf), "Hum: %.f %%", "Hum: ?? %%", humidity);
    u8g2_DrawStr(&u8g2, 0, 15, buf);

    len = strlcpy(buf, "EC:", sizeof(buf));
    len += snprintf_append(buf, len, sizeof(buf), " %.f", eca);
    snprintf(buf + len, sizeof(buf) - len, " uS/cm");
    u8g2_DrawStr(&u8g2, 0, 23, buf);

    len = strlcpy(buf, "PH:", sizeof(buf));
    len += snprintf_append(buf, len, sizeof(buf), " %.2f", pha);
    u8g2_DrawStr(&u8g2, 0, 31, buf);

    snprintf(buf, sizeof(buf), "%c%c%c", connected ? 'W' : '*', time_updated ? 'T' : '*', iot_connected ? 'G' : '*');
    u8g2_DrawStr(&u8g2, u8g2_GetDisplayWidth(&u8g2) - (6 * 3), 31, buf);

    u8g2_SendBuffer(&u8g2);

    return ESP_OK;
}

static void display_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL);

    // Let the reset settle.
    vTaskDelay(pdMS_TO_TICKS(100));

    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.sda = I2C_MASTER_SDA;
    u8g2_esp32_hal.scl = I2C_MASTER_SCL;
    u8g2_esp32_hal.reset = U8G2_ESP32_HAL_UNDEFINED; // The reset is done by the buses module.
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_ssd1306_i2c_128x32_univision_f(
            &u8g2,
            U8G2_R0,
            u8g2_esp32_i2c_byte_cb,
            u8g2_esp32_gpio_and_delay_cb);  // Init u8g2 structure.
    u8x8_SetI2CAddress(&u8g2.u8x8, I2C_ADDRESS_OLED);

    u8g2_InitDisplay(&u8g2);                // Send init sequence to the display.
    u8g2_SetPowerSave(&u8g2, 0);            // Init left the display in sleep mode, so wake it up.
    u8g2_ClearBuffer(&u8g2);
    u8g2_SendBuffer(&u8g2);

    while (true) {
        EventBits_t bits = xEventGroupWaitBits(context->event_group, wait_bits, pdFALSE, pdFALSE, portMAX_DELAY);
        // Even though we wait for some status bits like network/time/etc, these are used elsewhere as a form of
        // synchronization between tasks so make sure we don't clear them!
        xEventGroupClearBits(context->event_group, clear_bits);
        bool connected = (bits & CONTEXT_EVENT_NETWORK) == CONTEXT_EVENT_NETWORK;
        bool time_updated = (bits & CONTEXT_EVENT_TIME) == CONTEXT_EVENT_TIME;
        bool iot_connected = (bits & CONTEXT_EVENT_IOT) == CONTEXT_EVENT_IOT;
        ESP_ERROR_CHECK(display_draw(context, connected, time_updated, iot_connected));
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

esp_err_t display_init(context_t *context) {
    xTaskCreatePinnedToCore(display_task, "display", 4096, context, 15, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
