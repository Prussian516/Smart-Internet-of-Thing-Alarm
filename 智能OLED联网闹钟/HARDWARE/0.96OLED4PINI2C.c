#include "0.96OLED4PINI2C.h"
#include "word.h"
/*  0.96OLED4PIN 驱动代码  *
****************************
*	auchor:yetian
*	time:2020/9/10
*	
*
*
*****************************/



/*******SPI引脚分配 链接OLED屏，根据实际情况修改**************/

#define IIC_SCK_PIN 5
#define IIC_SDA_PIN 6

#define IIC_SCK_0 GPIOB->BRR = 0X0020  //设置sck接口 到引脚PB5 清零
#define IIC_SCK_1 GPIOB->BSRR = 0X0020  //置位
#define IIC_SDA_0 GPIOB->BRR = 0X0040  //  设置SDA接口到PB6
#define IIC_SDA_1 GPIOB->BSRR = 0X0040 //

const unsigned char *point;
unsigned char ACK = 0;
const unsigned char OLED_init_cmd[25]=  //oled初始化设置命令
{
  /*0xae,0X00,0X10,0x40,0X81,0XCF,0xff,0xa1,0xa4,
  0xA6,0xc8,0xa8,0x3F,0xd5,0x80,0xd3,0x00,0XDA,0X12,
  0x8d,0x14,0xdb,0x40,0X20,0X02,0xd9,0xf1,0xAF*/
       0xAE,//关闭显示
       0xD5,//设置时钟分频因子,震荡频率
       0x80,  //[3:0],分频因子;[7:4],震荡频率

       0xA8,//设置驱动路数
       0X3F,//默认0X3F(1/64)
       0xD3,//设置显示偏移
       0X00,//默认为0
       0x40,//设置显示开始行 [5:0],行数.                              
       0x8D,//电荷泵设置
       0x14,//bit2，开启/关闭
       0x20,//设置内存地址模式
       0x02,//[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
       0xA1,//段重定义设置,bit0:0,0->0;1,0->127;
       0xC8,//设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
       0xDA,//设置COM硬件引脚配置
       0x12,//[5:4]配置            
       0x81,//对比度设置
       0xEF,//1~255;默认0X7F (亮度设置,越大越亮)
       0xD9,//设置预充电周期
       0xf1,//[3:0],PHASE 1;[7:4],PHASE 2;
       0xDB,//设置VCOMH 电压倍率
       0x30,//[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;
       0xA4,//全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
       0xA6,//设置显示方式;bit0:1,反相显示;0,正常显示        
       0xAF,//开启显示     
};

unsigned char oled_show_tab[8][128];

/*********************使能IO口**********************************/ 
void IO_init(void)
{
	RCC->APB2ENR|=1<<3;    //使能PORTB时钟 
	GPIOB->CRL&=0X00FFFFFF;				//将B56口配置为通用开漏输出,最大50MH
	GPIOB->CRL|=0X07700000;				//B7配置为通用推挽输出,最大50MH
	GPIOB->ODR=0XFFFF;
}


