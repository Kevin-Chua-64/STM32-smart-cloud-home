#include "dht11.h"
#include "delay.h"
#include <stdio.h>

void DHT11_Init(void)
{
	//��GPIOBʱ�� -- APB2
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB, ENABLE);//��AFIOʱ��
	GPIO_PinRemapConfig (GPIO_Remap_SWJ_JTAGDisable, ENABLE);//ӳ��
	
	//��ʼ��PB3 - ��©���
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;//��©���
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;//û������ Ҫ������ٴ���
	GPIO_Init (GPIOB, &GPIO_InitStruct);
	
	DHT11_DATA_H;//���� ��������������
	delay_ms(1000);//��ʱ1s�ȴ�Խ�����ȶ���
}

void DHT11_Start(void)
{
	DHT11_DATA_L;//����������ʼ�ź�
	delay_ms(20);//��ʼ�źű���ʱ��
	DHT11_DATA_H;//�����ͷ����� -- �ȴ��ӻ�Ӧ��
}



//�ȴ�Ӧ��
//0 - �ɹ�  ���� - ʧ��
u8 DHT11_WaitAck(void)
{
	u8 timCnt = 0;
	
	while(DHT11_DATA_R == 1)//���������ȴ��ӻ�Ӧ��ķ�Ӧʱ��
	{
		timCnt++;
		delay_us(1);
		if(timCnt > 50)
			return 1;//��Ӧʧ��
	}
	
	timCnt = 0;
	//ͳ�ƴӻ�Ӧ��ʱ��
	while(DHT11_DATA_R == 0)
	{
		timCnt++;
		delay_us(1);
		if(timCnt > 100)
			return 2;//��Ӧʧ��
	}
	
	while(DHT11_DATA_R == 1);//���˴ӻ�����������֪ͨ�����ɼ�����
	
	return 0;
}

//��ȡλ����
u8 DHT11_GetBit(void)
{
	//����λ��0��1�ĵ͵�ƽʱ����ͬ������ֱ�ӹ������еĵ͵�ƽʱ��
	while(DHT11_DATA_R == 0){}
	
	delay_us(50);//��ʱ50us -- ����λ����1/0
	if(DHT11_DATA_R == 0)
		return 0;
	
	//��ʱ50us֮����Ϊ�ߵ�ƽ -- ������λ����1��68~74us��ʱ���֮��
	while(DHT11_DATA_R == 1);//����λ����1ʣ�µ�ʱ��
	return 1;
}

//�ɼ�40bit���� - ��λ��ǰ
void DHT11_GetData(u8 *data)//u8 data[5]
{
	u8 i, j;
	
	for(i = 0; i < 5; i++)//�ɼ�5��8bit����
	{
		data[i] = 0;//����
		for(j = 0; j < 8; j++)//�ɼ�8bit����
		{			
			if(DHT11_GetBit( ) == 1)
				data[i] |= (1 << (7-j));
		}
	}
}

//��ȡ��ʪ������
u8 DHT11_GetTempRH(float *temp, float *humi)
{
	u8 ret;
	u8 data[5];
	u8 checkSum;
//	u8 i;
	
	DHT11_Start( );//��ʼ�ź�
	ret = DHT11_WaitAck( );//�ȴ�Ӧ��
	if(ret)
		return 1;//Ӧ��ʧ��
	
	DHT11_GetData(data);//��ȡ40bit����
	
	checkSum = data[0] + data[1] + data[2] + data[3];
	if(checkSum != data[4])
		return 2;//У��ʧ��
	
	*humi = data[0] + data[1]/10;
	if(data[3] & 0x80)//�ж��Ƿ�ʱ����
		//����
		*temp = -1 * (data[2] + ((data[3] << 1) >> 1) / 10.0);
	else
		*temp = data[2] + data[3]/10.0;
	
//	for(i = 0;  i < 4; i++)
//		printf("data[%d] = %d\r\n", i, data[i]);
	
	return 0;//��ȡ���ݳɹ�
}
