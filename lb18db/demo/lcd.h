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
#pragma once

#define VSYNC_PIN (9)
#define HSYNC_PIN (8)
#define PIXCLK_PIN (10)
// DATA_PIN
#define D0_PIN (0)

// PIO related
#define LCD_DATA_SM (0)
#define LCD_CLK_SM  (1)

// Screen related

#define LCD_BUS_WIDTH (8)

#define SCR_WIDTH (372)
#define SCR_HEIGHT (242)

#define SCR_HSYNC (20)

#define SRC_X_OFFSET (26)
#define SRC_Y_OFFSET (3)

#define LCD_TARGET_HZ (60)
#define LCD_TARGET_PIXCLK (LCD_TARGET_HZ * (SCR_WIDTH + 40) * (SCR_HEIGHT + 10))

// Public variables and functions
extern unsigned char framebuf_bp0[SCR_WIDTH * SCR_HEIGHT];
extern unsigned char framebuf_bp1[SCR_WIDTH * SCR_HEIGHT];
extern volatile bool frame_sync;

void lcd_start();
unsigned char *lcd_swap_buffer();
void lcd_wait_vsync();