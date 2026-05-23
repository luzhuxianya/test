#ifndef __OLED_I2C_H
#define	__OLED_I2C_H

#include "sys.h"

#define OLED_RCC      RCC_APB2Periph_GPIOB
#define OLED_PORT     GPIOB
#define OLED_PIN_SCL  GPIO_Pin_7
#define OLED_PIN_SDA  GPIO_Pin_8

#define OLED_ADDRESS	  0x78 //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78
#define	DIS_DEGREE			0x7F




#define SCL3_0	GPIO_ResetBits(OLED_PORT,OLED_PIN_SCL)			// Serial Clock Input
#define SDA3_0	GPIO_ResetBits(OLED_PORT,OLED_PIN_SDA)				// Serial Data Input


#define SCL3_1	GPIO_SetBits(OLED_PORT,OLED_PIN_SCL)					// Serial Clock Input
#define SDA3_1	GPIO_SetBits(OLED_PORT,OLED_PIN_SDA)					// Serial Data Input

typedef struct 								//汉字字模结构体
{
	unsigned char Index[2];	
	unsigned char Msk[32];
}typFNT_GBK16;

typedef struct 								//字符字模结构体
{
	unsigned char Index[1];	
	unsigned char Msk[16];
}typFNT_GBK8;


void I2C_Configuration(void);
void I2C_WriteByte(uint8_t addr,uint8_t data);
void WriteCmd(unsigned char I2C_Command);
void WriteDat(unsigned char I2C_Data);
void OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y);
void OLED_Fill(unsigned char fill_Data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);
void OLED_ShowCN(unsigned char x, unsigned char y, char *fmt,...);
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch, unsigned char TextSize);
void ZXT(unsigned char num);
void OLED_Fast(unsigned char BMP[]);
void OLED_CLS(void);//清屏

extern unsigned char BMP2[8192];
extern unsigned char BMP1[1024];
extern unsigned char Dat_sp[16];


/******************Instructions for use***********************

	I2C_Configuration();
	
			OLED_ShowCN(2*16,0*2,1);
			OLED_ShowCN(3*16,0*2,2);
			OLED_ShowCN(4*16,0*2,3);
			OLED_ShowCN(5*16,0*2,4);
			
			OLED_ShowCN(0*16,1*2,5);
			OLED_ShowCN(1*16,1*2,6);
			OLED_ShowStr(2*16,1*2,":",2);
			
			OLED_ShowCN(0*16,2*2,7);
			OLED_ShowCN(1*16,2*2,8);
			OLED_ShowStr(2*16,2*2,":",2);

			OLED_ShowCN(0*16,3*2,9);
			OLED_ShowCN(1*16,3*2,10);
			OLED_ShowStr(4*8,3*2,":",2);
			
			
			sprintf(Dat_sp,"%2dLV",Set_lv);
			OLED_ShowStr(2*16+8,2*2,Dat_sp,2);
			
			sprintf(Dat_sp,"%4d ",0xfff-ADC_DMA_Light);
			OLED_ShowStr(2*16+8,1*2,Dat_sp,2);

			OLED_CLS();
			sprintf((char *)Dat_sp,"T:%3d C  H:%3d%%",(int)Temp,(int)Humi);
			OLED_ShowStr(0*8,0*2,Dat_sp,2);
			OLED_ShowChar(5*8,0*2,DIS_DEGREE,2);

*************************************************************/


#endif

