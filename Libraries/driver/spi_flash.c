/******************** (C) COPYRIGHT ********************
 * ???t??  ㏒ospi_flash.c
 * ?豕那?    ㏒ospi 米℅2?車|車?o‘那y?a      
 * 車2?t芍??車 ----------------------------
 *         | PB12-SPI2-NSS  : W25X40-CS  |
 *         | PB13-SPI2-SCK  : W25X40-CLK |
 *         | PB14-SPI2-MISO : W25X40-DO  |
 *         | PB15-SPI2-MOSI : W25X40-DIO |
 *         | PA8-WRITE    : W25X40-DIO   |
 *         | PA8-HOLD    : W25X40-DIO    |
 *          ----------------------------
 * ?a～?㊣?  ㏒oST3.0.0
 *
 * ℅¯??    ㏒o
 * 2??赤    ㏒o
**********************************************************************************/
#include "spi_flash.h"
#include "main.h"
/****************************************Copyright (c)**************************************************
**------------------------------------------------------------------------------------------------------*/


// Exported types ----------------------------------------------------------=/
#define  ZB25Q64_FLASH_ID       0x5E4017
#define  W25X80_FLASH_ID        0xEF3014
#define  W25X16_FLASH_ID	      0xEF3015
#define  W25X32_FLASH_ID      	0xEF3016
#define  AT25DF161_FLASH_ID    	0x1F4600	//0x1F4602
#define  AT25DF321_FLASH_ID     0x1F4700
#define  SST25VF040B_FLASH_ID	  0xBF258D
#define  SST25VF080B_FLASH_ID	  0xBF258E
#define  SST25VF016B_FLASH_ID	  0xBF2541
#define  SST25VF032B_FLASH_ID	  0xBF254A
// Private define ----------------------------------------------------------=/
#define EWSR     		0x50  //SST25VFXXX Auto Address Increment Programming instruction  /
#define EBSY     		0x70  //SST25VFXXX Enable SO to output RY/BY# status during AAI programming instruction  /
#define DBSY     		0x80  //SST25VFXXX Disable SO to output RY/BY# status during AAI programming instruction  /
#define BKE32      	0x52  //SST25VFXXX Erase 32 KByte block instruction /
#define BKE64       0xD8  //SST25VFXXX Erase 32 KByte Block Erase instruction /
#define AAIWP     	0xAD  //SST25VFXXX Enable-Write-Status-Register instruction  /
///////////////////////
#define WRITE   		0x02  //&FM  page program Write to Memory instruction /
#define WRSR    		0x01  //&FM  Write Status Register instruction /
#define WRSR2    		0x31  //Write Status Register Byte 2 instruction /
#define WREN     		0x06  //&FM  Write enable instruction /
#define WRDI     		0x04  //&FM  Write disable instruction /
#define READ      		0x03  //&FM  Read from Memory instruction /
//#define FRED   		0x0B  // fast Read from Memory instruction /
//#define FRDO      		0x3B  // fast Read dual output from Memory instruction /
#define RDSR     		0x05  //&FM  Read Status Register instruction  /
#define RDID      		0x9F  // Read identification /
//#define SCTE      		0xD8  // Sector Erase instruction /
#define SCTE       		0x20  // 4KB Sector Erase instruction /
//#define BUKE        	0xC7  // Bulk Erase instruction /
#define BLKE       		0xD8  // Block Erase instruction /
#define CHPE       		0xC7  // chip Erase instruction /
#define PWDN      		0xB9  // Power down  instruction /
#define PWUP        		0xAB  // Power Up  instruction /

#define WIP_Flag 		0x01  // Write In Progress (WIP) flag /

#define Dummy_Byte 	0xA5
// Private typedef ---------------------------------------------------------=/
// Private macro -----------------------------------------------------------=/
// Private variables -------------------------------------------------------=/
//uint32 ConSflashMax0;
uint32 ConMemoryMax=0;
uint8 SSTFlashFlag=FALSE;//??sflash ???SST25VFxxx
//uint8 dSectorBuffer[SECTOR_FSIZE-1800]={0};
//uint8 dSectorBuffer[4096-1800]={0};
// Private function prototypes ---------------------------------------------=/
// Private functions -------------------------------------------------------=/



