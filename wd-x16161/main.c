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
#include "hardware/pwm.h"
#include "lcd.h"
#include "font.h"

static void putpixel_raw(unsigned char *buf, int x, int y, uint8_t c) {
    if (c)
        buf[SCR_STRIDE * y + x / 8] |= 0x80u >> (x % 8);
    else
        buf[SCR_STRIDE * y + x / 8] &= ~(0x80u >> (x % 8));
}

static void putpixel(unsigned char *buf, int x, int y, uint16_t c) {
    uint8_t r = (c & 0x4) >> 2;
    uint8_t g = (c & 0x2) >> 1;
    uint8_t b = c & 0x1; 
    // TODO: Greyscale?

    int rx = y;
    int ry = SCR_HEIGHT - x - 1;
    putpixel_raw(buf, rx * 3, ry, r);
    putpixel_raw(buf, rx * 3 + 1, ry, g);
    putpixel_raw(buf, rx * 3 + 2, ry, b);
}

static void fill(uint8_t *buf, int x0, int y0, int x1, int y1, uint16_t c) {
    /*uint8_t r = c & 0xf800 >> 8;
    uint8_t g = c & 0x07e0 >> 3;
    uint8_t b = c & 0x001f << 3;*/
    for (int x = x0; x < x1; x++) {
        for (int y = y0; y < y1; y++) {
            putpixel(buf, x, y, c);
        }
    }
}

void put_char(unsigned char *buf, int x, int y, char c, int fg, int bg) {
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
}

#define FB_WIDTH (SCR_WIDTH / 3)
#define FB_HEIGHT (SCR_HEIGHT)

int main()
{
    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 1);
    sleep_ms(250);

    gpio_put(LED_PIN, 0);
    sleep_ms(250);

    lcd_start();

/*
    gpio_set_function(0, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_wrap(slice_num, 3);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 3);
    pwm_set_enabled(slice_num, true);*/
    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);
    gpio_put(0, 1);


    for (int i = 0; i < FB_WIDTH; i++) {
        putpixel(framebuf_bp0, i, 0, 0x7);
        putpixel(framebuf_bp0, i, FB_HEIGHT - 1, 0x7);
    }
    for (int i = 0; i < FB_HEIGHT; i++) {
        putpixel(framebuf_bp0, 0, i, 0x7);
        putpixel(framebuf_bp0, FB_WIDTH - 1, i, 0x7);
    }

    for (int i = 0; i < 8; i++) {
        put_string(framebuf_bp0, 2, 2 + i * 8, "Hello, world", i, 0);
    }

    while (1) {
    }

    return 0;
}