/*********************时钟初始化******************************/
void SYS_init(unsigned char PLL)
{
	
		 
	RCC->APB1RSTR = 0x00000000;//复位结束			 
	RCC->APB2RSTR = 0x00000000; 	  
  	RCC->AHBENR = 0x00000014;  //睡眠模式闪存和SRAM时钟使能.其他关闭.	  
  	RCC->APB2ENR = 0x00000000; //外设时钟关闭.			   
  	RCC->APB1ENR = 0x00000000;   
	RCC->CR |= 0x00000001;     //使能内部高速时钟HSION
	
	RCC->CFGR &= 0xF8FF0000;   //复位SW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]					 
	RCC->CR &= 0xFEF6FFFF;     //复位HSEON,CSSON,PLLON
	RCC->CR &= 0xFFFBFFFF;     //复位HSEBYP	   	  
	RCC->CFGR &= 0xFF80FFFF;   //复位PLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE
	while(((RCC->CFGR>>2)& 0x03 )!=0x00); 	
	RCC->CIR = 0x00000000;     //关闭所有中断		 
	//配置向量表				  
  
//	SCB->VTOR = 0x08000000|(0x0 & (u32)0x1FFFFF80);//设置NVIC的向量表偏移寄存器
	
 	RCC->CR|=0x00010000;  //外部高速时钟使能HSEON
	while(((RCC->CR>>17)&0x00000001)==0);//等待外部时钟就绪
	RCC->CFGR=0X00000400; //APB1=DIV2;APB2=DIV1;AHB=DIV1;
	PLL-=2;//抵消2个单位
	RCC->CFGR|=PLL<<18;   //设置PLL值 2~16
	RCC->CFGR|=1<<16;	  //PLLSRC ON 
	

	RCC->CR|=0x01000000;  //PLLON
	while(!(RCC->CR>>25));//等待PLL锁定
	RCC->CFGR|=0x00000002;//PLL作为系统时钟	 
	while(((RCC->CFGR>>2)&0x03)!=0x02);   //等待PLL作为系统时钟设置成功
	
	
}






/**************************IIC模块发送函数************************************************

 *************************************************************************/
//写入  最后将SDA拉高，以等待从设备产生应答
void IIC_write(unsigned char date)
{
	unsigned char i, temp;
	temp = date;
			
	for(i=0; i<8; i++)
	{	IIC_SCK_0;
		
        if ((temp&0x80)==0)
			 IIC_SDA_0;
        else IIC_SDA_1;
		temp = temp << 1;
		delay_us(1);			  
		IIC_SCK_1;
		delay_us(1);
		
	}
	IIC_SCK_0;
	delay_us(1);
	IIC_SDA_1;
	delay_us(1);
	IIC_SCK_1;
//								不需要应答
//	if (READ_SDA==0)
//		ACK = 1;
//	else ACK =0;
	delay_us(1);
	IIC_SCK_0;
	delay_us(1);
	

}
//启动信号
//SCL在高电平期间，SDA由高电平向低电平的变化定义为启动信号
void IIC_start()
{
	IIC_SDA_1;
	delay_us(1);
	IIC_SCK_1;
	delay_us(1);				   //所有操作结束释放SCL	
	IIC_SDA_0;
	delay_us(3);
	IIC_SCK_0;
	
        IIC_write(0x78);
        
}

//停止信号
//SCL在高电平期间，SDA由低电平向高电平的变化定义为停止信号
void IIC_stop()
{
	IIC_SDA_0;
	delay_us(1);
	IIC_SCK_1;
	delay_us(3);
	IIC_SDA_1;
	
}




/*****OLED功能函数********/
void OLED_send_cmd(unsigned char o_command)  //传输命令
{
	IIC_start();
	IIC_write(0x00);
	IIC_write(o_command);
	IIC_stop();
}

void OLED_send_data(unsigned char o_data) //传入数据
{
	IIC_start();
	IIC_write(0x40);
	IIC_write(o_data);
	IIC_stop();
}

void Column_set(unsigned char column)  //设置列地址
{
	OLED_send_cmd(0x10|(column>>4));  //设置列地址高位
	OLED_send_cmd(0x00|(column&0x0f));  //设置列地址低位
}

void Page_set(unsigned char page)
{
	OLED_send_cmd(0xb0+page);
}

void OLED_Refresh_Gram(void)  //更新Gram显存到OLED屏幕
{
	
	u8 page,list;  //定义页地址和列地址
	for(page=0;page<8;page++)
	{
		OLED_send_cmd(0xb0+page); //设置页地址（0~7）
		OLED_send_cmd(0x00);      //设置显示位置―列低地址
		OLED_send_cmd(0x10);      //设置显示位置―列高地址 
		for(list=0;list<128;list++)
		{
			OLED_send_data(oled_show_tab[page][list]);
		}
	}
	//memset(oled_show_tab,0,sizeof(oled_show_tab));	/*清空显存数组*/

}



