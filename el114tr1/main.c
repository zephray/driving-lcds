#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include "font.h"

const char text[] = 
"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. A diam maecenas sed enim ut sem viverra aliquet eget. Est ante in nibh mauris cursus mattis molestie a. Sed odio morbi quis commodo odio aenean sed adipiscing. Aliquam ut porttitor leo a diam sollicitudin tempor. Suspendisse interdum consectetur libero id faucibus nisl tincidunt. Ut tortor pretium viverra suspendisse potenti nullam ac. Mauris rhoncus aenean vel elit scelerisque. Nunc non blandit massa enim nec dui. Convallis tellus id interdum velit laoreet id donec.\n"
"Eleifend quam adipiscing vitae proin sagittis nisl rhoncus mattis. Scelerisque purus semper eget duis. Sit amet consectetur adipiscing elit ut aliquam purus sit amet. Vestibulum sed arcu non odio euismod lacinia. Elit ullamcorper dignissim cras tincidunt lobortis. Mauris rhoncus aenean vel elit scelerisque mauris. Dolor sed viverra ipsum nunc aliquet bibendum. Blandit cursus risus at ultrices mi tempus imperdiet nulla malesuada. Et molestie ac feugiat sed lectus vestibulum mattis. Viverra accumsan in nisl nisi scelerisque. Pulvinar mattis nunc sed blandit libero volutpat sed. Cras fermentum odio eu feugiat pretium. Arcu cursus euismod quis viverra nibh. Mauris ultrices eros in cursus turpis massa. Feugiat nisl pretium fusce id velit ut tortor pretium viverra. Ultricies leo integer malesuada nunc vel. Aliquet nec ullamcorper sit amet risus nullam eget. Turpis egestas pretium aenean pharetra.\n"
"Nunc scelerisque viverra mauris in aliquam sem fringilla. Pulvinar neque laoreet suspendisse interdum consectetur libero id faucibus. Mauris pharetra et ultrices neque ornare aenean euismod elementum. Consequat mauris nunc congue nisi vitae. Proin sagittis nisl rhoncus mattis rhoncus urna. Volutpat commodo sed egestas egestas fringilla phasellus faucibus. Donec et odio pellentesque diam volutpat commodo sed egestas egestas. Libero id faucibus nisl tincidunt eget nullam non nisi. Lorem dolor sed viverra ipsum nunc aliquet bibendum enim facilisis. Et pharetra pharetra massa massa. Feugiat in ante metus dictum at tempor commodo. Pharetra convallis posuere morbi leo urna molestie at elementum. Quis auctor elit sed vulputate mi sit amet mauris. Pulvinar neque laoreet suspendisse interdum consectetur libero. Semper risus in hendrerit gravida rutrum quisque non tellus orci. Quam id leo in vitae turpis massa sed elementum.\n"
"Magna ac placerat vestibulum lectus mauris. Mauris cursus mattis molestie a iaculis at erat pellentesque adipiscing. Mi ipsum faucibus vitae aliquet. Rhoncus urna neque viverra justo nec ultrices dui sapien. Et molestie ac feugiat sed lectus vestibulum mattis ullamcorper velit. Enim blandit volutpat maecenas volutpat. Nam aliquam sem et tortor consequat id porta nibh. Mattis molestie a iaculis at erat pellentesque adipiscing commodo. Tristique senectus et netus et malesuada fames. Tempus imperdiet nulla malesuada pellentesque elit eget gravida. Interdum velit laoreet id donec.\n"
"In cursus turpis massa tincidunt dui ut ornare lectus sit. Elementum nisi quis eleifend quam. Id semper risus in hendrerit. Tristique et egestas quis ipsum suspendisse ultrices. Sagittis id consectetur purus ut faucibus pulvinar elementum integer. Purus sit amet volutpat consequat mauris nunc congue. Amet dictum sit amet justo donec enim diam vulputate. Auctor urna nunc id cursus metus. Mattis enim ut tellus elementum sagittis vitae. Congue mauris rhoncus aenean vel elit. Lacus suspendisse faucibus interdum posuere lorem. Mauris in aliquam sem fringilla ut morbi tincidunt. Risus ultricies tristique nulla aliquet enim tortor at auctor urna."
"Massa vitae tortor condimentum lacinia quis vel eros donec. Quisque id diam vel quam elementum pulvinar etiam. Tempor nec feugiat nisl pretium fusce. Sed adipiscing diam donec adipiscing tristique risus nec. Arcu felis bibendum ut tristique. Aliquam vestibulum morbi blandit cursus risus at ultrices mi. Diam phasellus vestibulum lorem sed. Sit amet commodo nulla facilisi nullam vehicula. Enim neque volutpat ac tincidunt. Lectus sit amet est placerat in egestas erat imperdiet sed. Pellentesque sit amet porttitor eget dolor morbi non arcu risus. Interdum posuere lorem ipsum dolor. In metus vulputate eu scelerisque felis. Ipsum faucibus vitae aliquet nec. Id aliquet risus feugiat in. Lorem sed risus ultricies tristique nulla aliquet. Morbi enim nunc faucibus a pellentesque sit amet. Tempus egestas sed sed risus pretium quam. Et malesuada fames ac turpis egestas.\n"
"Dui nunc mattis enim ut tellus. Odio ut enim blandit volutpat maecenas. Molestie at elementum eu facilisis sed odio morbi quis. Pulvinar elementum integer enim neque volutpat ac tincidunt. Donec pretium vulputate sapien nec sagittis aliquam. Elementum sagittis vitae et leo. In hac habitasse platea dictumst quisque sagittis purus sit. Cras semper auctor neque vitae tempus. Platea dictumst vestibulum rhoncus est pellentesque elit. Adipiscing at in tellus integer feugiat. Nulla pharetra diam sit amet nisl. Et leo duis ut diam.\n"
"Placerat orci nulla pellentesque dignissim enim sit amet. Sagittis aliquam malesuada bibendum arcu. Lacus luctus accumsan tortor posuere ac. Et magnis dis parturient montes nascetur ridiculus mus. Cras sed felis eget velit aliquet. Velit ut tortor pretium viverra suspendisse potenti. Leo a diam sollicitudin tempor id eu nisl nunc. Elementum integer enim neque volutpat ac tincidunt vitae semper quis. Sodales ut eu sem integer. Facilisis leo vel fringilla est.";

