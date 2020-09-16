#include "0.96OLED4PINI2C.h"
#include "word.h"
/*  0.96OLED4PIN ��������  *
****************************
*	auchor:yetian
*	time:2020/9/10
*	
*
*
*****************************/



/*******SPI���ŷ��� ����OLED��������ʵ������޸�**************/

#define IIC_SCK_PIN 5
#define IIC_SDA_PIN 6

#define IIC_SCK_0 GPIOB->BRR = 0X0020  //����sck�ӿ� ������PB5 ����
#define IIC_SCK_1 GPIOB->BSRR = 0X0020  //��λ
#define IIC_SDA_0 GPIOB->BRR = 0X0040  //  ����SDA�ӿڵ�PB6
#define IIC_SDA_1 GPIOB->BSRR = 0X0040 //

const unsigned char *point;
unsigned char ACK = 0;
const unsigned char OLED_init_cmd[25]=  //oled��ʼ����������
{
  /*0xae,0X00,0X10,0x40,0X81,0XCF,0xff,0xa1,0xa4,
  0xA6,0xc8,0xa8,0x3F,0xd5,0x80,0xd3,0x00,0XDA,0X12,
  0x8d,0x14,0xdb,0x40,0X20,0X02,0xd9,0xf1,0xAF*/
       0xAE,//�ر���ʾ
       0xD5,//����ʱ�ӷ�Ƶ����,��Ƶ��
       0x80,  //[3:0],��Ƶ����;[7:4],��Ƶ��

       0xA8,//��������·��
       0X3F,//Ĭ��0X3F(1/64)
       0xD3,//������ʾƫ��
       0X00,//Ĭ��Ϊ0
       0x40,//������ʾ��ʼ�� [5:0],����.                              
       0x8D,//��ɱ�����
       0x14,//bit2������/�ر�
       0x20,//�����ڴ��ַģʽ
       0x02,//[1:0],00���е�ַģʽ;01���е�ַģʽ;10,ҳ��ַģʽ;Ĭ��10;
       0xA1,//���ض�������,bit0:0,0->0;1,0->127;
       0xC8,//����COMɨ�跽��;bit3:0,��ͨģʽ;1,�ض���ģʽ COM[N-1]->COM0;N:����·��
       0xDA,//����COMӲ����������
       0x12,//[5:4]����            
       0x81,//�Աȶ�����
       0xEF,//1~255;Ĭ��0X7F (��������,Խ��Խ��)
       0xD9,//����Ԥ�������
       0xf1,//[3:0],PHASE 1;[7:4],PHASE 2;
       0xDB,//����VCOMH ��ѹ����
       0x30,//[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;
       0xA4,//ȫ����ʾ����;bit0:1,����;0,�ر�;(����/����)
       0xA6,//������ʾ��ʽ;bit0:1,������ʾ;0,������ʾ        
       0xAF,//������ʾ     
};

unsigned char oled_show_tab[8][128];

/*********************ʹ��IO��**********************************/ 
void IO_init(void)
{
	RCC->APB2ENR|=1<<3;    //ʹ��PORTBʱ�� 
	GPIOB->CRL&=0X00FFFFFF;				//��B56������Ϊͨ�ÿ�©���,���50MH
	GPIOB->CRL|=0X07700000;				//B7����Ϊͨ���������,���50MH
	GPIOB->ODR=0XFFFF;
}


