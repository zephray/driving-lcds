#include "ST7637.H"
#include "main.h"
#include "uart.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void LCD_WriteCmd(uchar cmd){
	  LCD_RSB = 0;
    LCD_CSB = 0;
    LCD_RDB = 1;
    LCD_DataBus= cmd;
    LCD_WRB = 0;
    LCD_WRB = 1;
		LCD_RSB = 1;
    LCD_CSB = 1;
}

void LCD_WriteDat(uchar dat){
	  LCD_RSB = 1;
    LCD_CSB = 0;
    LCD_RDB = 1;
    LCD_DataBus= dat;
    LCD_WRB = 0;
    LCD_WRB = 1;
		LCD_RSB = 0;
    LCD_CSB = 1;
}

void LCD_Init()
{
    LCD_RST = 0;
		Delay(50);
		LCD_RST = 1;
		Delay(200);
		
        LCD_WriteCmd(0x01);                  //Software Reset    
        Delay(150);                     //Delay 150mS   
   
        LCD_WriteCmd(0xD7);                  //Autoread control   
        LCD_WriteDat (0x9F);                //Disable Autoread   
        LCD_WriteCmd(0xE0);                  //Control OTP/MTP   
        LCD_WriteDat (0x00);                //Read Mode   
        Delay(10);                      //Delay 10mS   
        LCD_WriteCmd(0xE3);                  //Control OTP/MTP   
        Delay(20);                      //Delay 20mS   
        LCD_WriteCmd(0xE1);                  //Close Read Mode   
//-----------------------------------------------------------------------------------//   
//------------------------------OTP/MTP Set [XXXX/OTPB/MTP ]-------------------------//   
//-----------------------------------------------------------------------------------//   
//------------------------------------OTPB Set---------------------------------------//   
        LCD_WriteCmd(0xC3);                  //Bias Set   
        LCD_WriteDat (0x00);                //   
        LCD_WriteCmd(0xC4);                  //Booster Set   
        LCD_WriteDat (0x05);                //   
        LCD_WriteCmd(0xC5);                  //Booster Efficiency Set   
        LCD_WriteDat (0x11);                //   
        LCD_WriteCmd(0xCB);                  //VG Booster Set   
        LCD_WriteDat (0x01);                //   
        LCD_WriteCmd(0xCC);                  //ID1 Set   
        LCD_WriteDat (0x45);                //   
        LCD_WriteCmd(0xCE);                  //ID3 Set   
        LCD_WriteDat (0x00);                //   
        LCD_WriteCmd(0xB7);                  //Glass Direction   
        LCD_WriteDat (0xC8);                //   
        LCD_WriteCmd(0xD0);                  //Follower Type set   
        LCD_WriteDat (0x1D);                //   
//------------------------------------MTP Set----------------------------------------//   
        LCD_WriteCmd(0xD7);                  //Autoread control   
        LCD_WriteDat (0xBF);                //   
        LCD_WriteCmd(0xC7);                  //V0 Offset Voltage Set   
        LCD_WriteDat (0x00);                //   
        LCD_WriteDat (0x00);                //   
        LCD_WriteCmd(0xB5);                  //N-line Set   
        LCD_WriteDat (0xA4);                //   
        LCD_WriteCmd(0xCD);                  //ID2 Set   
        LCD_WriteDat (0xD2);                //   
        LCD_WriteCmd(0xD0);                  //Set Vg Source   
        LCD_WriteDat (0x1D);                //   
        LCD_WriteCmd(0xB4);                  //PTL Saving Mode Set   
        LCD_WriteDat (0x18);                //Power Normal Mode   
//-----------------------------------------------------------------------------------//   
//----------------------------Command Table 1----------------------------------------//   
//-----------------------------------------------------------------------------------//   
        LCD_WriteCmd(0x11);                  //Sleep Out   
        LCD_WriteCmd(0x20);                  //Normal Display   
        LCD_WriteCmd(0x38);                  //Idle Mode Off   
        LCD_WriteCmd(0x25);                  //Contrast Difference Set   
        LCD_WriteDat (0x3F);                //   
   
        LCD_WriteCmd(0x13);                  //Partial Mode Off   
   
        LCD_WriteCmd(0x2A);                  //Column Range   
        LCD_WriteDat (0x00);                //Start Address-0   
        LCD_WriteDat (0x04);                //Start Address-4   
        LCD_WriteDat (0x00);                //End Address-0   
        LCD_WriteDat (0x83);                //End Address-131   
   
        LCD_WriteCmd(0x2B);                  //Page Range   
        LCD_WriteDat (0x00);                //Start Address-0   
        LCD_WriteDat (0x04);                //Start Address-4   
        LCD_WriteDat (0x00);                //End Address-0   
        LCD_WriteDat (0x83);                //End Address-131   
   
        LCD_WriteCmd(0x34);                  //TE Off   
        LCD_WriteCmd(0x33);                  //Scroll Area Set   
        LCD_WriteDat (0x00);                //Top Address   
        LCD_WriteDat (0x84);                //Height Address   
        LCD_WriteDat (0x00);                //Botton Address   
        LCD_WriteCmd(0x37);                  //Scroll Start Address Set   
        LCD_WriteDat (0x00);                //   
        LCD_WriteCmd(0x3A);                  //Pixel Format Set   
        LCD_WriteDat (0x05);                //   
        LCD_WriteCmd(0x36);                  //Memory Access Control   
        LCD_WriteDat (0x00);                //   
        LCD_WriteCmd(0x29);                  //Display On   
//-----------------------------------------------------------------------------------//   
//------------------------Command Table 2 [XXXX/XXXX/GAMM]---------------------------//   
//-----------------------------------------------------------------------------------//   
        LCD_WriteCmd(0xB0);                  //Duty Set   
        LCD_WriteDat (0x7F);                //   
        LCD_WriteCmd(0xB1);                  //First COM Set   
        LCD_WriteDat (0x00);                //   
        LCD_WriteCmd(0xB3);                  //OSC Div. Set   
        LCD_WriteDat (0x00);                //   
        LCD_WriteCmd(0xC0);                  //V0 Voltage Set   
        LCD_WriteDat (0x10);                //   
        LCD_WriteDat (0x01);                //   
        LCD_WriteCmd(0xC6);                  //V0 Bias Set   
        LCD_WriteDat (0x00);                //   
        LCD_WriteCmd(0xB7);                  //Glass Direction   
        LCD_WriteDat (0xC8);                //   
//------------------------------Gamma Table Set--------------------------------------//   
        LCD_WriteCmd(0xF9);                  //Gamma   
        LCD_WriteDat (0x00);                //   
        LCD_WriteDat (0x02);                //   
        LCD_WriteDat (0x04);                //   
        LCD_WriteDat (0x06);                //   
        LCD_WriteDat (0x08);                //   
        LCD_WriteDat (0x0A);                //   
        LCD_WriteDat (0x0C);                //   
        LCD_WriteDat (0x0E);                //   
        LCD_WriteDat (0x10);                //   
        LCD_WriteDat (0x12);                //   
        LCD_WriteDat (0x14);                //   
        LCD_WriteDat (0x16);                //   
        LCD_WriteDat (0x18);                //   
        LCD_WriteDat (0x1A);                //   
        LCD_WriteDat (0x1C);                //   
        LCD_WriteDat (0x1E);                //   
}

