/*
 * 文件名: main.c
 * 描述: 太阳能追光系统主控制程序
 * 功能: 通过光敏传感器检测四个方向的光强，控制舵机调整太阳能板角度，使其始终朝向光源
 * 硬件平台: STM32单片机
 */

 // 包含所需的头文件
#include "sys.h"      // 系统配置
#include "stdio.h"    // 标准输入输出
#include "delay.h"    // 延时函数
#include "key.h"      // 按键处理
#include "device.h"   // 外部设备控制
#include "tim.h"      // 定时器
#include "OLED_I2C.h" // OLED显示
#include "pwm.h"      // PWM控制
#include "serve.h"    // 舵机控制
#include "adc.h"      // 模数转换
#include "ds18b20.h"  // 温度传感器
#include "flash.h"    // Flash存储
#include "usart.h"

/********************宏定义***************************/
// 垂直舵机角度限制
// 垂直舵机上限角度
#define SERVOUP_LIMIT1    130
// 垂直舵机下限角度
#define SERVOUP_LIMIT2    40

// 水平舵机角度限制
// 水平舵机右限角度
#define SERVODOWN_LIMIT1  190
// 水平舵机左限角度
#define SERVODOWN_LIMIT2  10

// 舵机中心位置角度
// 舵机中心位置(平衡位置)
#define SERVO_CENTER      88

// 工作光强阈值
// 光强低于此值时不工作
#define WORK_SERVO_LIGHT  200

/********************变量定义***************************/
u8 Key_Value;  // 存储按键值
u8 menu;       // 当前菜单页面

// 定时器计时计数作用变量
u16 Time_Cnt[2];  // 用于定时器中断计数的数组

// 数据获取显示标志位
_Bool Flag_Get;   // 标记是否需要获取和显示数据
// 报警标志位
_Bool Flag_Alarm; // 标记是否需要报警

// 定义一个16位的无符号整数变量Pwn_setDOWN，并赋值为SERVO_CENTER，表示水平舵机当前角度
u16 Pwn_setDOWN = SERVO_CENTER;
// 定义一个16位的无符号整数变量Pwn_setUP，并赋值为SERVO_CENTER，表示垂直舵机当前角度
u16 Pwn_setUP = SERVO_CENTER;

// 设置检测灵敏度，越小越灵敏，表示上下左右光强差异的容忍度
u16 Sensitivity = 200;

// 声明两个16位有符号整数变量，用于存储上下和左右方向的光强差值
s16 diffUD, diffLR;

// 定义四个无符号16位变量，分别表示向上、向下、向左、向右的光强
u16 L_Up, L_Down, L_Left, L_Right;

// 工作模式: 0-自动, 1-手动
_Bool Mode;

// 电池电压
float Volat;

// 定义一个无符号16位整数变量Temp，用于存储温度值
u16 Temp;

/********************函数声明****************************/
// 按键处理函数
void Key_Proc(void);
// 显示动态数据
void Dis_Dat(void);
// 获取数据
void Get_Dat(void);
// 显示菜单静态页面
void Dis_Menu(void);
// 舵机处理函数
void Proc_Servo(void);
// 读取flash保存数据
void Read_Flash_Data(void);
// 写入数据到flash
void Write_Flash_Data(void);


/*
 * 函数名: main
 * 描述: 主函数，初始化硬件并进入主循环
 * 参数: 无
 * 返回值: int (程序执行状态)
 */
int main(void)
{
	// 设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	// 使能GPIOA、GPIOB、GPIOC和AFIO的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	// 禁用JTAG，只保留SWD调试功能，释放部分IO口
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	delay_init();         // 延时函数初始化
	KEY_Init();           // 按键初始化
	I2C_Configuration();  // I2C配置，用于OLED显示
	Servo_Init();         // 舵机初始化
	Device_Init();        // 外部设备IO口初始化
	Relay = 0;            // 继电器初始状态为关闭
	DS18B20_Init();       // DS18B20温度模块初始化
	ADC_Configuration();  // ADC初始化，用于读取光敏电阻和电池电压
	TIM4_Init(TIM4_PERIOD, TIM4_PRESCALE);  // 初始化定时器4，用于定时任务
	Read_Flash_Data();    // 读取掉电保存的数据(灵敏度设置)
	Dis_Dat();            // 初始化显示界面

	// 将两个舵机设置到中心位置
	Servo1_SetAngle(SERVO_CENTER);  // 设置水平舵机角度
	Servo2_SetAngle(SERVO_CENTER);  // 设置垂直舵机角度

	// 主循环
	while (1)
	{
		Key_Proc();  // 处理按键输入
		Get_Dat();   // 获取传感器数据并处理
	}
}


