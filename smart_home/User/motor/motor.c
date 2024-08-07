#include "motor.h"
#include "delay.h"

//���������ʼ�� Linux
void StepMotor_Init(void)
{
	//��GPIOAʱ��
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
	
	//��ʼ��PA4/5/6/7Ϊ�������
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//���츴�����
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;//û������ Ҫ������ٴ���
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//��ʼ��������ھ�ֹ״̬ -- ��Ȧ�����ϵ� -- ����͵�ƽ
	STEP_MOTOR_STOP;
}

//���Ʋ�������˶�����
//dir: 0 -- ����  1--����
//n : ת��Ȧ��
void StepMotor_Move(u8 dir, u8 n)
{
	u8 i;
	
	if(dir == 0)//����  ���� 1->2->3->4
	{
		for(i = 0; i < n; i++)
		{
			BEAT_1;//����1
			delay_us(800);
			BEAT_2;//����2
			delay_us(800);
			BEAT_3;//����3
			delay_us(800);
			BEAT_4;//����4
			delay_us(800);
		}
	}else //����  ���� 4->3->2->1
	{
		for(i = 0; i < n; i++)
		{
			BEAT_4;//����4
			delay_us(800);
			BEAT_3;//����3
			delay_us(800);
			BEAT_2;//����2
			delay_us(800);
			BEAT_1;//����1
			delay_us(800);
		}
	}
	STEP_MOTOR_STOP;
}

//ֱ�������ʼ��
void DcMotor_Init(void)
{
	//���PB4ռ��
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB, ENABLE);//��AFIOʱ��
	GPIO_PinRemapConfig (GPIO_Remap_SWJ_JTAGDisable, ENABLE);//ӳ��

#if USE_TIM_PWM
	//��ʼ��PB4/PB5Ϊ�������
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//���츴�����
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;//û������ Ҫ������ٴ���
	GPIO_Init (GPIOB, &GPIO_InitStruct);
	
	//ӳ�䵽TIM3
	GPIO_PinRemapConfig (GPIO_PartialRemap_TIM3, ENABLE);//������ӳ��
	
	//��TIM3ʱ�� - APB1
	RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM3, ENABLE);
	
	//ѡ���ڲ�ʱ��Դ
	TIM_InternalClockConfig (TIM3);
	
	//��ʼ��ʱ����Ԫ
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//��������
	TIM_TimeBaseInitStruct.TIM_Prescaler = 7200 - 1;//��Ƶֵ - ��������  72MHz / 7200 - 1/100 -- 100us
	TIM_TimeBaseInitStruct.TIM_Period = 100 - 1;//����ֵ - �������� 10ms - 100�� - 100us
	TIM_TimeBaseInit (TIM3, &TIM_TimeBaseInitStruct);
	
	//��ʼ�����ͨ��
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;//CNT<�Ƚ�ֵ - �ߵ�ƽ
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;//������
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;//ʹ�����ͨ��
	TIM_OCInitStruct.TIM_Pulse = 0;//CNT >= 0 -- �͵�ƽ
	
	TIM_OC1Init (TIM3, &TIM_OCInitStruct);//ͨ��1��ʼ��
	TIM_OC2Init (TIM3, &TIM_OCInitStruct);//ͨ��2��ʼ��
	
	//ʹ��TIM3
	TIM_Cmd (TIM3, ENABLE);
#else	
	//��ʼ��PB4/PB5Ϊ�������
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//�������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;//û������ Ҫ������ٴ���
	GPIO_Init (GPIOB, &GPIO_InitStruct);
	
	DC_MOTOR_STOP;
#endif
}

//���ת�ٵĿ���
//�������ٶ�  -99 ~ 99
void DcMotor_SetSpeed(s8 speed)
{
	if(speed < 0)//��ת
	{
		TIM_SetCompare1 (TIM3, 0);//BAK
		TIM_SetCompare2 (TIM3, -speed);//FOR
	}else //��ת
	{
		TIM_SetCompare1 (TIM3, speed);//BAK
		TIM_SetCompare2 (TIM3, 0);//FOR
	}
}
