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
#include "el.h"

unsigned char framebuf[SCR_STRIDE * SCR_HEIGHT];

void el_start() {
    memset(framebuf, 0x00, SCR_STRIDE * SCR_HEIGHT);

    gpio_init(HSYNC_PIN);
    gpio_set_dir(HSYNC_PIN, GPIO_OUT);
    gpio_init(VSYNC_PIN);
    gpio_set_dir(VSYNC_PIN, GPIO_OUT);
    gpio_init(PIXCLK_PIN);
    gpio_set_dir(PIXCLK_PIN, GPIO_OUT);
    
    for (int i = 0; i < 4; i++) {
        gpio_init(UD0_PIN + i);
        gpio_set_dir(UD0_PIN + i, GPIO_OUT);
        gpio_init(LD0_PIN + i);
        gpio_set_dir(LD0_PIN + i, GPIO_OUT);
    }
}

static void delay(uint32_t x) {
    while(x--);
}

void el_frame() {
    uint8_t *rdptr_ud = framebuf;
    uint8_t *rdptr_ld = framebuf + SCR_STRIDE * SCR_HEIGHT / 2;
    for (int y = 0; y < SCR_HEIGHT / 2; y++) {
        for (int x = 0; x < SCR_STRIDE; x++) {
            uint8_t du = *rdptr_ud++;
            uint8_t dl = *rdptr_ld++;
            for (int b = 0; b < 2; b++) {
                gpio_put(PIXCLK_PIN, 1);
                gpio_put(LD0_PIN, dl & 0x01);
                gpio_put(LD1_PIN, dl & 0x02);
                gpio_put(LD2_PIN, dl & 0x04);
                gpio_put(LD3_PIN, dl & 0x08);
                gpio_put(UD0_PIN, du & 0x01);
                gpio_put(UD1_PIN, du & 0x02);
                gpio_put(UD2_PIN, du & 0x04);
                gpio_put(UD3_PIN, du & 0x08);
                gpio_put(PIXCLK_PIN, 0);
                dl >>= 4;
                du >>= 4;
            }
        }
        gpio_put(HSYNC_PIN, 1);
        gpio_put(VSYNC_PIN, (y == 0) ? 1 : 0);
        delay(15);
        gpio_put(HSYNC_PIN, 0);
        delay(5);
        gpio_put(VSYNC_PIN, 0);
    }
}
