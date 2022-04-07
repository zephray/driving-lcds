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
#include "lcd.h"
#include "font.h"

// Choose from below
//#define GREYSCALE_NONE // Clamp to BW mono
//#define GREYSCALE_DITHER
//#define GREYSCALE_PWM // 7-frame PWM
//#define GREYSCALE_1ST_SD // 1st-order sigma delta
//#define GREYSCALE_2ND_SD // 2nd-order sigma delta
#define GREYSCALE_GLDP

#if (defined(GREYSCALE_1ST_SD)) || (defined(GREYSCALE_2ND_SD))
#define INJECT_NOISE
#endif

static unsigned char *framebuf;
static unsigned char fb0[SCR_WIDTH * SCR_HEIGHT + 256];
static unsigned char fb1[SCR_WIDTH * SCR_HEIGHT + 256];
static signed char err1[SCR_WIDTH * SCR_HEIGHT]; // Z-1
static signed char err2[SCR_WIDTH * SCR_HEIGHT]; // Z-2
static int frontbuf;
static int repeat;

#define FRAME_REPEAT 4 // 0 - 120Hz, 1 - 60Hz, 2 - 40Hz, 3 - 30Hz, 4 - 24Hz

static uint32_t framecnt = 0;
#if (defined(GREYSCALE_PWM))
#define FRAMECNT_MAX 7 // 0-6
#elif (defined(GREYSCALE_GLDP))
#define GLDP_LENGTH (31)
#define FRAMECNT_MAX (GLDP_LENGTH)
#include "gldp.h"
static int random_select = 1;
#else
#define FRAMECNT_MAX 1 // Doesn't matter
#endif

static inline uint8_t handle_pixel(uint8_t color, int8_t *err1, int8_t *err2, int x, int y) {
#ifdef GREYSCALE_NONE
    return (color > 15);
#elif (defined(GREYSCALE_DITHER))
    int err = *err1;
    int c = color + err;
    int output = (err > 15) ? 31 : 0;
    err = c - output;
    // Floyd-Steinberg
    // . * 1
    // 2 3 4
    // Star is the pixel in question, the error is pushed to the pixels
    // labeled 1-4
    if (x < (SCR_WIDTH - 1))
        err1[1] += err * 7 / 16;
    if (y < (SCR_HEIGHT - 1)) {
        if (x > 0)
            err1[SCR_WIDTH - 2] += err * 3 / 16;
        err1[SCR_WIDTH - 1] += err * 5 / 16;
        if (x < (SCR_WIDTH - 1))
            err1[SCR_WIDTH] += err * 1 / 16;
    }

    return !!output;
#elif (defined(GREYSCALE_PWM))
    return ((color/4) > framecnt);
#elif (defined(GREYSCALE_1ST_SD))
    static int dither = 1;
    dither = dither / 2 ^ -(dither % 2) & 0x428e;
    int err = *err1; // From last cycle
    // Each time, add the last raw minus last quantized
    int c = (int)color + err;
#ifdef INJECT_NOISE
    c += ((dither & 0x1) << 1) - 1;
#endif
    int output = (c > 15) ? 31 : 0;
    err = c - output;
    *err1 = err;

    return !!output;
#elif (defined(GREYSCALE_2ND_SD))
    static int dither = 1;
    dither = dither / 2 ^ -(dither % 2) & 0x428e;
    int er1 = *err1; // From last cycle
    int er2 = *err2; // From 2 cycles before
    // Each time, add the last raw minus last quantized
    int c = color + er1 * 2 - er2;
#ifdef INJECT_NOISE
    c += ((dither & 0x1) << 1) - 1;
#endif
    int output;
    // Force output to be 0/31 when input is 0/31
    if (color == 0) output = 0;
    else if (color == 31) output = 31;
    else output = (c > 15) ? 31 : 0;
    er2 = er1;
    er1 = c - output;
    // Clamp to avoid overflow
    if (er1 < -50) er1 = -50;
    if (er1 > 50) er1 = 50;
    *err1 = er1;
    *err2 = er2;
    
    return !!output;
#elif (defined(GREYSCALE_GLDP))
    random_select = random_select / 2 ^ -(random_select % 2) & 0x428e;
    int sel = (framecnt + random_select) % GLDP_LENGTH;
    uint8_t output = gldp[color * GLDP_LENGTH + sel];
    return output;
#endif
}

