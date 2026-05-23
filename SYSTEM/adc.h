#ifndef __ADC_H
#define __ADC_H
#include "sys.h"


#define ADC1_DR_Address    ((uint32_t)0x4001244C) //ADC1这个外设的地址（查参考手册得出）

#define ADCPORT		GPIOA	//定义ADC接口
#define ADC_CH0		GPIO_Pin_0	//定义ADC接口
#define ADC_CH1		GPIO_Pin_1	//定义ADC接口
#define ADC_CH2		GPIO_Pin_2	//定义ADC接口
#define ADC_CH3		GPIO_Pin_3	//定义ADC接口
#define ADC_CH4		GPIO_Pin_4	//定义ADC接口
#define ADC_CH5		GPIO_Pin_5	//定义ADC接口
#define ADC_CH6		GPIO_Pin_6	//定义ADC接口
#define ADC_CH7		GPIO_Pin_7	//定义ADC接口

#define ADC_GET_NUM													5

void ADC_DMA_Init(void);
void ADC_GPIO_Init(void);
void ADC_Configuration(void);

extern vu16 ADC_DMA_IN[ADC_GET_NUM];

//使用说明

/*

	ADC_Configuration();//ADC通道初始化


*/

#endif





