/*
 * 函数名: Read_Flash_Data
 * 描述: 从Flash中读取保存的设置参数
 * 参数: 无
 * 返回值: 无
 */
void Read_Flash_Data(void)
{
	u8 check, i = 0;
	// 读取Flash起始地址的数据作为校验值
	check = FLASH_R(FLASH_START_ADDR);
	// 如果校验值正确，说明Flash中有有效数据
	if (check == CHECK_FLASH)
	{
		i += 2;  // 跳过校验值所占的空间
		// 读取灵敏度设置
		Sensitivity = FLASH_R(FLASH_START_ADDR + i);
	}
	// 如果校验值不正确，则使用默认值(已在变量定义时设置)
}

/*
 * 函数名: Write_Flash_Data
 * 描述: 将当前设置参数写入Flash，实现掉电保存
 * 参数: 无
 * 返回值: 无
 */
void Write_Flash_Data(void)
{
	u8 i = 0;
	FLASH_Unlock();  // 解锁FLASH编程擦除控制器
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);  // 清除所有标志位
	FLASH_ErasePage(FLASH_START_ADDR);  // 擦除指定地址页

	// 写入校验值，用于标识Flash中有有效数据
	FLASH_ProgramHalfWord(FLASH_START_ADDR, CHECK_FLASH);

	i += 2;  // 跳过校验值所占的空间
	// 写入灵敏度设置
	FLASH_ProgramHalfWord(FLASH_START_ADDR + i, Sensitivity);

	// 清除标志位并锁定Flash
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_Lock();  // 锁定FLASH编程擦除控制器
}


/*
 * 函数名: Key_Proc
 * 描述: 按键处理函数，根据按键输入执行相应操作
 * 参数: 无
 * 返回值: 无
 */
void Key_Proc(void)
{
	// 根据当前模式选择按键扫描方式
	// 自动模式下不支持连续按键，手动模式下支持连续按键
	if (Mode == 0)
		Key_Value = KEY_Scan(0);  // 自动模式，不支持连续按键
	else
		Key_Value = KEY_Scan(1);  // 手动模式，支持连续按键

	// 根据按键值执行相应操作
	switch (Key_Value)
	{
		case 1:  // 中键
		{
			if (menu == 0)
				Mode = !Mode;  // 在主界面切换自动/手动模式
			break;
		}

		case 2:  // 右键
		{
			switch (menu)
			{
				case 0:  // 主界面
				{
					if (Mode == 1)  // 手动模式下
					{
						// 增加水平舵机角度，但不超过上限
						if (Pwn_setDOWN < SERVODOWN_LIMIT1)
							Pwn_setDOWN++;
					}
					else  // 自动模式下
					{
						// 进入设置菜单
						if (menu == 0)
							menu = 1;
						OLED_CLS();  // 清屏
					}
					break;
				}

				case 1:  // 设置菜单
				{
					menu = 0;  // 返回主界面
					Write_Flash_Data();  // 保存设置数据到Flash
					OLED_CLS();  // 清屏
					break;
				}

				default:
					break;
			}
			break;
		}

		case 3:  // 左键
		{
			switch (menu)
			{
				case 0:  // 主界面
				{
					if (Mode == 1)  // 手动模式下
					{
						// 减少水平舵机角度，但不低于下限
						if (Pwn_setDOWN > SERVODOWN_LIMIT2)
							Pwn_setDOWN--;
					}
					break;
				}

				default:
					break;
			}
			break;
		}


		case 4:  // 下键
		{
			switch (menu)
			{
				case 0:  // 主界面
				{
					if (Mode == 1)  // 手动模式下
					{
						// 增加垂直舵机角度，但不超过上限
						if (Pwn_setUP < SERVOUP_LIMIT1)
							Pwn_setUP++;
					}
					break;
				}

				case 1:
				{
					if (Sensitivity > 10)
						Sensitivity -= 10;
					break;
				}

				default:
					break;
			}
			break;
		}

		case 5:  // 上键
		{
			switch (menu)
			{
				case 0:  // 主界面
				{
					if (Mode == 1)  // 手动模式下
					{
						// 减少垂直舵机角度，但不低于下限
						if (Pwn_setUP > SERVOUP_LIMIT2)
							Pwn_setUP--;
					}
					break;
				}

				case 1:  // 设置菜单
				{
					// 增大灵敏度值(降低灵敏度)，但不高于500
					if (Sensitivity < 500)
						Sensitivity += 10;
					break;
				}

				default:
					break;
			}
			break;
		}

		case 6:  // 切换继电器状态来控制灯光开关
		{
			Relay = !Relay;  // 切换继电器状态
			break;
		}

		default:
			break;
	}

	// 如果有按键按下，更新显示或舵机位置
	if (Key_Value != 0)
	{
		if (Mode == 0)  // 自动模式下
			Dis_Dat();  // 更新显示数据
		else  // 手动模式下
		{
			// 对于中键和第六个按键，增加延时防抖
			if (Key_Value == 1 || Key_Value == 6)
				delay_ms(500);
			// 设置舵机角度
			Servo1_SetAngle(Pwn_setDOWN);  // 设置水平舵机角度
			Servo2_SetAngle(Pwn_setUP);    // 设置垂直舵机角度
		}
	}
}

