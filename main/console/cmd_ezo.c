#include <string.h>
#include <strings.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_err.h"
#include "esp_console.h"

#include "argtable3/argtable3.h"

#include "driver/ezo.h"

static struct {
    struct arg_str *module;
    struct arg_int *delay;
    struct arg_str *cmd;
    struct arg_end *end;
} cmd_args;

static struct {
    struct arg_str *module;
    struct arg_int *pause;
    struct arg_end *end;
} cmd_pause_args;

static ezo_sensor_t *
find_sensor(int argc, char **argv, void **argtable, struct arg_end *end, const char *desc, esp_err_t *err) {
    int errors = arg_parse(argc, argv, argtable);
    if (errors != 0) {
        arg_print_errors(stderr, end, argv[0]);
        *err = ESP_ERR_INVALID_ARG;
        return NULL;
    }
    ezo_sensor_t *sensor = ezo_find(desc);
    if (sensor == NULL) {
        printf("Unknown module named '%s'\n", desc);
        *err = ESP_ERR_INVALID_ARG;
        return NULL;
    }
    *err = ESP_OK;
    return sensor;
}

static int action(int argc, char **argv) {
    esp_err_t err;
    ezo_sensor_t *sensor = find_sensor(argc, argv, (void **) &cmd_args, cmd_args.end, cmd_args.module->sval[0], &err);
    if (err != ESP_OK) {
        return err;
    }
    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    err = ezo_send_command(sensor, cmd_args.delay->ival[0], "%s", cmd_args.cmd->sval[0]);
    if (err == ESP_OK) {
        char buf[EZO_MAX_BUFFER_LEN] = {0};
        err = ezo_parse_response(sensor, 1, "%s", buf);
        printf("Status: %d '%s'\n", sensor->status, buf);
    }
    xSemaphoreGive(sensor->lock);
    return err;
}

static int pause(int argc, char **argv) {
    esp_err_t err;
    ezo_sensor_t *sensor = find_sensor(argc, argv, (void **) &cmd_pause_args, cmd_pause_args.end,
                                       cmd_pause_args.module->sval[0], &err);
    if (err != ESP_OK) {
        return err;
    }
    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    sensor->pause = cmd_pause_args.pause->ival[0] ? true : false;
    xSemaphoreGive(sensor->lock);
    return ESP_OK;
}

esp_err_t cmd_ezo_init(void) {
    cmd_args.module = arg_str1(NULL, NULL, "<name>", "ezo module name");
    cmd_args.delay = arg_int1(NULL, NULL, "<ms>", "delay in ms to wait");
    cmd_args.cmd = arg_str1(NULL, NULL, "<cmd>", "command to send");
    cmd_args.end = arg_end(2);

    cmd_pause_args.module = arg_str1(NULL, NULL, "<name>", "ezo module name");
    cmd_pause_args.pause = arg_int1(NULL, NULL, "<1|0>", "pause/unpause the module");
    cmd_pause_args.end = arg_end(2);

    const esp_console_cmd_t ezo_cmd = {
            .command = "ezo",
            .help = "Run commands on an EZO module",
            .hint = NULL,
            .func = &action,
            .argtable = &cmd_args,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&ezo_cmd));

    const esp_console_cmd_t ezo_pause_cmd = {
            .command = "ezo_pause",
            .help = "Pause/unpause the automated readings of an EZO module",
            .hint = NULL,
            .func = &pause,
            .argtable = &cmd_pause_args,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&ezo_pause_cmd));

    return ESP_OK;
}