#include "OLED_I2C.h"
#include "delay.h"
#include "codetab.h"
#include <stdio.h>
#include <stdarg.h>
//#include <string.h>


unsigned char BMP2[8192];
unsigned char Dat_sp[16];


//****************************************************************************
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Read/Write Sequence
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void I2C_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(OLED_RCC  ,ENABLE);

	GPIO_InitStructure.GPIO_Pin = OLED_PIN_SCL|OLED_PIN_SDA;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(OLED_PORT, &GPIO_InitStructure);  
	OLED_Init();
}


void I2C_3(unsigned char mcmd)
{
unsigned char length = 8;			// Send Command

while(length--)
{
	if(mcmd & 0x80)
	{
		SDA3_1;
	}
	else
	{
		SDA3_0;
	}
	SCL3_1;
	SCL3_0;
	mcmd = mcmd << 1;
}
}


void I2C_Ack3()
{
	SDA3_1;
	SCL3_1;
	SCL3_0;
}


void I2C_NAck3()
{
	SDA3_0;
	SCL3_1;
	SCL3_0;
}


void I2C_Start3()
{
	SDA3_0;
	SCL3_1;
	SCL3_0;
	I2C_3(0x78);
	I2C_Ack3();
}


void I2C_Stop3()
{
	SCL3_1;
	SDA3_0;
	SDA3_1;
}


void WriteCmd(unsigned char I2C_Command)//写命令
{
	I2C_Start3();
	I2C_3(0x00);
	I2C_Ack3();
	I2C_3(I2C_Command);
	I2C_Ack3();
	I2C_Stop3();
}

void WriteDat(unsigned char I2C_Data)//写数据
{
	I2C_Start3();
	I2C_3(0x40);
	I2C_Ack3();
	I2C_3(I2C_Data);
	I2C_Ack3();
	I2C_Stop3();
}

void OLED_Init(void)
{
	delay_ms(100); //这里的延时很重要
	
	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode	
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
	
	OLED_CLS();
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);		//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
			{
				WriteDat(fill_Data);
			}
	}
}

void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          : 
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
void OLED_ON(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X14);  //开启电荷泵
	WriteCmd(0XAF);  //OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          : 
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X10);  //关闭电荷泵
	WriteCmd(0XAE);  //OLED休眠
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch, unsigned char TextSize)
{
	unsigned char c = 0,i = 0;
	switch(TextSize)
	{
		case 1:
		{
			c = ch - 32;
			if(x > 126)
			{
				x = 0;
				y++;
			}
			OLED_SetPos(x,y);
			for(i=0;i<6;i++)
				WriteDat(F6x8[c][i]);
		}break;
		case 2:
		{
			c = ch - 32;
			if(x > 120)
			{
				x = 0;
				y++;
			}
			OLED_SetPos(x,y);
			for(i=0;i<8;i++)
				WriteDat(F8X16[c*16+i]);
			OLED_SetPos(x,y+1);
			for(i=0;i<8;i++)
				WriteDat(F8X16[c*16+i+8]);
		}break;
	}
}


//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------

void OLED_ShowCN(unsigned char x, unsigned char y, char *fmt,...)
{
	unsigned char i,k;
	unsigned int HZnum;
	unsigned char UsartPrintfBuf[17];
	unsigned char *s = UsartPrintfBuf;
	va_list ap;
	
	va_start(ap, fmt);
	vsnprintf((char *)UsartPrintfBuf, sizeof(UsartPrintfBuf), fmt, ap);							//格式化
	va_end(ap);
  
	HZnum=sizeof(F16x16)/sizeof(typFNT_GBK16);
	while(*s)
	{
		if((*s) >= 128)
		{
			for (k=0;k<HZnum;k++) 
			{
				if ((F16x16[k].Index[0]==*(s))&&(F16x16[k].Index[1]==*(s+1)))
				{
					OLED_SetPos(x , y);
					for(i=0;i<16;i++)  							//控制16列的数据输出
					{
						WriteDat(F16x16[k].Msk[i]); 		//i+32*number汉字的前16个数据输出
					}

					OLED_SetPos(x,y+1);
					for(i=0;i<16;i++)	  						//控制16列的数据输出
					{
						WriteDat(F16x16[k].Msk[i+16]);	//i+32*number+16汉字的后16个数据输出
					}
					break;
				}
			}
			s+=2;
			x +=16;
		}
		else
		{
			OLED_ShowChar(x,y,*s,2);
			x+=8;
			s+=1;
		}
	}

}
//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          : 
// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
// Description    : 显示BMP位图
//--------------------------------------------------------------
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			WriteDat(BMP[j++]);
		}
	}
}



//将128*8的数组转为128*64
void Change_to_12864(unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y,n,BMP3[1024];
	for(y=0;y<64;y=y+8)
	{
		for(x=0;x<128;x++)
		{
			BMP3[j]=BMP[j];
			for(n=0;n<8;n++)
			{	
					if(BMP3[j]&0x01)
					BMP2[x+(y+n)*128]=1;
					BMP3[j]=BMP3[j]>>1;
			}		
			j++;			
		}
	}
}
//将128*64的数组转为128*8
void Change_to_1288(unsigned char BMP4[])
{
	unsigned int j=0;
	unsigned char x,y,n,temp;
	
	for(y=0;y<64;y=y+8)
	{
		for(x=0;x<128;x++)
		{
			for(n=0;n<8;n++)
			{		
					temp|=BMP4[x+(y+n)*128]<<7;
					if(n<7)
					temp=temp>>1;
			}
			BMP1[j]=temp;
			temp=0;
			j++;
		}
	}
}

//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		BMP2[uRow+uCol*128]=1;//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}    

//显示全屏
void OLED_Fast(unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;
	for(y=0;y<8;y++)
	{		
		OLED_SetPos(0,y);
    for(x=0;x<128;x++)
		{
			WriteDat(BMP[j]);
			j++;
		}
	}
}

//显示折线图
void ZXT(unsigned char num)
{
	static unsigned char num2=0;
	unsigned char x,y;
	if(num == 0)
		num = 1;
	LCD_DrawLine(122,63-num2,126,63-num);//画线
	num2=num;
	Change_to_1288((unsigned char *)BMP2);
	for(x=0;x<124;x++)//画横轴和横轴刻度
	{
		if(x%8==0&&x!=0)
		BMP1[x+7*128]|=0xC0;
		else
		BMP1[x+7*128]|=0x80;
	}
	for(y=0;y<8;y++)//画纵轴
	{
		BMP1[y*128]|=0xFF;
	}
	for(y=0;y<8;y++)//画纵轴刻度
	{
		BMP1[1+y*128]|=0x80;
	}
	for(y=0;y<64;y++)//图像左移4个像素
	{		
    for(x=0;x<124;x++)
		{
			BMP2[x+y*128]=BMP2[x+y*128+4];
		}
	}
	for(y=0;y<64;y++)//清除最右边的一条画线
	{		
    for(x=124;x<128;x++)
		{
			BMP2[x+y*128]=0;
		}
	}
}
