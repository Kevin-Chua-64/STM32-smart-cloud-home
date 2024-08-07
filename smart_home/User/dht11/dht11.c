#include "dht11.h"
#include "delay.h"
#include <stdio.h>

void DHT11_Init(void)
{
	//打开GPIOB时钟 -- APB2
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB, ENABLE);//打开AFIO时钟
	GPIO_PinRemapConfig (GPIO_Remap_SWJ_JTAGDisable, ENABLE);//映射
	
	//初始化PB3 - 开漏输出
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;//开漏输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;//没有特殊 要求按最低速处理
	GPIO_Init (GPIOB, &GPIO_InitStruct);
	
	DHT11_DATA_H;//空闲 都不操作数据线
	delay_ms(1000);//延时1s等待越过不稳定期
}

void DHT11_Start(void)
{
	DHT11_DATA_L;//主机产生起始信号
	delay_ms(20);//起始信号保持时间
	DHT11_DATA_H;//主机释放总线 -- 等待从机应答
}



//等待应答
//0 - 成功  其他 - 失败
u8 DHT11_WaitAck(void)
{
	u8 timCnt = 0;
	
	while(DHT11_DATA_R == 1)//过滤主机等待从机应答的反应时间
	{
		timCnt++;
		delay_us(1);
		if(timCnt > 50)
			return 1;//响应失败
	}
	
	timCnt = 0;
	//统计从机应答时间
	while(DHT11_DATA_R == 0)
	{
		timCnt++;
		delay_us(1);
		if(timCnt > 100)
			return 2;//响应失败
	}
	
	while(DHT11_DATA_R == 1);//过滤从机拉高数据线通知主机采集数据
	
	return 0;
}

//获取位数据
u8 DHT11_GetBit(void)
{
	//由于位数0、1的低电平时长相同，所以直接过滤所有的低电平时长
	while(DHT11_DATA_R == 0){}
	
	delay_us(50);//延时50us -- 区分位数据1/0
	if(DHT11_DATA_R == 0)
		return 0;
	
	//延时50us之后还是为高电平 -- 还是在位数据1的68~74us的时间段之中
	while(DHT11_DATA_R == 1);//过滤位数据1剩下的时长
	return 1;
}

//采集40bit数据 - 高位在前
void DHT11_GetData(u8 *data)//u8 data[5]
{
	u8 i, j;
	
	for(i = 0; i < 5; i++)//采集5个8bit数据
	{
		data[i] = 0;//清零
		for(j = 0; j < 8; j++)//采集8bit数据
		{			
			if(DHT11_GetBit( ) == 1)
				data[i] |= (1 << (7-j));
		}
	}
}

//获取温湿度数据
u8 DHT11_GetTempRH(float *temp, float *humi)
{
	u8 ret;
	u8 data[5];
	u8 checkSum;
//	u8 i;
	
	DHT11_Start( );//起始信号
	ret = DHT11_WaitAck( );//等待应答
	if(ret)
		return 1;//应答失败
	
	DHT11_GetData(data);//获取40bit数据
	
	checkSum = data[0] + data[1] + data[2] + data[3];
	if(checkSum != data[4])
		return 2;//校验失败
	
	*humi = data[0] + data[1]/10;
	if(data[3] & 0x80)//判断是否时负数
		//负数
		*temp = -1 * (data[2] + ((data[3] << 1) >> 1) / 10.0);
	else
		*temp = data[2] + data[3]/10.0;
	
//	for(i = 0;  i < 4; i++)
//		printf("data[%d] = %d\r\n", i, data[i]);
	
	return 0;//获取数据成功
}