/*
 * 函数名: Dis_Dat
 * 描述: 显示动态数据，根据当前菜单状态显示不同内容
 * 参数: 无
 * 返回值: 无
 */
void Dis_Dat(void)
{
	switch (menu)
	{
		case 0:  // 主界面
		{
			// 显示电池电压、温度、工作模式和四个方向的光强
			OLED_ShowCN(0 * 16, 0 * 2, "电池电压:%2.1f", Volat);  // 显示电池电压
			OLED_ShowCN(0 * 16, 1 * 2, "温度:%3d℃  %s", Temp, Mode ? "手动" : "自动");  // 显示温度和工作模式
			OLED_ShowCN(0 * 16, 2 * 2, "上:%4d  下:%4d", L_Up, L_Down);  // 显示上下光强
			OLED_ShowCN(0 * 16, 3 * 2, "左:%4d  右:%4d", L_Left, L_Right);  // 显示左右光强
			break;
		}

		case 1:  // 设置菜单
		{
			// 显示灵敏度设置界面
			OLED_ShowCN(0 * 16, 0 * 2, "  设置自动追光  ");  // 标题
			OLED_ShowCN(0 * 16, 1 * 2, "     灵敏度    ");   // 参数名称
			OLED_ShowCN(0 * 16, 2 * 2, "     %03d    ", Sensitivity);  // 当前灵敏度值
			break;
		}

		default:
			break;
	}
}

/*
 * 函数名: Get_Dat
 * 描述: 获取传感器数据并处理，在自动模式下调整舵机位置
 * 参数: 无
 * 返回值: 无
 */
void Get_Dat(void)
{
	// 在定时器中有定时，固定时间进入一次来获取数据和显示数据
	if (Flag_Get == 1)
	{
		// 将标志位清零，以重新计时
		Flag_Get = 0;

		// 获取四个方向的光强值(通过ADC)
		L_Up = ADC_DMA_IN[1];     // 上方光强
		L_Down = ADC_DMA_IN[2];   // 下方光强
		L_Left = ADC_DMA_IN[3];   // 左侧光强
		L_Right = ADC_DMA_IN[4];  // 右侧光强

		// 计算电池电压，ADC值转换为实际电压值
		Volat = ((3.3 * ADC_DMA_IN[0]) / 4096) * 2;

		// 获取温度值，除以10转换为整数的温度值
		Temp = DS18B20_Get_Temp() / 10;

		// 自动模式下的处理
		if (Mode == 0)
		{
			// 计算上下、左右的光强差
			diffUD = L_Up - L_Down;    // 上下光强差
			diffLR = L_Left - L_Right; // 左右光强差

			// 当四个方位的光照值都小于设定值时，让太阳能板回正
			if (L_Up < WORK_SERVO_LIGHT && L_Down < WORK_SERVO_LIGHT &&
				L_Left < WORK_SERVO_LIGHT && L_Right < WORK_SERVO_LIGHT)
			{
				// 当光线不充足时，舵机使太阳能板回归中心位置
				Pwn_setDOWN = Pwn_setUP = SERVO_CENTER;
			}
			else
			{
				// 只有四个方位光线相加大于4.5倍的设定值时才进行舵机调整
				// 这是为了确保有足够的光线进行有效的追踪
				if (L_Up + L_Down + L_Left + L_Right >= WORK_SERVO_LIGHT * 4.5)
				{
					// 调用舵机处理函数，根据光强差调整舵机角度
					Proc_Servo();
				}
			}
		}

		// 设置舵机角度，无论是自动还是手动模式
		Servo1_SetAngle(Pwn_setDOWN);  // 设置水平舵机角度
		Servo2_SetAngle(Pwn_setUP);    // 设置垂直舵机角度

		// 更新显示数据
		Dis_Dat();
	}
}

