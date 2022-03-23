//
// Copyright 2021 Wenting Zhang <zephray@outlook.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "eldata.pio.h"
#include "el.h"

PIO el_pio = pio0;

int el_udma_chan, el_ldma_chan;

unsigned char framebuf[SCR_STRIDE * SCR_HEIGHT];

static int el_cur_y = 0;

static inline void elsm_wait(void) {
    uint32_t sm_stall_mask = 1u << (EL_UDATA_SM + PIO_FDEBUG_TXSTALL_LSB);
    el_pio->fdebug = sm_stall_mask;
    while (!(el_pio->fdebug & sm_stall_mask));
}

static void delay(uint32_t x) {
    while(x--);
}

static void el_dma_start_line() {
    pio_sm_set_enabled(el_pio, EL_UDATA_SM, false);
    pio_sm_set_enabled(el_pio, EL_LDATA_SM, false);
    pio_sm_clear_fifos(el_pio, EL_UDATA_SM);
    pio_sm_clear_fifos(el_pio, EL_LDATA_SM);
    // Setup DMA
    dma_channel_start(el_udma_chan);
    dma_channel_start(el_ldma_chan);
    // start SM
    pio_enable_sm_mask_in_sync(el_pio, (1u << EL_UDATA_SM) | (1u << EL_LDATA_SM));
}

static void el_dma_start_frame() {
    uint32_t *rdptr_ud = (uint32_t *)framebuf;
    uint32_t *rdptr_ld = (uint32_t *)(framebuf + SCR_STRIDE * SCR_HEIGHT / 2);
    dma_channel_set_read_addr(el_udma_chan, rdptr_ud, false);
    dma_channel_set_read_addr(el_ldma_chan, rdptr_ld, false);
    el_dma_start_line();
}

static void el_dma_handler() {
    gpio_put(25, 1);

    dma_hw->ints0 = 1u << el_ldma_chan;

    elsm_wait();
    gpio_put(HSYNC_PIN, 1);
    gpio_put(VSYNC_PIN, (el_cur_y == 0) ? 1 : 0);
    delay(15);
    gpio_put(HSYNC_PIN, 0);
    delay(5);
    gpio_put(VSYNC_PIN, 0);
    el_cur_y ++;
    if (el_cur_y == SCR_HEIGHT / 2) {
        // End of frame, reset
        el_cur_y = 0;
        el_dma_start_frame();
    }
    else {
        el_dma_start_line();
    }

    gpio_put(25, 0);
}

static void el_sm_init() {
    static uint udata_offset, ldata_offset;

    for (int i = 0; i < 4; i++) {
        pio_gpio_init(el_pio, UD0_PIN + i);
        pio_gpio_init(el_pio, LD0_PIN + i);
    }
    pio_gpio_init(el_pio, PIXCLK_PIN);
    pio_sm_set_consecutive_pindirs(el_pio, EL_UDATA_SM, UD0_PIN, 4, true);
    pio_sm_set_consecutive_pindirs(el_pio, EL_UDATA_SM, PIXCLK_PIN, 1, true);
    pio_sm_set_consecutive_pindirs(el_pio, EL_LDATA_SM, LD0_PIN, 4, true);

    udata_offset = pio_add_program(el_pio, &el_udata_program);
    ldata_offset = pio_add_program(el_pio, &el_ldata_program);

    int cycles_per_pclk = 2;
    float div = clock_get_hz(clk_sys) / (EL_TARGET_PIXCLK * cycles_per_pclk);

    pio_sm_config cu = el_udata_program_get_default_config(udata_offset);
    sm_config_set_sideset_pins(&cu, PIXCLK_PIN);
    sm_config_set_out_pins(&cu, UD0_PIN, 4);
    sm_config_set_fifo_join(&cu, PIO_FIFO_JOIN_TX);
    sm_config_set_out_shift(&cu, true, true, 32);
    sm_config_set_clkdiv(&cu, div);
    pio_sm_init(el_pio, EL_UDATA_SM, udata_offset, &cu);

    pio_sm_config cl = el_ldata_program_get_default_config(ldata_offset);
    sm_config_set_out_pins(&cl, LD0_PIN, 4);
    sm_config_set_fifo_join(&cl, PIO_FIFO_JOIN_TX);
    sm_config_set_out_shift(&cl, true, true, 32);
    sm_config_set_clkdiv(&cl, div);
    pio_sm_init(el_pio, EL_LDATA_SM, ldata_offset, &cl);
}

static void el_dma_init() {
    el_udma_chan = dma_claim_unused_channel(true);
    dma_channel_config cu = dma_channel_get_default_config(el_udma_chan);
    channel_config_set_transfer_data_size(&cu, DMA_SIZE_32);
    channel_config_set_read_increment(&cu, true);
    channel_config_set_write_increment(&cu, false);
    channel_config_set_dreq(&cu, DREQ_PIO0_TX0 + EL_UDATA_SM);

    dma_channel_configure(el_udma_chan, &cu, &el_pio->txf[EL_UDATA_SM], NULL, SCR_STRIDE / 4, false);

    el_ldma_chan = dma_claim_unused_channel(true);
    dma_channel_config cl = dma_channel_get_default_config(el_ldma_chan);
    channel_config_set_transfer_data_size(&cl, DMA_SIZE_32);
    channel_config_set_read_increment(&cl, true);
    channel_config_set_write_increment(&cl, false);
    channel_config_set_dreq(&cl, DREQ_PIO0_TX0 + EL_LDATA_SM);

    dma_channel_configure(el_ldma_chan, &cl, &el_pio->txf[EL_LDATA_SM], NULL, SCR_STRIDE / 4, false);

    dma_channel_set_irq0_enabled(el_ldma_chan, true);

    irq_set_exclusive_handler(DMA_IRQ_0, el_dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

void el_start() {
    memset(framebuf, 0x00, SCR_STRIDE * SCR_HEIGHT);

    el_sm_init();

    gpio_init(HSYNC_PIN);
    gpio_set_dir(HSYNC_PIN, GPIO_OUT);
    gpio_init(VSYNC_PIN);
    gpio_set_dir(VSYNC_PIN, GPIO_OUT);

    el_dma_init();

    el_dma_start_frame();
}

