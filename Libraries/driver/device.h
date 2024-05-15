#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "at32f4xx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Types.h"


//=================================================
// Port define 
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO???,?????IO?!
//??n????16!
//IO?????
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO???,?????IO?!
//??n????16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //?? 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //?? 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //?? 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //?? 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //?? 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //?? 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //?? 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //?? 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //?? 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //??

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //?? 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //??

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //?? 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //??

// Port A
#define PA0															GPIO_Pins_0	// out
#define PA1															GPIO_Pins_1	// out
#define PA2															GPIO_Pins_2	// 
#define PA3															GPIO_Pins_3	// 
#define PA4															GPIO_Pins_4	// 
#define PA5															GPIO_Pins_5	// 
#define PA6															GPIO_Pins_6	// 
#define PA7															GPIO_Pins_7	// 
#define PA8															GPIO_Pins_8	// 
#define PA9															GPIO_Pins_9	// 
#define PA10														GPIO_Pins_10	// 
#define PA11														GPIO_Pins_11	// 
#define PA12														GPIO_Pins_12	// 
#define PA13														GPIO_Pins_13	// 
#define PA14														GPIO_Pins_14	// 
#define PA15														GPIO_Pins_15	// 

//Port B
#define PB0                             GPIO_Pins_0
#define PB1															GPIO_Pins_1	// 
#define PB2															GPIO_Pins_2	// 
#define PB3															GPIO_Pins_3	// 
#define PB4															GPIO_Pins_4	// 
#define PB5															GPIO_Pins_5	// 
#define PB6															GPIO_Pins_6	// 
#define PB7                             GPIO_Pins_7
#define PB8                             GPIO_Pins_8
#define PB9                             GPIO_Pins_9
#define PB10                            GPIO_Pins_10
#define PB11                            GPIO_Pins_11
#define PB12                            GPIO_Pins_12
#define PB13                            GPIO_Pins_13
#define PB14                            GPIO_Pins_14
#define PB15                            GPIO_Pins_15

// Port C
#define PC0                             GPIO_Pins_0
#define PC1															GPIO_Pins_1	// 
#define PC2															GPIO_Pins_2	// 
#define PC3															GPIO_Pins_3	// 
#define PC4															GPIO_Pins_4	// 
#define PC5															GPIO_Pins_5	// 
#define PC6															GPIO_Pins_6	// 
#define PC7                             GPIO_Pins_7
#define PC8                             GPIO_Pins_8
#define PC9                             GPIO_Pins_9
#define PC10                            GPIO_Pins_10
#define PC11                            GPIO_Pins_11
#define PC12                            GPIO_Pins_12
#define PC13                            GPIO_Pins_13
#define PC14                            GPIO_Pins_14
#define PC15                            GPIO_Pins_15

// Port D
#define PD0                             GPIO_Pins_0
#define PD1															GPIO_Pins_1	// 
#define PD2															GPIO_Pins_2	// 
#define PD3															GPIO_Pins_3	// 
#define PD4															GPIO_Pins_4	// 
#define PD5															GPIO_Pins_5	// 
#define PD6															GPIO_Pins_6	// 
#define PD7                             GPIO_Pins_7
#define PD8                             GPIO_Pins_8
#define PD9                             GPIO_Pins_9
#define PD10                            GPIO_Pins_10
#define PD11                            GPIO_Pins_11
#define PD12                            GPIO_Pins_12
#define PD13                            GPIO_Pins_13
#define PD14                            GPIO_Pins_14
#define PD15                            GPIO_Pins_15

// Port E
#define PE0                             GPIO_Pins_0
#define PE1															GPIO_Pins_1	// 
#define PE2															GPIO_Pins_2	// 
#define PE3															GPIO_Pins_3	// 
#define PE4															GPIO_Pins_4	// 
#define PE5															GPIO_Pins_5	// 
#define PE6															GPIO_Pins_6	// 
#define PE7                             GPIO_Pins_7
#define PE8                             GPIO_Pins_8
#define PE9                             GPIO_Pins_9
#define PE10                            GPIO_Pins_10
#define PE11                            GPIO_Pins_11
#define PE12                            GPIO_Pins_12
#define PE13                            GPIO_Pins_13
#define PE14                            GPIO_Pins_14
#define PE15                            GPIO_Pins_15

