#ifndef __SPIFLASH_H
	#define __SPIFLASH_H
	
#include "Types.h"


// Includes ---------------------------------------------------------------=/
// Exported constants -----------------------------------------------------=/
// Uncomment the line corresponding to the MIAXISBiometrics evaluation board
//   used to run the example /
//#define W25X16
//#define W25X32
//#define ConHZKMax			0x02000//(256*1280L)  //?????
//#define ConHZKPage			800L//1280      //??????

#define ConSflashMax40			0x80000//(256*2048L)  //Sflash???
#define ConMemoryMax40			0x7E000//(256*(2048L-ConHZKPage)) //?????????

#define ConSflashMax80			0x100000//(256*4096L)  //Sflash???
#define ConMemoryMax80			0xFE000//(256*(4096L-ConHZKPage)) //?????????

//#ifdef W25X16
#define ConSflashMax16			0x200000//(256*8192L)  //Sflash???
#define ConMemoryMax16			0x1FE000//(256*(8192L-ConHZKPage))//?????????
//#endif
//#ifdef W25X32
#define ConSflashMax32			0x400000//(256*16384L)  //Sflash???
#define ConMemoryMax32			0x3FE000//(256*(16384L-ConHZKPage))//?????????
//endif

//Sflash????
#define PAGE_FSIZE			256                      // 1 Page  = 256 Bytes;
#define SECTOR_FSIZE			(PAGE_FSIZE*16)          // 1 Setor =  4K Bytes;
#define BLOCK_32KFSIZE 			(SECTOR_FSIZE*8L)         // 4K Bytes*8  = 32768 Bytes 
#define BLOCK_FSIZE 			(SECTOR_FSIZE*16L)        // 1 Block =  64K Bytes;

//???????
#define LCDHZKAddr			(0x00000000)
#define ConHZKPageNumbs			1024L      			// ??????
#define ConHZKSfSize		        (PAGE_FSIZE*ConHZKPageNumbs)	// 256K Bytes 
#define ConUserAddr			ConHZKSfSize			// ???????


// Exported macro ---------------------------------------------------------=/
// Select SPI FLASH: Chip Select pin low  /

#define SPI_SCK_PORT			  				    GPIOA
#define SPI_SCK			  								  PA5	// out
#define SPI_SO_PORT			  				      GPIOA
#define SPI_SO			  								  PA7	// out
#define SPI_SI_PORT			  				      GPIOA
#define SPI_SI			  								  PA6	// in
/*#define SPI_WP_PORT			  				      GPIOA
#define SPI_WP			  								  PA8	  // out*/
#define SPI_CS0_PORT			  				    GPIOA
#define SPI_CS0           			        PA4	// out

// Deselect SPI FLASH: Chip Select pin high /
#define SPICS_0                         GPIO_ResetBits(SPI_CS0_PORT,SPI_CS0)
#define SPICS_1                         GPIO_SetBits(SPI_CS0_PORT,SPI_CS0)
#define SPIScl_0                        GPIO_ResetBits(SPI_SCK_PORT,SPI_SCK)  
#define SPIScl_1                        GPIO_SetBits(SPI_SCK_PORT,SPI_SCK)
#define SPIDiChk                        GPIO_ReadInputDataBit(SPI_SI_PORT, SPI_SI) 
#define SPIDo_0                         GPIO_ResetBits(SPI_SO_PORT,SPI_SO)
#define SPIDo_1                         GPIO_SetBits(SPI_SO_PORT,SPI_SO)
/*#define SPIWP_0                         GPIO_ResetBits(SPI_WP_PORT,SPI_WP)  
#define SPIWP_1                         GPIO_SetBits(SPI_WP_PORT,SPI_WP)*/

#define SPI_FLASH_CS0_LOW   		SPICS_0;\
                                        DelayUS(5);
#define SPI_FLASH_CS0_HIGH  		SPICS_1

// Exported functions ----------------------------------------------------=/
//---- user interface function -----
uint8 WriteSflash(uint32 Addr,uint32 Numb,uint8 *Buf);      //
uint8 ReadSflash(uint32 Addr,uint32 Numb,uint8 *Buf);       //
void ReadCnLattice(uint32 Ascii,uint8 *Buf,uint8 LeftFlag); //????????????

//---- High layer function --=/
uint8 OnlyWriteSflash(uint32 Addr,uint32 Numb,uint8 *Buf);      //
uint8 EraseSflash(uint32 Addr,uint32 Numb);              //??????
uint8 SectorEraseSflash(uint32 Addr);  
uint8 BlockEraseSflash(uint32 Addr);  
void FLASH_Init(void);
void SPI_FLASH_BufferErase( uint32 WriteAddr, uint32 NumByteToWrite);
void SPI_FLASH_SectorErase(uint32 SectorAddr);
void SPI_FLASH_BlockErase(uint32 BlockAddr);
void SPI_FLASH_ChipErase(void);
void SPI_SPageWrite(uint8* pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);
void SPI_FLASH_PageWrite(uint8* pBuffer, uint32 WriteAddr, uint16 NumByteToWrite);
//void SPI_FLASH_BufferEraseAndWrite(uint8* pBuffer, uint32 WriteAddr, uint32 NumByteToWrite);
void SPI_FLASH_BufferWriteNoErase(uint8* pBuffer, uint32 WriteAddr, uint32 NumByteToWrite);
void SPI_FLASH_BufferRead(uint8* pBuffer, uint32 ReadAddr, uint32 NumByteToRead);
void SPI_FLASH_BufferWrite(uint8* pBuffer, uint32 WriteAddr, uint32 NumByteToWrite);

uint32 SPI_FLASH_ReadID(void);
//---- Low layer function --=/
uint8 SPI_FLASH_ReadByte(void);
void SPI_FLASH_SendByte(uint8 byte);
uint8 SPI_FLASH_Detect(void);
void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WriteDisable(void);
void SPI_FLASH_WaitForWriteEnd(void);
void SPI_FLASH_WriteStatus(uint8 stu);

void SET_DATA_RESET(void);
void ReadSpiFlashBuf( uint8* pBuffer, uint32 ReadAddr, uint32 NumByteToRead);
void WriteSpiFlashBuf(uint8* pBuffer, uint32 WriteAddr, uint32 NumByteToWrite);
void Spi_Flash_Io_Init(void);
void DelayMS(uint16 i);            
void DelayUS(uint16 i);    
///////////////////////////////////////////////////////////////////////////////外部存储器W25Q16结构分配
//W25Q16  8192*256=2097152
#define UpdataCodeStartAddr           0          //升级程序的其实保存位置
#define UpdataCodeLen                 200000     //升级程序的最大长度
#define EthSetDataStartAddr           200100     //网络参数的保存起始地址
#define EthSetDataLen                 600        //网络数据的最大长度
#define UpdataCodeLenSaveStartAddr    200700     //升级程序的长度数据起始位置
#define UpdataCodeLenSaveMax          4          //升级程序的长度数据最大位
//#define AckTimeStartAddr              200800     //心跳包查询设备状态
//#define AckTimeLen                    2          //心跳包长度


//最终到达334000地址位置

#endif 