#define EPD_WIDTH   1600
#define EPD_HEIGHT  240
#define EPD_FB_SIZE (EPD_WIDTH*EPD_HEIGHT/8)

uint8_t framebuffer[EPD_FB_SIZE];

extern const char image[];

#define BS_PIN          0
#define DC_PIN          1
#define SCL_PIN         2   // Important: SPI0 SCK
#define SDA_PIN         3   // Important: SPI0 TX
#define S3CS_PIN        4
#define S2CS_PIN        5
#define MCS_PIN         6
#define SCS_PIN         7
#define S3BUSY_PIN      8
#define S2BUSY_PIN      9
#define SBUSY_PIN       10
#define MBUSY_PIN       11
#define RST_PIN         12

#define EPD_SPI         spi0
#define EPD_SPI_DREQ    DREQ_SPI0_TX
#define EPD_SPI_FREQ    (8*1000*1000)

#define ID_ALL          -1
//#define ID_ALL          0
#define ID_MASTER       0
#define ID_SLAVE1       1
#define ID_SLAVE2       2
#define ID_SLAVE3       3

static const int BUSY_PINS[] = {MBUSY_PIN, SBUSY_PIN, S2BUSY_PIN, S3BUSY_PIN};
static const int CS_PINS[] = {MCS_PIN, SCS_PIN, S2CS_PIN, S3CS_PIN};

static void delay(int t) {
    volatile int x = t;
    while (x--);
}

static void epd_select_all(void) {
    for (int id = 0; id < 4; id++) {
        gpio_put(CS_PINS[id], 0);
    }
}

static void epd_deselect_all(void) {
    for (int id = 0; id < 4; id++) {
        gpio_put(CS_PINS[id], 1);
    }
}

static void epd_select(int id) {
    if (id == -1)
        epd_select_all();
    else
        gpio_put(CS_PINS[id], 0);
}

static void epd_deselect(int id) {
    if (id == -1)
        epd_deselect_all();
    else
        gpio_put(CS_PINS[id], 1);
}

static void epd_mode_command(void) {
    gpio_put(DC_PIN, 0);
}

static void epd_mode_data(void) {
    gpio_put(DC_PIN, 1);
}

static void epd_reset(void) {
    gpio_put(RST_PIN, 0);
    sleep_ms(200);
    gpio_put(RST_PIN, 1);
    sleep_ms(20);
}

