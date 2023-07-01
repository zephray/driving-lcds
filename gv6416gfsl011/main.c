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
#include "pico/sd_card.h"
#include "hardware/pwm.h"
#include "lcd.h" // For resolution definition, should not call any functions
#include "font.h"

void put_char(int x, int y, char c, lcd_color_t color) {
    int i, j, p;
	for(i = 0; i < 6; i++)
	{
		for(j = 8; j > 0; j--)
		{
			p = charMap_ascii_mini[(unsigned char)c][i] << j ;
			if (p & 0x80)
                lcd_set_point(x + i, y + 8 - j, color);
            //else
                //lcd_set_point(x + i, y + 8 - j, bg);
		}
	}
}

void put_string(int x, int y, char *str, lcd_color_t color) {
    while (*str) {
        char c = *str++;
        if ((c == '\n') || (x > LCD_X_PIXELS)) {
            y += 8;
            x = 0;
        }
        if (c != '\n')
            put_char(x, y, c, color);
        x += 6;
    }
}

void sd_read_blocks(uint8_t *dst, uint32_t block, int block_count) {
    int batches = block_count / 32;
    int remainders = block_count % 32;
    uint32_t block_addr = block;
    for (int i = 0; i < batches; i++) {
        sd_readblocks_sync((uint32_t *)dst, block_addr, 32);
        dst += 32 * 512;
        block_addr += 32;
    }
    sd_readblocks_sync((uint32_t *)dst, block_addr, remainders);
}

typedef struct {
    uint8_t x;
    uint8_t y;
    char *str;
} lcd_char_flow_t;

const static lcd_char_flow_t lcd_char_flow[] = {
    {  0, 0, "H"},
    {  8, 0, "e"},
    { 16, 0, "l"},
    { 24, 0, "l"},
    { 32, 0, "o"},
    { 16, 8, "W"},
    { 24, 8, "o"},
    { 32, 8, "r"},
    { 40, 8, "l"},
    { 48, 8, "d"},
    { 56, 8, "!"}
};

#define DIM 10

int main()
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 1);
    sleep_ms(250);

    gpio_put(LED_PIN, 0);
    sleep_ms(250);

    lcd_init();
    int cnt = 0;
    // while (1) {
    //     lcd_clear();
    //     // for (uint8_t i = 0; i < sizeof(lcd_char_flow) / sizeof(lcd_char_flow_t);i ++) {
    //     //     cnt ++;
    //     //     put_string(lcd_char_flow[i].x, lcd_char_flow[i].y,
    //     //         lcd_char_flow[i].str, (lcd_color_t)(cnt%6 + 1));
    //     // }

    //     // sleep_ms(200);

    //     lcd_send_buffer();
    // }

    int x = -DIM;
    int pos = 0;
    char str[10];
    while(1) {
        lcd_clear();
        if (x == LCD_X_PIXELS) {
            // pos++;
            // if (pos>128)
            //     pos = 0;
            // lcd_test(pos);
            x = -DIM;
        }
        //snprintf(str, 10, "%d", pos);
        put_string(0, 0, "Hello", lcd_color_blue);
        //lcd_color_t color = ((cnt/100)%6+1);
        lcd_color_t color = lcd_color_green;
        cnt++;
        for (int xx = x; xx < x + DIM; xx++) {
            for (int y = (LCD_Y_PIXELS - DIM) / 2; y < (LCD_Y_PIXELS + DIM) / 2; y++) {
                lcd_set_point(xx, y, color);
            }
        }
        x++;
        lcd_send_buffer();
        sleep_ms(20);
    }

    return 0;
}
