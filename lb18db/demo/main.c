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
#include "lcd.h"
#include "image.h"
#include "font.h"

int offset_table[] = {26, 26, 27, 26, 26, 25};

static void putpixel(unsigned char *buf, int x, int y, uint8_t c) {
    x += offset_table[y % 6];
    y += 3;
    buf[SCR_WIDTH * y + x] = c;
}

static void fill(uint8_t *buf, int x0, int y0, int x1, int y1, uint8_t c) {
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            buf[SCR_WIDTH * y + x] = c;
        }
    }
}

static void copy_image(unsigned char *dst, unsigned char *src, int w, int h) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            putpixel(dst, x, y, src[y * w + x]);
        }
    }
}

void put_char(unsigned char *buf, int x, int y, char c, int fg, int bg) {
    int i, j, p;
	for(i = 0; i < 12; i++)
	{
		for(j = 8; j > 0; j--)
		{
			p = charMap_ascii[(unsigned char)c][i] >> j ;
			if (p & 0x01)
                putpixel(buf, x + j, y + i, fg);
            else
                putpixel(buf, x + j, y + i, bg);
		}
	}
}

void put_string(unsigned char *buf, int x, int y, char *str, int fg, int bg) {
    while (*str) {
        char c = *str++;
        if ((c == '\n') || ((x + 6) > SCR_WIDTH)) {
            y += 12;
            x = 0;
        }
        if (c != '\n')
            put_char(buf, x, y, c, fg, bg);
        x += 8;
    }
}

static void line(uint8_t *buf, int x0, int y0, int x1, int y1, uint8_t c) {
    if (x0 == x1) {
        for (int y = y0; y < y1; y++) {
            putpixel(buf, x0, y, c);
        }
    }
    else if (y0 == y1) {
        for (int x = x0; x < x1; x++) {
            putpixel(buf, x, y0, c);
        }
    }
}

static void rect(uint8_t *buf, int x0, int y0, int x1, int y1, uint8_t c) {
    x0 += SRC_X_OFFSET;
    x1 += SRC_X_OFFSET;
    y0 += SRC_Y_OFFSET;
    y1 += SRC_Y_OFFSET;
    for (int x = x0; x < x1; x++) {
        buf[y0 * SCR_WIDTH + x] = c;
        buf[(y1 - 1) * SCR_WIDTH + x] = c;
    }
    for (int y = y0; y < y1; y++) {
        buf[y * SCR_WIDTH + x0] = c;
        buf[y * SCR_WIDTH + x1 - 1] = c;
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

    lcd_start();

    int x = 0, y = 0;
    int dir = 0;

    //fill(framebuf_bp0, 200, 120, 440, 360, 1);
    //fill(framebuf_bp1, 200, 120, 440, 360, 1);
/*
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

        unsigned char *buffer = lcd_swap_buffer();
        fill(buffer, last_x, last_y, last_x + 100, last_y + 100, 0);
        fill(buffer, x, y, x + 100, y + 100, 255);
        buffer = lcd_swap_buffer();
        fill(buffer, last_x, last_y, last_x + 100, last_y + 100, 0);
        fill(buffer, x, y, x + 100, y + 100, 255);
    }*/

    /*unsigned char *buffer;
    char str[255];
    while(1) {
        for (int x = 0; x < 1; x++) {
            buffer = lcd_swap_buffer();
            memset(buffer, 0, SCR_WIDTH * SCR_HEIGHT);
            line(buffer, 0, 0, 312, 0, 255);
            line(buffer, 0, 229, 312, 229, 255);

            line(buffer, x, 0, x, 229, 255);
            snprintf(str, 255, "%d", x);
            put_string(buffer, 200, 100, str, 255, 0);
            sleep_ms(1000);
        }
    }*/
    unsigned char *buffer = lcd_swap_buffer();
    copy_image(buffer, image, 312, 230);
    buffer = lcd_swap_buffer();
    while(1);

    return 0;
}
