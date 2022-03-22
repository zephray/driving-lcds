/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "font.h"

#define LCD_WIDTH 255
#define LCD_HEIGHT 64

uint8_t framebuffer[LCD_WIDTH*LCD_HEIGHT];

#define A0_PIN  8
#define RW_PIN  9
#define E_PIN   10
#define CS_PIN  11
#define RST_PIN 12

#define I80

#define LCD_RS_LOW()    gpio_put(A0_PIN, 0);
#define LCD_RS_HIGH()   gpio_put(A0_PIN, 1);
#define LCD_CS_LOW()    gpio_put(CS_PIN, 0);
#define LCD_CS_HIGH()   gpio_put(CS_PIN, 1);
#define LCD_RST_LOW()   gpio_put(RST_PIN, 0);
#define LCD_RST_HIGH()  gpio_put(RST_PIN, 1);
#ifdef M68
#define LCD_E_LOW()     gpio_put(E_PIN, 0);
#define LCD_E_HIGH()    gpio_put(E_PIN, 1);
#define LCD_RW_LOW()    gpio_put(RW_PIN, 0);
#define LCD_RW_HIGH()   gpio_put(RW_PIN, 1);
#elif defined(I80)
#define LCD_RD_LOW()    gpio_put(E_PIN, 0);
#define LCD_RD_HIGH()   gpio_put(E_PIN, 1);
#define LCD_WR_LOW()    gpio_put(RW_PIN, 0);
#define LCD_WR_HIGH()   gpio_put(RW_PIN, 1);
#endif

static void delay(int t) {
    volatile int x = t;
    while (x--);
}

static void lcdBusPut(uint8_t d) {
    gpio_put_masked(0xff, d);
}

#ifdef M68
void lcdWriteCmd(unsigned char c) {
  LCD_RS_LOW();
  LCD_RW_LOW();
  LCD_CS_LOW();
  delay(100);
  lcdBusPut(c);
  LCD_E_HIGH();
  delay(100);
  LCD_E_LOW();
  delay(100);
  LCD_CS_HIGH();
  delay(100);
}

void lcdWriteDat(unsigned char d) {
  LCD_RS_HIGH();
  LCD_RW_LOW();
  LCD_CS_LOW();
  delay(100);
  lcdBusPut(d);
  LCD_E_HIGH();
  delay(100);
  LCD_E_LOW();
  delay(100);
  LCD_CS_HIGH();
  delay(100);
}
#elif defined(I80)
void lcdWriteCmd(unsigned char c) {
  LCD_RS_LOW();
  LCD_RD_HIGH();
  LCD_CS_LOW();
  lcdBusPut(c);
  LCD_WR_LOW();
  delay(1);
  LCD_WR_HIGH();
  LCD_CS_HIGH();
  delay(1);
}

void lcdWriteDat(unsigned char d) {
  LCD_RS_HIGH();
  LCD_RD_HIGH();
  LCD_CS_LOW();
  lcdBusPut(d);
  LCD_WR_LOW();
  delay(1);
  LCD_WR_HIGH();
  LCD_CS_HIGH();
  delay(1);
}
#endif

