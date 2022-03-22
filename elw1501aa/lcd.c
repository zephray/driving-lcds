//
// Copyright 2022 Wenting Zhang <zephray@outlook.com>
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
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "lcd.h"

// Hardware Connection
#define LCD_SPI  spi1
#define LCD_SPI_DREQ DREQ_SPI1_TX
#define LCD_RST  7
#define LCD_RS   8
#define LCD_CS   9
#define LCD_SCK  10
#define LCD_MOSI 11
#define LCD_SPI_FREQ (8*1000*1000)

uint8_t framebuffer[LCD_WIDTH * LCD_HEIGHT / 2];
static int lcd_dma;
static volatile bool lcd_busy;
static volatile bool lcd_update_req;

static void lcd_select(void) {
    gpio_put(LCD_CS, 0);
}

static void lcd_deselect(void) {
    gpio_put(LCD_CS, 1);
}

static void lcd_mode_command(void) {
    gpio_put(LCD_RS, 0);
}

static void lcd_mode_data(void) {
    gpio_put(LCD_RS, 1);
}

static void lcd_reset(void) {
    gpio_put(LCD_RST, 0);
    sleep_ms(200);
    gpio_put(LCD_RST, 1);
    sleep_ms(20);
}

static void lcd_send_byte(uint8_t byte) {
    lcd_select();
    spi_write_blocking(LCD_SPI, &byte, 1);
    lcd_deselect();
}

static void lcd_send_cmd(uint8_t cmd) {
    lcd_mode_command();
    lcd_send_byte(cmd);
}

static void lcd_send_dat(uint8_t dat) {
    lcd_mode_data();
    lcd_send_byte(dat);
}

static void lcd_send_word(uint16_t word) {
    lcd_mode_data();
    lcd_send_byte(word >> 8);
    lcd_send_byte(word);
}

static void lcd_send_buffer() {
    lcd_mode_data();
    lcd_select();
#if 0
    spi_write_blocking(LCD_SPI, framebuffer, sizeof(framebuffer));
    lcd_deselect();
#else
	// Don't start until the last transmission has finished
	lcd_update_req = 1;
	if (!lcd_busy) {
		lcd_update_req = 0;
		lcd_busy = true;
		dma_channel_set_read_addr(lcd_dma, framebuffer, false);
		dma_channel_set_trans_count(lcd_dma, sizeof(framebuffer), true);
	}
#endif
}

// This interrupt should be at the lowest priority
static void lcd_dma_isr() {
	dma_hw->ints0 = 1u << lcd_dma;
	// Wait till the FIFO is drained and all bytes are sent
	while ((spi_get_hw(LCD_SPI)->sr & SPI_SSPSR_BSY_BITS));
	lcd_deselect();
	lcd_busy = false;
	if (lcd_update_req) {
		lcd_update_req = 0;
		lcd_send_buffer();
	}
}

void lcd_init(void) {
    // Configure Pins
	gpio_set_function(LCD_RST,  GPIO_FUNC_SIO);
	gpio_set_function(LCD_RS,   GPIO_FUNC_SIO);
    gpio_set_function(LCD_CS,   GPIO_FUNC_SIO);
    gpio_set_function(LCD_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI, GPIO_FUNC_SPI);

	lcd_deselect();
	gpio_set_dir(LCD_RST, GPIO_OUT);
	gpio_set_dir(LCD_RS,  GPIO_OUT);
	gpio_set_dir(LCD_CS,  GPIO_OUT);

    // Configure SPI
    spi_init(LCD_SPI, LCD_SPI_FREQ);

    // Set up DMA
	lcd_dma = dma_claim_unused_channel(true);
	dma_channel_config c = dma_channel_get_default_config(lcd_dma);
	channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
	channel_config_set_dreq(&c, LCD_SPI_DREQ);
	channel_config_set_read_increment(&c, true);
	channel_config_set_write_increment(&c, false);
	dma_channel_configure(lcd_dma, &c, &spi_get_hw(LCD_SPI)->dr, framebuffer,
			sizeof(framebuffer), false);
	dma_channel_set_irq0_enabled(lcd_dma, true);
	irq_set_exclusive_handler(DMA_IRQ_0, lcd_dma_isr);
	irq_set_enabled(DMA_IRQ_0, true);

    // Reset LCD
    lcd_deselect();

    lcd_reset();

    // Send initialization sequence 
	lcd_send_cmd(0xfd);	// unlock controller
	lcd_send_cmd(0x12);
	lcd_send_cmd(0x81);	// set contrast control
	lcd_send_cmd(0x9c);
	lcd_send_cmd(0xa0);	// set segment remap
	lcd_send_cmd(0x53);
	lcd_send_cmd(0xa1);	// set display start line
	lcd_send_cmd(0x00);
	lcd_send_cmd(0xa2);	// set display offset
	lcd_send_cmd(0x00);
	lcd_send_cmd(0xa4);	// set normal display mode
	lcd_send_cmd(0xa8);	// set mux ratio
	lcd_send_cmd(0x7f);
	lcd_send_cmd(0xab);	// set enable internal VDD regulator mode
	lcd_send_cmd(0x01);
	lcd_send_cmd(0xb1);	// set phase length of phase 1, phase 2
	lcd_send_cmd(0x31);
	lcd_send_cmd(0xb3);	// set ratio of dividing freq & oscillation freq
	lcd_send_cmd(0xf1);
	lcd_send_cmd(0xb9);	// set greyscale table
	lcd_send_cmd(0xbc);	// set pre-charge voltage
	lcd_send_cmd(0x07);
	lcd_send_cmd(0xbe);	// set voltage vcomh
	lcd_send_cmd(0x07);
	lcd_send_cmd(0x15);	// set column address
	lcd_send_cmd(0x00);
	lcd_send_cmd(0x3f);
	lcd_send_cmd(0x75);	// set row address
	lcd_send_cmd(0x00);
	lcd_send_cmd(0x7f);

	lcd_send_cmd(0xaf);	// Display On

	lcd_busy = false;
	lcd_update_req = false;
}

void lcd_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    lcd_send_cmd(0x15);
	lcd_send_cmd(x1 / 2);
	lcd_send_cmd(x2 / 2);
    lcd_send_cmd(0x75);
    lcd_send_cmd(y1);
    lcd_send_cmd(y2);
}

void lcd_update(void) {
    lcd_send_buffer();
}
