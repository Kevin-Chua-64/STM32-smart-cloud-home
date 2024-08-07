#include "wifi.h"
#include "oled.h"
#include <string.h>
#include "motor.h"
#include "rgb.h"
#include "delay.h"
#include "relay.h"

Wifi_TypeDef wifi_t;

/*
函数功能：wifi UART初始化
函数形参：波特率
函数返回值：None
备注：None
*/
void Wifi_UartInit(u32 baud)
{
	//1.打开GPIOA时钟 – APB2
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
	
	//初始化PA2-TX -复用推挽输出
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//初始化PA3 - RX - 浮空输入
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//打开USART2时钟 -- APB1
	RCC_APB1PeriphClockCmd (RCC_APB1Periph_USART2, ENABLE);
	
	//初始化USART2
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = baud;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None ;//五硬件流控
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStruct.USART_Parity = USART_Parity_No;//无校验位
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init (USART2, &USART_InitStruct);
	
	USART_ITConfig (USART2, USART_IT_RXNE, ENABLE);//使能接收中断
	USART_ITConfig (USART2, USART_IT_IDLE, ENABLE);//使能空闲中断
	
	//设置优先级
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;//中断源编号
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//使能NVIC响应
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;//响应优先级
	NVIC_Init (&NVIC_InitStruct);

	//使能USART2
	USART_Cmd (USART2, ENABLE);
}


void USART2_IRQHandler(void)
{
	static u16 i = 0;
	u8 temp = temp;
	
	if(USART_GetITStatus (USART2, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit (USART2, USART_IT_RXNE);//清标志
		
		//保存wifi回显数据到rxBuf中
		wifi_t.rxBuf[i++] = USART_ReceiveData (USART2);
		//printf("%c", wifi_t.rxBuf[i-1]);//回显
	}else if(USART_GetITStatus (USART2, USART_IT_IDLE) == SET)
	{
		//通过软件序列清除空闲标志
		temp = USART2->SR;
		temp = USART2->DR;
		
		wifi_t.rxLen = i;			 //接收的字符数
		wifi_t.rxBuf[i] = '\0';//将接收的字符补充为完整的字符串
		i = 0;
		wifi_t.rxOver = 1;		 //接收完成标志
	}
}

//发送字符串
void Wifi_SendString(char *pStr)
{
	while(*pStr != '\0')
	{
		//等待发送缓冲区为空
		while(USART_GetFlagStatus (USART2, USART_FLAG_TXE) != SET){}
			
		USART_SendData (USART2, *pStr++);
	}
}

u8 Wifi_SendAT(char *sAT, char *expectAT)//"AT+CWMODE_CUR=1","OK"
{
	u16 timeCnt = 0, timeOut = 1000;
	
	//wifi连接时间较长
	if(strstr(sAT, "AT+CWJAP_CUR=\"") != NULL)
		timeOut = 8000;
	
	//发送AT指令
	Wifi_SendString(sAT);
	
	//等待指令回显接收完成
	while(1)
	{
		//等待接收完成标志
		while(wifi_t.rxOver != 1)
		{
			timeCnt++;
			delay_ms(1);
			
			if(timeCnt > timeOut)
				return 1;//超时
		}
		wifi_t.rxOver = 0;//清除接收标志
		timeCnt = 0;			//计数清零
		
		//查询接收的内容中是否右期望回显的内容
		if(strstr(wifi_t.rxBuf, expectAT) != NULL)
			break;
	}
	
	return 0;
}

u8 Wifi_Connect(char *ssid, char *pwd)
{
	u8 ret;
	char buf[64] = {0};
	
	//恢复出厂设置
	ret = Wifi_SendAT("AT+RESTORE\r\n", "ready");
	if(ret)
		return 1;
	
	//设置station模式
	ret = Wifi_SendAT("AT+CWMODE_CUR=1\r\n", "OK");
	if(ret)
		return 2;
	
	//连接wifi
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
	
	//设置单连接模式
	ret = Wifi_SendAT("AT+CIPMUX=0\r\n", "OK");
	if(ret)
		return 1;
	
	//设置透传模式
	ret = Wifi_SendAT("AT+CIPMODE=1\r\n", "OK");
	if(ret)
		return 2;
	
	//连接服务器
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
	Wifi_SendString("\r\n");//硬件bug，串口第一次发送数据，最开始会有一个空格，为了和后面指令进行区分
	delay_ms(100);
	Wifi_SendString("+++");
	delay_ms(1000);	
}

u8 Wifi_Init(void)
{
	u8 ret;
	
	//初始化配置
	Wifi_UartInit(115200);
	
	//退出透传
	Wifi_ExitSeriaNet( );
	
	//连接wifi
	ret = Wifi_Connect(WIFI_SSID, WIFI_PWD);
	if(ret)
		return 1;
	
	//连接服务器
	ret = Wifi_ConnectServer(SERVER_TYPE, SERVER_IP, SERVER_PORT);
	if(ret)
		return 2;
	
	//进入透传
	ret = Wifi_EnterSeriaNet( );
	if(ret)
		return 3;
	
	return 0;
}

//根据指定事件编号，解析相应的数据
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
	
	//找出V1在buf中的位置
	if((p = strstr(wifi_t.rxBuf, eventNum)) == NULL)
		return 0;
	
	if(*(p+3) != '-')
		p += 3;//偏移到首个数据字符位置
	else //负数
	{
		p += 4;//偏移到首个数据字符位置
		sign = -1;
	}
	
	for(bitNum = 1; bitNum < 4; bitNum++)//255
	{
		//判断是否是结束的 ',' 
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
	
	//等待接收完成标志
	while(wifi_t.rxOver == 0){};
	
	{
		wifi_t.rxOver = 0;
		
		//查询响应是否正常
		if(strstr(wifi_t.rxBuf, "200 OK") == NULL)
			return 1;//响应失败
		
		printf("\r\n*********************\r\n");
		//解析数据
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

// 0 为有；1为无
int Wifi_GetCurTime(char *week, char *date, char *time){
//	printf(wifi_t.rxBuf);
//	Date: Wed, 07 Dec 2022 14:45:07 GMT
	char *p;
	u8 bitNum = 0;
	
	//找出Date:在buf中的位置
	if((p = strstr(wifi_t.rxBuf, "Date:")) == NULL){
		return 1;
	}
	
	p += 6;//偏移到首个数据字符位置
	// 拿出week
	for(bitNum = 1; bitNum < 6; bitNum++)//Wed...Sun...
	{
		week[bitNum-1] = *(p+bitNum-1);
		//判断是否是结束的 ',' 
		if((*(p + bitNum) == ',')){
			week[bitNum] = '\0';
			p += bitNum;
			break;
		}
	}
	
	p += 2;//偏移到Date首个数据字符位置
	// 拿出date
	for(bitNum = 1; bitNum < 14; bitNum++)//07 Dec 2022 
	{
		date[bitNum-1] = *(p+bitNum-1);
		//判断是否是结束的 ',' 
		if((*(p + bitNum) == ' ') && (*(p + bitNum + 3) == ':')){
			date[bitNum] = '\0';
			p += (bitNum+1);
			break;
		}
	}
	
	// 拿出time
	for(bitNum = 1; bitNum < 10; bitNum++)//14:45:07 
	{
		time[bitNum-1] = *(p+bitNum-1);
		//判断是否是结束的 ' ' 
		if((*(p + bitNum) == ' ')){
			time[bitNum] = '\0';
			break;
		}
	}
	return 0;
}



