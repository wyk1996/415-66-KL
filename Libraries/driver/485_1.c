#include "at32_board.h"
#include "485_1.h"
#include "delay.h"



//-------------------------------------------------
void Uart1_Init(uint32_t BaudRate, uint16_t DataBits, uint16_t Parity, uint16_t StopBits)
{
    GPIO_InitType GPIO_InitStructure;
    USART_InitType USART_InitStructure;
    NVIC_InitType NVIC_InitStructure;

		RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_GPIOB, ENABLE);	//使能USART3，GPIOC时钟

	 GPIO_InitStructure.GPIO_Pins = GPIO_Pins_2;			
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
    GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure); 

    //USART3_485TX    GPIOB.10
    GPIO_InitStructure.GPIO_Pins = GPIO_Pins_10; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //USART3_485RX	  GPIOB.11
    GPIO_InitStructure.GPIO_Pins = GPIO_Pins_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    RCC_APB2PeriphResetCmd(RCC_APB1PERIPH_USART3,ENABLE);//复位串口1
    RCC_APB2PeriphResetCmd(RCC_APB1PERIPH_USART3,DISABLE);//停止复位

    //USART 初始化设置
    USART_InitStructure.USART_BaudRate = BaudRate;//串口波特率
    USART_InitStructure.USART_WordLength = DataBits;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = StopBits;//一个停止位
    USART_InitStructure.USART_Parity = Parity;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
    USART_Init(USART3, &USART_InitStructure);//初始化串口3
    
  	USART_INTConfig(USART3, USART_INT_RDNE, ENABLE);
    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1 ;  //抢占优先级2
    //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		    //子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			    //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	//	USART_INTConfig(USART6, USART_INT_RDNE, ENABLE);
  //  BSP_IntVectSet(BSP_INT_ID_USART3, BSP_IntHandlerUSART6); //????1?????,?????
  // BSP_IntEn(BSP_INT_ID_USART3);

    USART_Cmd(USART3, ENABLE);//使能串口3

}



/********************************************************************************
* 函数名 : Uart3_SendData
* 功  能 : 串口发送数据
* 说  明 : none
* 入  参 : none
* 返  回 : none
* 修  改 : none                       时  间 : none
********************************************************************************/
void Uart1_SendData(uint8_t *buf,uint8_t len)
{
	uint8_t i=0;

	GPIO_SetBits(GPIOB, GPIO_Pins_2);    //设置为发送模式
	delay_u(5);
  	for(i=0;i<len;i++)		//循环发送数据
	{
        while(USART_GetFlagStatus(USART3, USART_FLAG_TRAC)==RESET); //等待发送结束
        USART_SendData(USART3, buf[i]); //发送数据
	}
	while(USART_GetFlagStatus(USART3, USART_FLAG_TRAC)==RESET){} //等待发送结束
	GPIO_ResetBits(GPIOB, GPIO_Pins_2);    //设置为发送模式

}


void USART3_IRQHandler(void)
{
	uint8_t ch;
	
	if(USART_GetITStatus(USART3, USART_INT_RDNE) != RESET)
	{ 	
		USART_ClearITPendingBit(USART3, USART_INT_RDNE);
		ch = USART_ReceiveData(USART3);
		#warning "YXY数据处理"	
	} 
}

