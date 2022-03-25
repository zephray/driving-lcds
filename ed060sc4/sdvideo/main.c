#include "main.h"
#include "usart.h"
//#include "sdio_sd.h"
#include "sdcard.h"
#include "epd.h"
#include "image.h"
#include "sram.h"
//#include "dac.h"

//#include "gui.h"

__IO uint32_t TimingDelay;

unsigned char EPD_BufA[60416] @ ".extram";
unsigned char EPD_BufB[60416] @ ".extram";

void NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in 10 ms.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

int main(void)
{
  SD_Error state;
  //RCC_ClocksTypeDef RCC_Clocks;
  uint16_t i,f;
  unsigned long a;
  
  USART1_Config();
        
  printf("\r\n\r\nSTM32F2 Development Platform\r\nBuild by Zweb.\r\n");
  printf("Ready to turn on the Systick.\r\n");
        
  /* SysTick end of count event each 10ms */
  //RCC_GetClocksFreq(&RCC_Clocks);
  //SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);
  
  //printf("Systick opened successfully.\r\n");
  //printf("Main Clock Frequency: %d MHz\r\n",(RCC_Clocks.HCLK_Frequency/1000000));

  NVIC_Config();
  
  SRAM_GPIO_Config();
  SRAM_FSMC_Config();
  
  EPD_Init();
  EPD_Power_On();
  EPD_Clear();
  EPD_Power_Off();
  
  state=SD_Init();
  if(state==SD_OK)
  {
    printf("SD卡初始化成功！\r\n");
  }
  
  for (i=0;i<60000;i++)
    EPD_BufB[i] = 0x00;
  
  if(state==SD_OK)
  {
    for (f=0;f<5476;f++)
    {
      a = f*118*512;
      //READ SD CARD
      for (i=0;i<118;i++)
      {
        state = SD_ReadBlock(EPD_BufA+i*512, a+i*512, 512);
      }
      
      //DISPLAY
      EPD_Power_On();
      EPD_Refresh(EPD_BufB, EPD_BufA);
      EPD_Power_Off();
      
      //BACKUP SCREEN
      for (i=0;i<60000;i++)
      {
        EPD_BufB[i]=EPD_BufA[i];
      }
    }
  }
  while(1)
  {
    
  }
  
}
