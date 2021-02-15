#include "esp_log.h"

#include "ucg.h"

#include "error.h"
#include "lcd.h"
#include "lcd_ucg_hal.h"
#include "utils.h"

#define HANDLE(d) ((lcd_dev_t*)(d->user_ptr))

static const char *const TAG = "lcd_ucg_hal";

static ucg_int_t lcd_ucg_handle_l90fx(ucg_t *ucg) {
    lcd_dev_t *dev = HANDLE(ucg);
    uint16_t color = lcd_rgb565(ucg->arg.pixel.rgb.color[0], ucg->arg.pixel.rgb.color[1], ucg->arg.pixel.rgb.color[2]);
    if (ucg_clip_l90fx(ucg) != 0) {
        switch (ucg->arg.dir) {
            case 1: // ->Down
                lcd_vline(dev, color, ucg->arg.pixel.pos.x, ucg->arg.pixel.pos.y, ucg->arg.len);
                ucg->arg.pixel.pos.y += ucg->arg.len;
                return 1;
            case 2: // Left<-
                lcd_hline(dev, color, ucg->arg.pixel.pos.x - ucg->arg.len + 1, ucg->arg.pixel.pos.y, ucg->arg.len);
                ucg->arg.pixel.pos.x -= ucg->arg.len;
                return 1;
            case 3: // Up<-
                lcd_vline(dev, color, ucg->arg.pixel.pos.x, ucg->arg.pixel.pos.y - ucg->arg.len + 1, ucg->arg.len);
                ucg->arg.pixel.pos.y -= ucg->arg.len;
                return 1;
            case 0: // ->RIGHT
            default:
                lcd_hline(dev, color, ucg->arg.pixel.pos.x, ucg->arg.pixel.pos.y, ucg->arg.len);
                ucg->arg.pixel.pos.x += ucg->arg.len;
                return 1;
        }
    }
    return 0;
}

static ucg_int_t lcd_ucg_device(ucg_t *ucg, ucg_int_t msg, void *data) {
    switch (msg) {
        case UCG_MSG_DEV_POWER_UP: {
            // "data" is a pointer to ucg_com_info_t structure but we don't use it.
            // "arg" is not used
            // This message is sent once at the uC startup and for power up.
            LLOG(TAG, "DEV: UCG_MSG_DEV_POWER_UP");
            return ucg_com_PowerUp(ucg, -1, -1);
        }
        case UCG_MSG_GET_DIMENSION: {
            LLOG(TAG, "DEV: UCG_MSG_GET_DIMENSION");
            lcd_dev_t *dev = HANDLE(ucg);
            ((ucg_wh_t *) data)->w = dev->config.screen.width;
            ((ucg_wh_t *) data)->h = dev->config.screen.height;
            return 1;
        }
        case UCG_MSG_DRAW_PIXEL: {
            if (ucg_clip_is_pixel_visible(ucg)) {
                lcd_dev_t *dev = HANDLE(ucg);
                uint16_t color = lcd_rgb565(ucg->arg.pixel.rgb.color[0], ucg->arg.pixel.rgb.color[1],
                                            ucg->arg.pixel.rgb.color[2]);
                lcd_draw_pixel(dev, color, ucg->arg.pixel.pos.x, ucg->arg.pixel.pos.y);
            }
            return 1;
        }
        case UCG_MSG_DRAW_L90FX: {
            // Lines.
            return lcd_ucg_handle_l90fx(ucg);
        }
        case UCG_MSG_DRAW_L90SE: {
            // Gradient.
            LLOG(TAG, "DEV: UCG_MSG_DRAW_L90SE");
            return ucg_handle_l90se(ucg, lcd_ucg_device);
        }
        case UCG_MSG_SET_CLIP_BOX: {
            // Clip Box.
            // "data" is a pointer to ucg_box_t structure.
            __attribute__((unused)) ucg_box_t *clip = (ucg_box_t *) data;
            LLOG(TAG, "DEV: UCG_MSG_SET_CLIP_BOX (%d, %d) -> (%d, %d)",
                 clip->ul.x, clip->ul.y,
                 clip->ul.x + clip->size.w, clip->ul.y + clip->size.h);
            return ucg_dev_default_cb(ucg, msg, data);
        }
        default: {
            LLOG(TAG, "DEV: default msg: %d", msg);
            return ucg_dev_default_cb(ucg, msg, data);
        }
    }
}

static ucg_int_t lcd_ucg_extensions(ucg_t *ucg, ucg_int_t msg, void *data) {
    ARG_UNUSED(ucg);
    ARG_UNUSED(msg);
    ARG_UNUSED(data);
    ESP_LOGE(TAG, "EXT: msg: %d is not supported", msg);
    ESP_ERROR_CHECK(ESP_ERR_NOT_SUPPORTED);
    return 1;
}

static int16_t lcd_ucg_com(ucg_t *ucg, int16_t msg, uint16_t arg, uint8_t *data) {
    ARG_UNUSED(ucg);
    ARG_UNUSED(arg);
    ARG_UNUSED(data);
    switch (msg) {
        case UCG_COM_MSG_POWER_UP:
            LLOG(TAG, "COM: UCG_COM_MSG_POWER_UP");
            break;
        case UCG_COM_MSG_POWER_DOWN:
        case UCG_COM_MSG_DELAY:
        case UCG_COM_MSG_CHANGE_RESET_LINE:
        case UCG_COM_MSG_CHANGE_CD_LINE:
        case UCG_COM_MSG_CHANGE_CS_LINE:
        case UCG_COM_MSG_SEND_BYTE:
        case UCG_COM_MSG_REPEAT_1_BYTE:
        case UCG_COM_MSG_REPEAT_2_BYTES:
        case UCG_COM_MSG_REPEAT_3_BYTES:
        case UCG_COM_MSG_SEND_STR:
        case UCG_COM_MSG_SEND_CD_DATA_SEQUENCE:
        default:
            ESP_LOGE(TAG, "COM: msg: %d is not supported", msg);
            ESP_ERROR_CHECK(ESP_ERR_NOT_SUPPORTED);
    }
    return 1;
}

esp_err_t lcd_ucg_hal_init(lcd_dev_t *dev, ucg_t *ucg) {
    ucg->user_ptr = dev;
    ucg_Init(ucg, lcd_ucg_device, lcd_ucg_extensions, lcd_ucg_com);
    ucg_SetFontMode(ucg, UCG_FONT_MODE_TRANSPARENT);
    ucg_SetRotate90(ucg);
    return ESP_OK;
}
