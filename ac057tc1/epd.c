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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "epd.h"
#include "lut.h"

// Hardware Connection
#define EPD_SPI  spi1
#define EPD_SPI_DREQ DREQ_SPI1_TX
#define EPD_RST     12
#define EPD_RS      8
#define EPD_CS      9
#define EPD_SCK     10
#define EPD_MOSI    11
#define EPD_BUSYN   13
#define EPD_SPI_FREQ (4*1000*1000)

static void epd_select(void) {
    gpio_put(EPD_CS, 0);
}

static void epd_deselect(void) {
    gpio_put(EPD_CS, 1);
}

static void epd_mode_command(void) {
    gpio_put(EPD_RS, 0);
}

static void epd_mode_data(void) {
    gpio_put(EPD_RS, 1);
}

static void epd_reset(void) {
    gpio_put(EPD_RST, 0);
    sleep_ms(200);
    gpio_put(EPD_RST, 1);
    sleep_ms(20);
}

static void epd_send_byte(uint8_t byte) {
    epd_select();
    spi_write_blocking(EPD_SPI, &byte, 1);
    epd_deselect();
}

static void epd_send_cmd(uint8_t cmd) {
    epd_mode_command();
    epd_send_byte(cmd);
}

static void epd_send_dat(uint8_t dat) {
    epd_mode_data();
    epd_send_byte(dat);
}

static void epd_busy_high(void) {
    while (!gpio_get(EPD_BUSYN));
}

static void epd_busy_low(void) {
    while (gpio_get(EPD_BUSYN));
}

static void epd_send_buffer(uint8_t *buf, uint32_t size) {
    epd_mode_data();
    epd_select();
#if 1
    spi_write_blocking(EPD_SPI, buf, size);
    epd_deselect();
#else
    // Don't start until the last transmission has finished
    epd_update_req = 1;
    if (!epd_busy) {
        epd_update_req = 0;
        epd_busy = true;
        dma_channel_set_read_addr(epd_dma, framebuffer, false);
        dma_channel_set_trans_count(epd_dma, sizeof(framebuffer), true);
    }
#endif
}

void epd_load_lut(uint8_t n, const uint8_t *lut, uint32_t size) {
    static uint8_t padding[260] = {0};
    uint32_t padding_size = (n == VCOM_LUT_REG) ? VCOM_LUT_SIZE :
            (n == XON_LUT_REG) ? XON_LUT_SIZE : COLOR_LUT_SIZE;
    padding_size = padding_size - size;
    epd_send_cmd(n);
    epd_send_buffer(lut, size);
    epd_send_buffer(padding, padding_size);
}

void epd_load_lut_set(const lut_t *lut_set) {
    for (int i = 0; i < 10; i++)
        epd_load_lut(0x20 + i, lut_set[i].lut, lut_set[i].size);
}

