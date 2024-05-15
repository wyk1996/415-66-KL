/*====================================================================================================================================
//file:UART2.h
//name:mowenxing
//data:2021/03/23
//readme:
//===================================================================================================================================*/

#ifndef UART2_H
#define UART2_H

#include "main.h"


extern   void  UART2Dispinit(void);
extern   void  CLEARUART0FLAG(void);
extern   void  UART2SENDBUF(uint8 *buf,uint16  len);

#endif 


