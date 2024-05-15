#include "timer.h"
#include "bsp.h"
#include "main.h"
#include "common.h"
#include "4GMain.h"

TMR_TimerBaseInitType  TMR_TMReBaseStructure;
TMR_OCInitType  TMR_OCInitStructure;
//uint16_t CCR1_Val = 333;
uint16_t CCR2_Val = 249;                             //占空比只修改此数值    249/666  占空比约为37%
uint16_t PrescalerValue = 0;


uint32_t countdf = 0;
//定时器3中断服务程序	 
void TMR3_GLOBAL_IRQHandler(void)
{ 	
	__NOP();
//	uint32_t	curtime;
//	if(TMR3->STS&0X0001)//溢出中断
//	{
//		countdf++;
//		curtime = OSTimeGet(&timeerr);
//		
//		if(Uart2RecvControl.recv_index > 0)   //说明有数据
//		{
//			if((curtime >Uart2RecvControl.lastrecvtime) ?((curtime - Uart2RecvControl.lastrecvtime ) > CM_TIME_50_MSEC) \
//				:((0xffffffff - Uart2RecvControl.lastrecvtime + curtime) > CM_TIME_50_MSEC) )
//			{
//				 __set_PRIMASK(1);
//				mq_service_dwinrecv_send_disp(0,0,Uart2RecvControl.recv_index,Uart2RecvControl.recv_buf);
//				Uart2RecvControl.recv_index = 0;
//				__set_PRIMASK(0);
//			}
//		}
//		if((APP_GetModuleConnectState(0) == STATE_OK) &&( NetConfigInfo[NET_YX_SELCT].NetNum  == 1)) //已经连接上后台了
//		{
//			#if(NET_YX_SELCT != XY_JM)
//			if(Uart1RecvControl.recv_index > 0)   //4G说明有数据
//			{
//				if((curtime >Uart1RecvControl.lastrecvtime) ?((curtime - Uart1RecvControl.lastrecvtime ) > CM_TIME_5_MSEC) \
//					:((0xffffffff - Uart1RecvControl.lastrecvtime + curtime) > CM_TIME_5_MSEC) )
//				{
//					 __set_PRIMASK(1);
//					mq_service_4GUart_send_recv(0,0,Uart1RecvControl.recv_index,Uart1RecvControl.recv_buf);
//					Uart1RecvControl.recv_index = 0;
//					__set_PRIMASK(0);
//				}
//			}
//			#endif
//			__NOP();
//		}
//		else
//		{
//			if(Uart1RecvControl.recv_index > 0)   //4G说明有数据
//			{
//				if((curtime >Uart1RecvControl.lastrecvtime) ?((curtime - Uart1RecvControl.lastrecvtime) > CM_TIME_5_MSEC) \
//					:((0xffffffff - Uart1RecvControl.lastrecvtime + curtime) > CM_TIME_5_MSEC) )
//				{
//					 __set_PRIMASK(1);
//					mq_service_4GUart_send_recv(0,0,Uart1RecvControl.recv_index,Uart1RecvControl.recv_buf);
//					Uart1RecvControl.recv_index = 0;
//					__set_PRIMASK(0);
//				}
//			}
//		}
//		//判断是否超时
//	}				   
//	TMR3->STS&=~(1<<0);//清除中断标志位 	   
}



void uart_recvdispose(void)
{ 	
	uint32_t	curtime;
	countdf++;
	curtime = OSTimeGet(&timeerr);
	
	if(Uart2RecvControl.recv_index > 0)   //说明有数据
	{
		if((curtime >=Uart2RecvControl.lastrecvtime) ?((curtime - Uart2RecvControl.lastrecvtime ) > CM_TIME_50_MSEC) \
			:((0xffffffff - Uart2RecvControl.lastrecvtime + curtime) > CM_TIME_50_MSEC) )
		{
			 __set_PRIMASK(1);
			mq_service_dwinrecv_send_disp(0,0,Uart2RecvControl.recv_index,Uart2RecvControl.recv_buf);
			Uart2RecvControl.recv_index = 0;
			__set_PRIMASK(0);
		}
	}
	if((APP_GetModuleConnectState(0) == STATE_OK) &&( NetConfigInfo[NET_YX_SELCT].NetNum  == 1)) //已经连接上后台了
	{
		#if(NET_YX_SELCT != XY_JM)
		if(Uart1RecvControl.recv_index > 0)   //4G说明有数据
		{
			if((curtime >Uart1RecvControl.lastrecvtime) ?((curtime - Uart1RecvControl.lastrecvtime ) > CM_TIME_5_MSEC) \
				:((0xffffffff - Uart1RecvControl.lastrecvtime + curtime) > CM_TIME_5_MSEC) )
			{
				 __set_PRIMASK(1);
				mq_service_4GUart_send_recv(0,0,Uart1RecvControl.recv_index,Uart1RecvControl.recv_buf);
				Uart1RecvControl.recv_index = 0;
				__set_PRIMASK(0);
			}
		}
		#endif
		__NOP();
	}
	else
	{
		if(Uart1RecvControl.recv_index > 0)   //4G说明有数据
		{
			if((curtime >=Uart1RecvControl.lastrecvtime) ?((curtime - Uart1RecvControl.lastrecvtime) > CM_TIME_5_MSEC) \
				:((0xffffffff - Uart1RecvControl.lastrecvtime + curtime) > CM_TIME_5_MSEC) )
			{
				 __set_PRIMASK(1);
				mq_service_4GUart_send_recv(0,0,Uart1RecvControl.recv_index,Uart1RecvControl.recv_buf);
				Uart1RecvControl.recv_index = 0;
				__set_PRIMASK(0);
			}
		}
	}
}
//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
//时间中断：Tout(溢出时间,单位s) = ((arr+1)*psc+1)/Tclk
//Tclk(TIM3的时钟输出频率，单位Mhz)
void TIM3_Int_Init(u16 arr,u16 psc)
{
		NVIC_InitType NVIC_InitStructure;
	RCC->APB1EN|=1<<1;	//TIM3时钟使能    
 	TMR3->AR=arr;  	//设定计数器自动重装值//刚好1ms    
	TMR3->DIV=psc;  	//预分频器7200,得到10Khz的计数时钟		  
	TMR3->DIE|=1<<0;   //允许更新中断	  
	TMR3->CTRL1|=0x01;    //使能定时器3
	
  NVIC_InitStructure.NVIC_IRQChannel = BSP_INT_ID_TIM3;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//?????
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//????
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure);						 
}




