#include "wifi.h"
#include "oled.h"
#include <string.h>
#include "motor.h"
#include "rgb.h"
#include "delay.h"
#include "relay.h"

Wifi_TypeDef wifi_t;

/*
�������ܣ�wifi UART��ʼ��
�����βΣ�������
��������ֵ��None
��ע��None
*/
void Wifi_UartInit(u32 baud)
{
	//1.��GPIOAʱ�� �C APB2
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
	
	//��ʼ��PA2-TX -�����������
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//�����������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//��ʼ��PA3 - RX - ��������
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//��USART2ʱ�� -- APB1
	RCC_APB1PeriphClockCmd (RCC_APB1Periph_USART2, ENABLE);
	
	//��ʼ��USART2
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = baud;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None ;//��Ӳ������
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStruct.USART_Parity = USART_Parity_No;//��У��λ
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init (USART2, &USART_InitStruct);
	
	USART_ITConfig (USART2, USART_IT_RXNE, ENABLE);//ʹ�ܽ����ж�
	USART_ITConfig (USART2, USART_IT_IDLE, ENABLE);//ʹ�ܿ����ж�
	
	//�������ȼ�
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;//�ж�Դ���
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//ʹ��NVIC��Ӧ
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;//��ռ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;//��Ӧ���ȼ�
	NVIC_Init (&NVIC_InitStruct);

	//ʹ��USART2
	USART_Cmd (USART2, ENABLE);
}


