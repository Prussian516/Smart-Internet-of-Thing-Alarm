#include "stm32f10x.h"
#include "led.h"
#include "delay.h"
#include "0.96OLED4PINI2C.h"
#include "usart.h"
#include "dht11.h"
#include "rtc.h" 
#include "stdlib.h"




int main()
{
	char temp_str[12];
	char damp_str[12];
	char sec[2];
	float temp=0;
	float damp=0;
	u8 t;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_Init();
	IO_init();
	delay_init();
	OLED_init();
	//OLED_Fill(0,0,128,64,1);
	//OLED_Fill(0,0,128,64,0);
	OLED_clear();
	RTC_Init();   //RTC时钟初始化
	t=0;
	
	while(1)
	{
		t++;
		//if(t>127)
		
		
		
		OLED_DisplayString(32,0,16,16,"年");
		OLED_Fill(0,0,16,16,0);
		OLED_DisplayInt(0,0,16,16,calendar.w_year);	

		
		OLED_DisplayString(62,0,16,16,"月");
		OLED_Fill(56,0,16,16,0);
		OLED_DisplayInt(56,0,16,16,calendar.w_month);

		OLED_DisplayString(94,0,16,16,"日");
		OLED_Fill(78,0,16,16,0);
		OLED_DisplayInt(78,0,16,16,calendar.w_date);
		
		
		OLED_DisplayString(16,2,16,16,"时");  //待完成
		OLED_Fill(0,2,16,16,0);
		OLED_DisplayInt(0,2,16,16,calendar.hour);	

		
		OLED_DisplayString(48,2,16,16,"分");
		OLED_Fill(32,2,16,16,0);
		OLED_DisplayInt(32,2,16,16,calendar.min);

		
		
	
	}
	
	
	
	
	
	
	
	
	}
//	while(1)
//	{
//	ReadDHT11();
//	temp = (int)tdata[2]+(int)tdata[3]/10; //温度
//	damp = (int)tdata[0]+(int)tdata[1]/10; //湿度
//	
//	float_to_str(temp,temp_str,2,2,0);
//	float_to_str(damp,damp_str,2,2,0);
//		
//	OLED_DisplayString(0,0,16,16,"2020/9/12");
//	OLED_DisplayString(0,2,16,16,"温度");
//	OLED_DisplayString(32,2,16,16,temp_str);
//	OLED_DisplayString(0,4,16,16,"湿度");
//	OLED_DisplayString(32,4,16,16,damp_str);
//	OLED_DisplayString(0,6,16,16,"Are you ok ?");
//	if(temp>0)
//		LED0=!LED0;

//	delay_ms(500);
//		
//	}
//}


