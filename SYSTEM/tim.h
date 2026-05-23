#ifndef  __TIM_H
#define  __TIM_H
#include "sys.h"

//定时器进入一次间隔为  Hz = 72000000/(TIM_PRESCALE+1)  1/Hz 秒一次计数    1/Hz * TIM_PERIOD 进入一次中断
#define	TIM4_PRESCALE													7199
//TIM_PRESCALE为7199时 0.0001秒计数一次  TIM_PERIOD为9则是1毫秒进入一次中断
#define	TIM4_PERIOD														49

#define TIM4_1SEC_CNT_LIMIT										(1000000/(((TIM4_PRESCALE+1)*(TIM4_PERIOD+1))/72))
#define TIME4_500MS_CNT_LIMIT									(TIM4_1SEC_CNT_LIMIT/2)
#define TIME4_100MS_CNT_LIMIT									(TIM4_1SEC_CNT_LIMIT/4)


void TIM4_Init(u16 arr, u16 psc);
void TIM4_NVIC_Init(void);



/*使用说明


	TIM4_Init(TIM4_PERIOD,TIM4_PRESCALE);//初始化定时器


*/
#endif
