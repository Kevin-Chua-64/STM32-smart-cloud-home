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
	OLED_SendCommand(0xB0 | page);//������ʼҳ��ַ
	OLED_SendCommand(0x00 | (col & 0x0F));//������ʼ�е�ַ����λ
	OLED_SendCommand(0x10 | (col >> 4));//������ʼ�е�ַ����λ
}

void OLED_DisplayA(u8 page, u8 col)
{
	u8 i, j;
	
	for(i = 0; i < 2; i++)//����ҳ��
	{
		//������ʼ��ʾλ��
		OLED_SetPosition(page+i, col);
		
		for(j = 0; j < 8; j++)//��������
		{
			OLED_SendData(ascii_A[8*i+j]);
		}
	}
}

void OLED_DisplayChar(u8 page, u8 col, u8 ch)//'!'
{
	u8 i, j;
	//���ch���ַ������е�ƫ��λ��
	u32 offset = (ch - ' ') * 16;
	
	for(i = 0; i < 2; i++)//����ҳ��
	{
		//������ʼ��ʾλ��
		OLED_SetPosition(page+i, col);
		
		for(j = 0; j < 8; j++)//��������
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
		if(*p > 0xA0) //����
		{
			OLED_DisplayChinese(page, col, p);
			col += 16;
			p += 2;
		}else //Ӣ��
		{
			OLED_DisplayChar(page, col, *p++);
			col += 8;
		}
		
		//�ж���һ����Ҫ��ʾ���ַ������Ļ���Ӣ��
		(*p > 0xA0) ? (offset_col = 16) : (offset_col = 8);
		if(col + offset_col > 127)
		{
			page += 2;
			col = 0;
		}
	}
}

//ͬһҳ�ڹ����ַ�������Ҫ��ҳ
void OLED_RollString(u8 page, u8 col, u8 *p)
{
	while(*p != '\0')
	{
		if(*p > 0xA0) //����
		{
			OLED_DisplayChinese(page, col, p);
			col += 16;
			p += 2;
		}else //Ӣ��
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
	
	for(i = 0; i < (high-1)/8+1; i++)//����ҳ��
	{
		//������ʼ��ʾλ��
		OLED_SetPosition(page+i, col);
		
		for(j = 0; j < wide; j++)//��������
		{
			OLED_SendData(pPic[2+wide*i+j]);
		}
	}
}

void OLED_DisplayChinese(u8 page, u8 col, u8 *p)//"hello world"	"��" - ���롢λ��
{
	u8 i, j;
	u32 offset;
	
	//���Һ��������ֿ��е�λ��
	for(i = 0; i < strlen((char *)subChineseFont)/2; i++)
	{
		if(p[0]==subChineseFont[2*i] && p[1]==subChineseFont[2*i+1])
		{
			offset = i * 32;//�ҵ���
			break;
		}
	}
	
	if(i == strlen((char *)subChineseFont)/2)//���ֲ�����
		return ;
	
	for(i = 0; i < 2; i++)//����ҳ��
	{
		//������ʼ��ʾλ��
		OLED_SetPosition(page+i, col);
		
		for(j = 0; j < 16; j++)//��������
		{
			OLED_SendData(chinese_16[offset+16*i+j]);
		}
	}
}

void OLED_ClearNCol(u8 page, u8 col, u8 pageNum, u8 colNum)
									//   2				30				7					50
{
	u8 i, j;
	
	//�������
	if(page + pageNum - 1 > 7)
		pageNum = 7 - page + 1;
	if(col + colNum - 1 > 127)
		colNum = 127 - col + 1;
	
	for(i = 0; i < pageNum; i++)//����ҳ��
	{
		//������ʼ��ʾλ��
		OLED_SetPosition(page+i, col);
		
		for(j = 0; j < colNum; j++)//��������
		{
			OLED_SendData(0x00);
		}
	}
}
