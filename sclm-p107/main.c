/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <string.h>
#include "font.h"

#define PIXCLK_PIN 2
#define INV_PIN 1
#define LP_PIN 4
#define EI_PIN 5
#define D3_PIN 6
#define D2_PIN 7
#define D1_PIN 8
#define D0_PIN 9

static uint8_t framebuf[40];

static void delay(int t) {
    volatile int x = t * 10;
    while (x--);
}

static void frame(void) {
    uint8_t *rdptr = framebuf;
    for (int x = 0; x < 40; x++) {
        uint8_t d = *rdptr++;
        d = ~d;
        for (int b = 0; b < 2; b++) {
            gpio_put(PIXCLK_PIN, 1);
            gpio_put(D0_PIN, d & 0x08);
            gpio_put(D1_PIN, d & 0x04);
            gpio_put(D2_PIN, d & 0x02);
            gpio_put(D3_PIN, d & 0x01);
            delay(9);
            gpio_put(PIXCLK_PIN, 0);
            d >>= 4;
            delay(10);
        }
    }
    gpio_put(LP_PIN, 1);
    delay(15);
    gpio_put(LP_PIN, 0);
    delay(5);
}

void setpixel(int i, int j, int c) {
    int bitp = i*12+j + 16;
    int bytep = bitp >> 3;
    int bito = bitp & 7;
    if (c == 0)
        framebuf[bytep] |= (0x1 << bito);
    else
        framebuf[bytep] &= ~(0x1 << bito);
}


void disphar(int x, int y, char c, int cl) {
    if (c < 0x20)
        return;
    c -= 0x20;
    for (int yy = 0; yy < 7; yy++) {
        if ((y + yy) < 0) continue;
        if ((y + yy) >= 24) continue;
        for (int xx = 0; xx < 5; xx++) {
            if ((x + xx) < 0) continue;
            if ((x + xx) >= 24) continue;
            if ((font[c * 5 + xx] >> yy) & 0x01) {
                setpixel(x + xx, y + yy, cl);
            }
            else {
                setpixel(x + xx, y + yy, 0);
            }
        }
    }
}

int main() {
    gpio_init(PIXCLK_PIN);
    gpio_set_dir(PIXCLK_PIN, GPIO_OUT);
    gpio_init(INV_PIN);
    gpio_set_dir(INV_PIN, GPIO_OUT);
    gpio_init(LP_PIN);
    gpio_set_dir(LP_PIN, GPIO_OUT);
    gpio_init(EI_PIN);
    gpio_set_dir(EI_PIN, GPIO_OUT);
    gpio_init(D3_PIN);
    gpio_set_dir(D3_PIN, GPIO_OUT);
    gpio_init(D2_PIN);
    gpio_set_dir(D2_PIN, GPIO_OUT);
    gpio_init(D1_PIN);
    gpio_set_dir(D1_PIN, GPIO_OUT);
    gpio_init(D0_PIN);
    gpio_set_dir(D0_PIN, GPIO_OUT);

    gpio_put(EI_PIN, 0);

    memset(framebuf, 0x00, 40);

    int i = 0, dir = 0;
    while (true) {
        /*memset(framebuf, 0x00, 40);
        framebuf[i / 8] = 1 << (i % 8);
        i++;
        if (i == 320) i = 0;
        for (int i = 0; i < 500; i++)
            frame();*/
        gpio_put(INV_PIN, 1);
        frame();
        sleep_ms(10);
        gpio_put(INV_PIN, 0);
        frame();
        sleep_ms(10);

        memset(framebuf + 2, 0xff, 36);
        disphar(0, i, 'b', 1);
        disphar(6, i, 'e', 1);
        disphar(12, i, 'e', 1);
        disphar(18, i, 'f', 1);
        if (dir == 0) {
            i++;
            if (i == 6) {
                i = 4;
                dir = 1;
            }
        }
        else {
            i--;
            if (i == -1) {
                i = 1;
                dir = 0;
            }
        }
    }
}
