#ifndef __OLED_h
#define __OLED_h


#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"

void SYS_init(unsigned char PLL); //ʱ�ӳ�ʼ��
void IO_init(void);  //IO�ڳ�ʼ��  
//IICģ��
void IIC_write(unsigned char data);  //дI2C����  
void IIC_start();  //I2C�����ź�
void IIC_stop();   // I2C��ֹ�ź�

//OLED������ݷ��ͺ���
void OLED_send_cmd(unsigned char o_command);  //������������
void OLED_send_data(unsigned char o_data);  //��������
void Column_set(unsigned char column);  //������
void Page_set(unsigned char page);  //����ҳ




//OLED���ܼ��ɺ���
void OLED_clear(void);  //��������
void OLED_full(void);  //������Ļ
void OLED_init(void);  //oled��ʼ��
void OLED_Refresh_Gram(void);  //����Gram�Դ浽Oled��Ļ
void Picture_display(const unsigned char *ptr_pic);
void Picture_ReverseDisplay(const unsigned char *ptr_pic);
void OLED_Draw_Point(u8 x,u8 y,u8 c);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);  // ���/��� ��(x1,y1),(x2,y2)Ϊ�Խǵľ������� 
void OLED_SetCursorAddrese(u8 x,u8 y);
void OLED_DisplayString(u8 x,u8 y,u8 width,u8 height,u8 *str);
void OLED_DisplayInt(u8 x,u8 y,u8 width,u8 height,int num); //�ڣ�x,y������ʾ��width����height������num
#endif

