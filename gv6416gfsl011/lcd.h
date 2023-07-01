//
// This code is based on https://yuanze.wang/posts/st7049a-rgb-backlight-tn-lcd
//
#pragma once

#define LCD_X_PIXELS    64
#define LCD_Y_PIXELS    16

typedef enum {
    lcd_color_white = 0,
    lcd_color_yellow = 1,
    lcd_color_pink = 2,
    lcd_color_red = 3,
    lcd_color_cyan = 4,
    lcd_color_green = 5,
    lcd_color_blue = 6,
    lcd_color_black = 7,
    lcd_color_max
} lcd_color_t;

void lcd_init(void);
void lcd_set_point(int x, int y, lcd_color_t color);
void lcd_send_buffer();
void lcd_clear();
void lcd_test(uint8_t a);
