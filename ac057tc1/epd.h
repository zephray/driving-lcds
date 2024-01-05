//
// Copyright 2024 Wenting Zhang <zephray@outlook.com>
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
#pragma once

#define EPD_BLACK   0x0
#define EPD_WHITE   0x1
#define EPD_GREEN   0x2
#define EPD_BLUE    0x3
#define EPD_RED     0x4
#define EPD_YELLOW  0x5
#define EPD_ORANGE  0x6

#define EPD_WIDTH   600
#define EPD_HEIGHT  448
#define EPD_BUFLEN  (EPD_WIDTH * EPD_HEIGHT / 2)

void epd_init(void);
void epd_clear(uint8_t color);
void epd_disp_palette_3bpp(void);
void epd_disp_palette_6bpp(void);
void epd_disp_palette_9bpp(void);
void epd_disp_image_3bpp(uint8_t *image);
void epd_disp_image_6bpp(uint8_t *image);
void epd_disp_image_9bpp(uint8_t *image);
void epd_sleep(void);