void USART2_IRQHandler(void)
{
	static u16 i = 0;
	u8 temp = temp;
	
	if(USART_GetITStatus (USART2, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit (USART2, USART_IT_RXNE);//���־
		
		//����wifi�������ݵ�rxBuf��
		wifi_t.rxBuf[i++] = USART_ReceiveData (USART2);
		//printf("%c", wifi_t.rxBuf[i-1]);//����
	}else if(USART_GetITStatus (USART2, USART_IT_IDLE) == SET)
	{
		//ͨ���������������б�־
		temp = USART2->SR;
		temp = USART2->DR;
		
		wifi_t.rxLen = i;			 //���յ��ַ���
		wifi_t.rxBuf[i] = '\0';//�����յ��ַ�����Ϊ�������ַ���
		i = 0;
		wifi_t.rxOver = 1;		 //������ɱ�־
	}
}

//�����ַ���
void Wifi_SendString(char *pStr)
{
	while(*pStr != '\0')
	{
		//�ȴ����ͻ�����Ϊ��
		while(USART_GetFlagStatus (USART2, USART_FLAG_TXE) != SET){}
			
		USART_SendData (USART2, *pStr++);
	}
}

u8 Wifi_SendAT(char *sAT, char *expectAT)//"AT+CWMODE_CUR=1","OK"
{
	u16 timeCnt = 0, timeOut = 1000;
	
	//wifi����ʱ��ϳ�
	if(strstr(sAT, "AT+CWJAP_CUR=\"") != NULL)
		timeOut = 8000;
	
	//����ATָ��
	Wifi_SendString(sAT);
	
	//�ȴ�ָ����Խ������
	while(1)
	{
		//�ȴ�������ɱ�־
		while(wifi_t.rxOver != 1)
		{
			timeCnt++;
			delay_ms(1);
			
			if(timeCnt > timeOut)
				return 1;//��ʱ
		}
		wifi_t.rxOver = 0;//������ձ�־
		timeCnt = 0;			//��������
		
		//��ѯ���յ��������Ƿ����������Ե�����
		if(strstr(wifi_t.rxBuf, expectAT) != NULL)
			break;
	}
	
	return 0;
}

u8 Wifi_Connect(char *ssid, char *pwd)
{
	u8 ret;
	char buf[64] = {0};
	
	//�ָ���������
	ret = Wifi_SendAT("AT+RESTORE\r\n", "ready");
	if(ret)
		return 1;
	
	//����stationģʽ
	ret = Wifi_SendAT("AT+CWMODE_CUR=1\r\n", "OK");
	if(ret)
		return 2;
	
	//����wifi
	sprintf(buf, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", ssid, pwd);
	ret = Wifi_SendAT(buf, "OK");
	if(ret)
		return 3;
	
	return 0;
}

u8 Wifi_ConnectServer(char *type, char *ip, u16 port)
{
	u8 ret;
	char buf[64] = {0};
	
	//���õ�����ģʽ
	ret = Wifi_SendAT("AT+CIPMUX=0\r\n", "OK");
	if(ret)
		return 1;
	
	//����͸��ģʽ
	ret = Wifi_SendAT("AT+CIPMODE=1\r\n", "OK");
	if(ret)
		return 2;
	
	//���ӷ�����
	sprintf(buf, "AT+CIPSTART=\"%s\",\"%s\",%d\r\n", type, ip, port);
	ret = Wifi_SendAT(buf, "OK");
	if(ret)
		return 3;
	
	return 0;
}

u8 Wifi_EnterSeriaNet(void)
{
	u8 ret;
	ret = Wifi_SendAT("AT+CIPSEND\r\n", ">");
	if(ret)
		return 1;
	return 0;
}

void Wifi_ExitSeriaNet(void)
{
	Wifi_SendString("\r\n");//Ӳ��bug�����ڵ�һ�η������ݣ��ʼ����һ���ո�Ϊ�˺ͺ���ָ���������
	delay_ms(100);
	Wifi_SendString("+++");
	delay_ms(1000);	
}

u8 Wifi_Init(void)
{
	u8 ret;
	
	//��ʼ������
	Wifi_UartInit(115200);
	
	//�˳�͸��
	Wifi_ExitSeriaNet( );
	
	//����wifi
	ret = Wifi_Connect(WIFI_SSID, WIFI_PWD);
	if(ret)
		return 1;
	
	//���ӷ�����
	ret = Wifi_ConnectServer(SERVER_TYPE, SERVER_IP, SERVER_PORT);
	if(ret)
		return 2;
	
	//����͸��
	ret = Wifi_EnterSeriaNet( );
	if(ret)
		return 3;
	
	return 0;
}

//����ָ���¼���ţ�������Ӧ������
s16 Wifi_GetEventData(char *eventNum)//"V1"
{
	char *p;
	s16 sum = 0;
	s8 sign = 1;
	u8 bitNum = 0;
	
	if((p = strstr(eventNum, "V6")) != NULL)
	{
		p = strstr(wifi_t.rxBuf, "V6");
		return *(p+3) - '0';
	}
	
	//�ҳ�V1��buf�е�λ��
	if((p = strstr(wifi_t.rxBuf, eventNum)) == NULL)
		return 0;
	
	if(*(p+3) != '-')
		p += 3;//ƫ�Ƶ��׸������ַ�λ��
	else //����
	{
		p += 4;//ƫ�Ƶ��׸������ַ�λ��
		sign = -1;
	}
	
	for(bitNum = 1; bitNum < 4; bitNum++)//255
	{
		//�ж��Ƿ��ǽ����� ',' 
		if((*(p + bitNum) == ',') || (*(p + bitNum) == ' '))
			break;
	}
	
	while(bitNum--)//3 2 1 0
	{
		sum += (*p++ - '0');
		
		if(bitNum != 0)
			sum *= 10;//250
	}
	
	return (sign * sum);
}

u8 Wifi_Analysis_HTTPResponse(s8 *DCMotorSpeed, u8 *StepMotorPos, u8 *relayState)
{
	u8 r, g, b;
	u8 relay;
	s8 dcSpeed, stepMotor;
	
	//�ȴ�������ɱ�־
	while(wifi_t.rxOver == 0){};
	
	{
		wifi_t.rxOver = 0;
		
		//��ѯ��Ӧ�Ƿ�����
		if(strstr(wifi_t.rxBuf, "200 OK") == NULL)
			return 1;//��Ӧʧ��
		
		printf("\r\n*********************\r\n");
		//��������
		r = Wifi_GetEventData("V1");
		g = Wifi_GetEventData("V2");
		b = Wifi_GetEventData("V3");
		printf("RGB:%d  %d  %d\r\n", r, g, b);
		RGB_SetColor(r, g, b);
		
		dcSpeed = Wifi_GetEventData("V4");
		printf("dcSpeed = %d\r\n", dcSpeed);
//		DcMotor_SetSpeed(dcSpeed);
		*DCMotorSpeed = dcSpeed;
		
		
		stepMotor = Wifi_GetEventData("V5");
		printf("stepMotor = %d\r\n", stepMotor);
//		if(stepMotor != *StepMotorPos){
//			*StepMotorPos = stepMotor;
//			if(stepMotor-*StepMotorPos < 0)
//				StepMotor_Move(0, 40*(*StepMotorPos-stepMotor));	
//			else 
//				StepMotor_Move(1, 40*(stepMotor-*StepMotorPos));
//		}
		StepMotor_Move(0, 20*(stepMotor-5));
		*StepMotorPos = stepMotor;
		
		relay = Wifi_GetEventData("V6");
		printf("relay = %d\r\n", relay);
		*relayState = relay;
//		if(relay)
//			RELAY_ON;
//		else
//			RELAY_OFF;
		
		printf("\r\n*********************\r\n");
	}	
	
	return 0;
}

// 0 Ϊ�У�1Ϊ��
int Wifi_GetCurTime(char *week, char *date, char *time){
//	printf(wifi_t.rxBuf);
//	Date: Wed, 07 Dec 2022 14:45:07 GMT
	char *p;
	u8 bitNum = 0;
	
	//�ҳ�Date:��buf�е�λ��
	if((p = strstr(wifi_t.rxBuf, "Date:")) == NULL){
		return 1;
	}
	
	p += 6;//ƫ�Ƶ��׸������ַ�λ��
	// �ó�week
	for(bitNum = 1; bitNum < 6; bitNum++)//Wed...Sun...
	{
		week[bitNum-1] = *(p+bitNum-1);
		//�ж��Ƿ��ǽ����� ',' 
		if((*(p + bitNum) == ',')){
			week[bitNum] = '\0';
			p += bitNum;
			break;
		}
	}
	
	p += 2;//ƫ�Ƶ�Date�׸������ַ�λ��
	// �ó�date
	for(bitNum = 1; bitNum < 14; bitNum++)//07 Dec 2022 
	{
		date[bitNum-1] = *(p+bitNum-1);
		//�ж��Ƿ��ǽ����� ',' 
		if((*(p + bitNum) == ' ') && (*(p + bitNum + 3) == ':')){
			date[bitNum] = '\0';
			p += (bitNum+1);
			break;
		}
	}
	
	// �ó�time
	for(bitNum = 1; bitNum < 10; bitNum++)//14:45:07 
	{
		time[bitNum-1] = *(p+bitNum-1);
		//�ж��Ƿ��ǽ����� ' ' 
		if((*(p + bitNum) == ' ')){
			time[bitNum] = '\0';
			break;
		}
	}
	return 0;
}