void lcdInit() {
    LCD_CS_HIGH();
#ifdef M68
    LCD_E_LOW();
#elif defined(I80)
    LCD_RD_HIGH();
    LCD_WR_LOW();
#endif

    LCD_RST_LOW();
    sleep_ms(100);
    LCD_RST_HIGH();
    sleep_ms(100);

    lcdWriteCmd(0x30);        //Ext = 0
    lcdWriteCmd(0x94);        //Sleep Out
    lcdWriteCmd(0xd1);        //OSC On
    lcdWriteCmd(0x20);        //Power Control Set
    lcdWriteDat(0x08);        //Booster Must Be On First
    sleep_ms(20);
    lcdWriteCmd(0x20);         //Power Control Set
    lcdWriteDat(0x0B);        //Booster, Regulator, Follower ON
    sleep_ms(5);
    lcdWriteCmd(0x81);      //Electronic Control
    lcdWriteDat(0x3f);        //Vop=??
    lcdWriteDat(0x03);

    lcdWriteCmd(0xca);      //Display Control
    lcdWriteDat(0x04);
    lcdWriteDat(0x0f);
    lcdWriteDat(0x00);
    
    lcdWriteCmd(0xa6);       // Normal Display (ff is white, 00 is black)

    lcdWriteCmd(0xbb);       //COM Scan Direction
    lcdWriteDat(0x00);        // 0->79 159->80;
    
    lcdWriteCmd(0xbc);       //Data Scan Direction
    lcdWriteDat(0x02);        //Column Reverse
    lcdWriteDat(0x01);        //Inverse 3 Pixels Arrangement
    lcdWriteDat(0x02);        //3Byte 3Pixel mode
    
    lcdWriteCmd(0x75);        //Line Address Set(lines from 96 to 159 are used)
    lcdWriteDat(0x00);        //Start Line=0
    lcdWriteDat(0x3f);        //End Line =64-1
    
    lcdWriteCmd(0x15);        //Column Address Set
    lcdWriteDat(0x00);        //Start Column=0
    lcdWriteDat(0x54);        //End Column =84 ((84+1)*3 == 255)
    
    lcdWriteCmd(0x31);        //Ext = 1

    lcdWriteCmd(0x32);        //Analog Circuit Set
    lcdWriteDat(0x00);        //OSC Frequency =000 (Default)
    lcdWriteDat(0x01);        //Booster Efficiency=01(Default)
    lcdWriteDat(0x05);        //Bias=1/9

    lcdWriteCmd(0x20);
    lcdWriteDat(0x02); //0
    lcdWriteDat(0x09); //1
    lcdWriteDat(0x11); //2
    lcdWriteDat(0x13); //3
    lcdWriteDat(0x15); //4
    lcdWriteDat(0x16); //5
    lcdWriteDat(0x17); //6
    lcdWriteDat(0x18); //7
    lcdWriteDat(0x18); //8
    lcdWriteDat(0x19); //9
    lcdWriteDat(0x1a); //a
    lcdWriteDat(0x1b); //b
    lcdWriteDat(0x1c); //c
    lcdWriteDat(0x1d); //d
    lcdWriteDat(0x1d); //e
    lcdWriteDat(0x1f); //f

    lcdWriteCmd(0x21);
    lcdWriteDat(0x02); //0
    lcdWriteDat(0x10); //1
    lcdWriteDat(0x13); //2
    lcdWriteDat(0x15); //3
    lcdWriteDat(0x16); //4
    lcdWriteDat(0x17); //5
    lcdWriteDat(0x18); //6
    lcdWriteDat(0x19); //7
    lcdWriteDat(0x1a); //8
    lcdWriteDat(0x1b); //9
    lcdWriteDat(0x1b); //a
    lcdWriteDat(0x1c); //b
    lcdWriteDat(0x1c); //c
    lcdWriteDat(0x1d); //d
    lcdWriteDat(0x1e); //e
    lcdWriteDat(0x1f); //f

    lcdWriteCmd(0x34);        //Software Initial

#if 0
    /* EXT0 Mode */
    lcdWriteCmd(0x30);
    /* Initial Code "improves the EEPROM internal ACK" */
    lcdWriteCmd(0x07);
    lcdWriteDat(0x19);  /* Fixed Param */
    /* EXT1 Mode */
    lcdWriteCmd(0x31);
    /* EEPROM On */
    lcdWriteCmd(0xcd);
    lcdWriteDat(0x0); /* Read Mode */
    /* 100ms delay */
    sleep_ms(100);
    /* Start the Read */
    lcdWriteCmd(0xfd);
    /* 100ms delay */
    sleep_ms(100);
    /* Exit EEPROM Read */
    lcdWriteCmd(0xcc);
#endif

    lcdWriteCmd(0x30);        //Ext = 0
    lcdWriteCmd(0xaf);        //Display On
    sleep_ms(100);

    /*lcdWriteCmd(0x5c);
    for (int j = 0; j < 64; j++) {
        for (int i = 0; i < 85; i++) {
            lcdWriteDat(0xff);
            lcdWriteDat(0xff);
            lcdWriteDat(0xff);
        }
    }*/
}