void OLED_clear(void)  //清屏函数
{
	u8 i,n;
	for(i=0;i<8;i++)
	{
		OLED_send_cmd(0xb0+i);
		OLED_send_cmd(0x00);  //设置显示位置―列低地址
		OLED_send_cmd(0x10);  //设置显示位置―列高地址 
		for(n=0;n<128;n++)
		{
			OLED_send_data(0x00);
		}

	}
}

void OLED_full(void)  //填充函数，铺满整个屏幕
{
	u8 i,n;
	for(i=0;i<8;i++)
	{
		OLED_send_cmd(i);
		OLED_send_cmd(0x00);  //设置显示位置―列低地址
		OLED_send_cmd(0x10);  //设置显示位置―列高地址 
		for(n=0;n<128;n++)
		{
			OLED_send_data(0xFF);
		}

	}
}


void OLED_init(void)  //OLED初始化函数
{

	unsigned char i;
	for(i=0;i<25;i++)
	{
		OLED_send_cmd(OLED_init_cmd[i]);
	}
	OLED_clear();
	OLED_SetCursorAddrese(0,0);
	
}



/*函数功能:开启OLED显示 */
void OLED_Display_On(void)
{
	OLED_send_cmd(0X8D);  //SET DCDC命令（设置电荷泵）
	OLED_send_cmd(0X14);  //DCDC ON （开启电荷泵）
	OLED_send_cmd(0XAF);  //DISPLAY ON（OLED唤醒）
}
/*函数功能:关闭OLED显示*/ 
 
void OLED_Display_Off(void)
{
	OLED_send_cmd(0X8D);  //SET DCDC命令 （设置电荷泵）
	OLED_send_cmd(0X10);  //DCDC OFF （关闭电荷泵）
	OLED_send_cmd(0XAE);  //DISPLAY OFF （OLED休眠）
}
 




/*
函数功能：在显存数组上画一个点
函数参数：x，y为点的横纵坐标   c为这个点的亮灭（1亮0灭）
参数范围：x 0~128  y 0~8 
每一个数据是 低位在前，高位在后
*/
void OLED_Draw_Point(u8 x,u8 y,u8 c)
{
	y &=0x3F ; //保证y值不超过63
	x &= 0x7F;  //保证x值不超过127
	if(c)
	{
		oled_show_tab[x][7-y/8] |= 0x1 << (7-y%8); //点亮该点
	}
	else
	{
		oled_show_tab[x][7-y/8] |= ~(0x1 << (7-y%8));  //熄灭该点
	}
	
}

//填充（x1，y1)到(x2,y1+16)区域
//确保x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63	 	 
//dot:0,清空;1,填充	  
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot)  
{  
	char i = 0;
	OLED_SetCursorAddrese(x1,y1);
	if(dot)
	{
		for(i=x1;i<=x2;i++)
			OLED_send_data(0xff);
	}
	else
	{
		for(i=x1;i<=x2;i++)
			OLED_send_data(0x00);
	}
	OLED_SetCursorAddrese(x1,y1+1);
	if(dot)
	{
		for(i=x1;i<=x2;i++)
			OLED_send_data(0xff);
	}
	else
	{
		for(i=x1;i<=x2;i++)
			OLED_send_data(0x00);
	}
	
}


/*
函数功能: 设置光标位置
函数参数: x列的起始位置(0~127)
				  y页的起始位置(0~7)
比如: 0x8  高4位0000   低4位1000 
*/
void OLED_SetCursorAddrese(u8 x,u8 y)
{
		OLED_send_cmd(0xB0+y); 					//设置页地址
	  //第6列显示数据  6被分成2个4位(高位和低位)。
		OLED_send_cmd((x&0xF0)>>4|0x10);//设置列高起始地址(半字节)
		OLED_send_cmd((x&0x0F)|0x00);   //设置列低起始地址(半字节)			
}