static unsigned char *get_frontbuf() {
    return frontbuf ? fb1 : fb0;
}

void __no_inline_not_in_flash_func(core1_task)() {
    while (1) {
        if (repeat == FRAME_REPEAT) {
            repeat = 0;
            /* Check from core0 is buffer update is requested */
            if (multicore_fifo_rvalid()) {
                /* Buffer swap requested */
                multicore_fifo_pop_blocking(); // Result is ignored
                multicore_fifo_push_blocking((uint32_t)(get_frontbuf()));
                frontbuf = !frontbuf;
                framebuf = get_frontbuf();
            }
        }
        else {
            repeat++;
#ifdef GREYSCALE_DITHER
            continue;
#endif
        }

        /* Submit finished buffer and wait for Vsync */
        unsigned char *bp = lcd_swap_buffer();

#ifdef GREYSCALE_DITHER
        memset(err1, 0, sizeof(err1));
#endif

#ifdef GREYSCALE_GLDP
        random_select = 1;
#endif

        for (int y = 0; y < SCR_HEIGHT; y++) {
            for (int x = 0; x < SCR_STRIDE; x++) {
                unsigned char byte = 0;
                unsigned char *fb = &framebuf[y * SCR_WIDTH + x * 8];
                signed char *er1 = &err1[y * SCR_WIDTH + x * 8];
                #if (defined(GREYSCALE_2ND_SD))
                signed char *er2 = &err2[y * SCR_WIDTH + x * 8];
                #else
                signed char *er2 = NULL;
                #endif
                // This is a bug in hardware. LCD data bus is... reversed
                #if (LCD_BUS_WIDTH == 4)
                byte  = (handle_pixel(fb[0], &er1[0], &er2[0], x * 8 + 0, y) << 3);
                byte |= (handle_pixel(fb[1], &er1[1], &er2[1], x * 8 + 1, y) << 2);
                byte |= (handle_pixel(fb[2], &er1[2], &er2[2], x * 8 + 2, y) << 1);
                byte |= (handle_pixel(fb[3], &er1[3], &er2[3], x * 8 + 3, y) << 0);
                byte |= (handle_pixel(fb[4], &er1[4], &er2[4], x * 8 + 4, y) << 7);
                byte |= (handle_pixel(fb[5], &er1[5], &er2[5], x * 8 + 5, y) << 6);
                byte |= (handle_pixel(fb[6], &er1[6], &er2[6], x * 8 + 6, y) << 5);
                byte |= (handle_pixel(fb[7], &er1[7], &er2[7], x * 8 + 7, y) << 4);
                #else
                byte  = (handle_pixel(fb[0], &er1[0], &er2[0], x * 8 + 0, y) << 7);
                byte |= (handle_pixel(fb[1], &er1[1], &er2[1], x * 8 + 1, y) << 6);
                byte |= (handle_pixel(fb[2], &er1[2], &er2[2], x * 8 + 2, y) << 5);
                byte |= (handle_pixel(fb[3], &er1[3], &er2[3], x * 8 + 3, y) << 4);
                byte |= (handle_pixel(fb[4], &er1[4], &er2[4], x * 8 + 4, y) << 3);
                byte |= (handle_pixel(fb[5], &er1[5], &er2[5], x * 8 + 5, y) << 2);
                byte |= (handle_pixel(fb[6], &er1[6], &er2[6], x * 8 + 6, y) << 1);
                byte |= (handle_pixel(fb[7], &er1[7], &er2[7], x * 8 + 7, y) << 0);
                #endif
                *bp++ = byte;
            }
        }

        // For PWM/ GLDP
        framecnt++;

        if (framecnt == FRAMECNT_MAX)
            framecnt = 0;
    }
} 

void core1_entry() {
    multicore_fifo_push_blocking(0xc0ffee);

    framebuf = fb0;
    frontbuf = 0;
    repeat = 0;

    lcd_start();

    core1_task();
}
