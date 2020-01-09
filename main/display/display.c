#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/pcnt.h"

#include "u8g2.h"

#include "buses.h"
#include "error.h"
#include "context.h"
#include "u8g2_esp32_hal.h"

#define I2C_ADDRESS_OLED 0x78  /*!< Slave address for OLED display. */

static const char *TAG = "display";
static u8g2_t u8g2;
static const EventBits_t display_bits = CONTEXT_EVENT_TEMP_INDOOR | CONTEXT_EVENT_TEMP_WATER | CONTEXT_EVENT_PRESSURE
                                        | CONTEXT_EVENT_HUMIDITY | CONTEXT_EVENT_EC;

static size_t snprintf_append(char *buf, size_t len, size_t max_size, float value) {
    if (CONTEXT_VALUE_IS_VALID(value)) {
        return snprintf(buf + len, max_size - len, " %.1f", value);
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

static esp_err_t display_draw(context_t *context) {
    ARG_CHECK(context != NULL, ERR_PARAM_NULL);

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    char buf[128] = {0};

    portENTER_CRITICAL(&context->spinlock);
    float indoor = context->sensors.temp.indoor;
    float water = context->sensors.temp.water;
    float humidity = context->sensors.humidity;
    float ec = context->sensors.ec.value;
    rotary_encoder_position_t rotary = context->inputs.rotary.state.position;
    portEXIT_CRITICAL(&context->spinlock);

    size_t len = strlcpy(buf, "Tmp:", sizeof(buf));
    len += snprintf_append(buf, len, sizeof(buf), indoor);
    len += snprintf(buf + len, sizeof(buf) - len, " |");
    len += snprintf_append(buf, len, sizeof(buf), water);
    snprintf(buf + len, sizeof(buf) - len, " \260C");
    u8g2_DrawStr(&u8g2, 0, 7, buf);

    snprintf_value(buf, sizeof(buf), "Hum: %.1f %%", "Hum: ?? %%", humidity);
    u8g2_DrawStr(&u8g2, 0, 15, buf);

    snprintf_value(buf, sizeof(buf), "EC: %.1f uS/cm", "EC: ?? uS/cm", ec);
    u8g2_DrawStr(&u8g2, 0, 23, buf);

    snprintf(buf, sizeof(buf), "Rot: %d", rotary);
    u8g2_DrawStr(&u8g2, 0, 31, buf);
    u8g2_SendBuffer(&u8g2);

    return ESP_OK;
}

static void display_task(void *arg) {
    context_t *context = (context_t *) arg;
    ARG_ERROR_CHECK(context != NULL, ERR_PARAM_NULL)

    // Let the reset settle.
    vTaskDelay(pdMS_TO_TICKS(100));

    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.sda = I2C_MASTER_SDA;
    u8g2_esp32_hal.scl = I2C_MASTER_SCL;
    u8g2_esp32_hal.reset = GPIO_NUM_NC;     // The reset is done by the lcd module.
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

    while (1) {
        xEventGroupWaitBits(context->event_group, display_bits, pdTRUE, pdFALSE, portMAX_DELAY);
        ESP_ERROR_CHECK(display_draw(context));
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

esp_err_t display_init(context_t *context) {
    xTaskCreatePinnedToCore(display_task, "display", 4096, context, 15, NULL, tskNO_AFFINITY);
    return ESP_OK;
}
