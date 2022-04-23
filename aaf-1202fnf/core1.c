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

// Configuration of grayscale modulation

// Select method
//#define GRAYSCALE_NONE // Clamp to BW mono
//#define GRAYSCALE_DITHER // Spatial dithering with error-diffusion
//#define GRAYSCALE_PWM // PWM
//#define GRAYSCALE_1ST_SD // 1st-order sigma delta
#define GRAYSCALE_2ND_SD // 2nd-order sigma delta
//#define GRAYSCALE_GLDP_LINEAR
//#define GRAYSCALE_GLDP_NL

#ifdef GRAYSCALE_PWM
#define PWM_4G
//#define PWM_8G
//#define PWM_16G
#endif

#if (defined(GRAYSCALE_GLDP_LINEAR))
#define RANDOM_SELECTION
#endif

#if (defined(GRAYSCALE_1ST_SD)) || (defined(GRAYSCALE_2ND_SD))
#define INJECT_NOISE
#endif

static unsigned char *framebuf;
static unsigned char fb0[SCR_WIDTH * SCR_HEIGHT + 256];
static unsigned char fb1[SCR_WIDTH * SCR_HEIGHT + 256];
static signed char err1[SCR_WIDTH * SCR_HEIGHT]; // Z-1
static signed char err2[SCR_WIDTH * SCR_HEIGHT]; // Z-2
static int frontbuf;
static int repeat;

// 120Hz
//#define FRAME_REPEAT 4 // 0 - 120Hz, 1 - 60Hz, 2 - 40Hz, 3 - 30Hz, 4 - 24Hz
// 240Hz
#define FRAME_REPEAT 9 // 1 - 120Hz, 3 - 60Hz, 5 - 40Hz, 7 - 30Hz, 9 - 24Hz


// Frame counter related
#if (defined(GRAYSCALE_PWM))
static uint32_t framecnt = 0;
#ifdef PWM_4G
#define FRAMECNT_MAX 3
#elif defined(PWM_8G)
#define FRAMECNT_MAX 7 // 0-6
#elif defined(PWM_16G)
#define FRAMECNT_MAX 15
#endif
#elif (defined(GRAYSCALE_GLDP_LINEAR))
static uint32_t framecnt = 0;
#define GLDP_LENGTH (31)
#define FRAMECNT_MAX (GLDP_LENGTH)
#include "gldp_32_linear.h"
static int random_select = 1;
#elif (defined(GRAYSCALE_GLDP_NL))
static uint32_t framecnt[32] = {0};
#include "gldp_32_nonlinear.h"
#else
// None
#endif

static inline uint8_t handle_pixel(uint8_t color, int8_t *err1, int8_t *err2, int x, int y) {
#ifdef GRAYSCALE_NONE
    return (color > 31);
#elif (defined(GRAYSCALE_DITHER))
    int err = *err1;
    int c = color + err;
    int output = (err > 31) ? 63 : 0;
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
#elif (defined(GRAYSCALE_PWM))
#ifdef PWM_4G
    color = color / 16; 
#elif defined(PWM_8G)
    color = color / 8;
#elif defined(PWM_16G)
    color = color / 4;
#endif
    return (color > framecnt);
#elif (defined(GRAYSCALE_1ST_SD))
    //color = color >> 1;

#if 1
    // LFSR
    static int dither = 0x1234;
    //dither = dither / 2 ^ -(dither % 2) & 0x428e; // 15-bit LFSR
    dither = dither / 2 ^ -(dither % 2) & 0x2000250; // 26-bit LFSR
#else
    // PCG
    static uint64_t state;
    uint64_t oldstate = state;
    state = oldstate * 6364136223846793005ULL + 1ULL;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    uint32_t dither = (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
#endif

    int err = *err1; // From last cycle
    // Each time, add the last raw minus last quantized
    int c = (int)color + err;
#ifdef INJECT_NOISE
    c += ((dither & 0x1) << 1) - 1;
#endif
    int output = (c > 31) ? 63 : 0;
    err = c - output;
    *err1 = err;

    return !!output;
#elif (defined(GRAYSCALE_2ND_SD))
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
#if 0
    if (color == 0) output = 0;
    else if (color == 63) output = 63;
    else output = (c > 31) ? 63 : 0;
#else
    output = (c > 31) ? 63 : 0;
#endif
    er2 = er1;
    er1 = c - output;
    // Clamp to avoid overflow
    if (er1 < -50) er1 = -50;
    if (er1 > 50) er1 = 50;
    *err1 = er1;
    *err2 = er2;
    
    return !!output;
#elif (defined(GRAYSCALE_GLDP_LINEAR))
#ifdef RANDOM_SELECTION
    random_select = random_select / 2 ^ -(random_select % 2) & 0x428e;
    int sel = (framecnt + random_select) % GLDP_LENGTH;
#else
    int sel = framecnt;
#endif
    uint8_t output = gldp[(color / 2) * GLDP_LENGTH + sel];
    return output;
#elif (defined(GRAYSCALE_GLDP_NL))
    //int gldp_length = gldp[(color / 2)][0];
    int sel = framecnt[(color / 2)];
    uint8_t output = gldp[(color / 2)][1 + sel];
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
#ifdef GRAYSCALE_DITHER
            lcd_wait_vsync();
            continue;
#endif
        }

        /* Submit finished buffer and wait for Vsync */
        unsigned char *bp = lcd_swap_buffer();

#ifdef GRAYSCALE_DITHER
        memset(err1, 0, sizeof(err1));
#endif

#ifdef GRAYSCALE_GLDP_LINEAR
        random_select = 1;
#endif

        for (int y = 0; y < SCR_HEIGHT; y++) {
            for (int x = 0; x < SCR_STRIDE; x++) {
                unsigned char byte = 0;
                unsigned char *fb = &framebuf[y * SCR_WIDTH + x * 8];
                signed char *er1 = &err1[y * SCR_WIDTH + x * 8];
                #if (defined(GRAYSCALE_2ND_SD))
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
        #if (defined(FRAMECNT_MAX))
        framecnt++;

        if (framecnt == FRAMECNT_MAX)
            framecnt = 0;
        #elif (defined(GRAYSCALE_GLDP_NL))
        for (int i = 0; i < 32; i++) {
            framecnt[i]++;
            if (framecnt[i] == gldp[i][0])
                framecnt[i] = 0;
        }
        #endif
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
