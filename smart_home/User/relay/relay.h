#ifndef _RELAY_H
#define _RELAY_H

#include "stm32f10x.h"

#define RELAY_ON  (GPIOB->ODR |=(1<<1))
#define RELAY_OFF (GPIOB->ODR &=~(1<<1))

void Relay_Init(void);


#endif