void LCD_SetWindow(uchar x0,uchar y0,uchar x1,uchar y1)
{
    LCD_WriteCmd(0x2A);  //horizontal address area set   
    LCD_WriteDat(x0+LCD_OFFSET_X);   
    LCD_WriteDat(x1+LCD_OFFSET_X);   
   
    LCD_WriteCmd(0x2B);  //vertical address area set   
    LCD_WriteDat(y0+LCD_OFFSET_Y);   
    LCD_WriteDat(y1+LCD_OFFSET_Y);   
   
    LCD_WriteCmd(0x2c);   
}

void LCD_Fill(uchar x0,uchar y0,uchar x1,uchar y1,uint c)
{
	uchar i,j;

 	LCD_SetWindow(x0,y0,x1,y1);
	for(i=y0;i<=y1;i++)
	{
		for(j=x0;j<=x1;j++)
		{
				LCD_WriteDat(c>>8);
        LCD_WriteDat(c&0xff);
		}
	}
}

void LCD_DispBmp(uchar x0,uchar y0,uchar x1,uchar y1,uchar *pic)
{
	uchar i,j;

 	LCD_SetWindow(x0,y0,x1,y1);
	for(i=0;i<=(y1-y0);i++)
	{
		for(j=0;j<=(x1-x0);j++)
		{
				LCD_WriteDat(*pic++);
        LCD_WriteDat(*pic++);
		}
	}
}

void LCD_Point(uchar x,uchar y,uint c)
{
		//LCD_SetWindow(x,y,x,y);
		LCD_WriteCmd(0x70|(y>>4));//设置行地址  
    LCD_WriteCmd(0x60|(y&0x0f));   
    LCD_WriteCmd(0x10|(x>>4));//设置列地址
    LCD_WriteCmd(x&0x0f);   
		LCD_WriteDat(c>>8);
    LCD_WriteDat(c&0xff);
}