void lcdSetContrast(uint16_t contrast) {
    lcdWriteCmd(0x81);      //Electronic Control
    lcdWriteDat(contrast & 0x3f);        //Vop=??
    lcdWriteDat((contrast >> 6) & 0x7);
}

static void lcdSetPixel(int x, int y, uint8_t c) {
    framebuffer[y * LCD_WIDTH + x] = c;
}

void lcdClear() {
    memset(framebuffer, 0xff, LCD_WIDTH*LCD_HEIGHT);
}

void lcdDispChar(int x, int y, char c, uint8_t fg, uint8_t bg) {
    if (c < 0x20)
        return;
    c -= 0x20;
    for (int yy = 0; yy < 7; yy++) {
        if ((y + yy) < 0) continue;
        if ((y + yy) >= LCD_HEIGHT) continue;
        for (int xx = 0; xx < 5; xx++) {
            if ((x + xx) < 0) continue;
            if ((x + xx) >= LCD_WIDTH) continue;
            if ((font[c * 5 + xx] >> yy) & 0x01) {
                lcdSetPixel(x + xx, y + yy, fg);
            }
            else {
                lcdSetPixel(x + xx, y + yy, bg);
            }
        }
    }
}

void lcdDispString(int x, int y, char *str, uint8_t fg, uint8_t bg) {
    while (*str) {
        lcdDispChar(x, y, *str++, fg, bg);
        x += 6;
        if ((x + 6) > LCD_WIDTH) {
            y += 8;
            x = 0;
        }
    }
}

int lcdPrintf(int x, int y, uint8_t fg, uint8_t bg, const char *format, ...)
{
    char printf_buffer[100];

    int length = 0;

    va_list ap;
    va_start(ap, format);

    length = vsnprintf(printf_buffer, 100, format, ap);

    va_end(ap);

    lcdDispString(x, y, printf_buffer, fg, bg);

    return length;
}

void lcdUpdate()
{
    lcdWriteCmd(0x5c);
    for (int j = 0; j < 255*64; j++) {
        lcdWriteDat(framebuffer[j]);
    }
}


int main() {
    for (int i = 0; i <= 12; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
    }

    gpio_init(13);
    gpio_set_dir(13, GPIO_OUT);

    lcdInit();
    int i = 0;

    lcdClear();
    for (int x = 0; x < 255; x++) {
        for (int y = 8; y < 64; y++) {
            lcdSetPixel(x, y, x);
        }
    }
    lcdSetContrast(250);
    //lcdPrintf(0, 0, 0x00, 0xff, "Hello, world!");
    char label[] = "0123456789ABCDEF0123456789ABCDEF";
    for (int x = 0; x < 32; x++) {
        lcdDispChar(x * 8, 0, label[x], 0x00, 0xff);
    }

    lcdUpdate();


    while(1) {
        /*for (int contrast = 0x80; contrast < 0x1ff; contrast++) {
            gpio_put(13, 1);
            sleep_ms(50);
            gpio_put(13, 0);
            sleep_ms(50);
            
            //lcdDispString(0, 0, "Hello, world!", 0x00, 0xff);
            lcdSetContrast(contrast);
            lcdPrintf(0, 0, 0x00, 0xff, "Hello, world! %d", contrast);
            lcdUpdate();
        }*/
        gpio_put(13, 1);
        sleep_ms(500);
        gpio_put(13, 0);
        sleep_ms(500);
    }
}
