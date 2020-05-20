#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_attr.h"
#include "esp_system.h"
#include "soc/apb_ctrl_reg.h"
#include "soc/dport_access.h"
#include "soc/dport_reg.h"
#include "soc/i2s_struct.h"
#include "soc/lldesc.h"

#include "i2s_parallel.h"

static i2s_parallel_pin_config_t i2s_parallel_bus;

#define DMA_MAX 64
static lldesc_t __dma[DMA_MAX] = {0};
static SemaphoreHandle_t tx_sem = NULL;
static uint8_t *i2s_parallel_buffer = NULL;

static void i2s_parallel_config(uint32_t clk_div)
{
    DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_I2S0_CLK_EN);
    DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_I2S0_CLK_EN);
    DPORT_SET_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_I2S0_RST);
    DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_I2S0_RST);

    //Configure pclk, max: 20M
    I2S0.clkm_conf.val = 0;
    I2S0.clkm_conf.clkm_div_num = 2;
    I2S0.clkm_conf.clkm_div_b = 0;
    I2S0.clkm_conf.clkm_div_a =63;
    I2S0.clkm_conf.clk_en = 1;
    I2S0.clkm_conf.clk_sel = 2;
    I2S0.sample_rate_conf.val = 0;
    I2S0.sample_rate_conf.tx_bck_div_num = clk_div;

    I2S0.int_ena.val = 0;
    I2S0.int_clr.val = ~0;

    I2S0.conf.val = 0;
    I2S0.conf.tx_right_first = 1;
    I2S0.conf.tx_msb_right = 1;
    I2S0.conf.tx_dma_equal = 1;

    I2S0.conf1.val = 0;
    I2S0.conf1.tx_pcm_bypass = 1;
    I2S0.conf1.tx_stop_en = 1;
    I2S0.timing.val = 0;
    //Set lcd mode
    I2S0.conf2.val = 0;
    I2S0.conf2.lcd_en = 1;

    I2S0.fifo_conf.val = 0;
    I2S0.fifo_conf.tx_fifo_mod_force_en = 1;
    I2S0.fifo_conf.tx_data_num = 32;
    I2S0.fifo_conf.tx_fifo_mod = 4;

    I2S0.conf_chan.tx_chan_mod = 0;//remove

    I2S0.sample_rate_conf.tx_bits_mod = i2s_parallel_bus.bit_width;
    printf("--------I2S version  %x, bit_width: %d\n", I2S0.date, i2s_parallel_bus.bit_width);
}

static void i2s_parallel_set_pin(void)
{
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[i2s_parallel_bus.pin_clk], PIN_FUNC_GPIO);
    gpio_set_direction(i2s_parallel_bus.pin_clk, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(i2s_parallel_bus.pin_clk,GPIO_PULLUP_ONLY);
    gpio_matrix_out(i2s_parallel_bus.pin_clk, I2S0O_WS_OUT_IDX, true, 0);

    for(int i = 0; i < i2s_parallel_bus.bit_width; i++) {
        PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[i2s_parallel_bus.data[i]], PIN_FUNC_GPIO);
        gpio_set_direction(i2s_parallel_bus.data[i], GPIO_MODE_OUTPUT);
        gpio_set_pull_mode(i2s_parallel_bus.data[i],GPIO_PULLUP_ONLY);
        gpio_matrix_out(i2s_parallel_bus.data[i], I2S0O_DATA_OUT0_IDX + i, false, 0);
    }
}

static void IRAM_ATTR _i2s_isr(void *arg)
{
    if(I2S0.int_st.out_total_eof) {
        BaseType_t HPTaskAwoken = pdFALSE;
        xSemaphoreGiveFromISR(tx_sem, &HPTaskAwoken);
        if(HPTaskAwoken == pdTRUE) {
            portYIELD_FROM_ISR();
        }
    }
    I2S0.int_clr.val = ~0;
}

