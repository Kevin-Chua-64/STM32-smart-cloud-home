#include "rgb.h"
#include "delay.h"

//RGB��ʼ������
void RGB_Init(void)
{
	//��GPIOBʱ�� �C APB2
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE);
	
	//��ʼ��PB8/PB9 �C �������
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//�������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init (GPIOB, &GPIO_InitStruct);//����ִ�й����У���û���޸Ĳ�����ֵ
	
	RGB_CLK_H;//ʱ���߿��� �ߵ�ƽ
	RGB_SetColor(0, 0, 0);
}

//��λ��ǰ
void RGB_Send32Bits(u32 data)
{
	u8 i;
	
	for(i = 0; i < 32; i++)
	{
		RGB_CLK_L;//�½��� -- ׼������
		if(data & (0x80000000 >> i))
			RGB_DATA_H;
		else
			RGB_DATA_L;
		delay_us(5);//���ݱ���ʱ��
		
		RGB_CLK_H;//������ -- �ɼ�����
		delay_us(5);//���ݲɼ�ʱ��
	}
}

//�ϳ�32bit�Ҷ�����
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

//����RGB��ɫ
void RGB_SetColor(u8 r, u8 g, u8 b)
{
	u32 color = RGB_MakeColor(r, g, b);
	
	RGB_Send32Bits(0x00000000);//32bit��ʼ֡
	RGB_Send32Bits(color);//��һ��Ҷ�����
	RGB_Send32Bits(color);//�ڶ���Ҷ�����
}
