#ifndef _RGB_H_
#define _RGB_H_

#include "stm32f10x.h"

#define RGB_CLK_H 	GPIO_SetBits (GPIOB, GPIO_Pin_8)
#define RGB_CLK_L 	GPIO_ResetBits (GPIOB, GPIO_Pin_8)
#define RGB_DATA_H 	GPIO_SetBits (GPIOB, GPIO_Pin_9)
#define RGB_DATA_L 	GPIO_ResetBits (GPIOB, GPIO_Pin_9)

void RGB_Init(void);
void RGB_SetColor(u8 r, u8 g, u8 b);

#endif
