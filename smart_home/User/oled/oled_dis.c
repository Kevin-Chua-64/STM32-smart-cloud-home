#include "oled_dis.h"
#include "oled.h"
#include "font.h"
#include <string.h>

u8 ascii_A[] = {
0x00,0x00,0xC0,0x38,0xE0,0x00,0x00,0x00,
0x20,0x3C,0x23,0x02,0x02,0x27,0x38,0x20,/*"A",0*/
};

void OLED_SetPosition(u8 page, u8 col)
{
	OLED_SendCommand(0xB0 | page);//设置起始页地址
	OLED_SendCommand(0x00 | (col & 0x0F));//设置起始列地址低四位
	OLED_SendCommand(0x10 | (col >> 4));//设置起始列地址高四位
}

void OLED_DisplayA(u8 page, u8 col)
{
	u8 i, j;
	
	for(i = 0; i < 2; i++)//控制页数
	{
		//设置起始显示位置
		OLED_SetPosition(page+i, col);
		
		for(j = 0; j < 8; j++)//控制列数
		{
			OLED_SendData(ascii_A[8*i+j]);
		}
	}
}

void OLED_DisplayChar(u8 page, u8 col, u8 ch)//'!'
{
	u8 i, j;
	//算出ch在字符数组中的偏移位置
	u32 offset = (ch - ' ') * 16;
	
	for(i = 0; i < 2; i++)//控制页数
	{
		//设置起始显示位置
		OLED_SetPosition(page+i, col);
		
		for(j = 0; j < 8; j++)//控制列数
		{
			OLED_SendData(ascii_16[offset+8*i+j]);
		}
	}
}


void OLED_DisplayString(u8 page, u8 col, u8 *p)//"hello world"
{
	u8 offset_col = 0;
	
	while(*p != '\0')
	{
		if(*p > 0xA0) //中文
		{
			OLED_DisplayChinese(page, col, p);
			col += 16;
			p += 2;
		}else //英文
		{
			OLED_DisplayChar(page, col, *p++);
			col += 8;
		}
		
		//判断下一个需要显示的字符是中文还是英文
		(*p > 0xA0) ? (offset_col = 16) : (offset_col = 8);
		if(col + offset_col > 127)
		{
			page += 2;
			col = 0;
		}
	}
}

//同一页内滚动字符串不需要换页
void OLED_RollString(u8 page, u8 col, u8 *p)
{
	while(*p != '\0')
	{
		if(*p > 0xA0) //中文
		{
			OLED_DisplayChinese(page, col, p);
			col += 16;
			p += 2;
		}else //英文
		{
			OLED_DisplayChar(page, col, *p++);
			col += 8;
		}
	}
}

void OLED_DisplayPicture(u8 page, u8 col, u8 *pPic)//test1
{
	u8 i, j;
	
	u8 wide = pPic[0];
	u8 high = pPic[1];
	
	for(i = 0; i < (high-1)/8+1; i++)//控制页数
	{
		//设置起始显示位置
		OLED_SetPosition(page+i, col);
		
		for(j = 0; j < wide; j++)//控制列数
		{
			OLED_SendData(pPic[2+wide*i+j]);
		}
	}
}

void OLED_DisplayChinese(u8 page, u8 col, u8 *p)//"hello world"	"深" - 区码、位码
{
	u8 i, j;
	u32 offset;
	
	//查找汉字在子字库中的位置
	for(i = 0; i < strlen((char *)subChineseFont)/2; i++)
	{
		if(p[0]==subChineseFont[2*i] && p[1]==subChineseFont[2*i+1])
		{
			offset = i * 32;//找到了
			break;
		}
	}
	
	if(i == strlen((char *)subChineseFont)/2)//汉字不存在
		return ;
	
	for(i = 0; i < 2; i++)//控制页数
	{
		//设置起始显示位置
		OLED_SetPosition(page+i, col);
		
		for(j = 0; j < 16; j++)//控制列数
		{
			OLED_SendData(chinese_16[offset+16*i+j]);
		}
	}
}

void OLED_ClearNCol(u8 page, u8 col, u8 pageNum, u8 colNum)
									//   2				30				7					50
{
	u8 i, j;
	
	//参数检测
	if(page + pageNum - 1 > 7)
		pageNum = 7 - page + 1;
	if(col + colNum - 1 > 127)
		colNum = 127 - col + 1;
	
	for(i = 0; i < pageNum; i++)//控制页数
	{
		//设置起始显示位置
		OLED_SetPosition(page+i, col);
		
		for(j = 0; j < colNum; j++)//控制列数
		{
			OLED_SendData(0x00);
		}
	}
}