void TIM2_PWM_Conrtol(uint16_t Val)
{ 
	GPIO_InitType GPIO_InitStructure;
	NVIC_InitType NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_TMR2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_GPIOA | RCC_APB2PERIPH_AFIO, ENABLE);
	
	
	 GPIO_InitStructure.GPIO_Pins = GPIO_Pins_1 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		

	 /* -----------------------------------------------------------------------
    TMR2 Configuration: generate 4 PWM signals with 4 different duty cycles:
    The TMR2CLK frequency is set to SystemCoreClock (Hz), to get TMR2 counter
    clock at 24 MHz the Prescaler is computed as following:
     - Prescaler = (TMR2CLK / TMR2 counter clock) - 1

    The TMR2 is running at 36 KHz: TMR2 Frequency = TMR2 counter clock/(ARR + 1)
                                                  = 24 MHz / 666 = 36 KHz
    TMR2 Channel1 duty cycle = (TMR2_CCR1/ TMR2_ARR)* 100 = 50%
    TMR2 Channel2 duty cycle = (TMR2_CCR2/ TMR2_ARR)* 100 = 37.5%
    TMR2 Channel3 duty cycle = (TMR2_CCR3/ TMR2_ARR)* 100 = 25%
    TMR2 Channel4 duty cycle = (TMR2_CCR4/ TMR2_ARR)* 100 = 12.5%
  ----------------------------------------------------------------------- */
  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) (SystemCoreClock / 666666) - 1;               //此处修改频率1(khz)= 24000000/36(khz)=666666
  /* TMRe base configuration */
  TMR_TimeBaseStructInit(&TMR_TMReBaseStructure);
  TMR_TMReBaseStructure.TMR_Period = 665;
  TMR_TMReBaseStructure.TMR_DIV = PrescalerValue;
  TMR_TMReBaseStructure.TMR_ClockDivision = 0;
  TMR_TMReBaseStructure.TMR_CounterMode = TMR_CounterDIR_Up;

  TMR_TimeBaseInit(TMR2, &TMR_TMReBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
  TMR_OCStructInit(&TMR_OCInitStructure);
  TMR_OCInitStructure.TMR_OCMode = TMR_OCMode_PWM1;
 // TMR_OCInitStructure.TMR_OutputState = TMR_OutputState_Enable;
  //TMR_OCInitStructure.TMR_Pulse = CCR1_Val;
  //TMR_OCInitStructure.TMR_OCPolarity = TMR_OCPolarity_High;

  //TMR_OC1Init(TMR2, &TMR_OCInitStructure);

 // TMR_OC1PreloadConfig(TMR2, TMR_OCPreload_Enable);

  /* PWM1 Mode configuration: Channel2 */
  TMR_OCInitStructure.TMR_OutputState = TMR_OutputState_Enable;
  TMR_OCInitStructure.TMR_Pulse = Val;

  TMR_OC2Init(TMR2, &TMR_OCInitStructure);

  TMR_OC2PreloadConfig(TMR2, TMR_OCPreload_Enable);

  /* PWM1 Mode configuration: Channel3 */
 // TMR_OCInitStructure.TMR_OutputState = TMR_OutputState_Enable;
 // TMR_OCInitStructure.TMR_Pulse = CCR3_Val;

 // TMR_OC3Init(TMR2, &TMR_OCInitStructure);

 // TMR_OC3PreloadConfig(TMR2, TMR_OCPreload_Enable);

  /* PWM1 Mode configuration: Channel4 */
//  TMR_OCInitStructure.TMR_OutputState = TMR_OutputState_Enable;
 // TMR_OCInitStructure.TMR_Pulse = CCR4_Val;

 // TMR_OC4Init(TMR2, &TMR_OCInitStructure);

  //TMR_OC4PreloadConfig(TMR2, TMR_OCPreload_Enable);

 // TMR_ARPreloadConfig(TMR2, ENABLE);

  /* TMR2 enable counter */
  TMR_Cmd(TMR2, ENABLE);
}










