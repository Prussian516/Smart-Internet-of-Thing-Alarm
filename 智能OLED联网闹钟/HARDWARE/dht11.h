#ifndef _DHT11__H_
#define _DHT11__H_
#include "sys.h"
#define DHT_Out PCout(0)
#define DHT_In  PCin(0)

#define uchar unsigned char 
#define uint  unsigned int

unsigned char StartDHT11(void);
void DHT_PortIN(void);
void DHT_PortOUT(void);
u8 ReadDHT11(void);
void com(void);
//**********************************

//**********************************

extern u8 tdata[4];
extern u8 sbuf;
extern u8 check;

#endif
