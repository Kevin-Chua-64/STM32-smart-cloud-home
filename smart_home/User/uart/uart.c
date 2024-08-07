#include "uart.h"
#include <stdio.h>

//USART1��ʼ��
void Uart1_Init(u32 baud)
{
	//1.��ʼ��IO��
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);//a)��GPIOAʱ��
	
	//b)PA9 �C TX �C �����������
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//�����������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//c)PA10 �C RX �C ��������
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//d)����PA9��PA10Ĭ�Ͼ�ӳ�䵽USART1_TX/RX���Բ���Ҫӳ��
	
	//2.��ʼ��USART1
	//��USART1ʱ��
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1, ENABLE);
	
	//������Ҫ��
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = baud;//������
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;//ʹ�ܷ��ͺͽ���ģʽ
	USART_InitStruct.USART_Parity = USART_Parity_No;//����żУ��
	USART_InitStruct.USART_StopBits = USART_StopBits_1;//ֹͣλ = 1bit
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;;//����λ = 8bit
	USART_Init (USART1, &USART_InitStruct);
	
	//����NVIC
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;//�ж�Դ���
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//ʹ��NVIC��Ӧ
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;//��Ӧ���ȼ�
	NVIC_Init (&NVIC_InitStruct);
	
	//ʹ�ܽ����ж�
	USART_ITConfig (USART1, USART_IT_RXNE, ENABLE);
	
	//ʹ�ܴ���
	USART_Cmd (USART1, ENABLE);
}

//�жϷ�����
void USART1_IRQHandler(void)
{
	//�ж���ʲô�жϽ�����
	if(USART_GetITStatus (USART1, USART_IT_RXNE) == SET)//�����ж�
	{
		//��������жϱ�־
		USART_ClearITPendingBit (USART1, USART_IT_RXNE);
		
		printf("%c", USART_ReceiveData (USART1));
	}
}


//�����ֽں���
void Uart1_SendChar(u8 ch)
{
	//�ȴ����ͻ�����Ϊ��
	while(USART_GetFlagStatus (USART1, USART_FLAG_TXE) != SET)
	{
		;//��
	}
	
	//��������
	USART_SendData (USART1, ch);
}

//�����ַ�������
void Uart1_SendString(char *p)//"hello world"
{
	//�����ַ����Ľ����ַ��ж��Ƿ���Ҫ��������  '\0'
	//*p != '\0'
	//����һ���ַ� *p
	while(*p != '\0')
		Uart1_SendChar(*p++);
}

//����һ���ֽں���
u8 Uart1_RecieveByte(void)
{
	//�ȴ����ջ������ǿ�
	while(USART_GetFlagStatus (USART1, USART_FLAG_RXNE) != SET)
	{
		;
	}
	//��ȡ���� 
	return USART_ReceiveData (USART1);
}

/*******printf ֧��/��ӳ�� ����*******/
#if 1
#pragma import(__use_no_semihosting_swi) //ȡ��������״̬
 
struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
 
int fputc(int ch, FILE *f) {
	while(USART_GetFlagStatus (USART1, USART_FLAG_TXE) != SET) {};
	USART_SendData (USART1, ch);;
  return (ch);
}
int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}
 
void _ttywrch(int ch) {
  while((USART1->SR &(0X01<<7))==0){};
  USART1->DR=ch;
}
 
void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
#endif
