#include "infrared.h"
#include "stdio.h"

static Infrared_InfoTypdef infraredInfo_t;

/*
�������ܣ�������Ϣ�ṹ�崫��
�����βΣ�None
��������ֵ��None
��ע��None
���ߣ�Yao
���ڣ�2022/09/02
*/
Infrared_InfoTypdef *Infrared_GetInfo(void)
{
	return &infraredInfo_t;
}

/*
�������ܣ�����ܽų�ʼ��
�����βΣ�None
��������ֵ��None
��ע��DATA -- PA1 -��������
���ߣ�Yao
���ڣ�2022/09/02
*/
void Infrared_Init(void)
{
	//��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	//��ʼ��GPIO��Ϊ��������
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;				
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//TIM2_CH2ԭʼ�������PA1
	
	//��TIM2ʱ��
	RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM2, ENABLE);//��TIM2ʱ��
	
	TIM_InternalClockConfig (TIM2);//ѡ���ڲ�ʱ��Դ
	
	//��ʼ��ʱ����Ԫ
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;//ʱ�ӷ�Ƶ����
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//��������
	TIM_TimeBaseInitStruct.TIM_Period = 50000 - 1;//50ms - ����
	TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;//1us = 1/(72Mhz / 72)
	TIM_TimeBaseInit (TIM2, &TIM_TimeBaseInitStruct);
	
	//��ʼ������ͨ��
	TIM_ICInitTypeDef TIM_ICInitStruct;
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;	//����ͨ��2
	TIM_ICInitStruct.TIM_ICFilter = 0xF;					//����Ƶ��fSAMPLING=fDTS/32�� N=8
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling;//�½��ش���
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//ÿ���¼������������¼�
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//IC2ӳ����TI2��
	TIM_ICInit (TIM2, &TIM_ICInitStruct);
	
	//�����ж����ȼ�
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;//�ж�Դ���
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//ʹ��NVIC��Ӧ
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;//��ռ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;//��Ӧ���ȼ�
	NVIC_Init (&NVIC_InitStruct);
	
	//ʹ�ܲ����жϺ͸����ж�
	TIM_ITConfig (TIM2, TIM_IT_CC2, ENABLE);		//ʹ�ܲ����ж�
	TIM_ITConfig (TIM2, TIM_IT_Update, ENABLE);	//ʹ�ܸ����ж�
	
	//ʹ��TIM2
	TIM_Cmd (TIM2, ENABLE);
}


void TIM2_IRQHandler(void)
{
	static u8 startFlag = 0;//��ʼ�����������źű�־
	static u8 i = 0;
	
	if(TIM_GetITStatus (TIM2, TIM_IT_Update) == SET)//�����ж�
	{
		TIM_ClearITPendingBit (TIM2, TIM_IT_Update);	//����±�־
		
		if(startFlag == 1)
		{
			//�����ϣ��ɼ������в����������ж�10ms�������ź����ֻ��9ms
			//һ������ɼ�������˸����жϣ��ɼ����
			startFlag = 0;//�ɼ�������־
			infraredInfo_t.edgeCnt = i;  //��ƽ����
			i = 0;				//��һ�δ�0��ʼ����
			if(infraredInfo_t.edgeCnt > 10)//���˵�ɼ�����ʮ��������
			{
				infraredInfo_t.captureOver = 1;//�ɼ���ɱ�־
			}
		}
	}else if(TIM_GetITStatus (TIM2, TIM_IT_CC2) == SET)//����2�ж�
	{
		TIM_ClearITPendingBit (TIM2, TIM_IT_CC2);			//�岶��2��־
		
		//�л�����
		if(INFRARED_PIN_R == 0)//�½��ش���
		{
			TIM2->CCER &= ~(0x1 << 5);//�л�Ϊ�����ش���
			if(startFlag == 0)//��һ�ν���
			{
				TIM2->CNT = 0;
				startFlag = 1;//��ʼ�����־
				i = 0;//��ƽʱ��洢λ��
			}else//���ǵ�һ���½��ش���
			{
				infraredInfo_t.timeCnt[i++] = TIM2->CNT;//��CNT -- �ߵ�ƽʱ��  9000us
				TIM2->CNT = 0;//���CNT��Ϊ��һ�βɼ�׼��
			}
		}else//�����ش���
		{
			TIM2->CCER |= (0x1 << 5);//�л�Ϊ�½����ش���
			infraredInfo_t.timeCnt[i++] = TIM2->CNT;//��CNT -- �͵�ƽʱ��  9000us
			TIM2->CNT = 0;//���CNT��Ϊ��һ�βɼ�׼��
		}
	}
}

