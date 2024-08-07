//头文件模板 - 条件编译
//宏名 -- 一般每个头文件唯一，一般都直接已头文件名进行命名
#ifndef _MAIN_H_ 	//if not define 宏名
									//如果没定义则一直到#endif之间的语句都有用，否则就不会起作用
#define _MAIN_H_	//定义 宏名

#include "stm32f10x.h"

int add(int a, int b)
{
	return a+b;
}

#endif
