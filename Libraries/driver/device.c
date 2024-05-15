
#include "main.h"


/*====================================================================================================================================
//name:mowenxing data:   2021/03/23  
//fun name:  SystemReset    
//fun work:  ÏµÍ³¸´Î»
//in: 
//out:      
//ret: 
//ver: 
//===================================================================================================================================*/
void  SystemReset(void)
{
	__set_FAULTMASK(1);
  NVIC_SystemReset(); 
}

void  Mcu_Io_Init(void)
{
	GPIO_InitType GPIO_InitStructure;
	  /////////////////////////////////////////////////////////////
	RCC_APB2PeriphClockCmd( RCC_APB2PERIPH_AFIO, ENABLE );
	RCC_APB2PeriphClockCmd(	RCC_APB2PERIPH_GPIOF, ENABLE );
	RCC_APB2PeriphClockCmd(	RCC_APB2PERIPH_GPIOA, ENABLE );
	RCC_APB2PeriphClockCmd(	RCC_APB2PERIPH_GPIOC, ENABLE );
	RCC_APB2PeriphClockCmd(	RCC_APB2PERIPH_GPIOD, ENABLE );
	//RCC_APB2PeriphClockCmd(	RCC_APB2PERIPH_GPIOA, ENABLE );
	RCC_APB2PeriphClockCmd(	RCC_APB2PERIPH_GPIOB, ENABLE );
		GPIO_PinsRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	
	GPIO_InitStructure.GPIO_Pins =BUZZER;  
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);
	BUZZER_OFF; 
	
	GPIO_InitStructure.GPIO_Pins =IO_YX1;  
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_PU;
  GPIO_Init(IO_YX1_PORT, &GPIO_InitStructure);                                                


	
	GPIO_InitStructure.GPIO_Pins =KEY_OUT3;  
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(KEY_OUT3_PORT, &GPIO_InitStructure);
	KEY_OUT3_OFF;
	
	GPIO_InitStructure.GPIO_Pins =GPIO_Pins_3;  
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(KEY_OUT2_PORT, &GPIO_InitStructure);
	KEY_OUT2_OFF;
	
	GPIO_InitStructure.GPIO_Pins =GPIO_Pins_15;  
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(KEY_OUT1_PORT	, &GPIO_InitStructure);
	KEY_OUT1_OFF;//
	
	
	GPIO_InitStructure.GPIO_Pins =KEY_LED1;  
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(KEY_LED1_PORT	, &GPIO_InitStructure);
  
  	GPIO_InitStructure.GPIO_Pins =KEY_LED2;  
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(KEY_LED2_PORT	, &GPIO_InitStructure);
  
  	GPIO_InitStructure.GPIO_Pins =KEY_LED3;  
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(KEY_LED3_PORT	, &GPIO_InitStructure);
  
    	GPIO_InitStructure.GPIO_Pins =_4G_RET;  
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(_4G_RET_PORT	, &GPIO_InitStructure);
  
    	GPIO_InitStructure.GPIO_Pins =_4G_PWKEY;  
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(_4G_PWKEY_PORT	, &GPIO_InitStructure);
  
    	GPIO_InitStructure.GPIO_Pins =_4G_POWER;  
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(_4G_POWER_PORT	, &GPIO_InitStructure);
	
}
