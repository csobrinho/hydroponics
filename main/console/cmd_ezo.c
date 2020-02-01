#include <string.h>
#include <strings.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_err.h"
#include "esp_console.h"

#include "argtable3/argtable3.h"

#include "driver/ezo.h"
#include "sensors/ezo_ec.h"
#include "sensors/ezo_ph.h"
#include "sensors/ezo_rtd.h"

static struct {
    struct arg_str *module;
    struct arg_int *address;
    struct arg_int *delay;
    struct arg_str *cmd;
    struct arg_end *end;
} cmd_args;

static ezo_sensor_t *find_sensor(const char *name) {
    ezo_sensor_t *probes[] = {ezo_ec, ezo_ph, ezo_rtd};
    for (int i = 0; i < sizeof(probes) / sizeof(ezo_sensor_t *); ++i) {
        if (strncasecmp(name, probes[i]->type, strlen(name)) == 0) {
            return probes[i];
        }
    }
    return NULL;
}

static int action(int argc, char **argv) {
    int errors = arg_parse(argc, argv, (void **) &cmd_args);
    if (errors != 0) {
        arg_print_errors(stderr, cmd_args.end, argv[0]);
        return ESP_ERR_INVALID_ARG;
    }
    ezo_sensor_t *sensor = find_sensor(cmd_args.module->sval[0]);
    if (sensor == NULL) {
        printf("Unknown module named '%s'\n", cmd_args.module->sval[0]);
        return ESP_ERR_INVALID_ARG;
    }
    xSemaphoreTake(sensor->lock, portMAX_DELAY);
    esp_err_t err = ezo_send_command(sensor, cmd_args.delay->ival[0], "%s", cmd_args.cmd->sval[0]);
    if (err == ESP_OK) {
        char buf[EZO_MAX_BUFFER_LEN] = {0};
        err = ezo_parse_response(sensor, 1, "%s", buf);
        printf("Status: %d '%s'\n", sensor->status, buf);
    }
    xSemaphoreGive(sensor->lock);
    return err;
}

esp_err_t cmd_ezo_init(void) {
    cmd_args.module = arg_str1(NULL, NULL, "<ec|ph|rtd>", "ezo module name");
    cmd_args.address = arg_int1(NULL, NULL, "<addr>", "i2c address");
    cmd_args.delay = arg_int1(NULL, NULL, "<ms>", "delay in ms to wait");
    cmd_args.cmd = arg_str1(NULL, NULL, "<cmd>", "command to send");
    cmd_args.end = arg_end(2);

    const esp_console_cmd_t ezo_cmd = {
            .command = "ezo",
            .help = "Run commands on an Ezo module",
            .hint = NULL,
            .func = &action,
            .argtable = &cmd_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&ezo_cmd));

    return ESP_OK;
}