#include "rgb.h"
#include "delay.h"

//RGB初始化函数
void RGB_Init(void)
{
	//打开GPIOB时钟 – APB2
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE);
	
	//初始化PB8/PB9 – 推挽输出
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init (GPIOB, &GPIO_InitStruct);//函数执行过程中，并没有修改参数的值
	
	RGB_CLK_H;//时钟线空闲 高电平
	RGB_SetColor(0, 0, 0);
}

//高位在前
void RGB_Send32Bits(u32 data)
{
	u8 i;
	
	for(i = 0; i < 32; i++)
	{
		RGB_CLK_L;//下降沿 -- 准备数据
		if(data & (0x80000000 >> i))
			RGB_DATA_H;
		else
			RGB_DATA_L;
		delay_us(5);//数据保持时间
		
		RGB_CLK_H;//上升沿 -- 采集数据
		delay_us(5);//数据采集时间
	}
}

//合成32bit灰度数据
u32 RGB_MakeColor(u8 r, u8 g, u8 b)
{
	u32 color = 0;
	
	color |= (0x3u << 30);
	color |= (((u8)~b >> 6) << 28);
	color |= (((u8)~g >> 6) << 26);
	color |= (((u8)~r >> 6) << 24);
	color |= (b << 16);
	color |= (g << 8);
	color |= (r << 0);
	
	return color;
}

//设置RGB颜色
void RGB_SetColor(u8 r, u8 g, u8 b)
{
	u32 color = RGB_MakeColor(r, g, b);
	
	RGB_Send32Bits(0x00000000);//32bit起始帧
	RGB_Send32Bits(color);//第一点灰度数据
	RGB_Send32Bits(color);//第二点灰度数据
}
