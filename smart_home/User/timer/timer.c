#include "timer.h"

/*************************
函数名：timer_Init
函数功能：系统滴答中断的的初始化函数
函数形参：void
函数的返回值：void
备注:打开中断请求，1ms一次中断
*************************/
void timer_Init(void)
{
	SysTick->CTRL |=(1<<2); //72Mhz 内部时钟
	
	SysTick->LOAD=72000-1;  //实现1ms进入一次中断
	
	SysTick->VAL=0;  //当前值清零
	
	SysTick->CTRL |=(1<<1); //系统滴答的中断使能
	
	SysTick->CTRL |=(1<<0); //使能计数器
	
	//不需要核心级别中断使能 ----系统滴答处于内核种
}

/*************************
函数名：SysTick_Handler
函数功能：系统滴答的中断服务函数
函数形参：void
函数的返回值：void
备注：不需要调用 不需要声明
*************************/
u8 flag_1s=0;
void SysTick_Handler(void) //进入系统滴答中断条件 每一次产生下溢时间
{
	static u16 cnt=0;
	//每间隔1ms就进入一次
	if(SysTick->CTRL & (1<<16))  //判断进入
	{
		//不需要清除标志位 ----- 计数器重新加载值的时候就清零了
		cnt++;
		if (cnt==1000)
		{
			cnt = 0;
			flag_1s = 1;
		}
	}
}
