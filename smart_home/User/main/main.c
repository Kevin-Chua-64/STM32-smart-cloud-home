//在C语言中，程序的入口是main函数，单片机也是
#include "main.h"
#include "led.h"
#include "key.h"
#include "uart.h"
#include "delay.h"
#include <stdio.h>
#include "oled.h"
#include "oled_dis.h"
#include "motor.h"
#include "infrared.h"
#include "rgb.h"
#include "dht11.h"
#include "wifi.h"
#include "timer.h"
#include "relay.h"

#define DEVICE_ID_BASE_ADDR 0x1FFFF7E8

void Main_Init(void);
void Display_main(void);
void Display_menu(void);
void update(void);
void Display_DCMotor(void);
void Display_StepMotor(void);
void action(void);
void key_state_update(void);
void Display_Relay(void);
void Displsy_LED(void);
void display(void);
void timerUpdate(void);
void makeCmd(void);

u32 deviceSN[4];
char week[5], date[15], time[15];
float temp, humi;
char  cmd1[128], cmd2[128];
u8 temRet, wifiRet;
u8 funFlag=0, funArrow=1, DCMotorState=1, StepMotorPos=5, relayState=0, ledSelect=1, led1State=0, led2State=0, led3State=0;
s8 DCMotorSpeed=0;

int main(void){
	Main_Init();
	while(1){	
		key_state_update();
		action();
		display();
		if(flag_1s == 1){	// 1秒
			flag_1s = 0;
			timerUpdate();
		
		}	
	}
}

// 更新一次时间和温度
void timerUpdate(){
	
	//获取数据
	temRet = DHT11_GetTempRH(&temp, &humi);
	temRet = DHT11_GetTempRH(&temp, &humi);  //连续采集两次获得实时数据
	printf("11111");
//	makeCmd();
	printf("22222");
	Wifi_SendString(cmd2);	
	Wifi_Analysis_HTTPResponse(&DCMotorSpeed, &StepMotorPos, &relayState);
	wifiRet = Wifi_GetCurTime(week, date, time);
//	printf("\nwifiRet: %d\n", wifiRet);
	Wifi_SendString(cmd1);

}


// 初始化函数
void Main_Init(){
	u8 ret, i;
	//2 bits for pre-emption priority 2 bits for subpriority 
	NVIC_PriorityGroupConfig (NVIC_PriorityGroup_2);
	Led_Init( );
	Key_Init( );
	Uart1_Init(115200);
	OLED_Init( );
	DcMotor_Init( );
	StepMotor_Init( );
	Infrared_Init( );
	RGB_Init( );
	Relay_Init();
	DHT11_Init( );
	OLED_DisplayString(3, 30, "Loading!");
//	RGB_SetColor(0, 0, 0);
	ret = Wifi_Init( );
	if(ret)
		printf("Wifi 初始化失败\r\n");
	else
		printf("Wifi 初始化成功\r\n");
	for(i = 0; i < 3; i++){
		deviceSN[i] = *((u32 *)DEVICE_ID_BASE_ADDR + i);
	}
	timer_Init();
	OLED_ClearNCol(0,0,6,180);
	OLED_DisplayString(1,30, "Loading");
	OLED_DisplayString(4,5, "Successfully!");
	delay_ms(500);
	printf("*********************\r\n");
	printf("deviceSN = 0x%X%X%X\r\n", deviceSN[0], deviceSN[1],deviceSN[2]);
	printf("*********************\r\n");
	makeCmd();
	
	// 上报数据
	Wifi_SendString(cmd1);
	
	sprintf(cmd2, "\
GET /device/info?deviceid=0x%X%X%X HTTP/1.1\r\n\
Host:122.152.251.242:9002\r\n\
Connection:close\r\n\
\r\n\r\n\
", deviceSN[0], deviceSN[1],deviceSN[2]);
	OLED_ClearNCol(0,0,6,180);
}

void makeCmd(void){
	sprintf(cmd1, "\
GET /device/report?deviceid=0x%X%X%X&temperature=25.17&humidity=54.15&ledclore=rgb(0,0,0)&motor=%d&step_motor=%u&relay=%u HTTP/1.1\r\n\
Host:122.152.251.242:9002\r\n\
Connection:close\r\n\
\r\n\r\n\
", deviceSN[0], deviceSN[1],deviceSN[2], DCMotorSpeed, StepMotorPos, relayState);
	
}

