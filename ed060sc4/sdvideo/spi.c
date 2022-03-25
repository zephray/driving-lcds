#include "spi.h"

void SPI1_Config(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  //IO��ʼ��
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE); 

  //Configure SPI_SLAVE pins: SCK and MISO
  //Configure SCK and MOSI pins as Input Floating
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  //Configure MISO pin as Alternate Function Push Pull
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
 
  SPI_Cmd(SPI1, DISABLE);            //�����Ƚ���,���ܸı�MODE
  SPI_InitStructure.SPI_Direction =SPI_Direction_1Line_Rx;  //����ֻ����
  SPI_InitStructure.SPI_Mode =SPI_Mode_Slave;       //��
  SPI_InitStructure.SPI_DataSize =SPI_DataSize_8b;  //8λ
  SPI_InitStructure.SPI_CPOL =SPI_CPOL_Low;        //CPOL=0  ʱ�����յ�
  SPI_InitStructure.SPI_CPHA =SPI_CPHA_2Edge;       //CPHA=1 ���ݲ����2��
  SPI_InitStructure.SPI_NSS =SPI_NSS_Hard;          //Ӳ��NSS
  SPI_InitStructure.SPI_BaudRatePrescaler =SPI_BaudRatePrescaler_2;  //2��Ƶ����ģʽ��Ч��
  SPI_InitStructure.SPI_FirstBit =SPI_FirstBit_MSB; //��λ��ǰ
  SPI_InitStructure.SPI_CRCPolynomial =7;           //CRC7��δ��
   
  SPI_Init(SPI1,&SPI_InitStructure);
  SPI_Cmd(SPI1, ENABLE);
}

uint8_t SPI1_Recv(void)
{
  // Wait to receive a byte  
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE) == RESET);  
  
  // Return the byte read from the SPI bus  
  return SPI_I2S_ReceiveData(SPI1);
}  

void SPI1_ITConfig(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;           //SPI1�ж�
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //��ռ���ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        //��Ӧ���ȼ�0
  NVIC_Init(&NVIC_InitStructure);
           
  SPI_I2S_ITConfig(SPI1,SPI_I2S_IT_RXNE,ENABLE);
}
