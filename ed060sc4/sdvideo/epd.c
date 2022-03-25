/*******************************************************************************
  EPD DRIVER FOR STM32F2/4 w/ FSMC
  By ZephRay(zephray@outlook.com)
*******************************************************************************/
#include "epd.h"

unsigned char EPD_Frame[120000] @ ".extram";

//黑白黑白刷屏。最终到达白背景
#define FRAME_INIT_LEN 		18
const unsigned char wave_init[FRAME_INIT_LEN]=
{
0x55,0x55,0x55,0x55,
0xaa,0xaa,0xaa,0xaa,
0x55,0x55,0x55,0x55,
0xaa,0xaa,0xaa,0xaa,
0,0,
};

//W 2 B 1
const unsigned char wave_tb[16]=
{
  0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15,
  0x40, 0x41, 0x44, 0x45, 0x50, 0x51, 0x54, 0x55
};

const unsigned char wave_tw[16]=
{
  0x00, 0x02, 0x08, 0x0a, 0x20, 0x22, 0x28, 0x2a,
  0x80, 0x82, 0x88, 0x8a, 0xa0, 0xa2, 0xa8, 0xaa
};

unsigned char g_dest_data[200];				//送到电子纸的一行数据缓存

void DelayCycle(unsigned long x)
{
  while (x--)
  {
    asm("nop");
  }
}

void EPD_GPIO_Init()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);
  
  GPIOG->BSRRH = GPIO_Pin_12;//TURN OFF ALL VOLTAGES BEFORE NEXT
  GPIOG->BSRRH = GPIO_Pin_13;
  GPIOG->BSRRH = GPIO_Pin_14;
  GPIOG->BSRRH = GPIO_Pin_15;
  
  //Power controll
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOG,&GPIO_InitStructure);
  
  //Source&Gate Driver
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|
                                GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
  GPIO_Init(GPIOE,&GPIO_InitStructure);
  
  //Source Data
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_Init(GPIOC,&GPIO_InitStructure);
}

void EPD_Power_Off(void)
{
  GPIOG->BSRRH = GPIO_Pin_15;
  DelayCycle(400);
  GPIOG->BSRRH = GPIO_Pin_14;
  DelayCycle(20);
  GPIOG->BSRRH = GPIO_Pin_13;
  DelayCycle(20);
  GPIOG->BSRRH = GPIO_Pin_12;
  DelayCycle(400);
}

void EPD_Init(void)
{
  EPD_GPIO_Init();	
  
  EPD_SHR_L();
  EPD_GMODE1_H();
  EPD_GMODE2_H();
  EPD_XRL_H();

  EPD_Power_Off();

  EPD_LE_L();
  EPD_CL_L();
  EPD_OE_L();
  EPD_SPH_H();
  EPD_SPV_H();
  EPD_CKV_L();
}

void EPD_Send_Row_Data(u8 *pArray)  
{
  unsigned char i;
  
  EPD_LE_H(); 
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();
  
  EPD_LE_L();
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();
  
  EPD_OE_H();
  
  EPD_SPH_L();                                          
  
  for (i=0;i<200;i++)
  {
    GPIOC->ODR = pArray[i];
    EPD_CL_H();
    DelayCycle(1);
    EPD_CL_L();
  }
  
  EPD_SPH_H();
  
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();
  
  EPD_CKV_L();
  EPD_OE_L();
  
  EPD_CL_H();
  EPD_CL_L();
  EPD_CL_H();
  EPD_CL_L();
  
  EPD_CKV_H();     
}

void EPD_Vclock_Quick(void)
{
  unsigned char i;
  
  for (i=0;i<2;i++)
  {
    EPD_CKV_L();
    DelayCycle(20);
    EPD_CKV_H();
    DelayCycle(20);
  }
}

void EPD_Start_Scan(void)
{ 
  EPD_SPV_H();

  EPD_Vclock_Quick();
 
  EPD_SPV_L();

  EPD_Vclock_Quick();
  
  EPD_SPV_H();
  
  EPD_Vclock_Quick();
}

void line_data_init(u8 frame)
{
	int i;
	
	for(i=0; i<200; i++)
	{
		g_dest_data[i] = wave_init[frame];	
	}	
}

void EPD_Power_On(void)
{
  GPIOG->BSRRL = GPIO_Pin_12;
  DelayCycle(20);
  GPIOG->BSRRL = GPIO_Pin_13;
  DelayCycle(20);
  GPIOG->BSRRL = GPIO_Pin_14;
  DelayCycle(20);
  GPIOG->BSRRL = GPIO_Pin_15;
  DelayCycle(400);
}

void EPD_Clear(void)
{
  unsigned short line,frame;
  
  for(frame=0; frame<FRAME_INIT_LEN; frame++)			
  {
    EPD_Start_Scan();
    for(line=0; line<600; line++)
    {
      line_data_init(frame);							//14ms
      //line_data_init(frame);							//14ms	加该指令是保持时间与ChangePIC时一致
      //line_data_init(frame);							//14ms	加该指令是保持时间与ChangePIC时一致
      EPD_Send_Row_Data( g_dest_data );				//40ms
    }
    EPD_Send_Row_Data( g_dest_data );					//最后一行还需GATE CLK,故再传一行没用数据
  }
}

void EPD_ProcessFrame(unsigned char *last, unsigned char *now)
{
  int i,j,k,l;
  unsigned char la,na,tb,tw;
  
  k=0;l=0;
  for (i=0;i<600;i++)
  {
    for (j=0;j<100;j++)
    {
      la = last[k];
      na = now[k];
      k++;
      
      //PREV WHITE-> NOW BLACK
      tb = (~la)&na;
      //PREV BLACK-> NOW WHITE
      tw = (~na)&la;
      
      EPD_Frame[l++] = wave_tw[(tw>>4)] | wave_tb[(tb>>4)];
      EPD_Frame[l++] = wave_tw[tw&0xF] | wave_tb[tb&0xF];
    }
  }
}

void EPD_Refresh(unsigned char *last, unsigned char *now)
{
  unsigned short line,frame;
  unsigned long i;
  
  EPD_ProcessFrame(last,now);
  //for (i=0;i<120000;i++)
  //  EPD_Frame[i]=0x55;
  for(frame=0; frame<5; frame++)			
  {
    EPD_Start_Scan();
    for(line=0; line<600; line++)
    {
      EPD_Send_Row_Data( EPD_Frame + line*200 );				//40ms
    }
    EPD_Send_Row_Data( g_dest_data );					//最后一行还需GATE CLK,故再传一行没用数据
  }
}
