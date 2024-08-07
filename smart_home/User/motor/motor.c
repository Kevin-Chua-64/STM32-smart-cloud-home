#include "motor.h"
#include "delay.h"

//步进电机初始化 Linux
void StepMotor_Init(void)
{
	//打开GPIOA时钟
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
	
	//初始化PA4/5/6/7为推挽输出
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//推挽复用输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;//没有特殊 要求按最低速处理
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//初始化电机处于静止状态 -- 线圈不能上电 -- 输出低电平
	STEP_MOTOR_STOP;
}

//控制步进电机运动方向
//dir: 0 -- 左移  1--右移
//n : 转动圈数
void StepMotor_Move(u8 dir, u8 n)
{
	u8 i;
	
	if(dir == 0)//左移  节拍 1->2->3->4
	{
		for(i = 0; i < n; i++)
		{
			BEAT_1;//节拍1
			delay_us(800);
			BEAT_2;//节拍2
			delay_us(800);
			BEAT_3;//节拍3
			delay_us(800);
			BEAT_4;//节拍4
			delay_us(800);
		}
	}else //右移  节拍 4->3->2->1
	{
		for(i = 0; i < n; i++)
		{
			BEAT_4;//节拍4
			delay_us(800);
			BEAT_3;//节拍3
			delay_us(800);
			BEAT_2;//节拍2
			delay_us(800);
			BEAT_1;//节拍1
			delay_us(800);
		}
	}
	STEP_MOTOR_STOP;
}

//直流电机初始化
void DcMotor_Init(void)
{
	//解除PB4占用
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB, ENABLE);//打开AFIO时钟
	GPIO_PinRemapConfig (GPIO_Remap_SWJ_JTAGDisable, ENABLE);//映射

#if USE_TIM_PWM
	//初始化PB4/PB5为推挽输出
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//推挽复用输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;//没有特殊 要求按最低速处理
	GPIO_Init (GPIOB, &GPIO_InitStruct);
	
	//映射到TIM3
	GPIO_PinRemapConfig (GPIO_PartialRemap_TIM3, ENABLE);//部分重映射
	
	//打开TIM3时钟 - APB1
	RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM3, ENABLE);
	
	//选择内部时钟源
	TIM_InternalClockConfig (TIM3);
	
	//初始化时基单元
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//递增计数
	TIM_TimeBaseInitStruct.TIM_Prescaler = 7200 - 1;//分频值 - 计数周期  72MHz / 7200 - 1/100 -- 100us
	TIM_TimeBaseInitStruct.TIM_Period = 100 - 1;//重载值 - 计数次数 10ms - 100份 - 100us
	TIM_TimeBaseInit (TIM3, &TIM_TimeBaseInitStruct);
	
	//初始化输出通道
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;//CNT<比较值 - 高电平
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;//不反相
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;//使能输出通道
	TIM_OCInitStruct.TIM_Pulse = 0;//CNT >= 0 -- 低电平
	
	TIM_OC1Init (TIM3, &TIM_OCInitStruct);//通道1初始化
	TIM_OC2Init (TIM3, &TIM_OCInitStruct);//通道2初始化
	
	//使能TIM3
	TIM_Cmd (TIM3, ENABLE);
#else	
	//初始化PB4/PB5为推挽输出
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;//没有特殊 要求按最低速处理
	GPIO_Init (GPIOB, &GPIO_InitStruct);
	
	DC_MOTOR_STOP;
#endif
}

//电机转速的控制
//参数：速度  -99 ~ 99
void DcMotor_SetSpeed(s8 speed)
{
	if(speed < 0)//反转
	{
		TIM_SetCompare1 (TIM3, 0);//BAK
		TIM_SetCompare2 (TIM3, -speed);//FOR
	}else //正转
	{
		TIM_SetCompare1 (TIM3, speed);//BAK
		TIM_SetCompare2 (TIM3, 0);//FOR
	}
}
