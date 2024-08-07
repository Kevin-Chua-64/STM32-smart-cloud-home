#include "oled.h"
#include "delay.h"

//管脚初始化
//OLED_RES 	- PB6
//OLED_DC 	- PB12
//OLED_CS 	- PB7
//OLED_SCK 	- PB13
//OLED_MOSI	- PB15
//MODE3 - SCK空闲电平 = 高电平
void OLED_PinInit(void)
{
	//打开GPIOB时钟
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;//结构体变量
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	OLED_RES_H;//normal operation
	OLED_CS_H;//补选中从机 - 休眠
	OLED_SCK_H;//MDOE3 - 空闲为高电平
}

//SPI传输byte函数
void OLED_SPITransferByte(u8 byte)
{
	u8 i;
	
	for(i = 0; i < 8; i++)
	{
		OLED_SCK_L;//下降沿 - 准备数据
		if(byte & (0x80 >> i))
			OLED_MOSI_H;
		else
			OLED_MOSI_L;
		
		OLED_SCK_H;//上升沿- 采集数据
		//从机采集
	}
}

//OLED发送命令
void OLED_SendCommand(u8 cmd)
{
	OLED_CS_L;
	OLED_DC_L;
	OLED_SPITransferByte(cmd);
	OLED_CS_H;
}

//发送数据
void OLED_SendData(u8 data)
{
	OLED_CS_L;
	OLED_DC_H;
	OLED_SPITransferByte(data);
	OLED_CS_H;
}

//清屏函数
void OLED_Clear(u8 data)
{
	u8 i, j;
	
	for(i = 0; i < 8; i++)//控制页数
	{
		//设置起始地址
		OLED_SendCommand(0xB0 + i);//起始页地址  0~7
		OLED_SendCommand(0x00);//起始列地址低四位  0
		OLED_SendCommand(0x10);//起始列地址高四位  0
		
		for(j = 0; j < 128; j++)//控制列数
		{
			OLED_SendData(data);
		}
	}
}

void OLED_Init(void)
{
	OLED_PinInit( );
	
	OLED_RES_H;
	delay_ms(100);
	OLED_RES_L;//初始化芯片
	delay_ms(100);
	OLED_RES_H;
	delay_ms(100); 
	
	OLED_SendCommand(0xAE); //关闭显示
	OLED_SendCommand(0xD5); //设置时钟分频因子,震荡频率
	OLED_SendCommand(80);   //[3:0],分频因子;[7:4],震荡频率
	OLED_SendCommand(0xA8); //设置驱动路数
	OLED_SendCommand(0X3F); //默认0X3F(1/64)
	OLED_SendCommand(0xD3); //设置显示偏移
	OLED_SendCommand(0X00); //默认为0

	OLED_SendCommand(0x40); //设置显示开始行 [5:0],行数.

	OLED_SendCommand(0x8D); //电荷泵设置
	OLED_SendCommand(0x14); //bit2，开启/关闭
	OLED_SendCommand(0x20); //设置内存地址模式
	OLED_SendCommand(0x02); //[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
	OLED_SendCommand(0xA1); //段重定义设置,bit0:0,0->0;1,0->127;
	OLED_SendCommand(0xC8); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
	OLED_SendCommand(0xDA); //设置COM硬件引脚配置
	OLED_SendCommand(0x12); //[5:4]配置

	OLED_SendCommand(0x81); //对比度设置
	OLED_SendCommand(0xEF); //1~255;默认0X7F (亮度设置,越大越亮)
	OLED_SendCommand(0xD9); //设置预充电周期
	OLED_SendCommand(0xf1); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_SendCommand(0xDB); //设置VCOMH 电压倍率
	OLED_SendCommand(0x30); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_SendCommand(0xA4); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
	OLED_SendCommand(0xA6); //设置显示方式;bit0:1,反相显示;0,正常显示
	OLED_SendCommand(0xAF); //开启显示	
	
	OLED_Clear(0x00);//清屏
}
