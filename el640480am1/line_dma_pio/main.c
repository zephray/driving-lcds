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
#include "font.h"
#include "el.h"
#include "sample_text.h"

static void putpixel(unsigned char *buf, int x, int y, int c) {
    if (c)
        buf[SCR_STRIDE * y + x / 8] |= 1 << (x % 8);
    else
        buf[SCR_STRIDE * y + x / 8] &= ~(1 << (x % 8));
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

static void fill(uint8_t *buffer, int x0, int y0, int x1, int y1, int c) {
    uint8_t start_byte = (0xffu << (x0 % 8));
    uint8_t end_byte = (x1 % 8) ? (0xffu >> (8 - x1 % 8)) : 0x00;
    uint8_t x_bytes = (x1 + 8) / 8 - x0 / 8;
    // 0 to 8: unaligned: 2-0 = 2 bytes
    // 0 to 15: aligned: 2-0 = 2 bytes, correct
    // 0 to 16: unaligned: 2-0 = 3 bytes, correct
    uint32_t stride = SCR_STRIDE - x_bytes;
    uint8_t *fb = &buffer[y0 * SCR_STRIDE + x0 / 8];
    if (x_bytes == 1) {
        end_byte &= start_byte;
        if (c) {
            for (int y = y0; y <= y1; y++) {
                *fb |= end_byte;
                fb += SCR_STRIDE;
            }
        }
        else {
            for (int y = y0; y <= y1; y++) {
                *fb &= ~end_byte;
                fb += SCR_STRIDE;
            }
        }
    }
    else {
        if (c) {
            for (int y = y0; y <= y1; y++) {
                *fb++ |= start_byte;
                for (int i = 0; i < x_bytes - 2; i++)
                    *fb++ = 0xff;
                *fb++ |= end_byte;
                fb += stride;
            }
        }
        else {
            for (int y = y0; y <= y1; y++) {
                *fb++ &= ~start_byte;
                for (int i = 0; i < x_bytes - 2; i++)
                    *fb++ = 0x00;
                *fb++ &= ~end_byte;
                fb += stride;
            }
        }
    }
}

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

    el_start();

    //put_string(framebuf, 0, 0, text, 1, 0);

    int x = 0, y = 0;
    int dir = 0;

    while(1) {
        int last_x = x, last_y = y;

        bool xright = (x == SCR_WIDTH - 101);
        bool ydown = (y == SCR_HEIGHT - 101);
        bool yup = (y == 0);
        bool xleft = (x == 0);
        if (dir == 0) {
            // Right down
            if (ydown) dir = 1; // right up
            if (xright) dir = 2; // left down
            if (ydown && xright) dir = 3;
        }
        else if (dir == 1) {
            // Right up
            if (yup) dir = 0; // right down
            if (xright) dir = 3; // left up
            if (yup && xright) dir = 2;
        }
        else if (dir == 2) {
            // Left down
            if (ydown) dir = 3; // left up
            if (xleft) dir = 0; // right down
            if (ydown && xleft) dir = 1;
        }
        else if (dir == 3) {
            // Left up
            if (yup) dir = 2; // left down
            if (xleft) dir = 1; // right up
            if (yup && xleft) dir = 0;
        }

        if (dir == 0) {
            // Right down
            x++; y++;
        }
        else if (dir == 1) {
            // Right up
            x++; y--;
        }
        else if (dir == 2) {
            // Left down
            x--; y++;
        }
        else if (dir == 3) {
            // Left up
            x--; y--;
        }

        fill(framebuf, last_x, last_y, last_x + 100, last_y + 100, 0);
        fill(framebuf, x, y, x + 100, y + 100, 1);
        sleep_ms(20);
    }

    return 0;
}
