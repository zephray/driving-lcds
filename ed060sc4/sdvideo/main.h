#ifndef __MAIN_H__
#define __MAIN_H__

#include "stm32f2xx.h"
#include <stdio.h>

extern __IO uint8_t ShiftState;

/* Exported macro ------------------------------------------------------------*/
#define ABS(x)         (x < 0) ? (-x) : x
/* Exported functions ------------------------------------------------------- */
void TimingDelay_Decrement(void);
void Delay(__IO uint32_t nTime);

#endif