void  Spi_Flash_Io_Init(void)
{GPIO_InitType GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_GPIOA, ENABLE);
	///////////////////////////////////////////////////////////////25q16
  GPIO_InitStructure.GPIO_Pins = SPI_SCK; 
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(SPI_SCK_PORT, SPI_SCK);
	
	// Port a output
	GPIO_InitStructure.GPIO_Pins = SPI_SO; 
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(SPI_SO_PORT, SPI_SO);
	
	// Port a input
  GPIO_InitStructure.GPIO_Pins = SPI_SI;
	GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(SPI_SI_PORT, &GPIO_InitStructure);
	
  // Port a output
  GPIO_InitStructure.GPIO_Pins = SPI_CS0; 
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_SetBits(SPI_CS0_PORT, SPI_CS0);
	
	// Port C output
  /*GPIO_InitStructure.GPIO_Pins = SPI_WP; 
  GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_SetBits(SPI_WP_PORT, SPI_WP);*/
}
  

void DelayMS(uint16 i)               
{
   uint16 j,k;
   for(j=0;j<i;j++)
      for(k=0;k<110;k++);
}

void DelayUS(uint16 i)               
{
   for(;i!=0;i--);
}

void  OFF_IRQ(void)
{
	//OSIntEnter();//1?㊣??米赤3
	__set_PRIMASK(1);
	DelayMS(1);
}

void  ON_IRQ(void)
{
	DelayMS(1);
	//OSIntExit(); //∩辰?a?米赤3
	__set_PRIMASK(0); 
}

//-----------------------------------------------------------------------------
// ???  : FLASH_Init
// ?  ?    : Initializes the peripherals used by the SPI FLASH driver.
// ?  ?          : None
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void FLASH_Init(void)
{
    volatile uint32 FLASH_ID0=0;  	

    //SPI_MFInit();
     // Deselect the FLASH: Chip Select high /
    SPICS_1;
     
    SPI_FLASH_WriteEnable();
    SPI_FLASH_WriteDisable();
    FLASH_ID0=0;
    
    SPI_FLASH_WriteStatus(0x00);
    if(SPI_FLASH_Detect()==TRUE)
        FLASH_ID0 = SPI_FLASH_ReadID();
    
    if((FLASH_ID0&0xFFFFFF00) != AT25DF161_FLASH_ID && FLASH_ID0 != W25X16_FLASH_ID)
    {	
        DelayMS(5);
        if(SPI_FLASH_Detect()==TRUE)
            FLASH_ID0 = SPI_FLASH_ReadID();
    }
    ConMemoryMax = 0;
    if(FLASH_ID0 == W25X80_FLASH_ID)
    {
        ConMemoryMax=ConMemoryMax80;
        //SSTFlashFlag = 1;
    }
    else if(FLASH_ID0 == SST25VF040B_FLASH_ID)
    {
        ConMemoryMax=ConMemoryMax40;
        SSTFlashFlag = TRUE;
    }
    else if(FLASH_ID0 == SST25VF080B_FLASH_ID||(FLASH_ID0&0xFFFF0FFF)== W25X80_FLASH_ID)
    {
        ConMemoryMax=ConMemoryMax80;
        SSTFlashFlag = TRUE;
    }
    else if(FLASH_ID0 == SST25VF016B_FLASH_ID)
    {
        ConMemoryMax=ConMemoryMax16;
        SSTFlashFlag = TRUE;
    }
    else if(FLASH_ID0 == SST25VF032B_FLASH_ID)
    {
        ConMemoryMax=ConMemoryMax32;
        SSTFlashFlag = TRUE;
    }
    else if((FLASH_ID0&0xFFFFFF00) ==AT25DF161_FLASH_ID ||(FLASH_ID0&0xFFFF0FFF) == W25X16_FLASH_ID)
    {
        ConMemoryMax=ConMemoryMax16;
    }
    else if((FLASH_ID0&0xFFFFFF00) ==AT25DF321_FLASH_ID ||(FLASH_ID0&0xFFFF0FFF) == W25X32_FLASH_ID)
    {
        ConMemoryMax=ConMemoryMax32;
    }
    else// if(FLASH_ID0 == AT25DF161_FLASH_ID||FLASH_ID0 == W25X16_FLASH_ID)
    {
          //FlashLed(ConLedRedGreen,2,1);
          ConMemoryMax=ConMemoryMax16;
    }
				if(FLASH_ID0 == ZB25Q64_FLASH_ID)      //zb25q64
		{
	    	ConMemoryMax=ConMemoryMax32;      
		}
    SPI_FLASH_WriteStatus(0x00);
}
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_SectorErase
// ?  ?    : Erases the specified FLASH sector.
// ?  ?          : SectorAddr: address of the sector to erase.
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void SPI_FLASH_SectorErase(uint32 SectorAddr)
{
    // Send write enable instruction /
    SPI_FLASH_WriteEnable();

    // Sector Erase /
    // Select the FLASH: Chip Select low /
    SPICS_0;
    // Send Sector Erase instruction /
    SPI_FLASH_SendByte(SCTE);
    // Send SectorAddr high nibble address byte /
    SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
    // Send SectorAddr medium nibble address byte /
    SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
    // Send SectorAddr low nibble address byte /
    SPI_FLASH_SendByte(SectorAddr & 0xFF);
    // Deselect the FLASH: Chip Select high /
    SPICS_1;

    // Wait the end of Flash writing /
    SPI_FLASH_WaitForWriteEnd();
    SPI_FLASH_WriteDisable();
}
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_BlockErase
// ?  ?    : Erases the specified FLASH block
// ?  ?          : SectorAddr: address of the sector to erase.
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void SPI_FLASH_BlockErase(uint32 BlockAddr)
{

    // Send write enable instruction /
    SPI_FLASH_WriteEnable();

    // Sector Erase /
    // Select the FLASH: Chip Select low /
    SPICS_0;
    // Send Block Erase instruction /
    SPI_FLASH_SendByte(BLKE);
    // Send BlockAddr high nibble address byte /
    SPI_FLASH_SendByte((BlockAddr & 0xFF0000) >> 16);
    // Send BlockAddr medium nibble address byte /
    SPI_FLASH_SendByte((BlockAddr & 0xFF00) >> 8);
    // Send BlockAddr low nibble address byte /
    SPI_FLASH_SendByte(BlockAddr & 0xFF);
    // Deselect the FLASH: Chip Select high /
    SPICS_1;

    // Wait the end of Flash writing /
    SPI_FLASH_WaitForWriteEnd();
    SPI_FLASH_WriteDisable();

}
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_ChipErase
// ?  ?    : Erases the entire FLASH.
// ?  ?          : None
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void SPI_FLASH_ChipErase( void)
{
    // Send write enable instruction /
    SPI_FLASH_WriteEnable();

    // Bulk Erase /
    // Select the FLASH: Chip Select low /
    SPICS_0;
    // Send Bulk Erase instruction  /
    SPI_FLASH_SendByte(CHPE);
    // Deselect the FLASH: Chip Select high /
    SPICS_1;

    // Wait the end of Flash writing /
    SPI_FLASH_WaitForWriteEnd();
    SPI_FLASH_WriteDisable();
}

