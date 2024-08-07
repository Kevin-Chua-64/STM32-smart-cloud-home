#ifndef _WIFI_H_
#define _WIFI_H_

#include "stm32f10x.h"
#include <stdio.h>

typedef struct{
	u16 rxLen;
	u8 rxOver;
	char rxBuf[2048];
}Wifi_TypeDef;

//wifi名  - 修改成你自己的WiFi名和密码
#define WIFI_SSID		"zhengok"
//wifi密码
#define WIFI_PWD		"88888888"

#define SERVER_TYPE "TCP"
#define SERVER_IP		"122.152.251.242"
#define SERVER_PORT 9002

void Wifi_UartInit(u32 baud);
void Wifi_SendString(char *pStr);
u8 Wifi_SendAT(char *sAT, char *expectAT);

u8 Wifi_Connect(char *ssid, char *pwd);
u8 Wifi_ConnectServer(char *type, char *ip, u16 port);

u8 Wifi_Analysis_HTTPResponse(s8 *DCMotorSpeed, u8 *StepMotorPos, u8 *relayState);

u8 Wifi_Init(void);

int Wifi_GetCurTime(char *week, char *date, char *time);

#endif
