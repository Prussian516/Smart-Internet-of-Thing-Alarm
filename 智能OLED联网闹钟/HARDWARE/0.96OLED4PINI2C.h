#ifndef __OLED_h
#define __OLED_h


#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"

void SYS_init(unsigned char PLL); //时钟初始化
void IO_init(void);  //IO口初始化  
//IIC模块
void IIC_write(unsigned char data);  //写I2C数据  
void IIC_start();  //I2C启动信号
void IIC_stop();   // I2C终止信号

//OLED命令、数据发送函数
void OLED_send_cmd(unsigned char o_command);  //发送配置命令
void OLED_send_data(unsigned char o_data);  //发送数据
void Column_set(unsigned char column);  //设置列
void Page_set(unsigned char page);  //设置页




//OLED功能集成函数
void OLED_clear(void);  //清屏函数
void OLED_full(void);  //铺满屏幕
void OLED_init(void);  //oled初始化
void OLED_Refresh_Gram(void);  //更新Gram显存到Oled屏幕
void Picture_display(const unsigned char *ptr_pic);
void Picture_ReverseDisplay(const unsigned char *ptr_pic);
void OLED_Draw_Point(u8 x,u8 y,u8 c);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);  // 填充/清除 以(x1,y1),(x2,y2)为对角的矩形区域 
void OLED_SetCursorAddrese(u8 x,u8 y);
void OLED_DisplayString(u8 x,u8 y,u8 width,u8 height,u8 *str);
void OLED_DisplayInt(u8 x,u8 y,u8 width,u8 height,int num); //在（x,y）点显示宽width，高height的数字num
#endif