/*
�������ܣ���������NECλ��
�����βΣ�None
��������ֵ��0:�ɹ�  ������ʧ��
��ע��timeCnt - [0][1]��ͬ��ͷ������
			�߼� 1 ��560us ��+1680us ��
			�߼� 0 ��560us ��+560us �� 
			ֻ��Ҫ�жϸߵ�ƽʱ�伴��
			NECbit��λ��ǰ
���ߣ�Yao
���ڣ�2022/09/05
*/
u8 Infrared_AnalysisNECBit(u8 type)
{
	u16 i, j = 0;
	u16 edgeCnt = 0;
	u16 startNum = 0;
	
	switch(type)
	{
		case XYD_REMOTE: edgeCnt = 67; startNum = 3; break;//XYDң�ر������67��
		case HISENSE_REMOTE: edgeCnt = 99; startNum = 3; break;//��׼/����������ô���
		case HISENSE_REMOTE_1: edgeCnt = 87; startNum = 6; break;//��׼/����������ô���
	}
	
	if(edgeCnt != infraredInfo_t.edgeCnt)//�����ϵ� != ʵ�ʲɼ���
		return 1;
	
	printf("type = %d ������λ���ݽ��Ϊ\r\n", type);
	for(i = startNum; i < edgeCnt; i += 2)
	{
		if((infraredInfo_t.timeCnt[i] > 1380) && (infraredInfo_t.timeCnt[i] < 1980) )//1 -- 1680us
			infraredInfo_t.necBit[j++] = 1;
		else
			infraredInfo_t.necBit[j++] = 0;
		
		printf("%d", infraredInfo_t.necBit[j-1]);
		if(j % 4 == 0)
			printf(" ");
		if(j % 8 == 0)
			printf("\r\n");
	}
	
	printf("\r\n");
	return 0;//ת����NECbit�ɹ�
}

void Infrared_PrintInfo(void)
{
	u8 i; 
	
	if(infraredInfo_t.captureOver == 1)
	{
//		infraredInfo_t.captureOver = 0;
		printf("����ı�������:%d\r\n", infraredInfo_t.edgeCnt);
		for(i = 0; i < infraredInfo_t.edgeCnt; i++)
		{
			printf("%d ", infraredInfo_t.timeCnt[i]);
		}
		
		printf("\r\n");
	}
}

/*
�������ܣ�NEC bit����ת�����ֽ�
�����βΣ�ң�ص�����
��������ֵ��None
��ע��bit�洢��λ��ǰ
���ߣ�Yao
���ڣ�2022/09/05
*/
u8 Infrared_NECBitToByte(u8 type)
{
	u8 bytes, i, j;
	
	switch(type)
	{
		case XYD_REMOTE:  bytes = 4; break;//XYDң�ر������67��
		case HISENSE_REMOTE: bytes = 6; break;//��׼/����������ô���
		case HISENSE_REMOTE_1:  bytes = 4; break;//��׼/����������ô���
		default : return 1;
	}
	for(i = 0; i < bytes; i++)
	{
		for(j = 0; j < 8; j++)
		{
			infraredInfo_t.necCode[i] <<= 1;//�ճ����λ
			//xxxx x100
			infraredInfo_t.necCode[i] |= infraredInfo_t.necBit[8*i + 7-j];
			//xxxx xx10
		}
		printf("0x%X ", infraredInfo_t.necCode[i]);
	}
	return 0;
}

/*
�������ܣ���������NEC��
�����βΣ�ң�ص�����
��������ֵ��None
��ע���߼� 1 ��560us ��+1680us ��
			�߼� 0 ��560us ��+560us �� 
���ߣ�Yao
���ڣ�2022/09/05
*/
u8 Infrared_AnalysisNECCode(u8 type)
{
	u8 ret;
	
	//��ӡʱ��
	Infrared_PrintInfo( );
	
	if(infraredInfo_t.captureOver == 1)
	{
		infraredInfo_t.captureOver = 0;
		
		switch(type)
		{
			//XYD-����ң�� 9ms+4.5msͬ��ͷ
			case XYD_REMOTE: 
				if((infraredInfo_t.timeCnt[0] > 8500 && infraredInfo_t.timeCnt[0] < 9500) 
						&& (infraredInfo_t.timeCnt[1] > 4000 && infraredInfo_t.timeCnt[1] < 5000))
				{
					ret = Infrared_AnalysisNECBit( type );//���ͬ��ͷ�ɼ����󣬾�ת����Ӧ��NEC bit
					if(ret)
						return 1;//����NECλʧ��
					
					Infrared_NECBitToByte(XYD_REMOTE);
					
					infraredInfo_t.necCtrl = infraredInfo_t.necCode[2];//���������
				}
				break;
			case HISENSE_REMOTE:
				if((infraredInfo_t.timeCnt[0] > 8500 && infraredInfo_t.timeCnt[0] < 9500) 
						&& (infraredInfo_t.timeCnt[1] > 4000 && infraredInfo_t.timeCnt[1] < 5000))
				{
					ret = Infrared_AnalysisNECBit( HISENSE_REMOTE );//���ͬ��ͷ�ɼ����󣬾�ת����Ӧ��NEC bit
					if(ret)
						return 1;//����NECλʧ��
					
					Infrared_NECBitToByte(HISENSE_REMOTE);
					
					infraredInfo_t.necCtrl = (infraredInfo_t.necCode[3] << 8) | infraredInfo_t.necCode[2];//���������
					printf("������ = 0x%04x\r\n", infraredInfo_t.necCtrl);
				}
				break;
			case HISENSE_REMOTE_1:
				ret = Infrared_AnalysisNECBit( type );//���ͬ��ͷ�ɼ����󣬾�ת����Ӧ��NEC bit
				if(ret)
					return 1;//����NECλʧ��
				Infrared_NECBitToByte(HISENSE_REMOTE_1);
				infraredInfo_t.necCtrl = (infraredInfo_t.necCode[2] << 8) | infraredInfo_t.necCode[3];//���������
				printf("������ = 0x%04x\r\n", infraredInfo_t.necCtrl);
				break;
		}
	}
	
	return 0;
}
