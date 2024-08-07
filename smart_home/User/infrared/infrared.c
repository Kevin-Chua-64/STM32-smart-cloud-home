#include "infrared.h"
#include "stdio.h"

static Infrared_InfoTypdef infraredInfo_t;

/*
函数功能：红外信息结构体传递
函数形参：None
函数返回值：None
备注：None
作者：Yao
日期：2022/09/02
*/
Infrared_InfoTypdef *Infrared_GetInfo(void)
{
	return &infraredInfo_t;
}

/*
函数功能：红外管脚初始化
函数形参：None
函数返回值：None
备注：DATA -- PA1 -浮空输入
作者：Yao
日期：2022/09/02
*/
void Infrared_Init(void)
{
	//打开GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	//初始化GPIO口为浮空输入
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;				
	GPIO_Init (GPIOA, &GPIO_InitStruct);
	
	//TIM2_CH2原始分配就是PA1
	
	//打开TIM2时钟
	RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM2, ENABLE);//开TIM2时钟
	
	TIM_InternalClockConfig (TIM2);//选择内部时钟源
	
	//初始化时基单元
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;//时钟分频因子
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//递增计数
	TIM_TimeBaseInitStruct.TIM_Period = 50000 - 1;//50ms - 周期
	TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;//1us = 1/(72Mhz / 72)
	TIM_TimeBaseInit (TIM2, &TIM_TimeBaseInitStruct);
	
	//初始化输入通道
	TIM_ICInitTypeDef TIM_ICInitStruct;
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;	//配置通道2
	TIM_ICInitStruct.TIM_ICFilter = 0xF;					//采样频率fSAMPLING=fDTS/32， N=8
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Falling;//下降沿触发
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//每个事件都触发捕获事件
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//IC2映射在TI2上
	TIM_ICInit (TIM2, &TIM_ICInitStruct);
	
	//设置中断优先级
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;//中断源编号
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//使能NVIC响应
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;//响应优先级
	NVIC_Init (&NVIC_InitStruct);
	
	//使能捕获中断和更新中断
	TIM_ITConfig (TIM2, TIM_IT_CC2, ENABLE);		//使能捕获中断
	TIM_ITConfig (TIM2, TIM_IT_Update, ENABLE);	//使能更新中断
	
	//使能TIM2
	TIM_Cmd (TIM2, ENABLE);
}


void TIM2_IRQHandler(void)
{
	static u8 startFlag = 0;//开始捕获红外控制信号标志
	static u8 i = 0;
	
	if(TIM_GetITStatus (TIM2, TIM_IT_Update) == SET)//更新中断
	{
		TIM_ClearITPendingBit (TIM2, TIM_IT_Update);	//清更新标志
		
		if(startFlag == 1)
		{
			//理论上，采集过程中不会进入更新中断10ms，所有信号最高只有9ms
			//一旦进入采集后进入了更新中断，采集完成
			startFlag = 0;//采集结束标志
			infraredInfo_t.edgeCnt = i;  //电平个数
			i = 0;				//下一次从0开始保存
			if(infraredInfo_t.edgeCnt > 10)//如果说采集少于十个，忽略
			{
				infraredInfo_t.captureOver = 1;//采集完成标志
			}
		}
	}else if(TIM_GetITStatus (TIM2, TIM_IT_CC2) == SET)//捕获2中断
	{
		TIM_ClearITPendingBit (TIM2, TIM_IT_CC2);			//清捕获2标志
		
		//切换边沿
		if(INFRARED_PIN_R == 0)//下降沿触发
		{
			TIM2->CCER &= ~(0x1 << 5);//切换为上升沿触发
			if(startFlag == 0)//第一次进来
			{
				TIM2->CNT = 0;
				startFlag = 1;//开始捕获标志
				i = 0;//电平时间存储位置
			}else//不是第一次下降沿触发
			{
				infraredInfo_t.timeCnt[i++] = TIM2->CNT;//读CNT -- 高电平时长  9000us
				TIM2->CNT = 0;//清空CNT，为下一次采集准备
			}
		}else//上升沿触发
		{
			TIM2->CCER |= (0x1 << 5);//切换为下降沿沿触发
			infraredInfo_t.timeCnt[i++] = TIM2->CNT;//读CNT -- 低电平时长  9000us
			TIM2->CNT = 0;//清空CNT，为下一次采集准备
		}
	}
}