/*
 * 函数名: Proc_Servo
 * 描述: 舵机处理函数，根据光强差值调整舵机角度，实现追光功能
 * 参数: 无
 * 返回值: 无
 */
void Proc_Servo(void)
{
	// 检查上下光强差异是否超出灵敏度范围，如果超出则调整垂直舵机角度
	if (-1 * Sensitivity > diffUD || diffUD > Sensitivity)
	{
		if (L_Up > L_Down) // 上方光强大于下方光强
		{
			// 减小垂直舵机角度，使太阳能板向上倾斜
			Pwn_setUP--;
			// 确保不超出下限
			if (Pwn_setUP < SERVOUP_LIMIT2)
			{
				Pwn_setUP = SERVOUP_LIMIT2;
			}
		}
		else if (L_Up < L_Down) // 上方光强小于下方光强
		{
			// 增加垂直舵机角度，使太阳能板向下倾斜
			Pwn_setUP++;
			// 确保不超出上限
			if (Pwn_setUP > SERVOUP_LIMIT1)
			{
				Pwn_setUP = SERVOUP_LIMIT1;
			}
		}
	}

	// 检查左右光强差异是否超出灵敏度范围，如果超出则调整水平舵机角度
	if (-1 * Sensitivity > diffLR || diffLR > Sensitivity)
	{
		if (L_Left > L_Right)  // 左侧光强大于右侧光强
		{
			// 根据垂直舵机位置决定水平舵机的调整方向
			if (Pwn_setUP < SERVO_CENTER)  // 垂直舵机在中心位置以上
			{
				// 增加水平舵机角度
				Pwn_setDOWN++;
				// 确保不超出上限
				if (Pwn_setDOWN > SERVODOWN_LIMIT1)
				{
					Pwn_setDOWN = SERVODOWN_LIMIT1;
					// 如果光强差异过大，重置舵机位置
					if (L_Left - L_Right > Sensitivity * 2)
					{
						Pwn_setDOWN = Pwn_setUP = SERVO_CENTER;
					}
				}
			}
			else  // 垂直舵机在中心位置以下
			{
				// 减小水平舵机角度
				Pwn_setDOWN--;
				// 确保不低于下限
				if (Pwn_setDOWN < SERVODOWN_LIMIT2)
				{
					Pwn_setDOWN = SERVODOWN_LIMIT2;
					// 如果光强差异过大，重置舵机位置
					if (L_Left - L_Right > Sensitivity * 2)
					{
						Pwn_setDOWN = Pwn_setUP = SERVO_CENTER;
					}
				}
			}
		}
		else if (L_Left < L_Right) // 左侧光强小于右侧光强
		{
			// 根据垂直舵机位置决定水平舵机的调整方向
			if (Pwn_setUP < SERVO_CENTER)  // 垂直舵机在中心位置以上
			{
				// 减小水平舵机角度
				Pwn_setDOWN--;
				// 确保不低于下限
				if (Pwn_setDOWN < SERVODOWN_LIMIT2)
				{
					Pwn_setDOWN = SERVODOWN_LIMIT2;
					// 如果光强差异过大，重置舵机位置
					if (L_Right - L_Left > Sensitivity * 2)
					{
						Pwn_setDOWN = Pwn_setUP = SERVO_CENTER;
					}
				}
			}
			else  // 垂直舵机在中心位置以下
			{
				// 增加水平舵机角度
				Pwn_setDOWN++;
				// 确保不超出上限
				if (Pwn_setDOWN > SERVODOWN_LIMIT1)
				{
					Pwn_setDOWN = SERVODOWN_LIMIT1;
					// 如果光强差异过大，重置舵机位置
					if (L_Right - L_Left > Sensitivity * 2)
					{
						Pwn_setDOWN = Pwn_setUP = SERVO_CENTER;
					}
				}
			}
		}
	}
}

/*
 * 函数名: TIM4_IRQHandler
 * 描述: TIM4定时器中断处理函数，用于定时任务
 * 参数: 无
 * 返回值: 无
 */
void TIM4_IRQHandler(void)
{
	// 判断是否是TIM4更新中断
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		// 清除中断标志位
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

		// 通过计时变量设置数据获取和显示的标志位
		// 每隔TIM4_1SEC_CNT_LIMIT/10的时间(约100ms)获取和显示一次数据
		if (Flag_Get == 0 && Time_Cnt[0]++ >= TIM4_1SEC_CNT_LIMIT / 10)
		{
			Flag_Get = 1;  // 置起标志位，触发数据获取和显示
			Time_Cnt[0] = 0;  // 清零计数器，重新开始计时
		}
	}
}

