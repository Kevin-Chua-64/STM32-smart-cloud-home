#include "led.h"
#include "delay.h"

//LED初始化函数
void Led_Init(void)
{
	//打开GPIOB/GPIOA时钟 – APB2
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	
	//初始化PB11/PB14 – 推挽输出
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init (GPIOB, &GPIO_InitStruct);//函数执行过程中，并没有修改参数的值
	
	//PA11 - 推挽输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//初始化LED为灭 - PB11输出低电平
	LED1_OFF;
	LED2_OFF;
	LED3_OFF;
}


//跑马灯
void Led_HorseRunning(void)
{
	LED1_ON;
	delay_ms(100);
	LED1_OFF;
//	delay_ms(50);
	
	LED2_ON;
	delay_ms(100);
	LED2_OFF;
//	delay_ms(50);
	
	LED3_ON;
	delay_ms(100);
	LED3_OFF;
	delay_ms(100);
}

//流水灯
void Led_WaterRunning(void)
{
	LED1_ON;
	delay_ms(100);
	LED2_ON;
	delay_ms(100);
	LED3_ON;
	delay_ms(100);
	
	LED1_OFF;
	LED2_OFF;
	LED3_OFF;
	delay_ms(100);
}

