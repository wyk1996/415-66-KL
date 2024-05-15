#ifndef __HLW8110_H
#define __HLW8110_H	 

#include "main.h"


	


//IO端口定义
//#define LED0 PBout(5)	// DS0
//#define LED1 PEout(5)	// DS1	



//8112/8110 reg define
#define REG_SYSCON_ADDR         0x00
#define REG_EMUCON_ADDR         0x01
#define REG_HFCONST_ADDR        0x02
#define REG_EMUCON2_ADDR        0x13
#define REG_ANGLE_ADDR        	0x22			//相角寄存器
#define REG_UFREQ_ADDR          0x23     	//市电线性频率地址
#define REG_RMSIA_ADDR          0x24
#define REG_RMSIB_ADDR          0x25
#define REG_RMSU_ADDR           0x26
#define REG_PF_ADDR             0x27
#define REG_ENERGY_PA_ADDR			0x28
#define REG_ENERGY_PB_ADDR			0x29
#define REG_POWER_PA_ADDR       0x2C
#define REG_POWER_PB_ADDR       0x2D


#define REG_SAGCYC_ADDR         0x17
#define REG_SAGLVL_ADDR         0x18
#define REG_OVLVL_ADDR          0x19
#define REG_OIALVL_ADDR          0x1a

#define REG_INT_ADDR          	0x1D
#define REG_IE_ADDR          		0x40
#define REG_IF_ADDR          		0x41
#define REG_RIF_ADDR          	0x42

#define REG_RDATA_ADDR          0x44


#define REG_CHECKSUM_ADDR				0x6f
#define REG_RMS_IAC_ADDR				0x70
#define REG_RMS_IBC_ADDR				0x71
#define REG_RMS_UC_ADDR					0x72
#define REG_POWER_PAC_ADDR			0x73
#define REG_POWER_PBC_ADDR			0x74
#define REG_POWER_SC_ADDR				0x75
#define REG_ENERGY_AC_ADDR			0x76
#define REG_ENERGY_BC_ADDR			0x77


//宏定义
#define FALSE	0

#define D_TIME1_50MS	50



void Init_HLW8110(void);
void Calculate_HLW8110_MeterData(void);
void Init_HLW8112(void);
void HLW8112_Measure(void);

extern float   F_AC_V;						//电压值有效值
extern float   F_AC_I;						//A通道电流有效值
extern float   F_AC_E;						//A通道有功电能(量)
#endif

#define HLW8110		0		//使能1--使用UART通讯(HLW8110)，使能0--使用SPI通讯(HLW8112)	
#define HLW8112		1		//使能1--使用UART通讯(HLW8112)，使能0--使用SPI通讯(HLW8110)	




//直流校正系数

//8112A通道或8110通道校正系数
#define D_CAL_U		1000/200		//电压校正系数
#define D_CAL_A_I	1000/500		//A通道电流校正系数
#define D_CAL_A_P	1000/1000		//A通道功率校正系数
#define D_CAL_A_E	1000/100		//A通道电能校正系数


//8112 B通道校正系数
#define D_CAL_B_P	1000/1000		//B通道功率校正系数
#define D_CAL_B_I	1000/1000		//B通道电流校正系数
#define D_CAL_B_E	1000/1000		//B通道电能校正系数



#if HLW8110

#define IO_HLW8112_EN	PAout(5)

#define IO_HLW8112_CS	PAout(6)
#define IO_HLW8112_SCLK	PAout(7)

#endif




#if HLW8112

//#define IO_HLW8112_EN	PAout(5)

#define IO_HLW8112_CS	PBout(9)
#define IO_HLW8112_SCLK	PBout(8)

#define IO_HLW8112_SDI	PBout(7)
#define IO_HLW8112_SDO	PBin(6)

//#define IO_HLW8112_INT1	PDin(11)
#define IO_HLW8112_INT2	PBin(5)


#endif
	



