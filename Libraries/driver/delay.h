#ifndef __DELAY_H
#define __DELAY_H 			   
#include "main.h"

//********************************************************************************
//修改说明
//V1.1 20140803 
//1,delay_us,添加参数等于0判断,如果参数等于0,则直接退出. 
//2,修改ucosii下,delay_ms函数,加入OSLockNesting的判断,在进入中断后,也可以准确延时.
//V1.2 20150411  
//修改OS支持方式,以支持任意OS(不限于UCOSII和UCOSIII,理论上任意OS都可以支持)
//添加:delay_osrunning/delay_ostickspersec/delay_osintnesting三个宏定义
//添加:delay_osschedlock/delay_osschedunlock/delay_ostimedly三个函数
//V1.3 20150521
//修正UCOSIII支持时的2个bug：
//delay_tickspersec改为：delay_ostickspersec
//delay_intnesting改为：delay_osintnesting
////////////////////////////////////////////////////////////////////////////////// 

void delay_init(u8 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus);
void delay_u(u32 nus);
void delay_m(u16 nms);

#endif

#ifndef _SYS_H
#define _SYS_H


//////////////////////////////////////////////////////////////////////////////////  

//0,不支持os
//1,支持os
#define SYSTEM_SUPPORT_OS		1		//定义系统文件夹是否支持OS	
///////////////////////////////////////////////////////////////////////////////////
//定义一些常用的数据类型短关键字 
typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef const int32_t sc32;  
typedef const int16_t sc16;  
typedef const int8_t sc8;  

typedef __IO int32_t  vs32;
typedef __IO int16_t  vs16;
typedef __IO int8_t   vs8;

typedef __I int32_t vsc32;  
typedef __I int16_t vsc16; 
typedef __I int8_t vsc8;   

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  
typedef const uint16_t uc16;  
typedef const uint8_t uc8; 

typedef __IO uint32_t  vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t  vu8;

typedef __I uint32_t vuc32;  
typedef __I uint16_t vuc16; 
typedef __I uint8_t vuc8;  

#define ON	1
#define OFF	0
#define Write_Through() (*(__IO uint32_t*)0XE000EF9C=1UL<<2) //Cache透写模式

void Cache_Enable(void);                                    //使能STM32H7的L1-Cahce
void Stm32_Clock_Init(u32 plln,u32 pllm,u32 pllp,u32 pllq); //配置系统时钟
u8 Get_ICahceSta(void);//判断I_Cache是否打开
u8 Get_DCahceSta(void);//判断I_Dache是否打开

#if defined(__clang__) //使用V6编译器(clang)
void __attribute__((noinline)) WFI_SET(void);
void __attribute__((noinline)) INTX_DISABLE(void);
void __attribute__((noinline)) INTX_ENABLE(void);
void __attribute__((noinline)) MSR_MSP(u32 addr);
#elif defined (__CC_ARM)    //使用V5编译器(ARMCC)
//以下为汇编函数
void WFI_SET(void);		//执行WFI指令
void INTX_DISABLE(void);//关闭所有中断
void INTX_ENABLE(void);	//开启所有中断
void MSR_MSP(u32 addr);	//设置堆栈地址 
#endif


#endif





























