/**
  **************************************************************************
  * @file     spi_flash.c
  * @version  v2.0.8
  * @date     2022-04-02
  * @brief    spi_flash source code
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "spi_flash.h"
#include "flashdispos.h"


#define FlALSH_TABLE_MAX   8
typedef struct
{
	uint8_t  flash_type;
	uint32_t   start_add;
	uint32_t	len;
}_FALASH_TABLE;


const _FALASH_TABLE FlashTable[FlALSH_TABLE_MAX]  = 
{
	{CHARGE_RATE  ,0 		,4*1024		},
	
	{DWIN_INFO  ,4*1024, 1024*4		},
	
	{ONLINE_BILL  ,8*1024, 1024*4*2		},
	
	{OFFLINE_BILL  ,16*1024, 1024*4*30		},
	
	{OFFFSLINE_BILL  ,34*4*1024, 1024*4*30	},
	
	{RECORD_QUERY  ,64*4*1024, 1024*4*300	},
	
	{CARD_WL  ,364*4*1024, 1024*4*1		},
	
	{APP_CODE  ,365*4*1024, 1024*4*50		},    //程序  1  0x55 表示需要升级  非 0x55表示需要升级；2 - 5  表示程序长度    6以后全部是程序  
	
};
/**
  * @brief  spi configuration.
  * @param  none
  * @retval none
  */
int fal_partition_read(uint8_t flash_type,uint32_t offsetadd, uint8_t*pdata,uint32_t len)
{
	uint8_t i = 0;
	
	for(i = 0;i < FlALSH_TABLE_MAX;i++)
	{
		if(flash_type == FlashTable[i].flash_type)
		{
			SPI_FLASH_BufferRead(pdata,FlashTable[i].start_add + offsetadd, len);
			return 1;
		}
	}
	return -1;
}



/**
  * @brief  spi configuration.
  * @param  none
  * @retval none
  */
int fal_partition_write(uint8_t flash_type,uint32_t offsetadd, uint8_t*pdata,uint32_t len)
{
	uint8_t i = 0;
	
	for(i = 0;i < FlALSH_TABLE_MAX;i++)
	{
		if(flash_type == FlashTable[i].flash_type)
		{
			SPI_FLASH_BufferWrite(pdata,FlashTable[i].start_add + offsetadd, len);
			return 1;
		}
	}
	return -1;
}
