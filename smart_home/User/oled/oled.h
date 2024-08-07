#ifndef _OLED_H_
#define _OLED_H_

#include "stm32f10x.h"

#define OLED_DC_H 	GPIO_SetBits (GPIOB, GPIO_Pin_12)
#define OLED_DC_L 	GPIO_ResetBits (GPIOB, GPIO_Pin_12)
#define OLED_RES_H 	GPIO_SetBits (GPIOB, GPIO_Pin_6)
#define OLED_RES_L 	GPIO_ResetBits (GPIOB, GPIO_Pin_6)

#define OLED_CS_H 	GPIO_SetBits (GPIOB, GPIO_Pin_7)
#define OLED_CS_L 	GPIO_ResetBits (GPIOB, GPIO_Pin_7)
#define OLED_SCK_H 	GPIO_SetBits (GPIOB, GPIO_Pin_13)
#define OLED_SCK_L 	GPIO_ResetBits (GPIOB, GPIO_Pin_13)
#define OLED_MOSI_H 	GPIO_SetBits (GPIOB, GPIO_Pin_15)
#define OLED_MOSI_L 	GPIO_ResetBits (GPIOB, GPIO_Pin_15)

void OLED_SendCommand(u8 cmd);
void OLED_SendData(u8 data);

void OLED_Init(void);

#endif
