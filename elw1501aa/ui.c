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
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "lcd.h"
#include "font.h"
#include "ui.h"

#define UI_PRINTF_BUFFER_SIZE 128

#define BG_COLOR (0x0)
#define FG_COLOR (0xf)

#if 0
static uint16_t switch_endian_16(uint16_t v) {
    return (((v >> 8) & 0xff) | (v << 8));
}
#endif

static void _lcd_set_pixel(size_t x, size_t y, uint8_t c) {
    //framebuffer[y * LCD_WIDTH + x] = switch_endian_16(c);
    uint8_t mask = 0xf0;
    if (x & 1) {
        mask = 0x0f;
        c <<= 4;
    }
    framebuffer[(y * LCD_WIDTH + x) / 2] &= mask;
    framebuffer[(y * LCD_WIDTH + x) / 2] |= c;
}

void ui_disp_char(int x, int y, char c, uint8_t cl) {
    if (c < 0x20)
        return;
    c -= 0x20;
    for (int yy = 0; yy < 7; yy++) {
        if ((y + yy) < 0) continue;
        if ((y + yy) >= UI_HEIGHT) continue;
        for (int xx = 0; xx < 5; xx++) {
            if ((x + xx) < 0) continue;
            if ((x + xx) >= UI_WIDTH) continue;
            if ((font[c * 5 + xx] >> yy) & 0x01) {
                _lcd_set_pixel(x + xx, y + yy, cl);
            }
            else {
                _lcd_set_pixel(x + xx, y + yy, BG_COLOR);
            }
        }
    }
}

void ui_clear(uint8_t c) {
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT / 2; i++) {
        framebuffer[i] = c;
    }
}

void ui_disp_string(int x, int y, char *str, uint8_t c) {
    while (*str) {
        ui_disp_char(x, y, *str++, c);
        x += 6;
        if ((x + 6) > UI_WIDTH) {
            y += 8;
            x = 0;
        }
    }
}

int ui_disp_string_bb(char *str, int width) {
    int w = 0;
    int h = 8;
    while (*str) {
        w += 6;
        if ((w + 6) > width) {
            w = 0;
            h += 8;
        }
        str++;
    }
    return h;
}

static char hex_to_char(int i) {
    if (i < 10)
        return '0' + i;
    else
        return 'A' + i - 10;
}

void ui_disp_hex(int x, int y, uint32_t num, uint8_t c) {
    ui_disp_char(x, y, hex_to_char((num >> 12) & 0xf), c);
    ui_disp_char(x + 6, y, hex_to_char((num >> 8) & 0xf), c);
    ui_disp_char(x + 12, y, hex_to_char((num >> 4) & 0xf), c);
    ui_disp_char(x + 18, y, hex_to_char((num) & 0xf), c);
}

void ui_disp_num(int x, int y, uint32_t num, uint8_t c) {
    if (num < 10000) {
        // Display 4 digit number
        ui_disp_char(x, y, num / 1000 + '0', c);
        num = num % 1000;
        ui_disp_char(x + 6, y, num / 100 + '0', c);
        num = num % 100;
        ui_disp_char(x + 12, y, num / 10 + '0', c);
        num = num % 10;
        ui_disp_char(x + 18, y, num + '0', c);
    }
    else {
        ui_disp_char(x, y, num / 10000 + '0', c);
        num = num % 10000;
        ui_disp_char(x + 6, y, num / 1000 + '0', c);
        ui_disp_char(x + 12, y, ' ', c);
        ui_disp_char(x + 18, y, 'K', c);
    }
}


void ui_init(void) {
    ui_clear(BG_COLOR);
    ui_disp_string(0, 0, "Hello, world!", FG_COLOR);
}


int ui_printf(int x, int y, const char *format, ...)
{
    char printf_buffer[UI_PRINTF_BUFFER_SIZE];

    int length = 0;

    va_list ap;
    va_start(ap, format);

    length = vsnprintf(printf_buffer, UI_PRINTF_BUFFER_SIZE, format, ap);

    va_end(ap);

    // display it for now
    ui_disp_string(x, y, printf_buffer, FG_COLOR);

    lcd_update();

    return length;
}