#define PF6                            GPIO_Pins_6
#define PF7                            GPIO_Pins_7
//=================================================

#define ON	1
#define OFF	0

#define HIGH		1
#define LOW		0

#define TRUE		1
#define FALSE		0

#define __GNUC__


typedef  void (*pFunction)(void);

//设备I/O口定义 
////////////////////////////////////////////////////////////////////////io set
#define IO_YX1_PORT			  			    		  GPIOF
#define IO_YX1			  								 PF7	// out
#define IO_YX1_READ	                    	GPIO_ReadInputDataBit(IO_YX1_PORT,IO_YX1);


#define BUZZER_PORT			  				 GPIOF
#define BUZZER			  					PF6	// out
#define BUZZER_OFF	                      GPIO_ResetBits(BUZZER_PORT,BUZZER);
#define BUZZER_ON	                        GPIO_SetBits(BUZZER_PORT,BUZZER);

#define KEY_OUT3_PORT			  			        GPIOD
#define KEY_OUT3			  									PD4	// out
#define KEY_OUT3_OFF	                    GPIO_ResetBits(KEY_OUT3_PORT,KEY_OUT3);
#define KEY_OUT3_ON	                      GPIO_SetBits(KEY_OUT3_PORT,KEY_OUT3);

#define KEY_OUT2_PORT			  			        GPIOB
#define KEY_OUT2			  									PB3	// out
#define KEY_OUT2_OFF	                    GPIO_ResetBits(KEY_OUT2_PORT,KEY_OUT2);
#define KEY_OUT2_ON	                      GPIO_SetBits(KEY_OUT2_PORT,KEY_OUT2);

#define KEY_OUT1_PORT			  			        GPIOA
#define KEY_OUT1			  									PA15	// out
#define KEY_OUT1_OFF	                    GPIO_ResetBits(KEY_OUT1_PORT,KEY_OUT1);
#define KEY_OUT1_ON	                      GPIO_SetBits(KEY_OUT1_PORT,KEY_OUT1);


#define KEY_LED1_PORT			  			        GPIOB
#define KEY_LED1			  									PB4	// out
#define KEY_LED1_OFF	                    GPIO_ResetBits(KEY_LED1_PORT,KEY_LED1);
#define KEY_LED1_ON	                      GPIO_SetBits(KEY_LED1_PORT,KEY_LED1);

#define KEY_LED2_PORT			  			        GPIOC
#define KEY_LED2			  									PA13	// out
#define KEY_LED2_OFF	                    GPIO_ResetBits(KEY_LED2_PORT,KEY_LED2);
#define KEY_LED2_ON	                      GPIO_SetBits(KEY_LED2_PORT,KEY_LED2);

#define KEY_LED3_PORT			  			        GPIOB
#define KEY_LED3			  									PA1	// out
#define KEY_LED3_OFF	                    GPIO_ResetBits(KEY_LED3_PORT,KEY_LED3);
#define KEY_LED3_ON	                      GPIO_SetBits(KEY_LED3_PORT,KEY_LED3);



#define _4G_RET_PORT					 GPIOA
#define _4G_RET							 PA12	// out
#define _4G_RET_OFF						 GPIO_ResetBits(_4G_RET_PORT,_4G_RET);
#define _4G_RET_ON						  GPIO_SetBits(_4G_RET_PORT,_4G_RET);


#define _4G_PWKEY_PORT					 	GPIOA
#define _4G_PWKEY							 PA11	// out
#define _4G_PWKEY_OFF						 GPIO_ResetBits(_4G_PWKEY_PORT,_4G_PWKEY);
#define _4G_PWKEY_ON						  GPIO_SetBits(_4G_PWKEY_PORT,_4G_PWKEY);


#define _4G_POWER_PORT					 	GPIOA
#define _4G_POWER							 PA8	// out
#define _4G_POWER_OFF						 GPIO_ResetBits(_4G_POWER_PORT,_4G_POWER);
#define _4G_POWER_ON						  GPIO_SetBits(_4G_POWER_PORT,_4G_POWER);

void  SystemReset(void);
void  Mcu_Io_Init(void);
////////////////////////////////////////////////////////////////////////


#endif
