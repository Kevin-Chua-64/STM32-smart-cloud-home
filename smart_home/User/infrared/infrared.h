#ifndef _INFRARED_H_
#define _INFRARED_H_

#include "stm32f10x.h"

#define INFRARED_PIN_R GPIO_ReadInputDataBit (GPIOA, GPIO_Pin_1)

typedef struct
{
	u16 timeCnt[128];	//用来保存时间
	u8 edgeCnt;				//用来记录产生的边沿个数
	u8 captureOver;		//采集完成标志
	u8 necBit[256];		//存储NEC 位 码
	u8 necCode[64];		//存储NEC编码数据
	u32 necCtrl;				//nec控制码
}Infrared_InfoTypdef;

#define XYD_REMOTE 1
#define HISENSE_REMOTE 2
#define HISENSE_REMOTE_1 3

//extern Infrared_InfoTypdef infraredInfo_t;

Infrared_InfoTypdef *Infrared_GetInfo(void);
void Infrared_Init(void);
u8 Infrared_AnalysisNECCode(u8 type);
void Infrared_PrintInfo(void);
	
#endif