void SPI_SPageWrite(uint8* pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
    SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
}
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_PageWrite
// ?  ?    : Writes more than one byte to the FLASH with a single WRITE
//                  cycle(Page WRITE sequence). The number of byte can't exceed
//                  the FLASH page size.
// ?  ?          : - pBuffer : pointer to the buffer  containing the data to be
//                    written to the FLASH.
//                  - WriteAddr : FLASH's internal address to write to.
//                  - NumByteToWrite : number of bytes to write to the FLASH,
//                    must be equal or less than "SPI_FLASH_PageSize" value.
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void SPI_FLASH_PageWrite(uint8* pBuffer, uint32 WriteAddr, uint16 NumByteToWrite)
{
    // Enable the write access to the FLASH /
    SPI_FLASH_WriteEnable();

    // Select the FLASH: Chip Select low /
    SPICS_0;
    // Send "Write to Memory " instruction /
    SPI_FLASH_SendByte(WRITE);
    // Send WriteAddr high nibble address byte to write to /
    SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
    // Send WriteAddr medium nibble address byte to write to /
    SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
    // Send WriteAddr low nibble address byte to write to /
    SPI_FLASH_SendByte(WriteAddr & 0xFF);

    // while there is data to be written on the FLASH /
    while (NumByteToWrite--)
    {
        // Send the current byte /
        SPI_FLASH_SendByte(*pBuffer);
        // Point on the next byte to be written /
        pBuffer++;
    }

    // Deselect the FLASH: Chip Select high /
    SPICS_1;

    // Wait the end of Flash writing /
    SPI_FLASH_WaitForWriteEnd();
    SPI_FLASH_WriteDisable();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BufferWrite
* Description    : Writes block of data to the FLASH. In this function, the
*                  number of WRITE cycles are reduced, using Page WRITE sequence.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BufferWriteNoErase(uint8* pBuffer, uint32 WriteAddr, uint32 NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
  OFF_IRQ();
	
  Addr = WriteAddr % PAGE_FSIZE;
  count = PAGE_FSIZE - Addr;
  NumOfPage =  NumByteToWrite / PAGE_FSIZE;
  NumOfSingle = NumByteToWrite % PAGE_FSIZE;

	//SPI_FLASH_BufferErase(WriteAddr,NumByteToWrite);//2芍3y
	
  if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, PAGE_FSIZE);
        WriteAddr +=  PAGE_FSIZE;
        pBuffer += PAGE_FSIZE;
      }

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
      {
        temp = NumOfSingle - count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / PAGE_FSIZE;
      NumOfSingle = NumByteToWrite % PAGE_FSIZE;

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, PAGE_FSIZE);
        WriteAddr +=  PAGE_FSIZE;
        pBuffer += PAGE_FSIZE;
      }

      if (NumOfSingle != 0)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
	ON_IRQ();
}

