#include "stm32f10x.h"
#include "dht11.h"
#include "delay.h"
#include "sys.h"
u8 tdata[4]={0x00,0x00,0x00,0x00};
u8 sbuf,check;
//***************************************************************************/
//      //uchar       i;
//      uchar    U8FLAG,k;
//      uchar    U8count,U8temp;
//      uchar    U8T_data_H,U8T_data_L,U8RH_data_H,U8RH_data_L,U8checkdata;
//      uchar    U8T_data_H_temp,U8T_data_L_temp,U8RH_data_H_temp,U8RH_data_L_temp,U8checkdata_temp;
//      uchar    U8comdata;
//      uint   ReceiveHighByte;
//      uint   ReceiveLowByte;
//***************Global defination for DHT11 end****// 
void DHT_PortIN(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PC端口时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;     
    GPIO_Init(GPIOC,&GPIO_InitStructure);
}
void DHT_PortOUT(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PC端口时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     
    GPIO_Init(GPIOC,&GPIO_InitStructure);
}
unsigned char StartDHT11(void)
{
   DHT_PortOUT();
   DHT_Out = 0;
   delay_ms(20);  //DHT11>18ms
  
   DHT_Out = 1;    
   DHT_PortIN();
   delay_us(20);  

   if(!DHT_In)
   {
           while(!DHT_In);
           while(DHT_In);
           return 1;
   }
    return 0;
}

void com(void)
{
    u8 i,tt;
    tt = 0;
    for(i = 0;i<8;i++)  //循环8次读取数据
    {
        sbuf <<= 1;
        delay_us(20);
        while(!DHT_In);
        delay_us(25);
        tt = 100;
        while(tt++);
        if(DHT_In)
        {
             sbuf |= 0x01;  
             delay_us(30);
        }
        else
        {
            sbuf &= 0xfe;
        }
    }
}
u8 ReadDHT11(void)
{
    u8 sum;
    if(StartDHT11())    
    {
        com();
        tdata[0]=sbuf;  //第一位是湿度整数部分
        com();    
        tdata[1]=sbuf;  //第二位是湿度小数部分
        com();
        tdata[2]=sbuf; //第三位是温度整数部分
        com();
        tdata[3]=sbuf;  //第四位是温度小数部分
        com();
        check = sbuf;
        sum = (tdata[0]+tdata[1]+tdata[2]+tdata[3]);
    }
    if(check == sum)
    return(1);
    else    
    return 0;
}
