#include "main.h"
#include "lcd.h"
#include "delay.h"


extern uint16_t LCD_CS;
extern uint16_t LCD_RST;
extern uint16_t LCD_RS;
extern uint16_t LCD_WR;
extern uint16_t LCD_RD;

#define LCD_CS_LOW()    GPIOB->BRR  = LCD_CS
#define LCD_CS_HIGH()   GPIOB->BSRR = LCD_CS
#define LCD_RST_LOW()   GPIOB->BRR  = LCD_RST
#define LCD_RST_HIGH()  GPIOB->BSRR = LCD_RST
#define LCD_RS_LOW()    GPIOB->BRR  = LCD_RS
#define LCD_RS_HIGH()   GPIOB->BSRR = LCD_RS
#define LCD_WR_LOW()    GPIOB->BRR  = LCD_WR
#define LCD_WR_HIGH()   GPIOB->BSRR = LCD_WR
#define LCD_RD_LOW()    GPIOB->BRR  = LCD_RD
#define LCD_RD_HIGH()   GPIOB->BSRR = LCD_RD


void LCD_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_5|
                                GPIO_Pin_6; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

  GPIO_Init(GPIOB, &GPIO_InitStructure);
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
  //Delayms(1);
  //GPIOE->ODR = ReverseBits(c);
  GPIOA->ODR = c;
  LCD_WR_LOW();
  //Delayms(1);
  LCD_WR_HIGH();
  //Delayms(1);
  LCD_CS_HIGH();
}

void LCD_WriteDat(unsigned char d)
{
  LCD_RS_HIGH();
  LCD_CS_LOW();
  LCD_RD_HIGH();
  //Delayms(1);
  //GPIOE->ODR = ReverseBits(d);
  GPIOA->ODR = d;
  LCD_WR_LOW();
  //Delayms(1);
  LCD_WR_HIGH();
  //Delayms(1);
  LCD_CS_HIGH();
}

void LCD_Init()
{
  LCD_GPIO_Config();
  LCD_RST_LOW();
  Delayms(50);
  LCD_RST_HIGH();
  Delayms(300);

    LCD_WriteCmd(0xe2);//system reset
    Delayms(50);   
    LCD_WriteCmd(0x25);
    LCD_WriteCmd(0x2b);
    Delayms(20);
    LCD_WriteCmd(0xc4);
    LCD_WriteCmd(0xa1);
    LCD_WriteCmd(0xd1);
    LCD_WriteCmd(0xd5);
    LCD_WriteCmd(0xc8);
    LCD_WriteCmd(0x00);
    LCD_WriteCmd(0xeb);
    LCD_WriteCmd(0xa6);
    LCD_WriteCmd(0xa4);
    LCD_WriteCmd(0x81);
    LCD_WriteCmd(0xa5);//80
    LCD_WriteCmd(0xd8);
    LCD_WriteCmd(0xaf);
    Delayms(50);
    //LCD_WriteCmd(0xa5);
}

void LCD_SetXY(unsigned char x,unsigned char y)  //x<128(per 3Dot) , y<160
{
  x += 0x0b;      //此屏第一列有11字节偏移.
  //LCD_WriteCmd( 0xf9 );      //关闭窗口模式
  
  LCD_WriteCmd( 0x00 | (x&0x0f) );
  LCD_WriteCmd( 0x10 | (x>>4) );
  
  LCD_WriteCmd( 0x60 | (y&0x0f) );
  LCD_WriteCmd( 0x70 | (y>>4) );
}

void LCD_SetWindow(u8 wL,u8 wU,u8 wR,u8 wD)
{
  LCD_WriteCmd(0xf8);    //窗口操作使能.
  
  LCD_WriteCmd(0xf4);    //窗口左边界.
  LCD_WriteCmd(wL);    //0bh=11,地址 11~ 116 控制一列318个点.
  
  LCD_WriteCmd(0xf5);    //窗口上边界.
  LCD_WriteCmd(wU);    //0
  
  LCD_WriteCmd(0xf6);    //窗口右边界.
  LCD_WriteCmd(wR);    //75h=117
  
  LCD_WriteCmd(0xf7);    //窗口下边界.
  LCD_WriteCmd(wD);    //9fh=159
  
  /*
  LCD_WriteCmd(0x00|wL%16);    //起始列地址  0x00|CA[3:0]
  LCD_WriteCmd(0x10|wL/16);    //            0x10|CA[6:4]
  
  LCD_WriteCmd(0x60|wU%16);    //起始行地址  0x60|RA[3:0]
  LCD_WriteCmd(0x70|wU/16);    //            0x70|RA[7:4]
  */
  LCD_SetXY(0,0) ;
}

void LCD_DispImg(unsigned char *d)
{
  unsigned i,j,p;
  
  p = 0;
  LCD_SetWindow(0x25,0x00,0x75,0x9f);
  for (i=0;i<160;i++)
  {
    LCD_SetXY(0x1a,i);
    //LCD_WriteDat(0x00);//2 empty pixel
    for (j=0;j<80;j++)
      LCD_WriteDat(d[p++]);
  }
}