uint8 dSectorBuffer[SECTOR_FSIZE];
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_BufferWrite
// ?  ?    : Writes block of data to the FLASH. In this function, the
//                  number of WRITE cycles are reduced, using Page WRITE sequence.
// ?  ?          : - pBuffer : pointer to the buffer  containing the data to be
//                    written to the FLASH.
//                  - WriteAddr : FLASH's internal address to write to.
//                  - NumByteToWrite : number of bytes to write to the FLASH.
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void SPI_FLASH_BufferWrite(uint8* pBuffer, uint32 WriteAddr, uint32 NumByteToWrite)
{
    uint32 i,j,s;	
    uint32 dSector_Offset,SectorAdd;

    SPI_FLASH_SendByte(Dummy_Byte);
    SectorAdd = (WriteAddr/SECTOR_FSIZE)*SECTOR_FSIZE;
    dSector_Offset = WriteAddr%SECTOR_FSIZE;
    for(j=0,s=0; j<NumByteToWrite; s++)
    {		
        SPI_FLASH_BufferRead(dSectorBuffer, SectorAdd, SECTOR_FSIZE);
        SPI_FLASH_SectorErase(SectorAdd);

        for(; j<NumByteToWrite; ) 
        {
            dSectorBuffer[dSector_Offset++] = pBuffer[j++];
            if(dSector_Offset==SECTOR_FSIZE)
            {
                dSector_Offset = 0;
                break;
            }
        }

        for(i=0;i<16;i++)
        {
            SPI_FLASH_PageWrite(dSectorBuffer+i*PAGE_FSIZE, SectorAdd+i*PAGE_FSIZE, PAGE_FSIZE);
        }
        SectorAdd += SECTOR_FSIZE;
    }
}
//----------------------------------------------------------
//??:??Sflash????
//??: ????????,??????????
//??:Addr:   ????????
//      Numb:   ????????  
//---------------------------------------------------------=/
uint8 dSectorBuffer[SECTOR_FSIZE];
void SPI_FLASH_BufferErase( uint32 WriteAddr, uint32 NumByteToWrite)
{
//uint8 sbuf[6]={0};
    uint32 i,j;
    uint32 dSector_Offset,SectorAdd;

    //	SetSPI1_Speed(SPI_SPEED_HIGH);
    SPI_FLASH_SendByte(Dummy_Byte);
    SectorAdd = (WriteAddr/SECTOR_FSIZE)*SECTOR_FSIZE;
    for(dSector_Offset=WriteAddr%SECTOR_FSIZE,j=0; j<NumByteToWrite; )
    {
        if(dSector_Offset==0)
        {
            i=NumByteToWrite-j;
            if((SectorAdd%BLOCK_FSIZE)==0&&i>=BLOCK_FSIZE)
            {
                SPI_FLASH_BlockErase(SectorAdd);
                j+=BLOCK_FSIZE;
                SectorAdd += BLOCK_FSIZE;
                continue;
            }
        }
        if(dSector_Offset!=0||(NumByteToWrite-j)<SECTOR_FSIZE)
        {
            SPI_FLASH_BufferRead(dSectorBuffer, SectorAdd, SECTOR_FSIZE);
            //SPI_FM_Write(dSectorBuffer, ConSFlashBakStartAddr, SECTOR_FSIZE);			
            //sbuf[0]=0x5A;
            //for(i=0;i<4;i++) sbuf[i+1] = *(((uint8 *)&SectorAdd)+3-i);
            //SPI_FM_Write(sbuf, ConSFlashBakflgAddr, 5);
        }
        SPI_FLASH_SectorErase(SectorAdd);

        if(dSector_Offset!=0||(NumByteToWrite-j)<SECTOR_FSIZE)
        {
            for(; j<NumByteToWrite; ) 
            {
                dSectorBuffer[dSector_Offset++] = 0xFF;
                j++;
                if(dSector_Offset==SECTOR_FSIZE)
                {
                    dSector_Offset = 0;
                    break;
                }				
            }
            for(i=0;i<16;i++)
            {
                SPI_FLASH_PageWrite(dSectorBuffer+i*PAGE_FSIZE, SectorAdd+i*PAGE_FSIZE, PAGE_FSIZE);
            }
        }
        else j+=SECTOR_FSIZE;
        //sbuf[0]=0xFF;
        //SPI_FM_Write(sbuf, ConSFlashBakflgAddr, 1);
        SectorAdd += SECTOR_FSIZE;
    }
    //	SetSPI1_Speed(SPI_SPEED_MID);
}
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_BufferRead
// ?  ?    : Reads a block of data from the FLASH.
// ?  ?          : - pBuffer : pointer to the buffer that receives the data read
//                    from the FLASH.
//                  - ReadAddr : FLASH's internal address to read from.
//                  - NumByteToRead : number of bytes to read from the FLASH.
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void SPI_FLASH_BufferRead( uint8* pBuffer, uint32 ReadAddr, uint32 NumByteToRead)
{
    //	SetSPI1_Speed(SPI_SPEED_HIGH);
//    SPI_FLASH_SendByte(Dummy_Byte);
    // Select the FLASH: Chip Select low /
    SPICS_0;

    // Send "Read from Memory " instruction /
    SPI_FLASH_SendByte(READ);

    // Send ReadAddr high nibble address byte to read from /
    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
    // Send ReadAddr medium nibble address byte to read from /
    SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
    // Send ReadAddr low nibble address byte to read from /
    SPI_FLASH_SendByte(ReadAddr & 0xFF);

    while (NumByteToRead--) // while there is data to be read /
    {
        // Read a byte from the FLASH /
        *pBuffer = SPI_FLASH_ReadByte();
        // Point to the next location where the byte read will be saved /
        pBuffer++;
    }

    // Deselect the FLASH: Chip Select high /
    SPICS_1;
}
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_ReadID
// ?  ?    : Reads FLASH identification.
// ?  ?          : None
// ?  ?         : None
// ?  ?         : FLASH identification
//------------------------------------------------------------------------=/
uint32 SPI_FLASH_ReadID( void)
{
    uint32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

    // Select the FLASH: Chip Select low /
    SPICS_0;

    // Send "RDID " instruction /
    SPI_FLASH_SendByte(RDID);

    // Read a byte from the FLASH /
    Temp0 = SPI_FLASH_ReadByte();

    // Read a byte from the FLASH /
    Temp1 = SPI_FLASH_ReadByte();

    // Read a byte from the FLASH /
    Temp2 = SPI_FLASH_ReadByte();

    // Deselect the FLASH: Chip Select high /
    SPICS_1;

    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

    return Temp;
}

