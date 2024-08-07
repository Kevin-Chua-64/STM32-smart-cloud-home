#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "stm32f10x.h"

#define USE_TIM_PWM 1

#define STEP_MOTOR_STOP GPIO_ResetBits (GPIOA, GPIO_Pin_4 | GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7)
#define BEAT_1 GPIO_SetBits (GPIOA, GPIO_Pin_4|GPIO_Pin_7);\
               GPIO_ResetBits (GPIOA, GPIO_Pin_5|GPIO_Pin_6)
#define BEAT_2 GPIO_SetBits (GPIOA, GPIO_Pin_4|GPIO_Pin_6);\
               GPIO_ResetBits (GPIOA, GPIO_Pin_5|GPIO_Pin_7)							 
#define BEAT_3 GPIO_SetBits (GPIOA, GPIO_Pin_5|GPIO_Pin_6);\
               GPIO_ResetBits (GPIOA, GPIO_Pin_4|GPIO_Pin_7)
#define BEAT_4 GPIO_SetBits (GPIOA, GPIO_Pin_5|GPIO_Pin_7);\
               GPIO_ResetBits (GPIOA, GPIO_Pin_4|GPIO_Pin_6)

void StepMotor_Init(void);
void StepMotor_Move(u8 dir, u8 n);
	


#if USE_TIM_PWM
	void DcMotor_SetSpeed(s8 speed);
#else
#define DC_MOTOR_STOP \
									do{\
										GPIO_ResetBits (GPIOB, GPIO_Pin_4);\
										GPIO_ResetBits (GPIOB, GPIO_Pin_5);\
									}while(0)

#define DC_MOTOR_CW \
									do{\
										GPIO_ResetBits (GPIOB, GPIO_Pin_4);\
										GPIO_SetBits (GPIOB, GPIO_Pin_5);\
									}while(0)									
#define DC_MOTOR_REV \
									do{\
										GPIO_SetBits (GPIOB, GPIO_Pin_4);\
										GPIO_ResetBits (GPIOB, GPIO_Pin_5);\
									}while(0)
#endif

									
void DcMotor_Init(void);
									
									
#endif
