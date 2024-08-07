#include "led.h"
#include "delay.h"

//LED��ʼ������
void Led_Init(void)
{
	//��GPIOB/GPIOAʱ�� �C APB2
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	
	//��ʼ��PB11/PB14 �C �������
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//�������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init (GPIOB, &GPIO_InitStruct);//����ִ�й����У���û���޸Ĳ�����ֵ
	
	//PA11 - �������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//��ʼ��LEDΪ�� - PB11����͵�ƽ
	LED1_OFF;
	LED2_OFF;
	LED3_OFF;
}


//�����
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

//��ˮ��
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