//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_Detect 
// ?  ?  : Sends a byte through the SPI interface and return the byte
//           received from the SPI bus. ??SPI ?????????
// ?  ?  : csx
// ?  ?  : 0 no flash The value of the received byte.
//----------------------------------------------------------------------------=/
uint8 SPI_FLASH_Detect( void)
{
/*    //uint32 i=0;
// Loop while DR register in not emplty /
// while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_FLASH_CS0_LOW;
    while (i++<100000)
    {
        if(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != 0) break;
    }
    if(i==100000)
    {
        SPI_FLASH_CS0_HIGH;
        return FALSE;
    }
// Send byte through the SPI1 peripheral /
    SPI_I2S_SendData(SPI1, Dummy_Byte);

// Wait to receive a byte /
//while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    while (i++<200000)
    {
        if(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) != 0) break;
    }
// ?  ? the byte read from the SPI bus /
    SPI_I2S_ReceiveData(SPI1);
// Deselect the FLASH: Chip Select high /
    SPI_FLASH_CS0_HIGH;

    if(i==200000)  return FALSE;
    else */
    return TRUE;
}
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_WriteEnable
// ?  ?    : Enables the write access to the FLASH.
// ?  ?          : None
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void SPI_FLASH_WriteEnable(void)
{
// Select the FLASH: Chip Select low /
    SPICS_0;

// Send "Write Enable" instruction /vuyt
    SPI_FLASH_SendByte(WREN);

// Deselect the FLASH: Chip Select high /
    SPICS_1;
}
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_WriteDisable
// ?  ?    : Disable  the write access to the FLASH.
// ?  ?          : None
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void SPI_FLASH_WriteDisable(void)
{
    // Select the FLASH: Chip Select low /
    SPICS_0;

    // Send "Write Enable" instruction /
    SPI_FLASH_SendByte(WRDI);

    // Deselect the FLASH: Chip Select high /
    SPICS_1;
}
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_WaitForWriteEnd
// ?  ?    : Polls the status of the Write In Progress (WIP) flag in the
//                  FLASH's status  register  and  loop  until write  opertaion
//                  has completed.
// ?  ?          : None
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void SPI_FLASH_WaitForWriteEnd(void)
{
    uint8 FLASH_Status = 0;
    volatile uint8 FLASH_Status1 = 0;

    SPICS_0;
    SPI_FLASH_SendByte(RDSR);// Send "Read Status Register" instruction /
    do
    {// Loop as long as the memory is busy with a write cycle /
      // Send a dummy byte to generate the clock needed by the FLASH
      //and put the value of the status register in FLASH_Status variable /
        FLASH_Status = SPI_FLASH_ReadByte();
        FLASH_Status1= SPI_FLASH_ReadByte();

    }while ((FLASH_Status & WIP_Flag) == 1); // Write in progress /
    SPICS_1;
}
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_WriteStatus
// ?  ?    : ??????
// ?  ?          : ??????
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void SPI_FLASH_WriteStatus(uint8 stu)
{
    //uint16 i;

    // Send write enable instruction /
     SPI_FLASH_WriteEnable();

    // Send "Write Enable" instruction /
    SPICS_0;
    SPI_FLASH_SendByte(WRSR);
    SPI_FLASH_SendByte(stu);
    SPICS_1;
    SPI_FLASH_WaitForWriteEnd();
    
    SPICS_0;
    SPI_FLASH_SendByte(WRSR2);
    SPI_FLASH_SendByte(0x18);
    SPICS_1;
    SPI_FLASH_WaitForWriteEnd();
    SPI_FLASH_WriteDisable();
}
//////////////////////////////
//----------------------------------------------------------
//??:?Sflash?????
//??: 
//??:Addr: ????????????
//      Numb: ???????
//      *Buf: ??????????
//??: 0 ??  other:??
//---------------------------------------------------------=/
uint8 WriteSflash(uint32 Addr,uint32 Numb,uint8 *Buf)
{
    uint32 sad;

    if(Numb==0) return 1;
    if((Addr+Numb)>ConMemoryMax) return 2;	

    sad = Addr+ ConUserAddr;//
    WriteSpiFlashBuf(Buf, sad, Numb);
    return 0;
}
//----------------------------------------------------------
//??:?Sflash???????
//??:?
//??:Addr:   ????????????
//      Numb:   ???????  
//      *Buf:   ??????????
//??: 0 ??  other:??
//--------------------------------------------------------=/
uint8 ReadSflash(uint32 Addr,uint32 Numb,uint8 *Buf)
{
    uint32 sad;

    //LCD_MoveCur(3,5);
    //printf("%d %d %04X",Addr,Numb,ConMemoryMax);
    if(Numb==0) return 1;
    if((Addr+Numb)>ConMemoryMax) return 2;	

    sad = Addr+ ConUserAddr;//
//    SPI_FLASH_SendByte(Dummy_Byte);
    SPI_FLASH_BufferRead(Buf, sad, Numb);
    return 0;
}
//----------------------------------------------------------
//??:Sflash????
//??:?
//??:Addr:   ????????
//      Numb:   ????????  
//??: 0 ??  other:??
//--------------------------------------------------------=/
uint8 EraseSflash(uint32 Addr,uint32 Numb)
{
    uint32 sad;//SectorAddr;

    if(Numb==0) return 1;
    if((Addr+Numb)>ConMemoryMax) return 2;	

    sad = Addr+ ConUserAddr;//
    SPI_FLASH_BufferErase(sad, Numb);

    return 0;
}
//----------------------------------------------------------
//??:?Sflash?????????,?WriteSflash???
//??: ?
//??:Addr: ????????????
//      Numb: ???????
//      *Buf: ??????????
//??: 0 ??  other:??
//---------------------------------------------------------=/
uint8 OnlyWriteSflash(uint32 Addr,uint32 Numb,uint8 *Buf)
{
    uint32 j,p;	
    uint32 dPage_Offset,PageAdd;
    uint32 waddr;
    uint8 dpageBuffer[256];
    uint16 i;

    if(Numb==0) return 1;
    if((Addr+Numb)>ConMemoryMax) return 2;	

    SPI_FLASH_SendByte(Dummy_Byte);
    waddr = Addr+ ConUserAddr;//
    
    PageAdd = (waddr/PAGE_FSIZE)*PAGE_FSIZE;
    for(dPage_Offset=(waddr%PAGE_FSIZE),j=0,p=0; j<Numb; p++)
    {
        //PageAdd += (p*PAGE_FSIZE);
        if(dPage_Offset!=0||(Numb-j)<PAGE_FSIZE)//
                SPI_FLASH_BufferRead((uint8 *)dpageBuffer, PageAdd, PAGE_FSIZE);
        else for(i=0;i<256;i++)dpageBuffer[i] =0xFF;
        for(; j<Numb; ) 
        {
            dpageBuffer[dPage_Offset++] = Buf[j++];
            if(dPage_Offset==PAGE_FSIZE)
            {
                dPage_Offset = 0;
                break;
            }
        }
        SPI_FLASH_PageWrite((uint8 *)dpageBuffer, PageAdd, PAGE_FSIZE);
        PageAdd += PAGE_FSIZE;
    }
    return 0;
}
//----------------------------------------------------------
//??:SectorEraseSflash???
//??:??????
//??:Addr:   ????????
//??: 0 ??  other:??
//--------------------------------------------------------=/
uint8 SectorEraseSflash(uint32 Addr)
{

    if((Addr+SECTOR_FSIZE) > ConMemoryMax) return 1;	
    //	SetSPI1_Speed(SPI_SPEED_HIGH);
    SPI_FLASH_SendByte(Dummy_Byte);
    Addr = Addr+ ConUserAddr;//
          
    // Send write enable instruction /
    SPI_FLASH_WriteEnable();

    // Sector Erase /
    // Select the FLASH: Chip Select low /
    SPICS_0;
    // Send Sector Erase instruction /
    SPI_FLASH_SendByte(SCTE);
    // Send SectorAddr high nibble address byte /
    SPI_FLASH_SendByte((Addr & 0xFF0000) >> 16);
    // Send SectorAddr medium nibble address byte /
    SPI_FLASH_SendByte((Addr & 0xFF00) >> 8);
    // Send SectorAddr low nibble address byte /
    SPI_FLASH_SendByte(Addr & 0xFF);
    // Deselect the FLASH: Chip Select high /
    SPICS_1;

    // Wait the end of Flash writing /
    SPI_FLASH_WaitForWriteEnd();
    SPI_FLASH_WriteDisable();
    //	  SetSPI1_Speed(SPI_SPEED_MID);
    return 0;
}
//??: 0 ??  other:??
uint8 BlockEraseSflash(uint32 Addr)
{
    if((Addr+BLOCK_FSIZE) > ConMemoryMax) return 1;	
    //	SetSPI1_Speed(SPI_SPEED_HIGH);
    SPI_FLASH_SendByte(Dummy_Byte);
    Addr = Addr+ ConUserAddr;//
    // Send write enable instruction /
    SPI_FLASH_WriteEnable();

    // Sector Erase /
    // Select the FLASH: Chip Select low /
    SPICS_0;
    // Send Block Erase instruction /
    SPI_FLASH_SendByte(BLKE);
    // Send BlockAddr high nibble address byte /
    SPI_FLASH_SendByte((Addr & 0xFF0000) >> 16);
    // Send BlockAddr medium nibble address byte /
    SPI_FLASH_SendByte((Addr & 0xFF00) >> 8);
    // Send BlockAddr low nibble address byte /
    SPI_FLASH_SendByte(Addr & 0xFF);
    // Deselect the FLASH: Chip Select high /
    SPICS_1;

    // Wait the end of Flash writing /
    SPI_FLASH_WaitForWriteEnd();
    SPI_FLASH_WriteDisable();
    //	  SetSPI1_Speed(SPI_SPEED_MID);
    return 0;
}
//----------------------------------------------------------
//??:????????????
//??:Ascii: ?????
//      ??:	Q = AsciiH - 0A1H
//            W = AsciiL - 0A0H
//      OffsetCN = (Q*94+W-1)*32
//??:Ascii:  ?????
//      *Buf:  ??????????  
//  LeftFlag:  =TRUE???????
//---------------------------------------------------------=/
void ReadCnLattice(uint32 Ascii,uint8 *Buf,uint8 LeftFlag)
{
    uint32  OffsetCN;

    if(Ascii < 0xa1a0) return; //??????????

    OffsetCN = (uint32)((*((uint8 *)&Ascii)-0xa1)*94+(*(((uint8 *)&Ascii)+1)-0xa0)-1)*32;
    if(!LeftFlag) OffsetCN += 16;
//    SPI_FLASH_SendByte(0xFF);
    SPI_FLASH_BufferRead(Buf, OffsetCN, 16);
    //SPI_FLASH_BufferRead(Buf+8, OffsetCN+12, 6);
    return;	   
}


