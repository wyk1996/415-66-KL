#ifndef __SPI2_H
#define __SPI2_H

#include "main.h"

#define SPI_SPI1_PORT			  			        GPIOA
#define SPI_SPI1_SCS_PORT			  			    GPIOA
#define WIZ_SCS			  GPIO_Pins_4	// out
#define WIZ_SCS_LOW	                      GPIO_ResetBits(SPI_SPI1_SCS_PORT,WIZ_SCS);
#define WIZ_SCS_HIGH	                    GPIO_SetBits(SPI_SPI1_SCS_PORT,WIZ_SCS);

#define SPI_SPI1_SCLK_PORT			  			  GPIOA
#define WIZ_SCLK			GPIO_Pins_5	// out
#define SPI_SPI1_MISO_PORT			  			  GPIOA
#define WIZ_MISO			GPIO_Pins_6	// in
#define SPI_SPI1_MOSI_PORT			  			  GPIOA
#define WIZ_MOSI			GPIO_Pins_7	// out


void  Spi1_IoInit(void);
void WIZ_SPI_Init(void);
void WIZ_CS(uint8_t val);
uint8_t SPI2_SendByte(uint8_t byte);
#endif

