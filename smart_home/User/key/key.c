#include "key.h"
#include "delay.h"


//KEY2 - PA8
//KEY3 - PB10	
void Key_Init(void)
{
	//1.��GPIOAʱ��
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
	
	//2.����PA0/PA8-��������
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_8;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init (GPIOB, &GPIO_InitStruct);
}

u8 pressFlag = 0;

//����ɨ�躯��
//����ֵ��0-û�а���  1-KEY1   2-KEY2
//KEY1 - ���¸ߵ�ƽ���ɿ��͵�ƽ
//KEY2/3 - �����ǵ͵�ƽ���ɿ��ߵ�ƽ
//u8  - unsigned char  -- 0~255
//u16 - unsigned short -- 0~65535
//u32 - unsigned int 	 -- 0~42��
u8 Key_Scan(void)
{
	if(((KEY1_R == 1) || (KEY2_R == 0) || (KEY3_R == 0))
		&& (pressFlag == 0))
	{
		//���� -- ��ʱ -- 10ms
		delay_ms(10);
		
		//���ж� - ��ƽ�Ƿ�ʱ����״̬
		if(KEY1_R == 1)
		{
			pressFlag = 1;//���±�־
			return 1;
		}
		else if(KEY2_R == 0)
		{		
			pressFlag = 1;//���±�־
			return 2;
		}
		else if(KEY3_R == 0)
		{
			pressFlag = 1;//���±�־
			return 3;
		}
	}else if((KEY1_R == 0) && (KEY2_R == 1) && (KEY3_R == 1))//���ּ��
	{
		pressFlag = 0;//���ֱ�־
	}
	
	return 0;
}

