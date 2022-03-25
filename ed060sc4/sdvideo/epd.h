/*******************************************************************************
  EPD DRIVER FOR STM32F2/4 w/ FSMC
  By ZephRay(zephray@outlook.com)
*******************************************************************************/
#ifndef __EPD_H__
#define __EPD_H__

#include "main.h"

//SOURCE DRIVER
#define EPD_CL_L()        GPIOA->BSRRH = GPIO_Pin_1
#define EPD_CL_H()        GPIOA->BSRRL = GPIO_Pin_1
#define EPD_LE_L()        GPIOA->BSRRH = GPIO_Pin_2
#define EPD_LE_H()        GPIOA->BSRRL = GPIO_Pin_2
#define EPD_OE_L()        GPIOA->BSRRH = GPIO_Pin_3
#define EPD_OE_H()        GPIOA->BSRRL = GPIO_Pin_3
#define EPD_SHR_L()       GPIOA->BSRRH = GPIO_Pin_4
#define EPD_SHR_H()       GPIOA->BSRRL = GPIO_Pin_4
#define EPD_SPH_L()       GPIOA->BSRRH = GPIO_Pin_5
#define EPD_SPH_H()       GPIOA->BSRRL = GPIO_Pin_5     

//GATE DRIVER
#define EPD_GMODE1_L()    GPIOE->BSRRH = GPIO_Pin_3
#define EPD_GMODE1_H()    GPIOE->BSRRL = GPIO_Pin_3
#define EPD_GMODE2_L()    GPIOE->BSRRH = GPIO_Pin_2
#define EPD_GMODE2_H()    GPIOE->BSRRL = GPIO_Pin_2
#define EPD_XRL_L()       GPIOE->BSRRH = GPIO_Pin_4
#define EPD_XRL_H()       GPIOE->BSRRL = GPIO_Pin_4
#define EPD_SPV_L()       GPIOA->BSRRH = GPIO_Pin_6
#define EPD_SPV_H()       GPIOA->BSRRL = GPIO_Pin_6
#define EPD_CKV_L()       GPIOA->BSRRH = GPIO_Pin_7
#define EPD_CKV_H()       GPIOA->BSRRL = GPIO_Pin_7


void EPD_Init(void);
void EPD_Power_Off(void);
void EPD_Power_On(void);
void EPD_Clear(void);
void EPD_Refresh(unsigned char *last, unsigned char *now);

#endif