//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_ReadByte
// ?  ?    : Reads a byte from the SPI Flash.
//                  This function must be used only if the Start_Read_Sequence
//                  function has been previously called.
// ?  ?          : None
// ?  ?         : None
// ?  ?         : Byte Read from the SPI Flash.
//------------------------------------------------------------------------=/
uint8 SPI_FLASH_ReadByte(void)
{	
	  uint8 byte;

		uint8 i;

    byte=0;
    for (i=0; i<8; i++)
    {
        byte <<= 1;
        SPIScl_1;

        if(SPIDiChk)
        {
            byte |= 1;	//Wait SDO to go Hi
        }
				//__NOP();__NOP();__NOP();__NOP();
        SPIScl_0;
    }

    return byte;
	

}
//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_SendByte
// ?  ?    : Sends a byte through the SPI interface and return the byte
//                  received from the SPI bus.
// ?  ?          : byte : byte to send.
// ?  ?         : None
// ?  ?         : The value of the received byte.
//------------------------------------------------------------------------=/
void SPI_FLASH_SendByte(uint8 byte)
{
		
	  uint8 i;

    for(i=0; i<8; i++)
    {
        if(byte & 0x80)//((byte & 0x80)== 0x80)
        {
         SPIDo_1;
        }
        else
        {
            SPIDo_0;
        }
            SPIScl_1;     //toggle clock high
            byte <<= 1;
          
            //__NOP();__NOP();__NOP();__NOP();
            SPIScl_0;
    }


}


