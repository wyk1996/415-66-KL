/*====================================================================================================================================
//file:uart1.h
//name:mowenxing
//data:2021/03/23
//readme:
//===================================================================================================================================*/

#ifndef UART1_H
#define UART1_H

#include "main.h"



extern   void  UART1init(void);
extern   void  UART1SENDBUF(uint8 *buf,uint16  len);
void  UART14Ginit(void);
#endif 