static void i2s_parallel_interface_init(uint32_t clk_div)
{
    for(int i = 0; i < DMA_MAX; i++) {
        __dma[i].size = 0;
        __dma[i].length = 0;
        __dma[i].sosf = 0;
        __dma[i].eof = 1;
        __dma[i].owner = 1;
        __dma[i].buf = NULL;
        __dma[i].empty = 0;
    }
    i2s_parallel_set_pin();
    i2s_parallel_config(clk_div);
    tx_sem = xSemaphoreCreateBinary();
    esp_intr_alloc(ETS_I2S0_INTR_SOURCE, 0, _i2s_isr, NULL, NULL);
    I2S0.int_ena.out_total_eof = 1;
}

static inline void i2s_dma_start(void)
{
    I2S0.fifo_conf.dscr_en = 1;
    I2S0.out_link.start = 1;
    I2S0.conf.tx_start = 1;
    xSemaphoreTake(tx_sem, portMAX_DELAY);
    while (!I2S0.state.tx_idle);
    I2S0.conf.tx_start = 0;
    I2S0.fifo_conf.dscr_en = 0;
    I2S0.conf.tx_reset = 1;
    I2S0.conf.tx_reset = 0;
}

static inline void i2s_parallel_dma_write(const uint8_t *buf, size_t length)
{
    int len = length;
    int trans_len = 0;
    int cnt = 0;
    while(len) {
        trans_len = len > 4000 ? 4000 : len;
        __dma[cnt].size = trans_len;
        __dma[cnt].length = trans_len;
        __dma[cnt].buf = buf;
        __dma[cnt].eof = 0;
        __dma[cnt].empty = (uint32_t) &__dma[cnt+1];
        buf += trans_len;
        len -= trans_len;
        cnt++;
    }
    __dma[cnt-1].eof = 1;
    __dma[cnt-1].empty = 0;
    I2S0.out_link.addr = ((uint32_t)&__dma[0]) & 0xfffff;
    i2s_dma_start();
}

void i2s_parallel_write_data(uint8_t *data, size_t len)
{
#ifdef I2S_PARALLEL_BURST_BUFFER_SIZE
    for (int x = 0; x < len / I2S_PARALLEL_BURST_BUFFER_SIZE; x++) {
        memcpy(i2s_parallel_buffer, data, I2S_PARALLEL_BURST_BUFFER_SIZE);
        i2s_parallel_dma_write(i2s_parallel_buffer, I2S_PARALLEL_BURST_BUFFER_SIZE);
        data += I2S_PARALLEL_BURST_BUFFER_SIZE;
    }
    if (len % I2S_PARALLEL_BURST_BUFFER_SIZE) {
        memcpy(i2s_parallel_buffer, data, len % I2S_PARALLEL_BURST_BUFFER_SIZE);
        i2s_parallel_dma_write(i2s_parallel_buffer, len % I2S_PARALLEL_BURST_BUFFER_SIZE);
    }
#else
    i2s_parallel_dma_write((uint8_t *)data, len);
#endif
}

void i2s_parallel_init(i2s_parallel_pin_config_t *pin_config, uint32_t clk_div)
{
    if (pin_config == NULL) {
        return;
    }
    i2s_parallel_bus = *pin_config;
#ifdef I2S_PARALLEL_BURST_BUFFER_SIZE
    i2s_parallel_buffer = (uint8_t *)heap_caps_malloc(I2S_PARALLEL_BURST_BUFFER_SIZE, MALLOC_CAP_DMA);
#endif
    i2s_parallel_interface_init(clk_div);
}

void i2s_parallel_write_data16n(uint16_t data, size_t len)
{
    uint16_t *tmp = (uint16_t *) i2s_parallel_buffer;
    bool filled = false;
    while (len > 0) {
        size_t to_write = len >= I2S_PARALLEL_BURST_BUFFER_SIZE ? I2S_PARALLEL_BURST_BUFFER_SIZE : len;
        if (!filled) {
            for (int i = 0; i < to_write; i += sizeof(uint16_t)) {
                *tmp++ = data;
            }
            filled = true;
        }
        i2s_parallel_dma_write(i2s_parallel_buffer, to_write);
        len -= to_write;
    }
}
