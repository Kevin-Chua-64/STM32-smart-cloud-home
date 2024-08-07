#include "uart.h"
#include <stdio.h>

//USART1初始化
void Uart1_Init(u32 baud)
{
	//1.初始化IO口
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);//a)打开GPIOA时钟
	
	//b)PA9 – TX – 复用推挽输出
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//c)PA10 – RX – 浮空输入
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//d)由于PA9、PA10默认就映射到USART1_TX/RX所以不需要映射
	
	//2.初始化USART1
	//打开USART1时钟
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1, ENABLE);
	
	//配置四要素
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = baud;//波特率
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件流控
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;//使能发送和接收模式
	USART_InitStruct.USART_Parity = USART_Parity_No;//无奇偶校验
	USART_InitStruct.USART_StopBits = USART_StopBits_1;//停止位 = 1bit
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;;//数据位 = 8bit
	USART_Init (USART1, &USART_InitStruct);
	
	//配置NVIC
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;//中断源编号
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//使能NVIC响应
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;//响应优先级
	NVIC_Init (&NVIC_InitStruct);
	
	//使能接收中断
	USART_ITConfig (USART1, USART_IT_RXNE, ENABLE);
	
	//使能串口
	USART_Cmd (USART1, ENABLE);
}

//中断服务函数
void USART1_IRQHandler(void)
{
	//判断是什么中断进来的
	if(USART_GetITStatus (USART1, USART_IT_RXNE) == SET)//接收中断
	{
		//清除接收中断标志
		USART_ClearITPendingBit (USART1, USART_IT_RXNE);
		
		printf("%c", USART_ReceiveData (USART1));
	}
}


//发送字节函数
void Uart1_SendChar(u8 ch)
{
	//等待发送缓冲区为空
	while(USART_GetFlagStatus (USART1, USART_FLAG_TXE) != SET)
	{
		;//空
	}
	
	//发送数据
	USART_SendData (USART1, ch);
}

//发送字符串函数
void Uart1_SendString(char *p)//"hello world"
{
	//根据字符串的结束字符判断是否还需要继续发送  '\0'
	//*p != '\0'
	//发送一个字符 *p
	while(*p != '\0')
		Uart1_SendChar(*p++);
}

//接收一个字节函数
u8 Uart1_RecieveByte(void)
{
	//等待接收缓冲区非空
	while(USART_GetFlagStatus (USART1, USART_FLAG_RXNE) != SET)
	{
		;
	}
	//读取数据 
	return USART_ReceiveData (USART1);
}

/*******printf 支持/重映像 函数*******/
#if 1
#pragma import(__use_no_semihosting_swi) //取消半主机状态
 
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
