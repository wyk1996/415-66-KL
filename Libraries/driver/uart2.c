/*====================================================================================================================================
//file:UART2.c
//name:mowenxing
//data:2021/03/23
//readme:
//===================================================================================================================================*/
#include "uart2.h"
#include "at32f4xx_rcc.h"

/*====================================================================================================================================
//name：mowenxing data：   2021/03/23  
//fun name：   UART2Dispinit
//fun work：     初始化UART2  显示串口
//in： 无
//out:   无     
//ret：   无
//ver： 无
//===================================================================================================================================*/
void  UART2Dispinit(void)
{
   GPIO_InitType GPIO_InitStructure;
  USART_InitType USART_InitStructure;
	NVIC_InitType NVIC_InitStructure;

	/* config USART2 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_USART2 , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_GPIOA, ENABLE);

	
	/* USART2 GPIO config */
	/* Configure USART2 Tx (PA.2) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pins = GPIO_Pins_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);    
	/* Configure USART2 Rx (PA.3) as input floating */
	GPIO_InitStructure.GPIO_Pins = GPIO_Pins_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* USART2 mode config */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	
	USART_INTConfig(USART2, USART_INT_RDNE, ENABLE);
	
	//中断优先级设置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//从优先级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure);
	USART_Cmd(USART2, ENABLE);
}


_UART_DISPRECV_CONTORL Uart2RecvControl = {0};
/*====================================================================================================================================
//name：mowenxing data：   2021/03/23  
//fun name：   USART2_IRQHandler
//fun work：     UART2中断
//in： 无
//out:   无     
//ret：   无
//ver： 无
//===================================================================================================================================*/
 void USART2_RecvDispose(uint8_t ch)
 {
	 
	 Uart2RecvControl.recv_buf[Uart2RecvControl.recv_index] = ch;
	 Uart2RecvControl.lastrecvtime = OSTimeGet(&timeerr);
	 Uart2RecvControl.recv_index++;
	 if(Uart2RecvControl.recv_index >= MSG_RECV_LEN)
	 {
		 Uart2RecvControl.recv_index = 0;
	 }
	 
 }
 
/*====================================================================================================================================
//name：mowenxing data：   2021/03/23  
//fun name：   USART2_IRQHandler
//fun work：     UART2中断
//in： 无
//out:   无     
//ret：   无
//ver： 无
//===================================================================================================================================*/
 void USART2_IRQHandler(void)
{
	uint8_t ch;
	
	if(USART_GetITStatus(USART2, USART_INT_RDNE) != RESET)
	{ 	
		USART_ClearITPendingBit(USART2, USART_INT_RDNE);
			__set_PRIMASK(1);
			ch = USART_ReceiveData(USART2);
			USART2_RecvDispose(ch);
			 __set_PRIMASK(0);
	} 
}

/*====================================================================================================================================
//name：mowenxing data：   2021/03/23 
//fun name：   UART2SendByte
//fun work：     UART2发送一个BYTE
//in： IN：发送的单字节
//out:   无     
//ret：   无
//ver： 无
//===================================================================================================================================*/
void UART2SendByte(unsigned char IN)
{      
	while((USART2->STS&0X40)==0);//循环发送,直到发送完毕   
	USART2->DT = IN;  
}
/*====================================================================================================================================
//name：mowenxing data：   2021/03/23  
//fun name：   UART2SENDBUF
//fun work：     UART2发送一个缓冲数据
//in： buf：数据包缓冲  len：数据包长度
//out:   无     
//ret：   无
//ver： 无
//===================================================================================================================================*/
void UART2SENDBUF(uint8 *buf,uint16  len){  
     uint16  i;
	 #warning "是否需要关总中断"
//	   __set_PRIMASK(1);  
	   for(i=0;i<len;i++){
			 UART2SendByte(buf[i]);
	   }
//		 __set_PRIMASK(0);
}


 
//END
