#ifndef __ST7637_H__
#define __ST7637_H__

#include "main.h" 

sbit LCD_CSB = P3^2;
sbit LCD_RSB = P3^4;
sbit LCD_RDB = P3^6;
sbit LCD_WRB = P3^5;
sbit LCD_RST = P3^3;

#define LCD_DataBus P2

#define RGB(color) ((((color) >> 19) & 0x1f) << 11) \
                                            |((((color) >> 10) & 0x3f) << 5) \
                                            |(((color) >> 3) & 0x1f)
																						
#define LCD_OFFSET_X	4   
#define LCD_OFFSET_Y	4  

//底层操作函数
void LCD_WriteCmd(uchar cmd);
void LCD_WriteDat(uchar dat);
//用户函数
void LCD_Init();
void LCD_SetWindow(uchar x0,uchar y0,uchar x1,uchar y1);
void LCD_Fill(uchar x0,uchar y0,uchar x1,uchar y1,uint c);
void LCD_DispBmp(uchar x0,uchar y0,uchar x1,uchar y1,uchar *pic);
void LCD_Point(uchar x,uchar y,uint c);

#endif