#ifndef _INFRARED_H_
#define _INFRARED_H_

#include "stm32f10x.h"

#define INFRARED_PIN_R GPIO_ReadInputDataBit (GPIOA, GPIO_Pin_1)

typedef struct
{
	u16 timeCnt[128];	//��������ʱ��
	u8 edgeCnt;				//������¼�����ı��ظ���
	u8 captureOver;		//�ɼ���ɱ�־
	u8 necBit[256];		//�洢NEC λ ��
	u8 necCode[64];		//�洢NEC��������
	u32 necCtrl;				//nec������
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