/*********************ʱ�ӳ�ʼ��******************************/
void SYS_init(unsigned char PLL)
{
	
		 
	RCC->APB1RSTR = 0x00000000;//��λ����			 
	RCC->APB2RSTR = 0x00000000; 	  
  	RCC->AHBENR = 0x00000014;  //˯��ģʽ�����SRAMʱ��ʹ��.�����ر�.	  
  	RCC->APB2ENR = 0x00000000; //����ʱ�ӹر�.			   
  	RCC->APB1ENR = 0x00000000;   
	RCC->CR |= 0x00000001;     //ʹ���ڲ�����ʱ��HSION
	
	RCC->CFGR &= 0xF8FF0000;   //��λSW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]					 
	RCC->CR &= 0xFEF6FFFF;     //��λHSEON,CSSON,PLLON
	RCC->CR &= 0xFFFBFFFF;     //��λHSEBYP	   	  
	RCC->CFGR &= 0xFF80FFFF;   //��λPLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE
	while(((RCC->CFGR>>2)& 0x03 )!=0x00); 	
	RCC->CIR = 0x00000000;     //�ر������ж�		 
	//����������				  
  
//	SCB->VTOR = 0x08000000|(0x0 & (u32)0x1FFFFF80);//����NVIC��������ƫ�ƼĴ���
	
 	RCC->CR|=0x00010000;  //�ⲿ����ʱ��ʹ��HSEON
	while(((RCC->CR>>17)&0x00000001)==0);//�ȴ��ⲿʱ�Ӿ���
	RCC->CFGR=0X00000400; //APB1=DIV2;APB2=DIV1;AHB=DIV1;
	PLL-=2;//����2����λ
	RCC->CFGR|=PLL<<18;   //����PLLֵ 2~16
	RCC->CFGR|=1<<16;	  //PLLSRC ON 
	

	RCC->CR|=0x01000000;  //PLLON
	while(!(RCC->CR>>25));//�ȴ�PLL����
	RCC->CFGR|=0x00000002;//PLL��Ϊϵͳʱ��	 
	while(((RCC->CFGR>>2)&0x03)!=0x02);   //�ȴ�PLL��Ϊϵͳʱ�����óɹ�
	
	
}






/**************************IICģ�鷢�ͺ���************************************************

 *************************************************************************/
//д��  ���SDA���ߣ��Եȴ����豸����Ӧ��
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
//								����ҪӦ��
//	if (READ_SDA==0)
//		ACK = 1;
//	else ACK =0;
	delay_us(1);
	IIC_SCK_0;
	delay_us(1);
	

}
//�����ź�
//SCL�ڸߵ�ƽ�ڼ䣬SDA�ɸߵ�ƽ��͵�ƽ�ı仯����Ϊ�����ź�
void IIC_start()
{
	IIC_SDA_1;
	delay_us(1);
	IIC_SCK_1;
	delay_us(1);				   //���в��������ͷ�SCL	
	IIC_SDA_0;
	delay_us(3);
	IIC_SCK_0;
	
        IIC_write(0x78);
        
}

//ֹͣ�ź�
//SCL�ڸߵ�ƽ�ڼ䣬SDA�ɵ͵�ƽ��ߵ�ƽ�ı仯����Ϊֹͣ�ź�
void IIC_stop()
{
	IIC_SDA_0;
	delay_us(1);
	IIC_SCK_1;
	delay_us(3);
	IIC_SDA_1;
	
}




/*****OLED���ܺ���********/
void OLED_send_cmd(unsigned char o_command)  //��������
{
	IIC_start();
	IIC_write(0x00);
	IIC_write(o_command);
	IIC_stop();
}

void OLED_send_data(unsigned char o_data) //��������
{
	IIC_start();
	IIC_write(0x40);
	IIC_write(o_data);
	IIC_stop();
}

void Column_set(unsigned char column)  //�����е�ַ
{
	OLED_send_cmd(0x10|(column>>4));  //�����е�ַ��λ
	OLED_send_cmd(0x00|(column&0x0f));  //�����е�ַ��λ
}

void Page_set(unsigned char page)
{
	OLED_send_cmd(0xb0+page);
}

