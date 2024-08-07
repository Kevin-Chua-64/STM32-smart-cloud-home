#include "relay.h"

/*************************
函数名：Relay_Init
函数功能：继电器的初始化
函数形参：void
函数的返回值：void
备注：RELAY-----PB1
*************************/
void Relay_Init(void)
{
	//打开时钟
	RCC->APB2ENR |=(1<<3);
	
	//PB1的模式
	GPIOB->CRL &=~(0XF<<4);
	GPIOB->CRL |=(1<<4); //通用推挽输出
}





