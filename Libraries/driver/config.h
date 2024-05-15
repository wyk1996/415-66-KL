#ifndef _CONFIG_H_
#define _CONFIG_H_
#include  <ctype.h> 
#include "Types.h"
#include "at32f4xx.h"
#include "at32f4xx_usart.h"
//#include "device.h"
//#include "at24cxx.h"
//#include "wdg.h"
//#include "delay.h"
//#include "HP303.h"

#define Data_Set_StartAddr 0x1fc00 
//#define Data_Set_StartAddr 0x0000
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
#define PA0				GPIO_Pins_0	// out
#define PA1				GPIO_Pins_1	// out
#define PA2				GPIO_Pins_2	// 
#define PA3				GPIO_Pins_3	// 
#define PA4				GPIO_Pins_4	// 
#define PA5				GPIO_Pins_5	// 
#define PA6				GPIO_Pins_6	// 
#define PA7				GPIO_Pins_7	// 
#define PA8				GPIO_Pins_8	// 
#define PA9				GPIO_Pins_9	// 
#define PA10				GPIO_Pins_10	// 
#define PA11				GPIO_Pins_11	// 
#define PA12				GPIO_Pins_12	// 
#define PA13				GPIO_Pins_13	// 
#define PA14				GPIO_Pins_14	// 
#define PA15				GPIO_Pins_15	// 


 

//Port B

#define PB0                             GPIO_Pins_0
#define PB1				GPIO_Pins_1	// 
#define PB2				GPIO_Pins_2	// 
#define PB3				GPIO_Pins_3	// 
#define PB4				GPIO_Pins_4	// 
#define PB5				GPIO_Pins_5	// 
#define PB6				GPIO_Pins_6	// 
#define PB7                             GPIO_Pins_7
#define PB8                             GPIO_Pins_8
#define PB9                             GPIO_Pins_9
#define PB10                            GPIO_Pins_10
#define PB11                            GPIO_Pins_11
#define PB12                            GPIO_Pins_12
#define PB13                            GPIO_Pins_13
#define PB14                            GPIO_Pins_14
#define PB15                            GPIO_Pins_15


#define WIZ_SCS			        PA4	// out
#define WIZ_SCLK			PA5	// out
#define WIZ_MISO			PA6	// in
#define WIZ_MOSI			PA7	// out


#define WIZ_RESET		        PA2	// out

//eeprom  PB

#define I2C_SCK	                        PB6  //out
#define I2C_SDA	                        PB7  //bidirection



//

// Port C
#define PC6                             GPIO_Pins_6
#define PC13				GPIO_Pins_13	//
#define PC14				GPIO_Pins_14	//
#define PC15				GPIO_Pins_15	//

#define WIZ_INT			        PA3	// in

//=================================================
#define downfloorlen 10
#define upfloorlen   103

#pragma pack(1)
typedef struct _DISPLAY_MSG
{
	uint8 dischina[30];
	uint8 dischinalen;
	uint8 fontx[6];
	uint16 fontxlen;
	uint8 fonty[6];
	uint16 fontylen;
	uint16 floorfontlen;
}DISPLAY_MSG;
#pragma pack()
#define DISPLAY_MSG_LEN        sizeof(DISPLAY_MSG)

#pragma pack(1)
typedef struct _UPFLOOR_MSG
{
	uint8 upfloordata[upfloorlen];
}UPFLOOR_MSG;
#pragma pack()
#define UPFLOOR_MSG_LEN        sizeof(UPFLOOR_MSG)

#pragma pack(1)

typedef struct _DOWNFLOOR_MSG
{
	uint8 downfloordata[downfloorlen];
}DOWNFLOOR_MSG;
#pragma pack()
#define DOWNFLOOR_MSG_LEN        sizeof(DOWNFLOOR_MSG)

#pragma pack(1)
typedef struct _ONFLOOR_MSG
{
  uint8 onfloor[4];
}ONFLOOR_MSG;
#pragma pack()
#define ONFLOOR_MSG_LEN        sizeof(ONFLOOR_MSG)

#pragma pack(1)
typedef struct _VEDIO_MSG
{
  uint8 vip[4];
	uint8 vport[6];
	uint8 vuser[16];
	uint8 vpassword[16];

}VEDIO_MSG;
#pragma pack()
#define VEDIO_MSG_LEN        sizeof(VEDIO_MSG)

#pragma pack(1)
typedef struct _CONFIG_MSG
{
  uint8 op[4];//header: FIND;SETT;FACT...
	uint8 sw_ver[2];
  uint8 mac[6];
  uint8 lip[4];
  uint8 gw[4];
	uint8 sub[4];
  uint8 dns[4];	
	
  uint8 dhcp;
  uint8 debug;
	
  uint16 fw_len;
  uint8 state;
  
}CONFIG_MSG;
#pragma pack()

#define CONFIG_MSG_LEN        sizeof(CONFIG_MSG) - 4 // the 4 bytes OP will not save to EEPROM
//



#define MAX_BUF_SIZE		2920//1460
#define KEEP_ALIVE_TIME	        30	// 30sec


#define ON	1
#define OFF	0

#define HIGH		1
#define LOW		0

#define __GNUC__

// SRAM address range is 0x2000 0000 ~ 0x2000 BFFF (48KB)
#define SOCK_BUF_ADDR 	0x20000000


extern CONFIG_MSG  ConfigMsg, RecvMsg;

#define SOCK_DHCP             0
#define SOCK_HTTP             1
#define SOCK_TCPS             2
#define SOCK_HTTPC            3
#define SOCK_UDPS             4

#define AppBackAddress        0x08020000 //from 128K
#define ConfigAddr		        0x0800FC00


#define NORMAL_STATE            0
#define NEW_APP_IN_BACK         1 //there is new app in back address
#define CONFIGTOOL_FW_UP        2 //configtool update f/w in app

#endif