void epd_init(void) {
    // Configure Pins
    gpio_set_function(EPD_RST,  GPIO_FUNC_SIO);
    gpio_set_function(EPD_RS,   GPIO_FUNC_SIO);
    gpio_set_function(EPD_CS,   GPIO_FUNC_SIO);
    gpio_set_function(EPD_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(EPD_MOSI, GPIO_FUNC_SPI);

    epd_deselect();
    gpio_set_dir(EPD_RST, GPIO_OUT);
    gpio_set_dir(EPD_RS,  GPIO_OUT);
    gpio_set_dir(EPD_CS,  GPIO_OUT);

    // Configure SPI
    spi_init(EPD_SPI, EPD_SPI_FREQ);

    // Set up DMA
    // epd_dma = dma_claim_unused_channel(true);
    // dma_channel_config c = dma_channel_get_default_config(epd_dma);
    // channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    // channel_config_set_dreq(&c, EPD_SPI_DREQ);
    // channel_config_set_read_increment(&c, true);
    // channel_config_set_write_increment(&c, false);
    // dma_channel_configure(epd_dma, &c, &spi_get_hw(EPD_SPI)->dr, framebuffer,
    // 		sizeof(framebuffer), false);
    // dma_channel_set_irq0_enabled(epd_dma, true);
    // irq_set_exclusive_handler(DMA_IRQ_0, epd_dma_isr);
    // irq_set_enabled(DMA_IRQ_0, true);

    // Reset LCD
    epd_deselect();

    epd_reset();

    epd_busy_high();
    epd_send_cmd(0x00); // PSR
    epd_send_dat(0xEF);
    //epd_send_dat(0x88);
    epd_send_dat(0x08);
    epd_send_cmd(0x01); // PWR
    epd_send_dat(0x37);
    epd_send_dat(0x00);
    epd_send_dat(0x23); // 10V
    epd_send_dat(0x23); // -10V
    epd_send_cmd(0x03); // PFS
    epd_send_dat(0x00);
    epd_send_cmd(0x06); // BTST
    epd_send_dat(0xC7);
    epd_send_dat(0xC7);
    epd_send_dat(0x1D);
    epd_send_cmd(0x30); // PLL
    epd_send_dat(0x3C);
    epd_send_cmd(0x41); // TSE
    epd_send_dat(0x00);
    epd_send_cmd(0x50); // CDI
    epd_send_dat(0x37);
    epd_send_cmd(0x60); // TCON
    epd_send_dat(0x22);
    epd_send_cmd(0x61); // TRES
    epd_send_dat(0x02);
    epd_send_dat(0x58);
    epd_send_dat(0x01);
    epd_send_dat(0xC0);
    epd_send_cmd(0xE3); // PWS
    epd_send_dat(0xAA);
	
	sleep_ms(100);
    epd_send_cmd(0x50); // CDI
    epd_send_dat(0x37);
}

void epd_clear(uint8_t pattern)
{
    epd_send_cmd(0x61); // TRES
    epd_send_dat(0x02);
    epd_send_dat(0x58);
    epd_send_dat(0x01);
    epd_send_dat(0xC0);
    epd_send_cmd(0x10); // DTM1
    if (pattern < 0x10) {
        for(int j = 0; j < EPD_HEIGHT; j++) {
            for(int i = 0; i < EPD_WIDTH / 2; i++) {
                epd_send_dat((pattern << 4) | pattern);
            }
        }
    }
    else if (pattern == 0x10) {
        for (int c = 0; c < 8; c++) {
            for(int j = 0; j < 56; j++) {
                for(int i = 0; i < EPD_WIDTH / 2; i++) {
                    epd_send_dat((c << 4) | c);
                }
            }
        }
    }
    else if (pattern == 0x11) {
        for (int j = 0; j < EPD_HEIGHT; j++) {
            for(int i = 0; i < EPD_WIDTH / 2; i++) {
                int c = i * 2 / 75;
                epd_send_dat((c << 4) | c);
            }
        }
    }
    epd_send_cmd(0x04); // PON
    epd_busy_high();
    printf("Start refresh\n");
    epd_send_cmd(0x12); // DRF
    epd_busy_high();
    printf("Done refreshing\n");
    epd_send_cmd(0x02); // POF
    epd_busy_low();
    sleep_ms(500);
}

void epd_disp_image_field(uint8_t *image)
{
    unsigned long i,j;
    epd_send_cmd(0x61);//Set Resolution setting
    epd_send_dat(0x02);
    epd_send_dat(0x58);
    epd_send_dat(0x01);
    epd_send_dat(0xC0);
    epd_send_cmd(0x10);
    epd_send_buffer(image, EPD_BUFLEN);
    epd_send_cmd(0x04); // PON
    epd_busy_high();
    printf("Start refresh\n");
    epd_send_cmd(0x12); // DRF
    epd_busy_high();
    printf("Done refreshing\n");
    epd_send_cmd(0x02); // POF
    epd_busy_low();
	sleep_ms(200);
	
}

void epd_color_field(uint8_t field)
{
    unsigned long i,j;
    epd_send_cmd(0x61);//Set Resolution setting
    epd_send_dat(0x02);
    epd_send_dat(0x58);
    epd_send_dat(0x01);
    epd_send_dat(0xC0);
    epd_send_cmd(0x10);
    for (int j = 0; j < EPD_HEIGHT; j++) {
        for(int i = 0; i < EPD_WIDTH / 2; i++) {
            uint8_t color;
            if (field == 0) {
                color = (j / 224) * 4 + (i / 75);
            }
            else if (field == 1) {
                color = (i % 75) / 9;
                if (color > 7) color = 7;
            }
            else if (field == 2) {
                color = (j % 224) / 28;
            }
            epd_send_dat((color << 4) | color);
        }
    }
    epd_send_cmd(0x04); // PON
    epd_busy_high();
    printf("Start refresh\n");
    epd_send_cmd(0x12); // DRF
    epd_busy_high();
    printf("Done refreshing\n");
    epd_send_cmd(0x02); // POF
    epd_busy_low();
	sleep_ms(200);
}

static void epd_use_flash_waveform() {
    // Load waveform from flash
    epd_send_cmd(0x00); // PSR
    epd_send_dat(0xEF);
    epd_send_dat(0x08);
}

static void epd_use_reg_waveform() {
    // Use waveform from register
    epd_send_cmd(0x00); // PSR
    epd_send_dat(0xEF);
    epd_send_dat(0x88);
}

void epd_disp_palette_3bpp(void) {
    epd_use_flash_waveform();
    epd_color_field(0);
}

void epd_disp_palette_6bpp(void) {
    epd_use_flash_waveform();
    epd_clear(0x10);

    epd_use_reg_waveform();
    epd_load_lut_set(lut0);
    epd_clear(0x11);
}

void epd_disp_palette_9bpp(void) {
    epd_use_flash_waveform();
    epd_color_field(0);
    
    epd_use_reg_waveform();
    epd_load_lut_set(lut1);
    epd_color_field(1);
    
    epd_load_lut_set(lut0);
    epd_color_field(2);
}

void epd_disp_image_3bpp(uint8_t *image) {
    epd_use_flash_waveform();
    epd_disp_image_field(image);
}

void epd_disp_image_6bpp(uint8_t *image) {
    epd_use_flash_waveform();
    epd_disp_image_field(image);

    epd_use_reg_waveform();
    epd_load_lut_set(lut0);
    epd_disp_image_field(image + EPD_BUFLEN);
}

void epd_disp_image_9bpp(uint8_t *image) {
    epd_use_flash_waveform();
    epd_disp_image_field(image);

    epd_use_reg_waveform();
    epd_load_lut_set(lut1);
    epd_disp_image_field(image + EPD_BUFLEN);

    epd_load_lut_set(lut0);
    epd_disp_image_field(image + EPD_BUFLEN * 2);
}

void epd_sleep(void)
{
    sleep_ms(100);
    epd_send_cmd(0x07);
    epd_send_dat(0xA5);
    sleep_ms(100);
}

