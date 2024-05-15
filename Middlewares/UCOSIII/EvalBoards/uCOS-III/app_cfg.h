/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                             (c) Copyright 2012; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/OS-II is provided in source form for FREE evaluation, for educational
*               use or peaceful research.  If you plan on using uC/OS-II in a commercial
*               product you need to contact Micrium to properly license its use in your
*               product.  We provide ALL the source code for your convenience and to
*               help you experience uC/OS-II.  The fact that the source code is provided
*               does NOT mean that you can use it without paying a licensing fee.
*
*               Knowledge of the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                       APPLICATION CONFIGURATION
*
*                                       IAR Development Kits
*                                              on the
*
*                                    STM32F429II-SK KICKSTART KIT
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : FT
*********************************************************************************************************
*/

#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE ENABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define	XY_HY  0
#define	XY_YKC 1
#define	XY_AP  2 
#define XY_JM  3 	
#define	XY_MAX 4
#define NET_YX_SELCT XY_JM


#if((NET_YX_SELCT == XY_HY) || (NET_YX_SELCT == XY_JM) )
#define UPDATA_STATE     1        //1标志支持远程升级  0表示不支持远程升级
#define WLCARD_STATE     0		  //1表示有白名单卡    0表示不支持白名单卡
#else 
#define UPDATA_STATE     0        //1标志支持远程升级  0表示不支持远程升级
#define WLCARD_STATE     1		  //1表示有白名单卡    0表示不支持白名单卡
#endif




#define  APP_CFG_TASK_START_PRIO                           31u
#define		APP_CFG_TASK_4GRECV_PRIO                        3u   //串口接收优先级要高一点，要第一时间处理串口接收发送过来得数据
#define  APP_CFG_TASK_2_PRIO                               4u
#define  APP_CFG_TASK_CARD_PRIO                   		   30u   //卡任务运行的东西很多  20220805
#define  APP_CFG_TASK_CHARGE_PRIO                   	   6u
#define  APP_CFG_TASK_DWIN_PRIO                   	   	    7u
#define		APP_CFG_TASK_4GMAIN_PRIO                        8u
#define		APP_CFG_TASK_4GSEND_PRIO                        10u
/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_STK_SIZE                     100u
#define  APP_CFG_TASK_BLINKY_STK_SIZE                    128u
#define  APP_CFG_TASK_CARD_STK_SIZE                     250u
#define  APP_CFG_TASK_CHARGE_STK_SIZE                    150u
#define  APP_CFG_TASK_DWIN_STK_SIZE                     350u
#if(UPDATA_STATE)
#define  APP_CFG_TASK_4GMAIN_STK_SIZE                     300u
#else
#define  APP_CFG_TASK_4GMAIN_STK_SIZE                     150u
#endif
#define  APP_CFG_TASK_4GRECV_STK_SIZE                     250u
#define  APP_CFG_TASK_4GSEND_STK_SIZE                     300u

//#define  APP_CFG_TASK_START_STK_SIZE                     1u
//#define  APP_CFG_TASK_BLINKY_STK_SIZE                    1u
//#define  APP_CFG_TASK_CARD_STK_SIZE                     1u
//#define  APP_CFG_TASK_CHARGE_STK_SIZE                    1u
//#define  APP_CFG_TASK_DWIN_STK_SIZE                     2u
//#define  APP_CFG_TASK_4GMAIN_STK_SIZE                     1u
//#define  APP_CFG_TASK_4GRECV_STK_SIZE                     2u
//#define  APP_CFG_TASK_4GSEND_STK_SIZE                     3u
/*
*********************************************************************************************************
*                                            TASK STACK SIZES LIMIT
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_STK_SIZE_PCT_FULL             90u
#define  APP_CFG_TASK_START_STK_SIZE_LIMIT       (APP_CFG_TASK_START_STK_SIZE     * (100u - APP_CFG_TASK_START_STK_SIZE_PCT_FULL))    / 100u
#define  APP_CFG_TASK_BLINKY_STK_SIZE_LIMIT      (APP_CFG_TASK_BLINKY_STK_SIZE    * (100u - APP_CFG_TASK_START_STK_SIZE_PCT_FULL))    / 100u


/*
*********************************************************************************************************
*                                       TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#ifndef  TRACE_LEVEL_OFF
#define  TRACE_LEVEL_OFF                0
#endif

#ifndef  TRACE_LEVEL_INFO
#define  TRACE_LEVEL_INFO               1
#endif

#ifndef  TRACE_LEVEL_DBG
#define  TRACE_LEVEL_DBG                2
#endif

#define  APP_CFG_TRACE_LEVEL             TRACE_LEVEL_OFF
#define  APP_CFG_TRACE                   printf

#define  BSP_CFG_TRACE_LEVEL             TRACE_LEVEL_OFF
#define  BSP_CFG_TRACE                   printf

#define  APP_TRACE_INFO(x)               ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_CFG_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)                ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_CFG_TRACE x) : (void)0)

#define  BSP_TRACE_INFO(x)               ((BSP_CFG_TRACE_LEVEL  >= TRACE_LEVEL_INFO) ? (void)(BSP_CFG_TRACE x) : (void)0)
#define  BSP_TRACE_DBG(x)                ((BSP_CFG_TRACE_LEVEL  >= TRACE_LEVEL_DBG)  ? (void)(BSP_CFG_TRACE x) : (void)0)

#endif
