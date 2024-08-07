#include "key.h"
#include "delay.h"


//KEY2 - PA8
//KEY3 - PB10	
void Key_Init(void)
{
	//1.打开GPIOA时钟
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
	
	//2.配置PA0/PA8-浮空输入
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_8;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init (GPIOB, &GPIO_InitStruct);
}

u8 pressFlag = 0;

//按键扫描函数
//返回值：0-没有按下  1-KEY1   2-KEY2
//KEY1 - 按下高电平，松开低电平
//KEY2/3 - 按下是低电平，松开高电平
//u8  - unsigned char  -- 0~255
//u16 - unsigned short -- 0~65535
//u32 - unsigned int 	 -- 0~42亿
u8 Key_Scan(void)
{
	if(((KEY1_R == 1) || (KEY2_R == 0) || (KEY3_R == 0))
		&& (pressFlag == 0))
	{
		//消抖 -- 延时 -- 10ms
		delay_ms(10);
		
		//再判断 - 电平是否时按下状态
		if(KEY1_R == 1)
		{
			pressFlag = 1;//按下标志
			return 1;
		}
		else if(KEY2_R == 0)
		{		
			pressFlag = 1;//按下标志
			return 2;
		}
		else if(KEY3_R == 0)
		{
			pressFlag = 1;//按下标志
			return 3;
		}
	}else if((KEY1_R == 0) && (KEY2_R == 1) && (KEY3_R == 1))//松手检测
	{
		pressFlag = 0;//松手标志
	}
	
	return 0;
}

