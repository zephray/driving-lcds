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
#include "pico/stdlib.h"
#include "pico/sd_card.h"
#include "el.h"

void sd_read_blocks(uint8_t *dst, uint32_t block, int block_count) {
    int batches = block_count / 32;
    int remainders = block_count % 32;
    uint32_t block_addr = block;
    for (int i = 0; i < batches; i++) {
        sd_readblocks_sync((unsigned int *)dst, block_addr, 32);
        dst += 32 * 512;
        block_addr += 32;
    }
    sd_readblocks_sync((unsigned int *)dst, block_addr, remainders);
}

int main()
{
    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    for (int i = 0; i < 2; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }

    int result = sd_init_4pins();

    if (result != SD_OK) {
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }

    gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);
    sleep_ms(250);

    el_start();

    uint32_t block_addr = 0;
    uint8_t val = 0x00;
    for (int i = 0; i < 5262; i++) {
        unsigned char *fbuf = el_swap_buffer();

        sd_read_blocks(fbuf, block_addr, 75);
        block_addr += 75;
    }

    while(1) {
    }

    return 0;
}