// 展示主界面
void Display_main(){
	char buf[128];
	//如果获取数据成功，更新界面
	if(temRet){
		sprintf(buf, "T:%.1f℃", temp);
		OLED_DisplayString(6, 5, (u8 *)buf);
		sprintf(buf, "H:%d%%", (int)humi);
		OLED_DisplayString(6, 75, (u8 *)buf);
	}
	if(wifiRet==0){
		OLED_DisplayString(0, 5, (u8*)time);
		OLED_DisplayString(0, 90, (u8*)week);
		OLED_DisplayString(3, 15, (u8*)date);
	}
}

// 展示菜单
void Display_menu(){
	
	static int tempflag = 0;
	if(tempflag != funArrow){
		tempflag = funArrow;
		OLED_ClearNCol(0, 15, 8, 20);
	}
	OLED_DisplayString(0, 40, "通风控制");
	OLED_DisplayString(2, 40, "窗帘控制");
	OLED_DisplayString(4, 40, "智能插座");
	OLED_DisplayString(6, 40, "灯光控制");
	OLED_DisplayString(2*(funArrow-1), 15, "->");
}

// 直流电机功能界面
void Display_DCMotor(){
	char buf[5];
	u8 n=0, i;
	//静态变量存上次状态，用于判断状态是否变化
	static int temp = 0;	
	OLED_DisplayString(0, 40, "通风控制");	
	
	if(DCMotorState+DCMotorSpeed != temp){
		// 如果状态变化，清屏，记录状态
		OLED_ClearNCol(2, 0, 6, 128);
		temp = DCMotorState+DCMotorSpeed;
	}
	
	if(DCMotorState == 0){
		// 显示关闭图标
		OLED_DisplayPicture(4, 52, test1);
	}else{
		// 显示当前速度
		sprintf(buf, "%d  ", DCMotorSpeed);
		OLED_DisplayString(2, 57, (u8 *)buf);
		
		// 显示进度条
		OLED_DisplayChar(5, 63, '|');
		if(DCMotorSpeed > 0){
			n = DCMotorSpeed/20;
			for(i=0; i<n; i++){
				OLED_DisplayChar(5, 70+8*i, '*');
			}
		}else{
			n = (-1*DCMotorSpeed)/20;
			for(i=0; i<n; i++){
				OLED_DisplayChar(5, 56-8*i, '*');
			}
		}
		// 显示+/-标志
		if(DCMotorState == 1){
			OLED_DisplayChar(6, 120, '+');
		}
		if(DCMotorState == 2){
			OLED_DisplayChar(6, 120, '-');
		}
	}
}

// 步进电机功能界面
void Display_StepMotor(){
	char buf[5];
	u8 i;
	static int temp = 0;
	if(temp != StepMotorPos){
		temp = StepMotorPos;
		OLED_ClearNCol(5, 0, 3, 128);
	}
	// 显示窗帘位置
	OLED_DisplayString(0, 34, "窗帘控制");
	sprintf(buf, "%d  ", StepMotorPos);
	OLED_DisplayString(2, 60, (u8 *)buf);
	// 显示进度条
	OLED_DisplayChar(5, 18, '|');
	for(i=0; i<StepMotorPos; i++){
		OLED_DisplayChar(5, 26+8*i, '*');
	}
}

// 继电器功能界面
void Display_Relay(){
	static int temp = 0;
	u8 *p;
	
	if(temp != relayState){	//状态变化
		temp = relayState;
		OLED_ClearNCol(2, 0, 6, 128);
		
		if(relayState == 0){	// 1 -> 0：打印关动画
			p = close;
			while(*p != '\0'){
				OLED_DisplayPicture(4, 32, p);
				p+=258;
				delay_ms(20);
			}
		}else{	// 0 -> 1：打印开动画
			p = open;
			while(*p != '\0'){
				OLED_DisplayPicture(4, 32, p);
				p+=258;
				delay_ms(20);
			}
		}
	}
	OLED_DisplayString(0, 34, "智能插座");
	if(relayState == 0){
		// open动画的首页是关
		OLED_DisplayPicture(4, 32, open);
	}else{
		// close动画的首页是开
		OLED_DisplayPicture(4, 32, close);
	}
}