void OLED_Refresh_Gram(void)  //����Gram�Դ浽OLED��Ļ
{
	
	u8 page,list;  //����ҳ��ַ���е�ַ
	for(page=0;page<8;page++)
	{
		OLED_send_cmd(0xb0+page); //����ҳ��ַ��0~7��
		OLED_send_cmd(0x00);      //������ʾλ�èD�е͵�ַ
		OLED_send_cmd(0x10);      //������ʾλ�èD�иߵ�ַ 
		for(list=0;list<128;list++)
		{
			OLED_send_data(oled_show_tab[page][list]);
		}
	}
	//memset(oled_show_tab,0,sizeof(oled_show_tab));	/*����Դ�����*/

}



void OLED_clear(void)  //��������
{
	u8 i,n;
	for(i=0;i<8;i++)
	{
		OLED_send_cmd(0xb0+i);
		OLED_send_cmd(0x00);  //������ʾλ�èD�е͵�ַ
		OLED_send_cmd(0x10);  //������ʾλ�èD�иߵ�ַ 
		for(n=0;n<128;n++)
		{
			OLED_send_data(0x00);
		}

	}
}

void OLED_full(void)  //��亯��������������Ļ
{
	u8 i,n;
	for(i=0;i<8;i++)
	{
		OLED_send_cmd(i);
		OLED_send_cmd(0x00);  //������ʾλ�èD�е͵�ַ
		OLED_send_cmd(0x10);  //������ʾλ�èD�иߵ�ַ 
		for(n=0;n<128;n++)
		{
			OLED_send_data(0xFF);
		}

	}
}


void OLED_init(void)  //OLED��ʼ������
{

	unsigned char i;
	for(i=0;i<25;i++)
	{
		OLED_send_cmd(OLED_init_cmd[i]);
	}
	OLED_clear();
	OLED_SetCursorAddrese(0,0);
	
}



/*��������:����OLED��ʾ */
void OLED_Display_On(void)
{
	OLED_send_cmd(0X8D);  //SET DCDC������õ�ɱã�
	OLED_send_cmd(0X14);  //DCDC ON ��������ɱã�
	OLED_send_cmd(0XAF);  //DISPLAY ON��OLED���ѣ�
}
/*��������:�ر�OLED��ʾ*/ 
 
void OLED_Display_Off(void)
{
	OLED_send_cmd(0X8D);  //SET DCDC���� �����õ�ɱã�
	OLED_send_cmd(0X10);  //DCDC OFF ���رյ�ɱã�
	OLED_send_cmd(0XAE);  //DISPLAY OFF ��OLED���ߣ�
}
 




/*
�������ܣ����Դ������ϻ�һ����
����������x��yΪ��ĺ�������   cΪ����������1��0��
������Χ��x 0~128  y 0~8 
ÿһ�������� ��λ��ǰ����λ�ں�
*/
void OLED_Draw_Point(u8 x,u8 y,u8 c)
{
	y &=0x3F ; //��֤yֵ������63
	x &= 0x7F;  //��֤xֵ������127
	if(c)
	{
		oled_show_tab[x][7-y/8] |= 0x1 << (7-y%8); //�����õ�
	}
	else
	{
		oled_show_tab[x][7-y/8] |= ~(0x1 << (7-y%8));  //Ϩ��õ�
	}
	
}

//��䣨x1��y1)��(x2,y1+16)����
//ȷ��x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63	 	 
//dot:0,���;1,���	  
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
��������: ���ù��λ��
��������: x�е���ʼλ��(0~127)
				  yҳ����ʼλ��(0~7)
����: 0x8  ��4λ0000   ��4λ1000 
*/
void OLED_SetCursorAddrese(u8 x,u8 y)
{
		OLED_send_cmd(0xB0+y); 					//����ҳ��ַ
	  //��6����ʾ����  6���ֳ�2��4λ(��λ�͵�λ)��
		OLED_send_cmd((x&0xF0)>>4|0x10);//�����и���ʼ��ַ(���ֽ�)
		OLED_send_cmd((x&0x0F)|0x00);   //�����е���ʼ��ַ(���ֽ�)			
}



