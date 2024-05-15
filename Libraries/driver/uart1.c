/*====================================================================================================================================
//file:UART2.c
//name:mowenxing
//data:2021/03/23
//readme:
//===================================================================================================================================*/
#include "uart1.h"
#include "at32f4xx_rcc.h"
#include "4GMain.h"

/*====================================================================================================================================
//name：mowenxing data：   2021/03/23  
//fun name：   UART14Ginit
//fun work：    4G串口初始化
//in： 无
//out:   无     
//ret：   无
//ver： 无
//===================================================================================================================================*/
void  UART14Ginit(void)
{
   GPIO_InitType GPIO_InitStructure;
  USART_InitType USART_InitStructure;
	NVIC_InitType NVIC_InitStructure;

	/* config USART2 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_USART1 , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_GPIOA, ENABLE);

	
	/* USART2 GPIO config */
	/* Configure USART2 Tx (PA.2) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pins = GPIO_Pins_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);    
	/* Configure USART2 Rx (PA.3) as input floating */
	GPIO_InitStructure.GPIO_Pins = GPIO_Pins_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* USART2 mode config */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	
	USART_INTConfig(USART1, USART_INT_RDNE, ENABLE);
	
	//中断优先级设置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//从优先级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
}


_UART_4GRECV_CONTORL Uart1RecvControl = {0};

/**
 * @brief 
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
static uint8_t  get_xor_check_sum(uint8_t *pucData, int iDataLen)
{
    uint8_t ucXor = 0; 
	int i = 0; 
	
    for(i = 0; i < iDataLen; i++)
    {
        ucXor  ^= pucData[i]; 
    }
	
    return ucXor;
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
 void USART1_RecvDispose(uint8_t ch)
 {
	if((APP_GetModuleConnectState(0) == STATE_OK)   &&( NetConfigInfo[NET_YX_SELCT].NetNum  == 1) ) //已经连接上后台了
	{
		#if(NET_YX_SELCT != XY_JM)
		 Uart1RecvControl.recv_buf[Uart1RecvControl.recv_index] = ch;
		 Uart1RecvControl.lastrecvtime = OSTimeGet(&timeerr);
		 Uart1RecvControl.recv_index++;
		 if(Uart1RecvControl.recv_index >= URART_4GRECV_LEN)
		 {
			 Uart1RecvControl.recv_index = 0;
		 }
		#else
		//66数据接收有问题，临时判断帧头帧尾
		Uart1RecvControl.recv_buf[Uart1RecvControl.recv_index] = ch;
		Uart1RecvControl.lastrecvtime = OSTimeGet(&timeerr);
		Uart1RecvControl.recv_index++;
		if(Uart1RecvControl.recv_buf[0] != 0x68)
		{
			 Uart1RecvControl.recv_index = 0;
			return;
		}
		if(Uart1RecvControl.recv_index > 1)
		{
			if(Uart1RecvControl.recv_buf[1] != 0x03)
			{
				Uart1RecvControl.recv_index = 0;
				return;
			}
		}
		if(Uart1RecvControl.recv_index >= 10)
		{
			if(Uart1RecvControl.recv_buf[Uart1RecvControl.recv_index - 1] == 0x48)  //结尾
			{
				if(Uart1RecvControl.recv_buf[Uart1RecvControl.recv_index - 2] == get_xor_check_sum(Uart1RecvControl.recv_buf,Uart1RecvControl.recv_index - 2))
				{
					mq_service_4GUart_send_recv(0,0,Uart1RecvControl.recv_index,Uart1RecvControl.recv_buf);
					memset(Uart1RecvControl.recv_buf,0,sizeof(Uart1RecvControl.recv_buf));
					Uart1RecvControl.recv_index = 0;
				}
			}
			
		}
			
		
		 if(Uart1RecvControl.recv_index >= 300)
		 {
			 Uart1RecvControl.recv_index = 0;
		 }
		#endif
	}
	else
	{
		 Uart1RecvControl.recv_buf[Uart1RecvControl.recv_index] = ch;
		 Uart1RecvControl.lastrecvtime = OSTimeGet(&timeerr);
		 Uart1RecvControl.recv_index++;
		 if(Uart1RecvControl.recv_index >= URART_4GRECV_LEN)
		 {
			 Uart1RecvControl.recv_index = 0;
		 }
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
 void USART1_IRQHandler(void)
{
	uint8_t ch;
	
	if(USART_GetITStatus(USART1, USART_INT_RDNE) != RESET)
	{ 	
		USART_ClearITPendingBit(USART1, USART_INT_RDNE);
			__set_PRIMASK(1);
			ch = USART_ReceiveData(USART1);
			USART1_RecvDispose(ch);
			 __set_PRIMASK(0);
	} 
}

/*====================================================================================================================================
//name：mowenxing data：   2021/03/23 
//fun name：   UART1SendByte
//fun work：     UART2发送一个BYTE
//in： IN：发送的单字节
//out:   无     
//ret：   无
//ver： 无
//===================================================================================================================================*/
void UART1SendByte(unsigned char IN)
{      
	while((USART1->STS&0X40)==0);//循环发送,直到发送完毕   
	USART1->DT = IN;  
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
void UART1SENDBUF(uint8 *buf,uint16  len){  
     uint16  i;
	 #warning "是否需要关总中断"
//	   __set_PRIMASK(1);  
	   for(i=0;i<len;i++){
			 UART1SendByte(buf[i]);
	   }
//		 __set_PRIMASK(0);
}


 
//END
