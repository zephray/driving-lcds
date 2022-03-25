#include "main.h"
#include "lcd.h"
#include "delay.h"


extern uint16_t LCD_CS;
extern uint16_t LCD_RST;
extern uint16_t LCD_RS;
extern uint16_t LCD_WR;
extern uint16_t LCD_RD;

#define LCD_CS_LOW()    GPIOD->BRR  = LCD_CS
#define LCD_CS_HIGH()   GPIOD->BSRR = LCD_CS
#define LCD_RST_LOW()   GPIOD->BRR  = LCD_RST
#define LCD_RST_HIGH()  GPIOD->BSRR = LCD_RST
#define LCD_RS_LOW()    GPIOD->BRR  = LCD_RS
#define LCD_RS_HIGH()   GPIOD->BSRR = LCD_RS
#define LCD_WR_LOW()    GPIOD->BRR  = LCD_WR
#define LCD_WR_HIGH()   GPIOD->BSRR = LCD_WR
#define LCD_RD_LOW()    GPIOD->BRR  = LCD_RD
#define LCD_RD_HIGH()   GPIOD->BSRR = LCD_RD


void LCD_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

  GPIO_Init(GPIOE, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                GPIO_Pin_4; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

  GPIO_Init(GPIOD, &GPIO_InitStructure);
}

unsigned char ReverseBits(unsigned char num)
{
    num = ((num&0x0f)<<4) | ((num&0xf0)>>4);
    num = ((num&0x33)<<2) | ((num&0xcc)>>2);
    num = ((num&0x55)<<1) | ((num&0xaa)>>1);
    return num;
}

void LCD_WriteCmd(unsigned char c)
{
  LCD_RS_LOW();
  LCD_CS_LOW();
  LCD_RD_HIGH();
  Delayms(1);
  //GPIOE->ODR = ReverseBits(c);
  GPIOE->ODR = c;
  LCD_WR_LOW();
  Delayms(1);
  LCD_WR_HIGH();
  Delayms(1);
  LCD_CS_HIGH();
}

void LCD_WriteDat(unsigned char d)
{
  LCD_RS_HIGH();
  LCD_CS_LOW();
  LCD_RD_HIGH();
  Delayms(1);
  //GPIOE->ODR = ReverseBits(d);
  GPIOE->ODR = d;
  LCD_WR_LOW();
  Delayms(1);
  LCD_WR_HIGH();
  Delayms(1);
  LCD_CS_HIGH();
}

void LCD_Init()
{
  LCD_GPIO_Config();
  LCD_RST_LOW();
  Delayms(100);
  LCD_RST_HIGH();
  Delayms(100);

    LCD_WriteCmd(0xe2);//system reset
    Delayms(50);   
    LCD_WriteCmd(0xa2);
    LCD_WriteCmd(0xa0);
    LCD_WriteCmd(0xc8);
    LCD_WriteCmd(0x26);
    LCD_WriteCmd(0x81);
    LCD_WriteCmd(0x1e);
    LCD_WriteCmd(0x2f);
    Delayms(50);
    LCD_WriteCmd(0xaf);
    LCD_WriteCmd(0xf8);
    LCD_WriteCmd(0x00);
}

void LCD_DispImg(unsigned char *d)
{
  unsigned char i,j;
  unsigned int p;
  p = 0;
  for (i=0;i<8;i++)
  {
    LCD_WriteCmd(0xee);
    LCD_WriteCmd(0xb0|i);
    LCD_WriteCmd(0x10);
    LCD_WriteCmd(0x00);
    LCD_WriteCmd(0xe0); 
    for (j=0;j<128;j++)
      LCD_WriteDat(d[p++]);
  }
  LCD_WriteCmd(0xee);
    LCD_WriteCmd(0xb8);
    LCD_WriteCmd(0x10);
    LCD_WriteCmd(0x00);
    LCD_WriteCmd(0xe0); 
    for (j=0;j<128;j++)
      LCD_WriteDat(0xff);
}