static void epd_send_byte(int id, uint8_t byte) {
    epd_select(id);
    spi_write_blocking(EPD_SPI, &byte, 1);
    epd_deselect(id);
}

static void epd_send_cmd(int id, uint8_t cmd) {
    epd_mode_command();
    epd_send_byte(id, cmd);
}

static void epd_send_dat(int id, uint8_t dat) {
    epd_mode_data();
    epd_send_byte(id, dat);
}

static void epd_send_buffer(int id, uint8_t *buf, uint32_t length) {
    epd_mode_data();
    epd_select(id);
#if 1
    spi_write_blocking(EPD_SPI, buf, length);
    epd_deselect(id);
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

static void epd_wait_busy(int id) {
    sleep_ms(1);
#if 0
    if (id == -1) {
        for (int i = 0; i < 4; i++) {
            epd_wait_busy(i);
        }
    }
    else {
        while (gpio_get(BUSY_PINS[id]) == 0) {
            sleep_ms(1);
        }
    }
#else 
    for (int i = 0; i < 4; i++) {
        while (gpio_get(BUSY_PINS[i]) == 0) {
            sleep_ms(1);
        }
    }
#endif
}

const unsigned char lut_vcom1[] ={
    0x00,0x19,0x01,0x00,0x00,0x01,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,
};

const unsigned char lut_ww1[] ={
    0x00,0x19,0x01,0x00,0x00,0x01,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00
};

const unsigned char lut_bw1[] ={
    0x80,0x19,0x01,0x00,0x00,0x01,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00
};

const unsigned char lut_wb1[] ={
    0x40,0x19,0x01,0x00,0x00,0x01,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00
};

const unsigned char lut_bb1[] ={
    0x00,0x19,0x01,0x00,0x00,0x01,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00
};


void lut1(void)
{
    unsigned int count;
    epd_send_cmd(ID_ALL, 0x20);
    for(count=0;count<44;count++) epd_send_dat(ID_ALL, lut_vcom1[count]);

    epd_send_cmd(ID_ALL, 0x21);
    for(count=0;count<42;count++) epd_send_dat(ID_ALL, lut_ww1[count]);  

    epd_send_cmd(ID_ALL, 0x22);
    for(count=0;count<42;count++) epd_send_dat(ID_ALL, lut_bw1[count]);

    epd_send_cmd(ID_ALL, 0x23);
    for(count=0;count<42;count++) epd_send_dat(ID_ALL, lut_wb1[count]);

    epd_send_cmd(ID_ALL, 0x24);
    for(count=0;count<42;count++) epd_send_dat(ID_ALL, lut_bb1[count]);
}

void epd_init() {
    // Configure Pins
    gpio_init(BS_PIN);
    gpio_init(DC_PIN);
    gpio_init(RST_PIN);
    for (int id = 0; id < 4; id++) {
        gpio_init(CS_PINS[id]);
        gpio_init(BUSY_PINS[id]);
    }
    gpio_set_function(SCL_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SDA_PIN, GPIO_FUNC_SPI);

    epd_deselect_all();
    gpio_put(BS_PIN, 0); // 4-wire interface
    gpio_set_dir(BS_PIN,  GPIO_OUT);
    gpio_set_dir(DC_PIN,  GPIO_OUT);
    gpio_set_dir(RST_PIN, GPIO_OUT);
    for (int id = 0; id < 4; id++) {
        gpio_set_dir(CS_PINS[id], GPIO_OUT);
        gpio_set_dir(BUSY_PINS[id], GPIO_IN);
    }

    // Configure SPI
    spi_init(EPD_SPI, EPD_SPI_FREQ);

    epd_reset();

    // Send initialization sequence
    //epd_send_cmd(ID_MASTER, 0xe0);
    //epd_send_dat(ID_MASTER, 0x01); // master mode operation
    //sleep_ms(100);

    epd_send_cmd(ID_ALL, 0x00); // panel setting
    epd_send_dat(ID_ALL, 0xbf);
    //epd_send_dat(ID_ALL, 0x1f);
    epd_send_dat(ID_ALL, 0x0d);

    //epd_send_cmd(ID_ALL, 0x30);
    //epd_send_dat(ID_ALL, 0x3c);

    //epd_send_cmd(ID_ALL, 0x82); // vcom
    //epd_send_dat(ID_ALL, 0x00);

    epd_send_cmd(ID_ALL, 0x61); // resolution setting
    epd_send_dat(ID_ALL, 400 / 256);
    epd_send_dat(ID_ALL, 400 % 256);
    epd_send_dat(ID_ALL, 300 / 256);
    epd_send_dat(ID_ALL, 300 % 256);

    epd_send_cmd(ID_ALL, 0xe0); // cascade settings
    epd_send_dat(ID_ALL, 0x03);
    epd_send_cmd(ID_ALL, 0xe5);
    epd_send_dat(ID_ALL, 0x10);

    lut1();

    epd_send_cmd(ID_MASTER, 0x04);
    epd_wait_busy(ID_ALL);

    // epd_send_cmd(ID_ALL, 0x50);
    // epd_send_dat(ID_ALL, 0x97);
}

void epd_refresh() {
    epd_send_cmd(ID_ALL, 0x12);
    epd_wait_busy(ID_ALL);
}

void epd_sleep() {
    epd_send_cmd(ID_ALL, 0x50);
    epd_send_dat(ID_ALL, 0xf7);
    
    epd_send_cmd(ID_ALL, 0x02); // power off
    epd_wait_busy(ID_ALL);
    epd_send_cmd(ID_ALL, 0x07); // deep sleep
    epd_send_dat(ID_ALL, 0xa5);
}

void epd_clean(uint8_t old, uint8_t new) {
    memset(framebuffer, old, EPD_FB_SIZE);
    epd_send_cmd(ID_ALL, 0x10);
    epd_send_buffer(ID_ALL, framebuffer, 15000);
    memset(framebuffer, new, EPD_FB_SIZE);
    epd_send_cmd(ID_ALL, 0x13);
    epd_send_buffer(ID_ALL, framebuffer, 15000);
}

void epd_update() {
    static uint8_t empty[1000];
    memset(empty, 0xff, 1000);
    epd_send_cmd(ID_ALL, 0x10);
    for (int i = 0; i < 15; i++) {
        epd_send_buffer(ID_ALL, empty, 1000);
    }

    for (int i = 0; i < 4; i++) {
        epd_send_cmd(i, 0x13);
        uint32_t ram_offset = (i == 0 ? 1 : (i == 1) ? 0 : i) * 400 / 8;
        uint8_t *p = image + ram_offset;
        for (int j = 0; j < 240; j++) {
            epd_send_buffer(i, p, 400 / 8);
            p += EPD_WIDTH / 8;
        }
        
    }
}

static void epd_set_pixel(int x, int y, uint8_t c) {
    int id = x / 400;
    size_t offset = id * 400 * EPD_HEIGHT / 8 + y * 50 + x / 8;
    int b = x % 8;
    framebuffer[offset] &= ~(0x80 >> b) ;
    if (c)
        framebuffer[offset] |= 0x80 >> b;
}

void epd_disp_char(int x, int y, char c, uint8_t fg, uint8_t bg) {
    if (c < 0x20)
        return;
    c -= 0x20;
    for (int yy = 0; yy < 7; yy++) {
        if ((y + yy) < 0) continue;
        if ((y + yy) >= EPD_HEIGHT) continue;
        for (int xx = 0; xx < 5; xx++) {
            if ((x + xx) < 0) continue;
            if ((x + xx) >= EPD_WIDTH) continue;
            if ((font[c * 5 + xx] >> yy) & 0x01) {
                epd_set_pixel(x + xx, y + yy, fg);
            }
            else {
                epd_set_pixel(x + xx, y + yy, bg);
            }
        }
    }
}

void epd_disp_string(int x, int y, char *str, uint8_t fg, uint8_t bg) {
    while (*str) {
        epd_disp_char(x, y, *str++, fg, bg);
        x += 6;
        if ((x + 6) > EPD_WIDTH) {
            y += 8;
            x = 0;
        }
    }
}

int main() {
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, 1);

    epd_init();
    gpio_put(25, 0);

    epd_clean(0xff, 0x00);
    gpio_put(25, 1);

    epd_refresh();
    gpio_put(25, 0);


    epd_clean(0x00, 0xff);
    gpio_put(25, 1);

    epd_refresh();
    gpio_put(25, 0);

    //epd_disp_string(0, 0, text, 0, 1);
    epd_update();
    gpio_put(25, 1);
    epd_refresh();
    gpio_put(25, 0);

    while(1) {
        //
        gpio_put(25, 0);
        sleep_ms(500);
        gpio_put(25, 1);
        sleep_ms(500);
        
    }
}
