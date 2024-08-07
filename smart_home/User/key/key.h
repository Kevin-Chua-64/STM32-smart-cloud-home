#ifndef _KEY_H_
#define _KEY_H_

#include "stm32f10x.h"

#define KEY1_R	GPIO_ReadInputDataBit (GPIOA, GPIO_Pin_0)
#define KEY2_R	GPIO_ReadInputDataBit (GPIOA, GPIO_Pin_8)
#define KEY3_R	GPIO_ReadInputDataBit (GPIOB, GPIO_Pin_10)

void Key_Init(void);
u8 Key_Scan(void);

#endif