void float_to_str(float n,char *reChar,int zsize,int xsize,int flag)//功能将浮点数n转成字符串，保存到以reChar地址开头的字符数组中（可实现插入）
//flag=0:直接转换浮点数到指定地址    flag=1：将转换的浮点数插入到指定地址位置
//zsize：n整数部分最大可能的位数。   xsize：n小数部分最大可能的位数+1(包含小数点)。
//reChar：用于返回处理后的字符串
{
 
    int z,x,i=0,j=0;
	//char a[1+zsize+xsize];
	char a[10];
	
    n=n+0.00001;//+0.00001避免浮点数精度丢失,可根据你传感器输出实际数字位数修改，不要超出类型范围
    z=(int)n;
    x=(int)((n-z)*10);//取小数数字，最大1位小数 *10  2位*100 3位*1000 需要自行修改 你也可以自己写10次方函数利用xsize运算 
 
    while(x/10!=0)
    {
        a[i++]=x%10+'0';
        x=x/10;
    }
    a[i++]=x+'0';
    a[i++]='.';
    while(z/10!=0)
    {
        a[i++]=z%10+'0';
        z=z/10;
    }
    a[i++]=z+'0';
        a[i]=0;
 
    for(i=zsize+xsize-1;i>=0;i--)
        reChar[j++]=a[i];
    if(flag==0)
        reChar[j]=0;
}


u8 int_num_length(int n)  //返回整数的长度
{
	int length=0;
	if(n>0)  //n>0时
	{
		while(n)
		{
			length++;
			n=n/10;
		}	
		return length;
	}
	else if(n==0) //若n==0
	{
		return 1;
	}
	else
	{
		n=n*-1;
		while(n)
		{
			length++;
			n=n/10;
		}
		return length;
	}
}


void int_to_str(int n,u8 *str)  //整数转字符串
{
	int length=int_num_length(n); //获取数字长度
	str=str+int_num_length(n)-1;
	while (n)
	{
		*str=n%10 + '0'; //转化为字符
		n/=10;  //取位
		str--;
	}
}

void OLED_DisplayInt(u8 x,u8 y,u8 width,u8 height,int num) //在（x,y）点显示宽width，高height的数字num
{
	u8 num_str[30]; //按需求更改
	u8 *num_p=num_str;
	u8 i = 0;
	int_to_str(num,&num_str[0]); //数字转字符串
	num_p=&num_str[0]; //指针复位
	for(i=0;i<int_num_length(num);i++)
	{
		OLED_DisplayString(x+i*8,y,16,16,num_p);
		num_p++;
	}

}


