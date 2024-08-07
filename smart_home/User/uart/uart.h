#ifndef _UART_H_
#define _UART_H_

#include "stm32f10x.h"

void Uart1_Init(u32 baud);
void Uart1_SendChar(u8 ch);
void Uart1_SendString(char *p);
u8 Uart1_RecieveByte(void);


#endif