// LED灯功能界面
void Displsy_LED(){
	static int temp = 0;
	u8 sta;
	sta = ledSelect + led1State + led2State + led3State;
	if(temp != sta){
		temp = sta;
		OLED_ClearNCol(2, 0, 6, 128);
	}
	OLED_DisplayString(0, 40, "灯光控制");
	OLED_DisplayString(2, 40, "LED1");
	OLED_DisplayString(4, 40, "LED2");
	OLED_DisplayString(6, 40, "LED3");
	// 打印灯开关状态
	if(led1State) OLED_DisplayString(2, 80, "ON  ");
	else OLED_DisplayString(2, 80, "OFF  ");
	if(led2State) OLED_DisplayString(4, 80, "ON  ");
	else OLED_DisplayString(4, 80, "OFF  ");
	if(led3State) OLED_DisplayString(6, 80, "ON  ");
	else OLED_DisplayString(6, 80, "OFF  ");
	// 打印箭头
	OLED_DisplayString(ledSelect*2, 15, "->");
}


// 按键更新状态
void key_state_update(){
	u8 key=0;
	key = Key_Scan();
	
	if (key == 1){
		if (funFlag == 0) funFlag = 1;
		else if (funFlag == 1){
			funFlag = 0;
			funArrow = 1;
		}else{
			funFlag = 1;
			ledSelect = 1;
		}
	}
	
	else if (key == 2){
		if (funFlag == 1) funArrow = (funArrow%4)+1;
		else if (funFlag == 2) DCMotorState = ((DCMotorState+1)%3);
		else if (funFlag == 3){
			if (StepMotorPos != 0){
				StepMotorPos -= 1;
				StepMotor_Move(0, 40);
			}
		}else if (funFlag == 4) relayState = 0;
		else if (funFlag == 5) ledSelect = ( (ledSelect%3)+1 );
	}
	
	else if (key == 3){
		if (funFlag == 1) funFlag = funArrow+1;
		else if (funFlag == 2){
			if (DCMotorState == 1){
				DCMotorSpeed += 10;
				if (DCMotorSpeed >= 100) DCMotorSpeed = 100;
			}else if (DCMotorState == 2){
				DCMotorSpeed -= 10;
				if (DCMotorSpeed <= -100) DCMotorSpeed = -100;
			}
		}else if (funFlag == 3){
			StepMotorPos += 1;
			if (StepMotorPos >= 10) StepMotorPos = 10;
			else StepMotor_Move(1, 40);
		}
		else if (funFlag == 4)relayState = 1;
		else if (funFlag == 5){
			if (ledSelect == 1) led1State = ( (led1State+1)%2 );
			else if (ledSelect == 2) led2State = ( (led2State+1)%2 );
			else if (ledSelect == 3) led3State = ( (led3State+1)%2 );
		}
	}
}

// 执行状态更新
void action(){
	if (DCMotorState== 0) DcMotor_SetSpeed(0);
	else if (DCMotorState != 0) DcMotor_SetSpeed(DCMotorSpeed);
	
	if (relayState== 0) RELAY_OFF;
	else if (relayState == 1) RELAY_ON;
	
	if (led1State== 0) LED1_OFF;
	else if (led1State == 1) LED1_ON;
	if (led2State== 0) LED2_OFF;
	else if (led2State == 1) LED2_ON;
	if (led3State== 0) LED3_OFF;
	else if (led3State == 1) LED3_ON;		
}

// 控制显示
void display(){
	static int tempflag = 0;
	if(tempflag != funFlag){
		// 状态更新->清屏
		tempflag = funFlag; //记录上次状态
		OLED_ClearNCol(0, 0, 8, 128);	//清屏
	}
	switch(funFlag){
		case 0: Display_main(); break;
		case 1: Display_menu(); break;
		case 2: Display_DCMotor(); break;
		case 3: Display_StepMotor(); break;
		case 4: Display_Relay(); break;
		case 5: Displsy_LED(); break;
		default: break;
	}
}







