//
// This code is based on https://yuanze.wang/posts/st7049a-rgb-backlight-tn-lcd
//
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "lcd.h"

// Hardware Connection
#define LCD_SPI  spi1
#define LCD_SPI_DREQ DREQ_SPI1_TX
#define LCD_RS   8
#define LCD_CS   9
#define LCD_SCK  10
#define LCD_MOSI 11
#define LCD_SPI_FREQ (4*1000*1000)

#define LCD_DATA_SIZE   (320*4/2) //1个字节包含2个像素的数据

static uint8_t lcd_gram[LCD_DATA_SIZE];

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

void lcd_clear() {
    memset(lcd_gram, 0, sizeof(lcd_gram));
}

void lcd_send_buffer() {
    lcd_send_cmd(0x2A); //设置Column地址
    lcd_send_dat(0);
    lcd_send_dat(159);
    lcd_send_cmd(0x2B); //设置Row地址
    lcd_send_dat(0);
    lcd_send_dat(3);
    lcd_send_cmd(0x2C); //写RAM
    lcd_mode_data();
    lcd_select();
#if 1
    spi_write_blocking(LCD_SPI, lcd_gram, sizeof(lcd_gram));
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
// static void lcd_dma_isr() {
// 	dma_hw->ints0 = 1u << lcd_dma;
// 	// Wait till the FIFO is drained and all bytes are sent
// 	while ((spi_get_hw(LCD_SPI)->sr & SPI_SSPSR_BSY_BITS));
// 	lcd_deselect();
// 	lcd_busy = false;
// 	if (lcd_update_req) {
// 		lcd_update_req = 0;
// 		lcd_send_buffer();
// 	}
// }

void lcd_init(void) {
    // Configure Pins
	gpio_set_function(LCD_RS,   GPIO_FUNC_SIO);
    gpio_set_function(LCD_CS,   GPIO_FUNC_SIO);
    gpio_set_function(LCD_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI, GPIO_FUNC_SPI);

	lcd_deselect();
	gpio_set_dir(LCD_RS,  GPIO_OUT);
	gpio_set_dir(LCD_CS,  GPIO_OUT);

    // Configure SPI
    spi_init(LCD_SPI, LCD_SPI_FREQ);

    // Set up DMA
	// lcd_dma = dma_claim_unused_channel(true);
	// dma_channel_config c = dma_channel_get_default_config(lcd_dma);
	// channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
	// channel_config_set_dreq(&c, LCD_SPI_DREQ);
	// channel_config_set_read_increment(&c, true);
	// channel_config_set_write_increment(&c, false);
	// dma_channel_configure(lcd_dma, &c, &spi_get_hw(LCD_SPI)->dr, framebuffer,
	// 		sizeof(framebuffer), false);
	// dma_channel_set_irq0_enabled(lcd_dma, true);
	// irq_set_exclusive_handler(DMA_IRQ_0, lcd_dma_isr);
	// irq_set_enabled(DMA_IRQ_0, true);

    lcd_deselect();

    // Send initialization sequence 
    lcd_send_cmd(0x11); //退出休眠模式

    lcd_send_cmd(0xD2); //设置电源模式
    lcd_send_dat(0x00); //开启所有电源

    lcd_send_cmd(0xC0); //设置LCD面板驱动电压
    //5.6V Vop = 0.04*140 + 3*0 可以调整对比度
    lcd_send_dat(140);
    lcd_send_dat(0);

    lcd_send_cmd(0xB0); //设置Duty
    lcd_send_dat(0x03); //4 Duty

    lcd_send_cmd(0xB2); //设置扫描频率
    lcd_send_dat(0x10); //设置扫描频率为100Hz 0x10-0x1F代表50Hz到200Hz

    lcd_send_cmd(0xB4);
    lcd_send_dat(0x22);

    lcd_send_cmd(0xB5); //设置LCD驱动模式
    //lcd_send_dat(0x04);//ROE=0, MSE=1, MS=2 fields, L(R/G/B)F=2
    lcd_send_dat(0x04);
    lcd_send_dat(0x01);
    lcd_send_dat(0x01);
    lcd_send_dat(0x01);

    lcd_send_cmd(0xB6); //设置LED驱动波形
    //起始20 宽度200 调整宽度可以调整背光亮度
    lcd_send_dat(20);
    lcd_send_dat(20);
    lcd_send_dat(20);
    lcd_send_dat(200);
    lcd_send_dat(200);
    lcd_send_dat(200);

    lcd_send_cmd(0xB7); //设置LCD扫描方向
    lcd_send_dat(0x40); //MX=1 MY=0 MS=0 BGR=0

    lcd_send_cmd(0x29); //开显示
}

void lcd_test(uint8_t a) {
    lcd_send_cmd(0xB6); //设置LED驱动波形
    //起始20 宽度200 调整宽度可以调整背光亮度
    lcd_send_dat(a);
    lcd_send_dat(a);
    lcd_send_dat(a);
    lcd_send_dat(63);
    lcd_send_dat(63);
    lcd_send_dat(63);
}

void lcd_set_point(int x, int y, lcd_color_t color) {
    if (x < 0) return;
    if (y < 0) return;
    if (x >= LCD_X_PIXELS) return;
    if (y >= LCD_Y_PIXELS) return;
    uint8_t *gram_target = NULL;

    if (y < LCD_Y_PIXELS/2) { //上半屏比较复杂
        if (x < LCD_X_PIXELS/2) { //左半屏
            if (y == 3 || y == 4) {
                gram_target = &lcd_gram[112] + x;
            } else if (y == 2 || y == 5) {
                gram_target = &lcd_gram[272] + x;
            } else if (y == 1 || y == 6) {
                gram_target = &lcd_gram[432] + x;
            } else if (y == 0 || y == 7) {
                gram_target = &lcd_gram[592] + x;
            }
        } else { //右半屏
            if (y == 3 || y == 4) {
                gram_target = &lcd_gram[16] + (x-LCD_X_PIXELS/2);
            } else if (y == 2 || y == 5) {
                gram_target = &lcd_gram[176] + (x-LCD_X_PIXELS/2);
            } else if (y == 1 || y == 6) {
                gram_target = &lcd_gram[336] + (x-LCD_X_PIXELS/2);
            } else if (y == 0 || y == 7) {
                gram_target = &lcd_gram[496] + (x-LCD_X_PIXELS/2);
            }
        }
    } else { //是下半屏
        if (y == 11 || y == 12) {
            gram_target = &lcd_gram[48] + (LCD_X_PIXELS-1-x);
        } else if (y == 10 || y == 13) {
            gram_target = &lcd_gram[208] + (LCD_X_PIXELS-1-x);
        } else if (y == 9 || y == 14) {
            gram_target = &lcd_gram[368] + (LCD_X_PIXELS-1-x);
        } else if (y == 8 || y == 15) {
            gram_target = &lcd_gram[528] + (LCD_X_PIXELS-1-x);
        }
        y -= LCD_Y_PIXELS/2; //方便后续处理
    }

    if (x < LCD_X_PIXELS/2) { //是左半屏
        if (y < 4) { //前半个像素
            *gram_target &= 0xF0;
            *gram_target |= color;
        } else { //后半个像素
            *gram_target &= 0x0F;
            *gram_target |= color<<4;
        }
    } else { //是右半屏
        if (y < 4) { //前半个像素
            *gram_target &= 0x0F;
            *gram_target |= color<<4;
        } else { //后半个像素
            *gram_target &= 0xF0;
            *gram_target |= color;
        }
    }
}
