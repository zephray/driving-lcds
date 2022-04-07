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
#include "pico/multicore.h"
#include "pico/sd_card.h"
#include "hardware/pwm.h"
#include "hardware/vreg.h"
#include "lcd.h" // For resolution definition, should not call any functions
#include "core1.h"

#define SDVIDEO
//#define GREYBAR
//#define IMAGE
//#define MOVING_BLOCK

#ifdef IMAGE
#include "image160160.h"
#endif

#ifdef SDVIDEO
// 480x160: 150 blocks, 320x100: 63 blocks
#define BLOCK_PER_FRAME 63
#define FRAMES  21313
#endif

#ifdef MOVING_BLOCK
#define BG  10
#define FG  20
#define DIM 20
#endif

// Overclock
#define VREG_VSEL   VREG_VOLTAGE_1_20
//#define SYSCLK_KHZ  250000
#define SYSCLK_KHZ  300000

static void putpixel(uint8_t *buf, int x, int y, uint8_t c) {
    buf[SCR_WIDTH * y + x] = c;
}

static void fill(uint8_t *buf, int x0, int y0, int x1, int y1, uint8_t c) {
    if (x0 < 0)
        x0 = 0;
    if (y0 < 0)
        y0 = 0;
    if (x1 >= SCR_WIDTH)
        x1 = SCR_WIDTH - 1;
    if (y1 >= SCR_HEIGHT)
        y1 = SCR_HEIGHT - 1;
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            putpixel(buf, x, y, c);
        }
    }
}

/*void put_char(unsigned char *buf, int x, int y, char c, int fg, int bg) {
    int i, j, p;
	for(i = 0; i < 6; i++)
	{
		for(j = 8; j > 0; j--)
		{
			p = charMap_ascii_mini[(unsigned char)c][i] << j ;
			if (p & 0x80)
                putpixel(buf, x + i, y + 8 - j, fg);
            else
                putpixel(buf, x + i, y + 8 - j, bg);
		}
	}
}

void put_string(unsigned char *buf, int x, int y, char *str, int fg, int bg) {
    while (*str) {
        char c = *str++;
        if ((c == '\n') || (x > SCR_WIDTH)) {
            y += 8;
            x = 0;
        }
        if (c != '\n')
            put_char(buf, x, y, c, fg, bg);
        x += 6;
    }
}*/

static unsigned char *lcd_flip() {
    multicore_fifo_push_blocking(0x1234); // Push something to request flip
    return (unsigned char *)multicore_fifo_pop_blocking();
}

void sd_read_blocks(uint8_t *dst, uint32_t block, int block_count) {
    int batches = block_count / 32;
    int remainders = block_count % 32;
    uint32_t block_addr = block;
    for (int i = 0; i < batches; i++) {
        sd_readblocks_sync((uint32_t *)dst, block_addr, 32);
        dst += 32 * 512;
        block_addr += 32;
    }
    sd_readblocks_sync((uint32_t *)dst, block_addr, remainders);
}

int main()
{
    vreg_set_voltage(VREG_VSEL);
	sleep_ms(10);
    set_sys_clock_khz(SYSCLK_KHZ, true);

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    multicore_launch_core1(core1_entry);
    uint32_t g = multicore_fifo_pop_blocking();

    gpio_put(LED_PIN, 1);
    sleep_ms(250);

    gpio_put(LED_PIN, 0);
    sleep_ms(250);

/*
    gpio_set_function(0, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_wrap(slice_num, 3);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 3);
    pwm_set_enabled(slice_num, true);*/
    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(0, 1);

#ifdef SDVIDEO
    int result = sd_init_4pins();
    unsigned char *fbuf;
    uint32_t block_addr = 0;
    uint8_t val = 0x00;
    for (int i = 0; i < FRAMES; i++) {
        fbuf = lcd_flip();

        sd_read_blocks(fbuf, block_addr, BLOCK_PER_FRAME);
        block_addr += BLOCK_PER_FRAME;
        for (int i = 0; i < BLOCK_PER_FRAME*512; i++) {
            fbuf[i] = 31 - (fbuf[i] >> 3);
        }
    }
    fbuf = lcd_flip();

    while (1);
#endif

#ifdef GREYBAR
    unsigned char *fbuf = lcd_flip();

    for (int i = 0; i < 32; i++) {
        fill(fbuf, i * 10, 0, (i + 1) * 10, 80, i);
    }

    fbuf = lcd_flip();
    while(1);
#endif

#ifdef IMAGE
    unsigned char *fbuf = lcd_flip();
    
    for (int i = 0; i < SCR_WIDTH * SCR_HEIGHT; i++) {
        fbuf[i] = gImage_image160160[i] >> 3;
    }

    fbuf = lcd_flip();
    while(1);
#endif

#ifdef MOVING_BLOCK
    int x = -DIM;
    while(1) {
        unsigned char *fbuf = lcd_flip();
        memset(fbuf, BG, SCR_WIDTH * SCR_HEIGHT);
        fill(fbuf, x, (SCR_HEIGHT - DIM) / 2, x + DIM, (SCR_HEIGHT + DIM) / 2, FG);
        x++;
        if (x == SCR_WIDTH)
            x = -DIM;
    }
#endif

    return 0;
}
