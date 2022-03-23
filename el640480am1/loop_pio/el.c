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

unsigned char framebuf[SCR_STRIDE * SCR_HEIGHT];

static inline void elsm_put(uint32_t ud, uint32_t ld) {
    while (pio_sm_is_tx_fifo_full(el_pio, EL_UDATA_SM));
    *(volatile uint32_t *)&el_pio->txf[EL_LDATA_SM] = ld;
    *(volatile uint32_t *)&el_pio->txf[EL_UDATA_SM] = ud;
}

static inline void elsm_wait(void) {
    uint32_t sm_stall_mask = 1u << (EL_UDATA_SM + PIO_FDEBUG_TXSTALL_LSB);
    el_pio->fdebug = sm_stall_mask;
    while (!(el_pio->fdebug & sm_stall_mask));
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

void el_start() {
    memset(framebuf, 0x00, SCR_STRIDE * SCR_HEIGHT);

    el_sm_init();

    gpio_init(HSYNC_PIN);
    gpio_set_dir(HSYNC_PIN, GPIO_OUT);
    gpio_init(VSYNC_PIN);
    gpio_set_dir(VSYNC_PIN, GPIO_OUT);
}

static void delay(uint32_t x) {
    while(x--);
}

void el_frame() {
    uint32_t *rdptr_ud = (uint32_t *)framebuf;
    uint32_t *rdptr_ld = (uint32_t *)(framebuf + SCR_STRIDE * SCR_HEIGHT / 2);
    for (int y = 0; y < SCR_HEIGHT / 2; y++) {
        pio_sm_set_enabled(el_pio, EL_UDATA_SM, false);
        pio_sm_set_enabled(el_pio, EL_LDATA_SM, false);
        // prefill FIFO
        elsm_put(*rdptr_ud++, *rdptr_ld++);
        // start SM
        pio_enable_sm_mask_in_sync(el_pio, (1u << EL_UDATA_SM) | (1u << EL_LDATA_SM));
        // loop filling FIFO
        for (int x = 1; x < SCR_STRIDE / 4; x++) {
            uint32_t du = *rdptr_ud++;
            uint32_t dl = *rdptr_ld++;
            elsm_put(du, dl);
        }
        elsm_wait();
        gpio_put(HSYNC_PIN, 1);
        gpio_put(VSYNC_PIN, (y == 0) ? 1 : 0);
        delay(15);
        gpio_put(HSYNC_PIN, 0);
        delay(5);
        gpio_put(VSYNC_PIN, 0);
    }
}
