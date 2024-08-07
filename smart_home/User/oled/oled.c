#include "oled.h"
#include "delay.h"

//�ܽų�ʼ��
//OLED_RES 	- PB6
//OLED_DC 	- PB12
//OLED_CS 	- PB7
//OLED_SCK 	- PB13
//OLED_MOSI	- PB15
//MODE3 - SCK���е�ƽ = �ߵ�ƽ
void OLED_PinInit(void)
{
	//��GPIOBʱ��
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;//�ṹ�����
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//�������
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	OLED_RES_H;//normal operation
	OLED_CS_H;//��ѡ�дӻ� - ����
	OLED_SCK_H;//MDOE3 - ����Ϊ�ߵ�ƽ
}

//SPI����byte����
void OLED_SPITransferByte(u8 byte)
{
	u8 i;
	
	for(i = 0; i < 8; i++)
	{
		OLED_SCK_L;//�½��� - ׼������
		if(byte & (0x80 >> i))
			OLED_MOSI_H;
		else
			OLED_MOSI_L;
		
		OLED_SCK_H;//������- �ɼ�����
		//�ӻ��ɼ�
	}
}

//OLED��������
void OLED_SendCommand(u8 cmd)
{
	OLED_CS_L;
	OLED_DC_L;
	OLED_SPITransferByte(cmd);
	OLED_CS_H;
}

//��������
void OLED_SendData(u8 data)
{
	OLED_CS_L;
	OLED_DC_H;
	OLED_SPITransferByte(data);
	OLED_CS_H;
}

//��������
void OLED_Clear(u8 data)
{
	u8 i, j;
	
	for(i = 0; i < 8; i++)//����ҳ��
	{
		//������ʼ��ַ
		OLED_SendCommand(0xB0 + i);//��ʼҳ��ַ  0~7
		OLED_SendCommand(0x00);//��ʼ�е�ַ����λ  0
		OLED_SendCommand(0x10);//��ʼ�е�ַ����λ  0
		
		for(j = 0; j < 128; j++)//��������
		{
			OLED_SendData(data);
		}
	}
}

void OLED_Init(void)
{
	OLED_PinInit( );
	
	OLED_RES_H;
	delay_ms(100);
	OLED_RES_L;//��ʼ��оƬ
	delay_ms(100);
	OLED_RES_H;
	delay_ms(100); 
	
	OLED_SendCommand(0xAE); //�ر���ʾ
	OLED_SendCommand(0xD5); //����ʱ�ӷ�Ƶ����,��Ƶ��
	OLED_SendCommand(80);   //[3:0],��Ƶ����;[7:4],��Ƶ��
	OLED_SendCommand(0xA8); //��������·��
	OLED_SendCommand(0X3F); //Ĭ��0X3F(1/64)
	OLED_SendCommand(0xD3); //������ʾƫ��
	OLED_SendCommand(0X00); //Ĭ��Ϊ0

	OLED_SendCommand(0x40); //������ʾ��ʼ�� [5:0],����.

	OLED_SendCommand(0x8D); //��ɱ�����
	OLED_SendCommand(0x14); //bit2������/�ر�
	OLED_SendCommand(0x20); //�����ڴ��ַģʽ
	OLED_SendCommand(0x02); //[1:0],00���е�ַģʽ;01���е�ַģʽ;10,ҳ��ַģʽ;Ĭ��10;
	OLED_SendCommand(0xA1); //���ض�������,bit0:0,0->0;1,0->127;
	OLED_SendCommand(0xC8); //����COMɨ�跽��;bit3:0,��ͨģʽ;1,�ض���ģʽ COM[N-1]->COM0;N:����·��
	OLED_SendCommand(0xDA); //����COMӲ����������
	OLED_SendCommand(0x12); //[5:4]����

	OLED_SendCommand(0x81); //�Աȶ�����
	OLED_SendCommand(0xEF); //1~255;Ĭ��0X7F (��������,Խ��Խ��)
	OLED_SendCommand(0xD9); //����Ԥ�������
	OLED_SendCommand(0xf1); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_SendCommand(0xDB); //����VCOMH ��ѹ����
	OLED_SendCommand(0x30); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_SendCommand(0xA4); //ȫ����ʾ����;bit0:1,����;0,�ر�;(����/����)
	OLED_SendCommand(0xA6); //������ʾ��ʽ;bit0:1,������ʾ;0,������ʾ
	OLED_SendCommand(0xAF); //������ʾ	
	
	OLED_Clear(0x00);//����
}
