#include "relay.h"

/*************************
��������Relay_Init
�������ܣ��̵����ĳ�ʼ��
�����βΣ�void
�����ķ���ֵ��void
��ע��RELAY-----PB1
*************************/
void Relay_Init(void)
{
	//��ʱ��
	RCC->APB2ENR |=(1<<3);
	
	//PB1��ģʽ
	GPIOB->CRL &=~(0XF<<4);
	GPIOB->CRL |=(1<<4); //ͨ���������
}





