#ifndef _OLED_DIS_H_
#define _OLED_DIS_H_

#include "stm32f10x.h"

void OLED_DisplayA(u8 page, u8 col);
void OLED_DisplayChar(u8 page, u8 col, u8 ch);
void OLED_DisplayString(u8 page, u8 col, u8 *p);

void OLED_RollString(u8 page, u8 col, u8 *p);

void OLED_DisplayPicture(u8 page, u8 col, u8 *pPic);
void OLED_DisplayChinese(u8 page, u8 col, u8 *p);

void OLED_ClearNCol(u8 page, u8 col, u8 pageNum, u8 colNum);

extern u8 test_pic[];
extern u8 rooster[] ;

extern u8 test1[];
extern u8 close[];
extern u8 open[];
#endif