void float_to_str(float n,char *reChar,int zsize,int xsize,int flag)//���ܽ�������nת���ַ��������浽��reChar��ַ��ͷ���ַ������У���ʵ�ֲ��룩
//flag=0:ֱ��ת����������ָ����ַ    flag=1����ת���ĸ��������뵽ָ����ַλ��
//zsize��n�������������ܵ�λ����   xsize��nС�����������ܵ�λ��+1(����С����)��
//reChar�����ڷ��ش������ַ���
{
 
    int z,x,i=0,j=0;
	//char a[1+zsize+xsize];
	char a[10];
	
    n=n+0.00001;//+0.00001���⸡�������ȶ�ʧ,�ɸ����㴫�������ʵ������λ���޸ģ���Ҫ�������ͷ�Χ
    z=(int)n;
    x=(int)((n-z)*10);//ȡС�����֣����1λС�� *10  2λ*100 3λ*1000 ��Ҫ�����޸� ��Ҳ�����Լ�д10�η���������xsize���� 
 
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


u8 int_num_length(int n)  //���������ĳ���
{
	int length=0;
	if(n>0)  //n>0ʱ
	{
		while(n)
		{
			length++;
			n=n/10;
		}	
		return length;
	}
	else if(n==0) //��n==0
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


void int_to_str(int n,u8 *str)  //����ת�ַ���
{
	int length=int_num_length(n); //��ȡ���ֳ���
	str=str+int_num_length(n)-1;
	while (n)
	{
		*str=n%10 + '0'; //ת��Ϊ�ַ�
		n/=10;  //ȡλ
		str--;
	}
}

void OLED_DisplayInt(u8 x,u8 y,u8 width,u8 height,int num) //�ڣ�x,y������ʾ��width����height������num
{
	u8 num_str[30]; //���������
	u8 *num_p=num_str;
	u8 i = 0;
	int_to_str(num,&num_str[0]); //����ת�ַ���
	num_p=&num_str[0]; //ָ�븴λ
	for(i=0;i<int_num_length(num);i++)
	{
		OLED_DisplayString(x+i*8,y,16,16,num_p);
		num_p++;
	}

}


void OLED_DisplayString(u8 x,u8 y,u8 width,u8 height,u8 *str)  //��ʾ�ַ���������width=16,height=16
{
	u8 addr=0,i;
	u16 font=0;
	while(*str!='\0') //������ʾ
	{
		//ȡģ�ӿո�ʼ�ģ������±�
		//д8*16ASCII�ַ����ϰ벿��
		if(*str >= ' '&& *str <= '~') //��ʾӢ��
		{
			addr=*str-' '; //ȡģ�ӿո�ʼ�ģ������±�
			//д8*16ASCII�ַ����ϰ벿��
			OLED_SetCursorAddrese(x,y); //���ù���λ��
			for(i=0;i<width/2;i++)      //����дwidth��
			{
				 OLED_send_data(ASCII_8_16[addr][i]); 
			}
			//д8*16ASCII�ַ����°벿��
			OLED_SetCursorAddrese(x,y+1); //���ù���λ��
			for(i=0;i<width/2;i++)        //����дwidth��
			{
				 OLED_send_data(ASCII_8_16[addr][i+width/2]); 
			}
			str++;//������ʾ��һ���ַ�
			x+=width/2; //����һ��λ�ü�����ʾ����			
		}
		else //��ʾ����
		{
			OLED_SetCursorAddrese(x,y); //���ù���λ��
			font=((*str)<<8)+(*(str+1));
			switch(font)
			{
				case 0xCEC2:addr=0;break;//��
				case 0xCAAA:addr=1;break;//ʪ
				case 0xB6C8:addr=2;break;//��
				case 0xCAFD:addr=3;break;//��
				case 0xBEDD:addr=4;break;//��
				case 0xB3C9:addr=5;break;//��
				case 0xB9A6:addr=6;break;//��
				case 0xB7A2:addr=7;break;//��
				case 0xCBCD:addr=8;break;//��
				case 0xC1AC:addr=9;break;//��
				case 0xBDD3:addr=10;break;//��
				case 0xB7FE:addr=11;break;//��
				case 0xCEF1:addr=12;break;//��
				case 0xC6F7:addr=13;break;//��
				case 0xC9CF:addr=14;break;//��
				case 0xCFC2:addr=15;break;//��
				case 0xCFDE:addr=16;break;//��
				case 0xD6B5:addr=17;break;//ֵ
				case 0xC9E8:addr=18;break;//��
				case 0xD6C3:addr=19;break;//��
				case 0xB4AB:addr=20;break;//��
				case 0xD6DC:addr=21;break;//��
				case 0xC6DA:addr=22;break;//��
				case 0xBFD8:addr=23;break;//��
				case 0xD6C6:addr=24;break;//��
				case 0xB2CE:addr=25;break;//��
				case 0xD7DC:addr=26;break;//��
				case 0xB1ED:addr=27;break;//��
				case 0xB1B8:addr=28;break;//��
				case 0xCDF8:addr=29;break;//��
				case 0xC2E7:addr=30;break;//��
				case 0xD0C5:addr=31;break;//��
				case 0xCFA2:addr=32;break;//Ϣ
				case 0xCAA7:addr=33;break;//ʧ
				case 0xB0DC:addr=34;break;//��
				case 0xB1A3:addr=35;break;//��
				case 0xB4E6:addr=36;break;//��
				case 0xB9FD:addr=37;break;//��
				case 0xB8DF:addr=38;break;//��
				case 0xA1E6:addr=39;break;//��
				case 0xD0C7:addr=40;break;//��
				case 0xC8D5:addr=42;break;//��
				case 0xD2BB:addr=43;break;//һ
				case 0xB6FE:addr=44;break;//��
				case 0xC8FD:addr=45;break;//��
				case 0xCBC4:addr=46;break;//��
				case 0xCEE5:addr=47;break;//��
				case 0xC1F9:addr=48;break;//��
				case 0xC4EA:addr=49;break;//��
				case 0xD4C2:addr=50;break;//��
				case 0xBCE4:addr=51;break;//��
				case 0xC3EB:addr=52;break;//��
				case 0xD2F5:addr=53;break;//��
				case 0xC7E7:addr=54;break;//��
				case 0xB6E0:addr=55;break;//��
				case 0xD4C6:addr=56;break;//��
				case 0xB4F3:addr=57;break;//��
				case 0xD3EA:addr=58;break;//��
				case 0xD0A1:addr=59;break;//С
				case 0xD1A9:addr=60;break;//ѩ
				case 0xCCA8:addr=61;break;//̨
				case 0xB7E7:addr=62;break;//��
				case 0xB1A9:addr=63;break;//��
				case 0xD4E7:addr=64;break;//��
				case 0xBAC3:addr=65;break;//��
				case 0xCDED:addr=66;break;//��
				case 0xB0B2:addr=67;break;//��
				case 0xB7D6:addr=68;break;//��
				
				
				default : break;
			}
			for(i=0;i<width;i++) //����дwidth��
			{
				//OLED_WR_Byte(ChineseFont_16_16[addr][i],OLED_DATA); 
				OLED_send_data(ChineseFont_16_16[addr][i]);
			}
			
			//д8*16ASCII�ַ����°벿��
			OLED_SetCursorAddrese(x,y+1); //���ù���λ��
			for(i=0;i<width;i++)          //����дwidth��
			{
				 OLED_send_data(ChineseFont_16_16[addr][i+width]); 
			}
			str+=2;//������ʾ��һ���ַ�
			x+=width; //����һ��λ�ü�����ʾ����
		}
	}
}



	



