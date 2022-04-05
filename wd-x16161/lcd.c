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
#include "lcddata.pio.h"
#include "lcd.h"

PIO lcd_pio = pio0;

int lcd_dma_chan;

unsigned char framebuf_bp0[SCR_STRIDE * SCR_HEIGHT];
unsigned char framebuf_bp1[SCR_STRIDE * SCR_HEIGHT];

static int frame_state = 0;
static bool ac = 0;
volatile int frame_scroll_lines = 0;
volatile bool swap_buffer = false;

static void lcd_sm_load_reg(uint sm, enum pio_src_dest dst, uint32_t val) {
    pio_sm_put_blocking(lcd_pio, sm, val);
    pio_sm_exec(lcd_pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(lcd_pio, sm, pio_encode_out(dst, 32));
}

static void lcd_sm_load_isr(uint sm, uint32_t val) {
    lcd_sm_load_reg(sm, pio_isr, val);
}

static void lcd_dma_init_channel(uint chan, uint dreq, volatile uint32_t *dst) {
    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, dreq);

    dma_channel_configure(chan, &c, dst, NULL, SCR_STRIDE_WORDS * SCR_REFRESH_LINES, false);
}

static void lcd_pio_irq_handler() {
    /*if (swap_buffer) {
        frame_state = !frame_state;
        swap_buffer = false;
    }
    uint8_t *framebuf = frame_state ? framebuf_bp1 : framebuf_bp0;*/
    uint8_t *framebuf = framebuf_bp0;

    // Set AC pin
    ac = !ac;
    gpio_put(AC_PIN, ac);

    uint32_t *rdptr_ud = (uint32_t *)(framebuf);
    uint32_t *rdptr_ld = (uint32_t *)(framebuf + SCR_STRIDE * SCR_HEIGHT / 2);
    dma_channel_set_read_addr(lcd_dma_chan, rdptr_ud, false);

    pio_sm_set_enabled(lcd_pio, LCD_DATA_SM, false);

    pio_sm_clear_fifos(lcd_pio, LCD_DATA_SM);

    pio_sm_restart(lcd_pio, LCD_DATA_SM);

    // Load configuration values
    lcd_sm_load_reg(LCD_DATA_SM, pio_y, SCR_REFRESH_LINES - 2);
    lcd_sm_load_reg(LCD_DATA_SM, pio_isr, SCR_LINE_TRANSFERS - 1);

    // Setup DMA
    dma_channel_start(lcd_dma_chan);

    // Clear IRQ flag
    lcd_pio->irq = 0x02;
    // start SM
    pio_enable_sm_mask_in_sync(lcd_pio, (1u << LCD_DATA_SM));
}

static void lcd_sm_init() {
    static uint program_offset;

    for (int i = 0; i < 8; i++) {
        pio_gpio_init(lcd_pio, D0_PIN + i);
    }
    pio_gpio_init(lcd_pio, PIXCLK_PIN);
    pio_gpio_init(lcd_pio, HSYNC_PIN);
    pio_gpio_init(lcd_pio, VSYNC_PIN);
    pio_sm_set_consecutive_pindirs(lcd_pio, LCD_DATA_SM, D0_PIN, 8, true);
    pio_sm_set_consecutive_pindirs(lcd_pio, LCD_DATA_SM, PIXCLK_PIN, 1, true);
    pio_sm_set_consecutive_pindirs(lcd_pio, LCD_DATA_SM, VSYNC_PIN, 1, true);
    pio_sm_set_consecutive_pindirs(lcd_pio, LCD_DATA_SM, HSYNC_PIN, 1, true);

    program_offset = pio_add_program(lcd_pio, &lcd_data_program);

    //printf("EL USM offset: %d, EL LSM offset: %d\n", udata_offset, ldata_offset);

    int cycles_per_pclk = 2;
    float div = clock_get_hz(clk_sys) / (LCD_TARGET_PIXCLK * cycles_per_pclk);

    pio_sm_config cu = lcd_data_program_get_default_config(program_offset);
    sm_config_set_sideset_pins(&cu, PIXCLK_PIN);
    sm_config_set_out_pins(&cu, D0_PIN, 8);
    sm_config_set_set_pins(&cu, HSYNC_PIN, 3);
    sm_config_set_fifo_join(&cu, PIO_FIFO_JOIN_TX);
    sm_config_set_out_shift(&cu, true, true, 32);
    sm_config_set_clkdiv(&cu, div);
    pio_sm_init(lcd_pio, LCD_DATA_SM, program_offset, &cu);

    lcd_pio->inte0 = PIO_IRQ0_INTE_SM1_BITS;
    irq_set_exclusive_handler(PIO0_IRQ_0, lcd_pio_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
}

static void lcd_dma_init() {
    lcd_dma_chan = dma_claim_unused_channel(true);
    lcd_dma_init_channel(lcd_dma_chan, DREQ_PIO0_TX0 + LCD_DATA_SM, &lcd_pio->txf[LCD_DATA_SM]);
}

void lcd_start() {
    memset(framebuf_bp0, 0x00, SCR_STRIDE * SCR_HEIGHT);
    memset(framebuf_bp1, 0x00, SCR_STRIDE * SCR_HEIGHT);
    
    gpio_init(AC_PIN);
    gpio_set_dir(AC_PIN, GPIO_OUT);
    gpio_put(AC_PIN, ac);
    gpio_init(EN_PIN);
    gpio_set_dir(EN_PIN, GPIO_OUT);
    gpio_put(EN_PIN, 1);

    lcd_sm_init();
    lcd_dma_init();
    lcd_pio_irq_handler();
}

/*void lcd_debug() {
    printf("PIO USM PC: %d, LSM PC: %d, IRQ: %d\n", lcd_pio->sm[LCD_DATA_SM].addr, lcd_pio->sm[lcd_LDATA_SM].addr, lcd_pio->irq);
}*/