void OLED_DisplayString(u8 x,u8 y,u8 width,u8 height,u8 *str)  //显示字符串函数，width=16,height=16
{
	u8 addr=0,i;
	u16 font=0;
	while(*str!='\0') //连续显示
	{
		//取模从空格开始的，计算下标
		//写8*16ASCII字符的上半部分
		if(*str >= ' '&& *str <= '~') //显示英文
		{
			addr=*str-' '; //取模从空格开始的，计算下标
			//写8*16ASCII字符的上半部分
			OLED_SetCursorAddrese(x,y); //设置光标的位置
			for(i=0;i<width/2;i++)      //横向写width列
			{
				 OLED_send_data(ASCII_8_16[addr][i]); 
			}
			//写8*16ASCII字符的下半部分
			OLED_SetCursorAddrese(x,y+1); //设置光标的位置
			for(i=0;i<width/2;i++)        //横向写width列
			{
				 OLED_send_data(ASCII_8_16[addr][i+width/2]); 
			}
			str++;//继续显示下一个字符
			x+=width/2; //在下一个位置继续显示数据			
		}
		else //显示中文
		{
			OLED_SetCursorAddrese(x,y); //设置光标的位置
			font=((*str)<<8)+(*(str+1));
			switch(font)
			{
				case 0xCEC2:addr=0;break;//温
				case 0xCAAA:addr=1;break;//湿
				case 0xB6C8:addr=2;break;//度
				case 0xCAFD:addr=3;break;//数
				case 0xBEDD:addr=4;break;//据
				case 0xB3C9:addr=5;break;//成
				case 0xB9A6:addr=6;break;//功
				case 0xB7A2:addr=7;break;//发
				case 0xCBCD:addr=8;break;//送
				case 0xC1AC:addr=9;break;//连
				case 0xBDD3:addr=10;break;//接
				case 0xB7FE:addr=11;break;//服
				case 0xCEF1:addr=12;break;//务
				case 0xC6F7:addr=13;break;//器
				case 0xC9CF:addr=14;break;//上
				case 0xCFC2:addr=15;break;//下
				case 0xCFDE:addr=16;break;//限
				case 0xD6B5:addr=17;break;//值
				case 0xC9E8:addr=18;break;//设
				case 0xD6C3:addr=19;break;//置
				case 0xB4AB:addr=20;break;//传
				case 0xD6DC:addr=21;break;//周
				case 0xC6DA:addr=22;break;//期
				case 0xBFD8:addr=23;break;//控
				case 0xD6C6:addr=24;break;//制
				case 0xB2CE:addr=25;break;//参
				case 0xD7DC:addr=26;break;//总
				case 0xB1ED:addr=27;break;//表
				case 0xB1B8:addr=28;break;//备
				case 0xCDF8:addr=29;break;//网
				case 0xC2E7:addr=30;break;//络
				case 0xD0C5:addr=31;break;//信
				case 0xCFA2:addr=32;break;//息
				case 0xCAA7:addr=33;break;//失
				case 0xB0DC:addr=34;break;//败
				case 0xB1A3:addr=35;break;//保
				case 0xB4E6:addr=36;break;//存
				case 0xB9FD:addr=37;break;//过
				case 0xB8DF:addr=38;break;//高
				case 0xA1E6:addr=39;break;//℃
				case 0xD0C7:addr=40;break;//星
				case 0xC8D5:addr=42;break;//日
				case 0xD2BB:addr=43;break;//一
				case 0xB6FE:addr=44;break;//二
				case 0xC8FD:addr=45;break;//三
				case 0xCBC4:addr=46;break;//四
				case 0xCEE5:addr=47;break;//五
				case 0xC1F9:addr=48;break;//六
				case 0xC4EA:addr=49;break;//年
				case 0xD4C2:addr=50;break;//月
				case 0xBCE4:addr=51;break;//间
				case 0xC3EB:addr=52;break;//秒
				case 0xD2F5:addr=53;break;//阴
				case 0xC7E7:addr=54;break;//晴
				case 0xB6E0:addr=55;break;//多
				case 0xD4C6:addr=56;break;//云
				case 0xB4F3:addr=57;break;//大
				case 0xD3EA:addr=58;break;//雨
				case 0xD0A1:addr=59;break;//小
				case 0xD1A9:addr=60;break;//雪
				case 0xCCA8:addr=61;break;//台
				case 0xB7E7:addr=62;break;//风
				case 0xB1A9:addr=63;break;//暴
				case 0xD4E7:addr=64;break;//早
				case 0xBAC3:addr=65;break;//好
				case 0xCDED:addr=66;break;//晚
				case 0xB0B2:addr=67;break;//安
				case 0xB7D6:addr=68;break;//分
				
				
				default : break;
			}
			for(i=0;i<width;i++) //横向写width列
			{
				//OLED_WR_Byte(ChineseFont_16_16[addr][i],OLED_DATA); 
				OLED_send_data(ChineseFont_16_16[addr][i]);
			}
			
			//写8*16ASCII字符的下半部分
			OLED_SetCursorAddrese(x,y+1); //设置光标的位置
			for(i=0;i<width;i++)          //横向写width列
			{
				 OLED_send_data(ChineseFont_16_16[addr][i+width]); 
			}
			str+=2;//继续显示下一个字符
			x+=width; //在下一个位置继续显示数据
		}
	}
}



	



