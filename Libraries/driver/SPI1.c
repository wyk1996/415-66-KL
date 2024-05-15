
#include "spi1.h"

void  Spi1_IoInit(void)
{GPIO_InitType GPIO_InitStructure;
			RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_GPIOA|RCC_APB2PERIPH_SPI1, ENABLE);
		  // SPI 1
    GPIO_InitStructure.GPIO_Pins = WIZ_SCLK | WIZ_MISO | WIZ_MOSI;
    GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(SPI_SPI1_PORT, &GPIO_InitStructure);

    // Port A output
    GPIO_InitStructure.GPIO_Pins = WIZ_SCS; 
    GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(SPI_SPI1_SCS_PORT, WIZ_SCS);
  ///////////////////////////////////////////////////////////////25q16
}

void WIZ_SPI_Init(void)
{
	  SPI_InitType   SPI_InitStructure; 

	  	/* SPI Config -------------------------------------------------------------*/
	  SPI_InitStructure.SPI_TransMode = SPI_TRANSMODE_FULLDUPLEX;
  SPI_InitStructure.SPI_Mode = SPI_MODE_MASTER;
  SPI_InitStructure.SPI_FrameSize = SPI_FRAMESIZE_8BIT;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_LOW;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1EDGE;
  SPI_InitStructure.SPI_NSSSEL = SPI_NSSSEL_SOFT;
  SPI_InitStructure.SPI_MCLKP = SPI_MCLKP_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FIRSTBIT_MSB;
  SPI_InitStructure.SPI_CPOLY = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

	  
	  /* Enable SPI */
	  SPI_Enable(SPI1, ENABLE);

}

// Connected to Data Flash
void WIZ_CS(uint8_t val)
{
	if (val == LOW) {
   		WIZ_SCS_LOW; 
	}else if (val == HIGH){
   		WIZ_SCS_HIGH; 
	}
}


uint8_t SPI2_SendByte(uint8_t byte)
{
	  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TE) == RESET);
         
	  SPI_I2S_TxData(SPI1, byte);
          
	  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RNE) == RESET);
          
	  return SPI_I2S_RxData(SPI1);
}