/*
函数功能：解析红外NEC位码
函数形参：None
函数返回值：0:成功  其他：失败
备注：timeCnt - [0][1]是同步头的内容
			逻辑 1 ：560us 低+1680us 高
			逻辑 0 ：560us 低+560us 高 
			只需要判断高电平时间即可
			NECbit低位在前
作者：Yao
日期：2022/09/05
*/
u8 Infrared_AnalysisNECBit(u8 type)
{
	u16 i, j = 0;
	u16 edgeCnt = 0;
	u16 startNum = 0;
	
	switch(type)
	{
		case XYD_REMOTE: edgeCnt = 67; startNum = 3; break;//XYD遥控边沿最多67个
		case HISENSE_REMOTE: edgeCnt = 99; startNum = 3; break;//标准/理论上有这么多个
		case HISENSE_REMOTE_1: edgeCnt = 87; startNum = 6; break;//标准/理论上有这么多个
	}
	
	if(edgeCnt != infraredInfo_t.edgeCnt)//理论上的 != 实际采集的
		return 1;
	
	printf("type = %d 解析的位数据结果为\r\n", type);
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
	return 0;//转换成NECbit成功
}

void Infrared_PrintInfo(void)
{
	u8 i; 
	
	if(infraredInfo_t.captureOver == 1)
	{
//		infraredInfo_t.captureOver = 0;
		printf("捕获的边沿总数:%d\r\n", infraredInfo_t.edgeCnt);
		for(i = 0; i < infraredInfo_t.edgeCnt; i++)
		{
			printf("%d ", infraredInfo_t.timeCnt[i]);
		}
		
		printf("\r\n");
	}
}

/*
函数功能：NEC bit数据转换成字节
函数形参：遥控的类型
函数返回值：None
备注：bit存储低位在前
作者：Yao
日期：2022/09/05
*/
u8 Infrared_NECBitToByte(u8 type)
{
	u8 bytes, i, j;
	
	switch(type)
	{
		case XYD_REMOTE:  bytes = 4; break;//XYD遥控边沿最多67个
		case HISENSE_REMOTE: bytes = 6; break;//标准/理论上有这么多个
		case HISENSE_REMOTE_1:  bytes = 4; break;//标准/理论上有这么多个
		default : return 1;
	}
	for(i = 0; i < bytes; i++)
	{
		for(j = 0; j < 8; j++)
		{
			infraredInfo_t.necCode[i] <<= 1;//空出最低位
			//xxxx x100
			infraredInfo_t.necCode[i] |= infraredInfo_t.necBit[8*i + 7-j];
			//xxxx xx10
		}
		printf("0x%X ", infraredInfo_t.necCode[i]);
	}
	return 0;
}

/*
函数功能：解析红外NEC码
函数形参：遥控的类型
函数返回值：None
备注：逻辑 1 ：560us 低+1680us 高
			逻辑 0 ：560us 低+560us 高 
作者：Yao
日期：2022/09/05
*/
u8 Infrared_AnalysisNECCode(u8 type)
{
	u8 ret;
	
	//打印时间
	Infrared_PrintInfo( );
	
	if(infraredInfo_t.captureOver == 1)
	{
		infraredInfo_t.captureOver = 0;
		
		switch(type)
		{
			//XYD-红外遥控 9ms+4.5ms同步头
			case XYD_REMOTE: 
				if((infraredInfo_t.timeCnt[0] > 8500 && infraredInfo_t.timeCnt[0] < 9500) 
						&& (infraredInfo_t.timeCnt[1] > 4000 && infraredInfo_t.timeCnt[1] < 5000))
				{
					ret = Infrared_AnalysisNECBit( type );//如果同步头采集无误，就转换对应的NEC bit
					if(ret)
						return 1;//解析NEC位失败
					
					Infrared_NECBitToByte(XYD_REMOTE);
					
					infraredInfo_t.necCtrl = infraredInfo_t.necCode[2];//保存控制码
				}
				break;
			case HISENSE_REMOTE:
				if((infraredInfo_t.timeCnt[0] > 8500 && infraredInfo_t.timeCnt[0] < 9500) 
						&& (infraredInfo_t.timeCnt[1] > 4000 && infraredInfo_t.timeCnt[1] < 5000))
				{
					ret = Infrared_AnalysisNECBit( HISENSE_REMOTE );//如果同步头采集无误，就转换对应的NEC bit
					if(ret)
						return 1;//解析NEC位失败
					
					Infrared_NECBitToByte(HISENSE_REMOTE);
					
					infraredInfo_t.necCtrl = (infraredInfo_t.necCode[3] << 8) | infraredInfo_t.necCode[2];//保存控制码
					printf("控制码 = 0x%04x\r\n", infraredInfo_t.necCtrl);
				}
				break;
			case HISENSE_REMOTE_1:
				ret = Infrared_AnalysisNECBit( type );//如果同步头采集无误，就转换对应的NEC bit
				if(ret)
					return 1;//解析NEC位失败
				Infrared_NECBitToByte(HISENSE_REMOTE_1);
				infraredInfo_t.necCtrl = (infraredInfo_t.necCode[2] << 8) | infraredInfo_t.necCode[3];//保存控制码
				printf("控制码 = 0x%04x\r\n", infraredInfo_t.necCtrl);
				break;
		}
	}
	
	return 0;
}
