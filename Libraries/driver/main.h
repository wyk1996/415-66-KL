

#ifndef __MAIN_H
#define __MAIN_H
#include  <ctype.h>
/////////////////////////////////////CORE include file
#include "at32f4xx.h"
//#include "mcu_init.h"
#include "at32f4xx_usart.h"
#include "at32f4xx_gpio.h"
#include "at32f4xx_dma.h"
#include "at32f4xx_flash.h"
/////////////////////////////////////c include file
#include "Types.h"
#include <stdlib.h>
#include "Types.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
//#include "device.h"
#include "delay.h"
#include "device.h"
#include "at32_board.h"
/////////////////////////////////////ucos include file
//#include <ucos_ii.h>
//#include "app_cfg.h"
//#include <os_cfg.h>
//#include <os_cpu.h>
/////////////////////////////////////periphrr include file
#include "rc522.h"
#include "HLW8110.h"
#include "timer.h"
#include "adc.h"
#include "uart2.h"
#include "uart1.h"
#include "485_1.h"
#include "spi1.h"
#include "spi_flash.h"
/////////////////////////////////////other include file

#include "at32f4xx_rtc.h"
#include "at32f4xx_rcc.h"
#include "at32f4xx_bkp.h"
#include "at32f4xx_dma.h"
#include "at32f4xx_adc.h"
//#include "timer.h"
#include "at32f4xx_flash.h"
#include "at32f4xx_pwr.h"
///////////////////////////////////////
//#include  <stdio.h>
//#include  <string.h>
//#include  <ctype.h>
//#include  <stdlib.h>
//#include  <stdarg.h>
//#include  <math.h>


/*
*********************************************************************************************************
*                                                 OS
*********************************************************************************************************
*/

#include  <os.h>


/*
*********************************************************************************************************
*                                              LIBRARIES
*********************************************************************************************************
*/
#include "at32_board.h"
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>

/*
*********************************************************************************************************
*                                              APP / BSP
*********************************************************************************************************
*/

#include  <app_cfg.h>
#include  <bsp.h>
#include  "ertc.h"
#include "delay.h"
//#include  <bsp_int.h>

extern OS_ERR timeerr;
extern OS_ERR ERRSTATE;
void AppTaskCard(void *p_arg);
void AppTaskCharge(void *p_arg);
void AppTaskDwin(void *p_arg);
void AppTask4GMain(void *p_arg);
void AppTask4GRecv(void *p_arg);
void AppTask4GSend(void *p_arg);



extern OS_Q DwinMq;
extern OS_Q ChTaskMq;
extern OS_Q Send4GMq;
extern OS_Q Recv4GMq;


typedef __packed struct SYS_TEMTIME
{
	unsigned char ucSec;
	unsigned char ucMin;
	unsigned char ucHour;
	/* 星期:0-6,sunday-saturday */
	unsigned char ucWeek;
	unsigned char ucDay;
	unsigned char ucMonth;
	unsigned char ucYear;
}SYSTEM_RTCTIME;

#if(UPDATA_STATE)
#define URART_4GRECV_LEN   600    //4G接收最大个数
#else
#define URART_4GRECV_LEN   250      //4G接收最大个数
#endif
#define MSG_RECV_LEN		50      //消息发送数据得最大个数 包括迪文串口接收
typedef struct
{
	uint32_t  lastrecvtime;		//上一次接收时间
	uint8_t recv_buf[URART_4GRECV_LEN];		//接收缓存
	uint16_t recv_index;			//接收下表
}_UART_4GRECV_CONTORL;

typedef struct
{
	uint32_t  lastrecvtime;		//上一次接收时间
	uint8_t recv_buf[MSG_RECV_LEN];		//接收缓存
	uint16_t recv_index;			//接收下表
}_UART_DISPRECV_CONTORL;
extern _UART_DISPRECV_CONTORL Uart2RecvControl;
extern _UART_4GRECV_CONTORL Uart1RecvControl;

void cp_pwm_full(void);
void cp_pwm_ch_puls(uint16_t pwm);
void cp_set_pwm_val(uint16_t pwm);
uint32_t cp56time(void * pq);
void set_date(uint32_t ERTC_Year,uint32_t ERTC_Month, uint32_t ERTC_Date);
void set_time(uint32_t ERTC_Hours,uint32_t ERTC_Minutes, uint32_t ERTC_Seconds);
uint8_t *   APP_GetPid(void);
void   JumpToProgramCode(void);
#endif