//-----------------------------------------------------------------------------
// ???  : SPI_FLASH_BufferRead
// ?  ?    : Reads a block of data from the FLASH.
// ?  ?          : - pBuffer : pointer to the buffer that receives the data read
//                    from the FLASH.
//                  - ReadAddr : FLASH's internal address to read from.
//                  - NumByteToRead : number of bytes to read from the FLASH.
// ?  ?         : None
// ?  ?         : None
//------------------------------------------------------------------------=/
void ReadSpiFlashBuf( uint8* pBuffer, uint32 ReadAddr, uint32 NumByteToRead)
{
    //	SetSPI1_Speed(SPI_SPEED_HIGH);
//    SPI_FLASH_SendByte(Dummy_Byte);
    // Select the FLASH: Chip Select low /
	  OFF_IRQ();
    SPICS_0;

    // Send "Read from Memory " instruction /
    SPI_FLASH_SendByte(READ);

    // Send ReadAddr high nibble address byte to read from /
    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
    // Send ReadAddr medium nibble address byte to read from /
    SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
    // Send ReadAddr low nibble address byte to read from /
    SPI_FLASH_SendByte(ReadAddr & 0xFF);

    while (NumByteToRead--) // while there is data to be read /
    {
        // Read a byte from the FLASH /
        *pBuffer = SPI_FLASH_ReadByte();
        // Point to the next location where the byte read will be saved /
        pBuffer++;
    }

    // Deselect the FLASH: Chip Select high /
    SPICS_1;
		ON_IRQ();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BufferWrite
* Description    : Writes block of data to the FLASH. In this function, the
*                  number of WRITE cycles are reduced, using Page WRITE sequence.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void WriteSpiFlashBuf(uint8* pBuffer, uint32 WriteAddr, uint32 NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
	
  OFF_IRQ();
	
  Addr = WriteAddr % PAGE_FSIZE;
  count = PAGE_FSIZE - Addr;
  NumOfPage =  NumByteToWrite / PAGE_FSIZE;
  NumOfSingle = NumByteToWrite % PAGE_FSIZE;

	SPI_FLASH_BufferErase(WriteAddr,NumByteToWrite);
	
  if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, PAGE_FSIZE);
        WriteAddr +=  PAGE_FSIZE;
        pBuffer += PAGE_FSIZE;
      }

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
      {
        temp = NumOfSingle - count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / PAGE_FSIZE;
      NumOfSingle = NumByteToWrite % PAGE_FSIZE;

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, PAGE_FSIZE);
        WriteAddr +=  PAGE_FSIZE;
        pBuffer += PAGE_FSIZE;
      }

      if (NumOfSingle != 0)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
	ON_IRQ();
}