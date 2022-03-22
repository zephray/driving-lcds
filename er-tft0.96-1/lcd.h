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

// Using horizontal mode causes visible diagnal image tearing. Using vertical
// mode only produces verital image tearing, which is less perceptiable.

//#define LCD_HORIZONTAL
#define LCD_VERTICAL

#ifdef LCD_HORIZONTAL
#define LCD_WIDTH (160)
#define LCD_HEIGHT (80)
#define LCD_OFFSET_X (1)
#define LCD_OFFSET_Y (26)
#else
#define LCD_WIDTH (80)
#define LCD_HEIGHT (160)
#define LCD_OFFSET_X (26)
#define LCD_OFFSET_Y (1)
#endif

extern uint16_t framebuffer[LCD_WIDTH * LCD_HEIGHT];

void lcd_init(void);
void lcd_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_update(void);