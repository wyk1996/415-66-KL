#ifndef __RC522_H
#define __RC522_H	
#include "at32f4xx.h"
//#include "delay.h"

#include "main.h"

/************管脚配置**************/
//SCK 配置
#define RC522_SCK_RCC			RCC_APB2PERIPH_GPIOB
#define RC522_SCK_PORT		GPIOB				  	    													  
#define RC522_SCK_PIN			GPIO_Pins_13
//MOSI 配置
#define RC522_MOSI_RCC		RCC_APB2PERIPH_GPIOB
#define RC522_MOSI_PORT		GPIOB				  	    													  
#define RC522_MOSI_PIN		GPIO_Pins_15
//MISO 配置	
#define RC522_MISO_RCC		RCC_APB2PERIPH_GPIOB
#define RC522_MISO_PORT		GPIOB				  	    													  
#define RC522_MISO_PIN		GPIO_Pins_14
//CS 配置	
#define RC522_CS_RCC		  RCC_APB2PERIPH_GPIOB
#define RC522_CS_PORT	  	GPIOB			  	    													  
#define RC522_CS_PIN		  GPIO_Pins_12
//RST 配置	
#define RC522_RST_RCC		  RCC_APB2PERIPH_GPIOB
#define RC522_RST_PORT	  GPIOB				  	    													  
#define RC522_RST_PIN		  GPIO_Pins_0
//延时函数 配置	
#define RC522_delay_us		delay_u
#define RC522_delay_ms		delay_m

#define RC522_SCK_1  			GPIO_SetBits(RC522_SCK_PORT,RC522_SCK_PIN)
#define RC522_SCK_0  			GPIO_ResetBits(RC522_SCK_PORT,RC522_SCK_PIN)
#define RC522_MOSI_1  		GPIO_SetBits(RC522_MOSI_PORT,RC522_MOSI_PIN)
#define RC522_MOSI_0  		GPIO_ResetBits(RC522_MOSI_PORT,RC522_MOSI_PIN)
#define RC522_CS_1  			GPIO_SetBits(RC522_CS_PORT,RC522_CS_PIN)
#define RC522_CS_0  			GPIO_ResetBits(RC522_CS_PORT,RC522_CS_PIN)
#define RC522_RST_1  			GPIO_SetBits(RC522_RST_PORT,RC522_RST_PIN)
#define RC522_RST_0  			GPIO_ResetBits(RC522_RST_PORT,RC522_RST_PIN)
#define RC522_READ_MISO 	GPIO_ReadInputDataBit(RC522_MISO_PORT,RC522_MISO_PIN)
//基础函数
void RC522_Init(void);
uint8_t RC522_ReadCard(uint8_t Card_addr,uint8_t *Card_Data);
uint8_t RC522_WriteCard(uint8_t Card_addr, uint8_t *Card_Data);


void RC522_Reset(void);
void RC522_Init(void);
char RC522_PcdRequest(uint8_t req_code,uint8_t *pTagType);
char RC522_PcdAnticoll(uint8_t *pSnr);
char RC522_PcdSelect(uint8_t *pSnr);
char RC522_PcdAuthState(uint8_t auth_mode,uint8_t addr,uint8_t *pKey,uint8_t *pSnr);
char RC522_PcdRead(uint8_t addr,uint8_t *p);
char RC522_PcdWrite(uint8_t addr,uint8_t *p);
char RC522_PcdHalt(void);
void RC522_CalulateCRC(uint8_t *pIn ,uint8_t len,uint8_t *pOut);
char RC522_PcdReset(void);
char RC522_M500PcdConfigISOType(uint8_t type);

uint8_t RC522_ReadRawRC(uint8_t Address);
void RC522_WriteRawRC(uint8_t Address, uint8_t value);
void RC522_SetBitMask(uint8_t reg,uint8_t mask);
void RC522_ClearBitMask(uint8_t reg, uint8_t mask);
char RC522_PcdComMF522(uint8_t Command,uint8_t *pIn,uint8_t InLenByte,uint8_t *pOut,uint8_t *pOutLenBit);
void RC522_PcdAntennaOn(void);
void RC522_PcdAntennaOff(void);
char RC522_PcdValue(uint8_t dd_mode,uint8_t addr,uint8_t *pValue);
char RC522_PcdBakValue(uint8_t sourceaddr, uint8_t goaladdr);
char RC522_SetValue(uint8_t addr,uint32_t Value);

__packed typedef struct                          //电卡用户信息
{
    uint8_t         CardCode[10];                   //密码 	ASICC
    uint32_t        balance;                       //卡内余额
    uint8_t         lockstate;					//结算状态 0xff表示未结算（上锁）   0x00表示已经结算（解锁）
    uint16_t      CardType;         //卡类型B0/B1，C0/C1
	  uint8_t   MCUID[12];    //记录缓存设备MCUID号   
	  uint8_t   Card_MCUID[12];  //存储在卡上面的ID号
	  uint8_t   Storage_MCUID[16]; //要写入卡时
    uint8_t		uidByte[10];					//记录此次刷卡的卡号
} _m1_card_info;

typedef enum
{
    SLOT_CARD_UNDEFINE = 0,
    SLOT_CARD_START,   //刷卡开始
    SLOT_CARD_STOP,    //刷卡停止
} _SLOT_CARD_CONTROL;

typedef struct
{
    uint8_t m1_if_balance;				//0表示已经结算  1表示未结算
    uint32_t down_time;					//倒计时
    _SLOT_CARD_CONTROL SlotCardControl;	//刷卡控制
} _m1_control;
#endif
