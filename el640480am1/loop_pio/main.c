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

    put_string(framebuf, 0, 0, text, 1, 0);

    while(1) {
        el_frame();
    }

    return 0;
}
