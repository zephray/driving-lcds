#include "main.h"
#include "lcd.h"
#include "delay.h"

unsigned char LCD_FB_1[9600];
unsigned char LCD_FB_2[9600];
unsigned char LCD_FB_3[9600];
unsigned char LCD_FB_4[9600];

#define LCD_FR  GPIO_Pin_0
#define LCD_LP  GPIO_Pin_1
#define LCD_XCK GPIO_Pin_2
#define LCD_AC  GPIO_Pin_3

#define LCD_FR_LOW()    GPIOD->BRR  = LCD_FR
#define LCD_FR_HIGH()   GPIOD->BSRR = LCD_FR
#define LCD_LP_LOW()    GPIOD->BRR  = LCD_LP
#define LCD_LP_HIGH()   GPIOD->BSRR = LCD_LP
#define LCD_XCK_LOW()   GPIOD->BRR  = LCD_XCK
#define LCD_XCK_HIGH()  GPIOD->BSRR = LCD_XCK
#define LCD_AC_LOW()    GPIOD->BRR  = LCD_AC
#define LCD_AC_HIGH()   GPIOD->BSRR = LCD_AC

void delay(int x)
{
  while(x--);
}
void LCD_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

  GPIO_Init(GPIOE, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3; 
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

void LCD_DispImg(unsigned char *d)
{
  unsigned char i,j;
  unsigned int p,q;
  unsigned char d1,d2,d3,d4;
  p = 0;
  q = 0;
  for (i=0;i<240;i++)
  {
    for (j=0;j<40;j++)
    {
      d1 = 0;
      d2 = 0;
      d3 = 0;
      d4 = 0;
      d1 = (d[q]&0x80)|((d[q]&0x08)<<3);
      d2 = ((d[q]&0x40)<<1)|((d[q]&0x04)<<4);
      d3 = ((d[q]&0x20)<<2)|((d[q]&0x02)<<5);
      d4 = ((d[q]&0x10)<<3)|((d[q]&0x01)<<6);
      q++;
      d1 |= ((d[q]&0x80)>>2)|((d[q]&0x08)<<1);
      d2 |= ((d[q]&0x40)>>1)|((d[q]&0x04)<<2);
      d3 |= ((d[q]&0x20))|((d[q]&0x02)<<3);
      d4 |= ((d[q]&0x10)<<1)|((d[q]&0x01)<<4);
      q++;
      d1 |= ((d[q]&0x80)>>4)|((d[q]&0x08)>>1);
      d2 |= ((d[q]&0x40)>>3)|((d[q]&0x04));
      d3 |= ((d[q]&0x20)>>2)|((d[q]&0x02)<<1);
      d4 |= ((d[q]&0x10)>>1)|((d[q]&0x01)<<2);
      q++;
      d1 |= ((d[q]&0x80)>>6)|((d[q]&0x08)>>3);
      d2 |= ((d[q]&0x40)>>5)|((d[q]&0x04)>>2);
      d3 |= ((d[q]&0x20)>>4)|((d[q]&0x02)>>1);
      d4 |= ((d[q]&0x10)>>3)|((d[q]&0x01));
      q++;
      LCD_FB_1[p] = d1;
      LCD_FB_2[p] = d2;
      LCD_FB_3[p] = d3;
      LCD_FB_4[p] = d4;
      p++;
    }
  }
}

void LCD_ReversePol()
{
 if ((GPIOD->IDR)&LCD_AC)
    LCD_AC_LOW();
  else
    LCD_AC_HIGH();
}

void LCD_RefreshI(unsigned char *fb)
{
  unsigned char i,j;
  
  LCD_FR_HIGH();
  for (i=0;i<240;i++)
  {
    for (j=0;j<40;j++)
    {
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[i*40+j]>>4;
      LCD_XCK_LOW();
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[i*40+j];
      LCD_XCK_LOW();
    }
    LCD_LP_HIGH();
    LCD_LP_LOW();
    LCD_FR_LOW();
  }
  for (i=0;i<8;i++)
  {
    
    for (j=0;j<40;j++)
    {
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[239*40+j]>>4;
      LCD_XCK_LOW();
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[239*40+j]>>4;
      LCD_XCK_LOW();
    }
    LCD_LP_HIGH();
    LCD_LP_LOW();
  }
  LCD_LP_HIGH();
  LCD_LP_LOW();
}

void LCD_RefreshL4(unsigned char *fb)
{
  unsigned char i,j;
  
  LCD_FR_HIGH();
  for (i=0;i<240;i++)
  {
    for (j=0;j<40;j++)
    {
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[i*40+j]>>4;
      LCD_XCK_LOW();
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[i*40+j];
      LCD_XCK_LOW();
    }
    LCD_LP_HIGH();
    LCD_LP_LOW();
    LCD_FR_LOW();
  }
  for (i=0;i<8;i++)
  {
    
    for (j=0;j<40;j++)
    {
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[239*40+j]>>4;
      LCD_XCK_LOW();
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[239*40+j]>>4;
      LCD_XCK_LOW();
    }
    LCD_LP_HIGH();
    LCD_LP_LOW();
  }
  LCD_LP_HIGH();
  LCD_LP_LOW();
}

void LCD_RefreshL3(unsigned char *fb)
{
  unsigned char i,j;
  
  LCD_FR_HIGH();
  for (i=0;i<240;i++)
  {
    for (j=0;j<40;j++)
    {
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[i*40+j]>>4;
      LCD_XCK_LOW();
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[i*40+j];
      LCD_XCK_LOW();
    }
    LCD_LP_HIGH();
    delay(104);
    LCD_LP_LOW();
    LCD_FR_LOW();
  }
  for (i=0;i<8;i++)
  {
    
    for (j=0;j<40;j++)
    {
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[239*40+j]>>4;
      LCD_XCK_LOW();
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[239*40+j]>>4;
      LCD_XCK_LOW();
    }
    LCD_LP_HIGH();
    delay(104);
    LCD_LP_LOW();
  }
  LCD_LP_HIGH();
  LCD_LP_LOW();
}

void LCD_RefreshL2(unsigned char *fb)
{
  unsigned char i,j;
  
  LCD_FR_HIGH();
  for (i=0;i<240;i++)
  {
    for (j=0;j<40;j++)
    {
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[i*40+j]>>4;
      LCD_XCK_LOW();
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[i*40+j];
      LCD_XCK_LOW();
    }
    LCD_LP_HIGH();
    delay(418);
    LCD_LP_LOW();
    LCD_FR_LOW();
  }
  for (i=0;i<8;i++)
  {
    
    for (j=0;j<40;j++)
    {
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[239*40+j]>>4;
      LCD_XCK_LOW();
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[239*40+j]>>4;
      LCD_XCK_LOW();
    }
    LCD_LP_HIGH();
    delay(418);
    LCD_LP_LOW();
  }
  LCD_LP_HIGH();
  LCD_LP_LOW();
}

void LCD_RefreshL1(unsigned char *fb)
{
  unsigned char i,j;
  
  LCD_FR_HIGH();
  for (i=0;i<240;i++)
  {
    for (j=0;j<40;j++)
    {
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[i*40+j]>>4;
      LCD_XCK_LOW();
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[i*40+j];
      LCD_XCK_LOW();
    }
    LCD_LP_HIGH();
    delay(1044);
    LCD_LP_LOW();
    LCD_FR_LOW();
  }
  for (i=0;i<8;i++)
  {
    
    for (j=0;j<40;j++)
    {
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[239*40+j]>>4;
      LCD_XCK_LOW();
      LCD_XCK_HIGH();
      GPIOE->ODR=fb[239*40+j]>>4;
      LCD_XCK_LOW();
    }
    LCD_LP_HIGH();
    delay(1044);
    LCD_LP_LOW();
  }
  LCD_LP_HIGH();
  LCD_LP_LOW();
}

void LCD_Refresh()//7-4-2-1
{
  LCD_RefreshI(LCD_FB_1);//1
  LCD_RefreshI(LCD_FB_2);//1
  //LCD_ReversePol();
  LCD_RefreshI(LCD_FB_1);//2
  LCD_RefreshI(LCD_FB_3);//1
  //LCD_ReversePol();
  LCD_RefreshI(LCD_FB_1);//3
  LCD_RefreshI(LCD_FB_4);//1
  //LCD_ReversePol();
  LCD_RefreshI(LCD_FB_1);//4
  LCD_RefreshI(LCD_FB_2);//2
  //LCD_ReversePol();
  LCD_RefreshI(LCD_FB_1);//5
  LCD_RefreshI(LCD_FB_3);//2
  //LCD_ReversePol();
  LCD_RefreshI(LCD_FB_1);//6
  LCD_RefreshI(LCD_FB_2);//3
  LCD_RefreshI(LCD_FB_1);//7
  LCD_RefreshI(LCD_FB_2);//4
  LCD_RefreshI(LCD_FB_1);//8
  //LCD_ReversePol();
  /*LCD_RefreshL1(LCD_FB_1);
  LCD_RefreshL2(LCD_FB_2);
  LCD_RefreshL3(LCD_FB_3);
  LCD_RefreshL4(LCD_FB_4);*/
}