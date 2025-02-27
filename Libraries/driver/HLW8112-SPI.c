
/*=========================================================================================================
  * File Name	 : HLW8112-SPI.c
  * Describe 	 : HLW8112,使用SPI的通讯方式
  * Author	   : Tuqiang
  * Version	   : V1.3
  * Record	   : 2019/04/16，V1.2
  * Record	   : 2020/04/02, V1.3	
==========================================================================================================*/
/* Includes ----------------------------------------------------------------------------------------------*/
#include "HLW8110.h"
#include "delay.h"


//extern unsigned char UART_EN;
//extern unsigned char SPI_EN;

#if HLW8112

#define HIGH	1
#define LOW		0

union IntData
{
	u16  inte;			
	u8 byte[2];		
};
union LongData
{
    u32  word;		
    u16  inte[2];		
    u8   byte[4];		
};


unsigned int    U16_TempData;	
unsigned int    U16_IFData;
unsigned int    U16_RIFData;

unsigned int 		U16_IEData;
unsigned int		U16_INTData;


unsigned int 		U16_SAGCYCData;
unsigned int 		U16_SAGLVLData;

unsigned int 		U16_SYSCONData;
unsigned int 		U16_EMUCONData;
unsigned int 		U16_EMUCON2Data;




unsigned int    U16_LineFData;
unsigned int    U16_AngleData;
unsigned int    U16_PFData;



//--------------------------------------------------------------------------------------------
unsigned int	U16_RMSIAC_RegData; 			// A通道电流转换系数
unsigned int	U16_RMSIBC_RegData; 			// B通道电流转换系数
unsigned int	U16_RMSUC_RegData; 				// 电压通道转换系数
unsigned int	U16_PowerPAC_RegData; 		// A通道功率转换系数
unsigned int	U16_PowerPBC_RegData; 		// B通道功率转换系数
unsigned int	U16_PowerSC_RegData; 			// 视在功率转换系数,如果选择A通道，则是A通道视在功率转换系数。A和B通道只能二者选其一
unsigned int	U16_EnergyAC_RegData; 		// A通道有功电能(量)转换系数 
unsigned int	U16_EnergyBC_RegData; 		// A通道有功电能(量)转换系数
unsigned int	U16_CheckSUM_RegData; 		// 转换系数的CheckSum
unsigned int	U16_CheckSUM_Data; 				// 转换系数计算出来的CheckSum

unsigned int	U16_Check_SysconReg_Data; 						
unsigned int	U16_Check_EmuconReg_Data; 						
unsigned int	U16_Check_Emucon2Reg_Data; 			
			
//--------------------------------------------------------------------------------------------
unsigned long 	U32_RMSIA_RegData;			//A通道电流寄存器值
unsigned long 	U32_RMSU_RegData;			//电压寄存器值
unsigned long 	U32_POWERPA_RegData;		//A通道功率寄存器值
unsigned long 	U32_ENERGY_PA_RegData;		//A通道有功电能（量）寄存器值


unsigned long 	U32_RMSIB_RegData;			//B通道电流寄存器值
unsigned long		U32_POWERPB_RegData;		//B通道功率寄存器值
unsigned long 	U32_ENERGY_PB_RegData;	    //B通道有功电能（量）寄存器值
//--------------------------------------------------------------------------------------------

float   F_AC_V;						//电压值有效值
float   F_AC_I;						//A通道电流有效值
float   F_AC_P;						//A通道有功功率
float   F_AC_E;						//A通道有功电能(量)
float   F_AC_BACKUP_E;		//A通道电量备份	
float   F_AC_PF;						//功率因素，A通道和B通道只能选其一 
float		F_Angle;


float   F_AC_I_B;						//B通道电流有效值
float   F_AC_P_B;						//B通道有功功率
float 	F_AC_E_B;						//B通道有功电能(量)
float   F_AC_BACKUP_E_B;		//B通道电量备份	

float   F_AC_LINE_Freq;     //市电线性频率
float   F_IF_RegData;     	//IF寄存器值
//--------------------------------------------------------------------------------------------


/*=====================================================
 * Function : void HLW8112_SPI_WriteByte(unsigned char u8_data)
 * Describe : 
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/09
=====================================================*/
void HLW8112_SPI_WriteByte(unsigned char u8_data)
{
	unsigned char i;
	unsigned char x;
	x = u8_data;
	for(i = 0;i<8;i++)
		{
						
			if(x&(0x80>>i))
				IO_HLW8112_SDI = HIGH;
			else
				IO_HLW8112_SDI = LOW;

	
			IO_HLW8112_SCLK = HIGH;
                        delay_u(10);
			IO_HLW8112_SCLK = LOW;
                        delay_u(10);

		}
}
/*=====================================================
 * Function : unsigned char HLW8112_SPI_ReadByte(void)
 * Describe : 
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/09
=====================================================*/
unsigned char HLW8112_SPI_ReadByte(void)
{
  unsigned char i;
  unsigned char u8_data;
  u8_data = 0x00;
  for(i = 0;i<8;i++)
  {
	u8_data <<= 1;
	
    IO_HLW8112_SCLK = HIGH;
    delay_u(10);
    IO_HLW8112_SCLK = LOW;
    delay_u(10);

    if (IO_HLW8112_SDO == HIGH)
       u8_data |= 0x01;
	   
  }
  
  return u8_data;
        
}

/*=====================================================
 * Function : void HLW8112_SPI_WriteCmd(unsigned char u8_cmd)
 * Describe : 
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/09
=====================================================*/
void HLW8112_SPI_ReadReg(unsigned char u8_RegAddr)
{

    HLW8112_SPI_WriteByte(u8_RegAddr);
}

/*=====================================================
 * Function : void HLW8112_SPI_WriteReg(unsigned char u8_RegAddr)
 * Describe : 
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/09
=====================================================*/
void HLW8112_SPI_WriteReg(unsigned char u8_RegAddr)
{
    unsigned char u8_tempdata;
    u8_tempdata = (u8_RegAddr | 0x80);
    HLW8112_SPI_WriteByte(u8_tempdata);
}



/*=====================================================
 * Function : void HLW8112_WriteREG_EN(void)
 * Describe : 
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/09
=====================================================*/
void HLW8112_WriteREG_EN(void)
{
	//打开写HLW8112 Reg功能
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteByte(0xea);
  HLW8112_SPI_WriteByte(0xe5);
  IO_HLW8112_CS = HIGH;
}
/*=====================================================
 * Function : void HLW8112_WriteREG_DIS(void)
 * Describe : 
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/09
=====================================================*/
void HLW8112_WriteREG_DIS(void)
{
	//关闭写HLW8112 Reg功能
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteByte(0xea);
  HLW8112_SPI_WriteByte(0xdc);
  IO_HLW8112_CS = HIGH;
}

/*=====================================================
 * Function : void Write_HLW8112_RegData(unsigned char ADDR_Reg,unsigned char Data_length,unsigned long u32_data)
 * Describe : 写寄存器
 * Input    : ADDR_Reg,Data_length，u32_data
 * Output   : 
 * Return   : 
 * Record   : 2018/05/09
=====================================================*/
void Write_HLW8112_RegData(unsigned char ADDR_Reg,unsigned char u8_reg_length,unsigned long u32_data)
{
  unsigned char i;
  union LongData u32_t_data;
  union LongData u32_p_data;
  
  u32_p_data.word = u32_data;
  u32_t_data.word = 0x00000000;
  
  for (i = 0; i < u8_reg_length; i++ )
  	{
  		u32_t_data.byte[i] = u32_p_data.byte[i];	//写入REG时，需要注意MCU的联合体(union)定义，字节是高位在bytep[0]，还是在byte[3]
 
  	}
    
  HLW8112_SPI_WriteReg(ADDR_Reg);
  for (i = 0; i < u8_reg_length; i++ )
  	{
  		HLW8112_SPI_WriteByte(u32_t_data.byte[i]);
  	}

  
}
/*=====================================================
 * Function : unsigned long Read_HLW8112_RegData(unsigned char ADDR_Reg,unsigned char u8_reg_length)
 * Describe : 读取寄存器值
 * Input    : ADDR_Reg,u8_Data_length
 * Output   : u8_Buf[4],寄存器值
 * Return   : u1(True or False)
 * Record   : 2018/05/09
=====================================================*/
unsigned long Read_HLW8112_RegData(unsigned char ADDR_Reg,unsigned char u8_reg_length)
{
  unsigned char i;
  union LongData u32_t_data1;

  
  
  IO_HLW8112_CS = LOW;
  u32_t_data1.word = 0x00000000;

//高位在前	
	HLW8112_SPI_ReadReg(ADDR_Reg);
	for (i = 0; i<u8_reg_length; i++ )
  	{
  		u32_t_data1.byte[u8_reg_length-1-i] = HLW8112_SPI_ReadByte();
  		
  	}	
	
  
   IO_HLW8112_CS = HIGH;
   
  return u32_t_data1.word;
}

/*=====================================================
 * Function : void Judge_CheckSum_HLW8112_Calfactor(void)
 * Describe : 验证地址0x70-0x77地址的系数和
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2019/03/18
=====================================================*/
unsigned char Judge_CheckSum_HLW8112_Calfactor(void)
{
  unsigned long a;
  unsigned char d;
 
  //读取RmsIAC、RmsIBC、RmsUC、PowerPAC、PowerPBC、PowerSC、EnergAc、EnergBc的值

  U16_RMSIAC_RegData = Read_HLW8112_RegData(REG_RMS_IAC_ADDR,2);
  U16_RMSIBC_RegData = Read_HLW8112_RegData(REG_RMS_IBC_ADDR,2);
  U16_RMSUC_RegData = Read_HLW8112_RegData(REG_RMS_UC_ADDR,2);
  U16_PowerPAC_RegData = Read_HLW8112_RegData(REG_POWER_PAC_ADDR,2);
  U16_PowerPBC_RegData = Read_HLW8112_RegData(REG_POWER_PBC_ADDR,2);
  U16_PowerSC_RegData = Read_HLW8112_RegData(REG_POWER_SC_ADDR,2);
  U16_EnergyAC_RegData = Read_HLW8112_RegData(REG_ENERGY_AC_ADDR,2);
  U16_EnergyBC_RegData = Read_HLW8112_RegData(REG_ENERGY_BC_ADDR,2);
 
  U16_CheckSUM_RegData = Read_HLW8112_RegData(REG_CHECKSUM_ADDR,2);
  
  a = 0;
  
  a = ~(0xffff+U16_RMSIAC_RegData + U16_RMSIBC_RegData + U16_RMSUC_RegData + 
        U16_PowerPAC_RegData + U16_PowerPBC_RegData + U16_PowerSC_RegData + 
          U16_EnergyAC_RegData + U16_EnergyBC_RegData  );
  
  U16_CheckSUM_Data = a & 0xffff;
  
  
  
  printf("转换系数寄存器\r\n");
	printf("U16_RMSIAC_RegData = %x\n " ,U16_RMSIAC_RegData);
	printf("U16_RMSIBC_RegData = %x\n " ,U16_RMSIBC_RegData);
	printf("U16_RMSUC_RegData = %x\n " ,U16_RMSUC_RegData);
	printf("U16_PowerPAC_RegData = %x\n " ,U16_PowerPAC_RegData);
	printf("U16_PowerPBC_RegData = %x\n " ,U16_PowerPBC_RegData);
	printf("U16_PowerSC_RegData = %x\n " ,U16_PowerSC_RegData);
	printf("U16_EnergyAC_RegData = %x\n " ,U16_EnergyAC_RegData);
	printf("U16_EnergyBC_RegData = %x\n " ,U16_EnergyBC_RegData);
	
	printf("U16_CheckSUM_RegData = %x\n " ,U16_CheckSUM_RegData);
	printf("U16_CheckSUM_Data = %x\n " ,U16_CheckSUM_Data);
  
  if ( U16_CheckSUM_Data == U16_CheckSUM_RegData)
  {
    d = 1;
    printf("系数寄存器校验ok\r\n");
    
  }
  else
  {
    d = 0;
    printf("系数寄存器校验fail\r\n");
  }
  
  return d; 
}


/*=====================================================
 * Function : void Set_OIALVL(void)
 * Describe : 电流过压阀值，设置
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2020/12/04
=====================================================*/
void Set_OIALVL(void)
{
    unsigned int a;
  
  //设置方法,设置100mA过流,OIALVL = 0xb1
  
  
  HLW8112_WriteREG_EN();	//打开写8112 Reg
  
  //打开过压、过流等功能
/*  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON2_ADDR);
  HLW8112_SPI_WriteByte(0x0f);          //电量寄存器读后不清零
  HLW8112_SPI_WriteByte(0xff);
  IO_HLW8112_CS = HIGH;
*/
  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_OIALVL_ADDR);
  HLW8112_SPI_WriteByte(0x00);
  HLW8112_SPI_WriteByte(0xb1);
  IO_HLW8112_CS = HIGH;
  U16_TempData = Read_HLW8112_RegData(REG_OIALVL_ADDR,2);

  
  //设置INT寄存器, 电流通道过流输出，INT = 32E9，INT2过流输出 ,INT1过零输出
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_INT_ADDR);
  HLW8112_SPI_WriteByte(0x32);        
  HLW8112_SPI_WriteByte(0xE9);               
  IO_HLW8112_CS = HIGH;
  
  
  //设置IE寄存器, IE
  a = Read_HLW8112_RegData(REG_IE_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_IE_ADDR);
  HLW8112_SPI_WriteByte((a>>8)&0xff); //    
  HLW8112_SPI_WriteByte(a|0x80); 			//电流A过流使能 OIAIE = 1; 
 // HLW8112_SPI_WriteByte(a&0xff);               
  IO_HLW8112_CS = HIGH;
  
  U16_TempData = Read_HLW8112_RegData(REG_IE_ADDR,2);
  
  HLW8112_WriteREG_DIS();	//关闭写8112 Reg
  
  
  
 
}

/*=====================================================
 * Function : void Set_OVLVL(void)
 * Describe : 电压过压阀值，设置
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2019/03/18
=====================================================*/
void Set_OVLVL(void)
{
    unsigned int a;
  
  //设置方法,0x5b21,设置210V过压,OVLVL = 0x5a8b
  
  
  HLW8112_WriteREG_EN();	//打开写8112 Reg
  
  //打开过压、过流等功能
/*  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON2_ADDR);
  HLW8112_SPI_WriteByte(0x0f);          //电量寄存器读后不清零
  HLW8112_SPI_WriteByte(0xff);
  IO_HLW8112_CS = HIGH;
*/
  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_OVLVL_ADDR);
  HLW8112_SPI_WriteByte(0x5a);
  HLW8112_SPI_WriteByte(0x8b);
  IO_HLW8112_CS = HIGH;
   U16_TempData = Read_HLW8112_RegData(REG_OVLVL_ADDR,2);

  
  //设置INT寄存器, 电压通道过零输出，INT = 3219，INT2过压输出 
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_INT_ADDR);
  HLW8112_SPI_WriteByte(0x32);        
  HLW8112_SPI_WriteByte(0xc9);               
  IO_HLW8112_CS = HIGH;
  
  
  //设置IE寄存器, IE
  a = Read_HLW8112_RegData(REG_IE_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_IE_ADDR);
  HLW8112_SPI_WriteByte((a>>8)|0x02); //电压过压中断使能，OVIE= 1      
 // HLW8112_SPI_WriteByte(0x02); //电压过压中断使能，OVIE= 1   
  HLW8112_SPI_WriteByte(a&0xff);               
  IO_HLW8112_CS = HIGH;
  
  U16_TempData = Read_HLW8112_RegData(REG_IE_ADDR,2);
  
  HLW8112_WriteREG_DIS();	//关闭写8112 Reg
  
  
  
 
}


/*=====================================================
 * Function : void Set_underVoltage(void)
 * Describe : 电压欠压阀值，设置INT2输出
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2020/08/13
=====================================================*/
void Set_underVoltage(void)
{
    unsigned int a;
  
  //设置方法,0x5b21,设置210V过压,OVLVL = 0x5a8b
  
  
  HLW8112_WriteREG_EN();	//打开写8112 Reg
  
/*	
	IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_SAGCYC_ADDR);		//欠压设置
  HLW8112_SPI_WriteByte(0x00);
//  HLW8112_SPI_WriteByte(0x05);
	HLW8112_SPI_WriteByte(0x01);
  IO_HLW8112_CS = HIGH;
   U16_TempData = Read_HLW8112_RegData(REG_SAGCYC_ADDR,2);
*/
  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_SAGLVL_ADDR);		//欠压设置180V
  HLW8112_SPI_WriteByte(0x4E);
  HLW8112_SPI_WriteByte(0x1C);
	
//	HLW8112_SPI_WriteByte(0x6C);		//欠压设置280V
//	HLW8112_SPI_WriteByte(0x7C);
	
	
  IO_HLW8112_CS = HIGH;
   U16_TempData = Read_HLW8112_RegData(REG_OVLVL_ADDR,2);

  
  //设置INT寄存器, 电压通道过零输出，INT = 3219，INT2欠压输出 
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_INT_ADDR);
  HLW8112_SPI_WriteByte(0x32);        
  HLW8112_SPI_WriteByte(0xD9);               
  IO_HLW8112_CS = HIGH;
  
  
  //设置IE寄存器, IE
  a = Read_HLW8112_RegData(REG_IE_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_IE_ADDR);
  HLW8112_SPI_WriteByte((a>>8)|0x08); //电压欠压中断使能，OVIE= 1      
 // HLW8112_SPI_WriteByte(0x02); //电压过压中断使能，OVIE= 1   
  HLW8112_SPI_WriteByte(a&0xff);               
  IO_HLW8112_CS = HIGH;
  
  U16_TempData = Read_HLW8112_RegData(REG_IE_ADDR,2);
  
  HLW8112_WriteREG_DIS();	//关闭写8112 Reg
  
  
  
 
}


/*=====================================================
 * Function : void Set_V_Zero(void)
 * Describe : 电压通道过零设置
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2019/03/18
=====================================================*/
void Set_V_Zero(void)
{
  unsigned int a;
  
  HLW8112_WriteREG_EN();	         //打开写8112 Reg

  //设置EMUCON寄存器,REG_EMUCON_ADDR = REG_EMUCON_ADDR | 0x0180
  a = Read_HLW8112_RegData(REG_EMUCON_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON_ADDR);
  HLW8112_SPI_WriteByte((a>>8)|0x01);          
  HLW8112_SPI_WriteByte((a&0xff)|0x80); // 正向和负向过零点均发生变化，ZXD0 = 1，ZXD1 = 1
  IO_HLW8112_CS = HIGH;
  
  
  //设置EMUCON2寄存器, REG_EMUCON2_ADDR = REG_EMUCON2_ADDR | 0x0024
  a = Read_HLW8112_RegData(REG_EMUCON2_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON2_ADDR);
  HLW8112_SPI_WriteByte(a>>8);          
  HLW8112_SPI_WriteByte((a&0xff)|0x24); // ZxEN = 1,WaveEn = 1;
  IO_HLW8112_CS = HIGH;
    
  

  //设置IE寄存器
  a = Read_HLW8112_RegData(REG_IE_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_IE_ADDR);
  HLW8112_SPI_WriteByte((a>>8)|0x40); //电压过零中断使能，ZX_UIE = 1         
  HLW8112_SPI_WriteByte(a&0xff);               
  IO_HLW8112_CS = HIGH;
  
  //设置INT寄存器, 电压通道过零输出，INT = 3219,INT1输出电压过零
  //a = Read_HLW8112_RegData(REG_IE_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_INT_ADDR);
  HLW8112_SPI_WriteByte(0x32);        
  HLW8112_SPI_WriteByte(0x19);               
  IO_HLW8112_CS = HIGH;
  
  
  
  HLW8112_WriteREG_DIS();	//关闭写8112 Reg
}

/*=====================================================
 * Function : void Set_Leakage(void)
 * Describe : 电流B通道漏电检测功能，只能用比较器功能,INT2输出
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2019/03/18
=====================================================*/
void Set_Leakage(void)
{
  unsigned int a;
  
   HLW8112_WriteREG_EN();	//打开写8112 Reg
  

  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_SYSCON_ADDR);
  HLW8112_SPI_WriteByte(0x0a);          //-------------高8bit，关闭ADC电流通道B
  HLW8112_SPI_WriteByte(0x04);          //-------------低8bit，
  IO_HLW8112_CS = HIGH;

  //设置comp_off = 1,打开B通道比较器
  a = Read_HLW8112_RegData(REG_EMUCON_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON_ADDR);  
  HLW8112_SPI_WriteByte((a>>8)&0xcf);   //   打开比较器    
  HLW8112_SPI_WriteByte(a&0xfd);  
  IO_HLW8112_CS = HIGH;
  
  
  //设置INT寄存器, INT2比较器漏电输出
  a = Read_HLW8112_RegData(REG_INT_ADDR,2); 
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_INT_ADDR);
  HLW8112_SPI_WriteByte((a>>8));        
  HLW8112_SPI_WriteByte(0x29);               
  IO_HLW8112_CS = HIGH;
  
  
  //设置IE寄存器, IE
  a = Read_HLW8112_RegData(REG_IE_ADDR,2);  
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_IE_ADDR);
  HLW8112_SPI_WriteByte((a>>8)|0x80); //漏电中断使能，LeakageIE= 1        
  HLW8112_SPI_WriteByte(a&0xff);               
  IO_HLW8112_CS = HIGH;
  
  U16_TempData = Read_HLW8112_RegData(REG_IE_ADDR,2);
  
  HLW8112_WriteREG_DIS();	//关闭写8112 Reg
}


/*=====================================================
 * Function : void Init_HLW8112(void)
 * Describe : 初始化8112
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/09
=====================================================*/
void Init_HLW8112(void)
{
	GPIO_InitType GPIO_InitStructure;
	
			delay_init(200);	  			//延时初始化
			delay_init(200);	  			//延时初始化
			delay_init(200);	  			//延时初始化
			delay_init(200);	  			//延时初始化
			delay_init(200);	  			//延时初始化
			delay_init(200);	  			//延时初始化
			delay_init(200);	  			//延时初始化
			delay_init(200);	  			//延时初始化
			delay_init(200);	  			//延时初始化
			delay_init(200);	  			//延时初始化
  //SPI init
  delay_m(100);
	
	
//STM32 IO init
	RCC_APB2PeriphClockCmd(	RCC_APB2PERIPH_GPIOB, ENABLE );
	   	 
	  
	  GPIO_InitStructure.GPIO_Pins = GPIO_Pins_7 | GPIO_Pins_8 | GPIO_Pins_9;
    GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
    

    // Port A output
    GPIO_InitStructure.GPIO_Pins = GPIO_Pins_6; 
    GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pins_6);
	
	/*GPIOB->CTRLL&=0X00FFFFFF;  //IO状态设置,设置PB7、PB6
	GPIOB->CTRLH&=0XFFFFFF00;  //IO状态设置,设置PB8、PB9
	GPIOB->CTRLL|=0X38000000;  //IO状态设置，PB7、PB8、PB9-输出、PB6-输入	 
	GPIOB->CTRLL|=0X00000033;  //IO状态设置，PB7、PB8、PB9-输出、PB6-输入	 
    */
//	GPIOD->CRL&=0XFFF00FFF;  //IO状态设置,设置PD11、PD12
//	GPIOD->CRL|=0X00088000;  //IO状态设置，PD11、PD12-输入	 
	
//STM32 IO init
  
//  IO_HLW8112_EN = 1;             //使能一直是拉高状态 所以就不用这条
  delay_m(10);
  IO_HLW8112_CS = HIGH;
  delay_u(2);
  delay_u(2);

  IO_HLW8112_SCLK = LOW;
  delay_u(2);
  IO_HLW8112_SDI = LOW;
  HLW8112_WriteREG_EN();	//打开写8112 Reg
// ea + 5a 电流通道A设置命令，指定当前用于计算视在功率、功率因数、相角、
//瞬时有功功率、瞬时视在功率和有功功率过载的信号指示 的通道为通道A 
// ea + a5 电流通道A设置命令，指定当前用于计算视在功率、功率因数、相角、
//瞬时有功功率、瞬时视在功率和有功功率过载的信号指示 的通道为通道A 
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteByte(0xea);
  HLW8112_SPI_WriteByte(0x5a);
  IO_HLW8112_CS = HIGH;
  
//写SYSCON REG, 关闭电流通道B，电压通道 PGA = 1，电流通道A PGA =  16;
//三路通道满幅值（峰峰值800ma,有效值/1.414 = 565mV）
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_SYSCON_ADDR);
//HLW8112_SPI_WriteByte(0x0a);         //开启电流通道A,PGA = 16   ------高8bit
  HLW8112_SPI_WriteByte(0x0f);          //开启电流通道A和电流通道B PGA =  16;需要在EMUCON中关闭比较器------高8bit
  HLW8112_SPI_WriteByte(0x04);          //-------------低8bit
  IO_HLW8112_CS = HIGH;
 


//写EMUCON REG, 使能A通道PF脉冲输出和有功能电能寄存器累加
   IO_HLW8112_CS = LOW;
   HLW8112_SPI_WriteReg(REG_EMUCON_ADDR);
    HLW8112_SPI_WriteByte(0x10);        //最高位一定设置成0001，关闭比较器,打开B通道  
    HLW8112_SPI_WriteByte(0x03);        //打开A通道和B通道电量累计功能
    IO_HLW8112_CS = HIGH;
  
//写EMUCON2 REG, 3.4HZ数据输出，选择内部基准,打开过压、过流等功能
//3.4HZ(300ms) 6.8HZ(150ms) 13.65HZ(75ms) 27.3HZ(37.5ms)
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON2_ADDR);
  HLW8112_SPI_WriteByte(0x0f);          //电量寄存器读后不清零
  HLW8112_SPI_WriteByte(0xff);
  IO_HLW8112_CS = HIGH;
	
/*	delay_m(10);
Write_HLW8112_RegData(REG_RMS_UC_ADDR,2,0xa4c3);   //????为什么写不成功
  delay_m(10);*/
  
//关闭所有中断
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_IE_ADDR);
  HLW8112_SPI_WriteByte(0x00);                          
  HLW8112_SPI_WriteByte(0x00);               
  IO_HLW8112_CS = HIGH;

  HLW8112_WriteREG_DIS();	//关闭写8112 Reg




//读取RmsIAC、RmsIBC、RmsUC、PowerPAC、PowerPBC、PowerSC、EnergAc、EnergBc的值
  U16_RMSIAC_RegData = Read_HLW8112_RegData(REG_RMS_IAC_ADDR,2);        //0xcad1
  U16_RMSIBC_RegData = Read_HLW8112_RegData(REG_RMS_IBC_ADDR,2);        //0xcacf
  U16_RMSUC_RegData = Read_HLW8112_RegData(REG_RMS_UC_ADDR,2);          //0xa4c3
  U16_PowerPAC_RegData = Read_HLW8112_RegData(REG_POWER_PAC_ADDR,2);    //0xab47
  U16_PowerPBC_RegData = Read_HLW8112_RegData(REG_POWER_PBC_ADDR,2);    //0xab49
  U16_PowerSC_RegData = Read_HLW8112_RegData(REG_POWER_SC_ADDR,2);      //0xab41
  U16_EnergyAC_RegData = Read_HLW8112_RegData(REG_ENERGY_AC_ADDR,2);    //0xe9cc
  U16_EnergyBC_RegData = Read_HLW8112_RegData(REG_ENERGY_BC_ADDR,2);    //0xe9ce
 //printf("U16_RMSUC_RegData = %x\r\n " ,U16_RMSUC_RegData);
 Judge_CheckSum_HLW8112_Calfactor();

  
  Set_V_Zero();         //设置INT1
  
// Set_OVLVL();          //设置INT2
 Set_underVoltage();
  
//  Set_Leakage();        //设置INT2，打开B通道B较器


}

/*=========================================================================================================
 * Function : void Check_WriteReg_Success(void)
 * Describe : 检验写入的REG是否正确写入
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2020/04/02
==========================================================================================================*/
void Check_WriteReg_Success(void)
{
	U16_Check_SysconReg_Data = Read_HLW8112_RegData(REG_SYSCON_ADDR,2);
	U16_Check_EmuconReg_Data = Read_HLW8112_RegData(REG_EMUCON_ADDR,2); 
	U16_Check_Emucon2Reg_Data = Read_HLW8112_RegData(REG_EMUCON2_ADDR,2); 
	
//	printf("写入的SysconReg寄存器:%lx\n " ,U16_Check_SysconReg_Data); 
//	printf("写入的EmuconReg寄存器:%lx\n " ,U16_Check_EmuconReg_Data); 
//	printf("写入的Emucon2Reg寄存器寄存器:%lx\n " ,U16_Check_Emucon2Reg_Data);      
	
}



/*=====================================================
 * Function : void Read_HLW8112_PA_I(void)
 * Describe : 读取A通道电流
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/09
=====================================================*/
void Read_HLW8112_PA_I(void)
{ 
   float a;
   //计算公式,U16_AC_I = (U32_RMSIA_RegData * U16_RMSIAC_RegData)/(电流系数* 2^23）
   U32_RMSIA_RegData = Read_HLW8112_RegData(REG_RMSIA_ADDR,3);
   
	 if ((U32_RMSIA_RegData &0x800000) == 0x800000)
	 {
			F_AC_I = 0;
	 }
	 else
	 {
		a = (float)U32_RMSIA_RegData;
		a = a * U16_RMSIAC_RegData;
		a  = a/0x800000;                     //电流计算出来的浮点数单位是mA,比如5003.12 
		a = a/1;  								// 1 = 电流系数
		a = a/1000;              //a= 5003ma,a/1000 = 5.003A,单位转换成A
		a = a * D_CAL_A_I;				//D_CAL_A_I是校正系数，默认是1
		F_AC_I = a;
	 }
    
}


/*=====================================================
 * Function : void Read_HLW8112_PB_I(void)
 * Describe : 读取B通道电流
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2019/03/28
=====================================================*/
void Read_HLW8112_PB_I(void)
{ 
   float a;
	
   //计算,U16_AC_I_B = (U32_RMSIB_RegData * U16_RMSIBC_RegData)/2^23
   U32_RMSIB_RegData = Read_HLW8112_RegData(REG_RMSIB_ADDR,3);
   if ((U32_RMSIB_RegData &0x800000) == 0x800000)
	 {
			F_AC_I_B = 0;
	 }
	 else
	 {
   a = (float)U32_RMSIB_RegData;
   a = a * U16_RMSIBC_RegData;
   a  = a/0x800000;                     //电流计算出来的浮点数单位是mA,比如5003.12
   a = a/1;  				// 1 = 电流系数
	a = a/1000;								//a= 5003ma,a/1000 = 5.003A,单位转换成A
   a = a * D_CAL_B_I;									//D_CAL_B_I是校正系数，默认是1
   F_AC_I_B = a;
	 }
    
}

/*=====================================================
 * Function : void Read_HLW8112_U(void)
 * Describe : 读取电压
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/09
=====================================================*/
void Read_HLW8112_U(void)
{
   float a;
   //读取电压寄存器值
   U32_RMSU_RegData = Read_HLW8112_RegData(REG_RMSU_ADDR,3);
  
	//计算:U16_AC_V = (U32_RMSU_RegData * U16_RMSUC_RegData)/2^23
	
	 if ((U32_RMSU_RegData &0x800000) == 0x800000)
	 {
			F_AC_V = 0;
	 }
  else
	{
	a =  (float)U32_RMSU_RegData;
	a = a*U16_RMSUC_RegData;  
	a = a/0x400000;     
	a = a/1;  						// 1 = 电压系数
	a = a/100; 				 		//计算出a = 22083.12mV,a/100表示220.8312V，电压转换成V
	a = a*D_CAL_U;				//D_CAL_U是校正系数，默认是1		
	F_AC_V = a;
	}
   
}


/*=====================================================
 * Function : void Read_HLW8112_PA(void)
 * Describe : 读取A通道功率
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/09
=====================================================*/
void Read_HLW8112_PA(void)
{
     float a;
	unsigned long b;

   //读取功率寄存器值
    U32_POWERPA_RegData = Read_HLW8112_RegData(REG_POWER_PA_ADDR,4);
  
  //计算,U16_AC_P = (U32_POWERPA_RegData * U16_PowerPAC_RegData)/(2^31*电压系数*电流系数)
	//单位为W,比如算出来5000.123，表示5000.123W


   
   if (U32_POWERPA_RegData > 0x80000000)
   {
     b = ~U32_POWERPA_RegData;
     a = (float)b;
   }
   else
     a =  (float)U32_POWERPA_RegData;
     
   
    a = a*U16_PowerPAC_RegData;
    a = a/0x80000000;            
    a = a/1;  												// 1 = 电流系数
    a = a/1;  												// 1 = 电压系数
   	a = a * D_CAL_A_P;								//D_CAL_A_P是校正系数，默认是1   
    F_AC_P = a;									 			//单位为W,比如算出来5000.123，表示5000.123W 
}
/*=====================================================
 * Function : void Read_HLW8112_PB(void)
 * Describe : 读取B通道功率
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2019/03/28
=====================================================*/

void Read_HLW8112_PB(void)
{
     float a;
	unsigned long b;


   //读取功率寄存器值
    U32_POWERPB_RegData = Read_HLW8112_RegData(REG_POWER_PB_ADDR,4);
  
  //计算,U16_AC_P_B = (U32_POWERPB_RegData * U16_PowerPBC_RegData)/(1000*2^31)
   
   if (U32_POWERPB_RegData > 0x80000000)
   {
     b = ~U32_POWERPB_RegData;
     a = (float)b;
   }
   else
   {
     a =  (float)U32_POWERPB_RegData;
   }
   
    a = a*U16_PowerPBC_RegData;
    a = a/0x80000000;    
    a = a/1;  			    // 1 = 电流系数
    a = a/1;  				// 1 = 电压系数  
		a = a * D_CAL_B_P;						//D_CAL_A_P是校正系数，默认是1  
    F_AC_P_B = a; 
}


/*=====================================================
 * Function : void void Read_HLW8112_EA(void)
 * Describe : 读取A通道有功电量
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/09
=====================================================*/
void Read_HLW8112_EA(void)
{
//因为掉电数据不保存，所以系统上电后，应读出存在EEPROM内的电量数据，总电量 = EEPROM内电量数据+此次计算的电量	
  float a;

   //读取功率寄存器值
  U32_ENERGY_PA_RegData = Read_HLW8112_RegData(REG_ENERGY_PA_ADDR,3);
  
	//电量计算,电量 = (U32_ENERGY_PA_RegData * U16_EnergyAC_RegData * HFCONST) /(K1*K2 * 2^29 * 4096)
	//HFCONST:默认值是0x1000, HFCONST/(2^29 * 4096) = 0x20000000
  a =  (float)U32_ENERGY_PA_RegData;	
  
  a = a*U16_EnergyAC_RegData;
  a = a/0x20000000;             //电量单位是0.001KWH,比如算出来是2.002,表示2.002KWH
//因为K1和K2都是1，所以a/(K1*K2) = a      
   a = a/1;  					// 1 = 电流系数
   a = a/1;  				// 1 = 电压系数
	a = a * D_CAL_A_E;     				//D_CAL_A_E是校正系数，默认是1
  F_AC_E = a;
  
  
  if (F_AC_E >= 1)    //每读到1度电就清零
  {
    F_AC_BACKUP_E += F_AC_E;
    
    
  //IO_HLW8112_SDI = LOW;
  HLW8112_WriteREG_EN();	//打开写8112 Reg
  
  //清零 REG_ENERGY_PA_ADDR寄存器
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON2_ADDR);
  HLW8112_SPI_WriteByte(0x0b);          //电量寄存器读后清零
  HLW8112_SPI_WriteByte(0xff);
  IO_HLW8112_CS = HIGH;  
  
  
  U32_ENERGY_PA_RegData = Read_HLW8112_RegData(REG_ENERGY_PA_ADDR,3);   //读后清零
  U32_ENERGY_PA_RegData = Read_HLW8112_RegData(REG_ENERGY_PA_ADDR,3);   //读后清零
  F_AC_E = 0;
  //每读到0.001度电就清零,然后再设置读后不清零
   
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON2_ADDR);
  HLW8112_SPI_WriteByte(0x0f);          ////电量寄存器B读后不清零
  HLW8112_SPI_WriteByte(0xff);
  IO_HLW8112_CS = HIGH;
  
  HLW8112_WriteREG_DIS();	//关闭写8112 Reg
  
  
  }
}

/*=====================================================
 * Function : void void Read_HLW8112_EB(void)
 * Describe : 读取B通道有功电量
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2019/03/28
=====================================================*/

void Read_HLW8112_EB(void)
{
//因为掉电数据不保存，所以系统上电后，应读出存在EEPROM内的电量数据，总电量 = EEPROM内电量数据+此次计算的电量	
  float a;

   //读取功率寄存器值
  U32_ENERGY_PB_RegData = Read_HLW8112_RegData(REG_ENERGY_PB_ADDR,3);
  
	//电量计算,电量 = (U32_ENERGY_PA_RegData * U16_EnergyAC_RegData * HFCONST) /(K1*K2 * 2^29 * 4096)
	//HFCONST:默认值是0x1000, HFCONST/(2^29 * 4096) = 0x20000000
  a =  (float)U32_ENERGY_PB_RegData;	
  a = a*U16_EnergyBC_RegData;
 //HFConst(默认值1000H = 2^12),0x1000/(2^29*2^12) = 0x20000000
  a = a/0x20000000;             //电量单位是0.001KWH,比如算出来是2.002,表示2.002KWH 
	//因为K1和K2都是1，所以a/(K1*K2) = a 
  a = a/1;  										// 1 = 电流系数,系数计算可以参考资料
  a = a/1;  										// 1 = 电压系数,系数计算可以参考资料
	a = a * D_CAL_B_E;     				//D_CAL_B_E是校正系数，免校准应用默认是1
  F_AC_E_B = a;
  
  
  if (F_AC_E_B >= 1)    		//每读到1度电就清零
  {
    F_AC_BACKUP_E_B += F_AC_E_B;
    
    
  //IO_HLW8112_SDI = LOW;
  HLW8112_WriteREG_EN();	//打开写8112 Reg
  
  //清零 REG_ENERGY_PB_ADDR寄存器
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON2_ADDR);
  HLW8112_SPI_WriteByte(0x07);          //电量寄存器B读后清零,A不清零
  HLW8112_SPI_WriteByte(0xff);
  IO_HLW8112_CS = HIGH;  
  
  
  U32_ENERGY_PB_RegData = Read_HLW8112_RegData(REG_ENERGY_PB_ADDR,3);   //读后清零
  U32_ENERGY_PB_RegData = Read_HLW8112_RegData(REG_ENERGY_PB_ADDR,3);   //读后清零
  F_AC_E_B = 0;
  //每读到0.001度电就清零,然后再设置读后不清零
   
  IO_HLW8112_CS = LOW;
  HLW8112_SPI_WriteReg(REG_EMUCON2_ADDR);
  HLW8112_SPI_WriteByte(0x0f);          //电量寄存器读后不清零
  HLW8112_SPI_WriteByte(0xff);
  IO_HLW8112_CS = HIGH;
  
  HLW8112_WriteREG_DIS();	//关闭写8112 Reg
  
  
  }
}


/*=====================================================
 * Function : void Read_HLW8112_Linefreq(void)
 * Describe : 读取市电线性频率50HZ,60HZ,本代码使用的是内置晶振
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2019/03/18
=====================================================*/
void Read_HLW8112_Linefreq(void)
{
  float a;
  unsigned long b;
  b = Read_HLW8112_RegData(REG_UFREQ_ADDR,2);
  U16_LineFData = b;
  a = (float)b;
  a = 3579545/(8*a);       
  
  F_AC_LINE_Freq = a;
  
}
/*=====================================================
 * Function : void Read_HLW8112_PF(void)
 * Describe : 读取功率因素
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2019/03/18
=====================================================*/
void Read_HLW8112_PF(void)
{
  float a;
  unsigned long b;
  b = Read_HLW8112_RegData(REG_PF_ADDR,3);
	U16_PFData = b;
 	
  
  if (b>0x800000)       //为负，容性负载
  {
      a = (float)(0xffffff-b + 1)/0x7fffff;
  }
  else
  {
      a = (float)b/0x7fffff;
  }
  
  // 如果功率很小，接近0，那么因PF = 有功/视在功率 = 1，那么此处应将PF 设成 0；
  
  if (F_AC_P < 0.3) // 小于0.3W
	  a = 0; 
  
  F_AC_PF = a;
  
}


/*=====================================================
 * Function : void Read_HLW8112_Angle(void)
 * Describe : 读取相位角
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2019/04/12
=====================================================*/
void Read_HLW8112_Angle(void)
{

	float a;
	unsigned long b;
	b = Read_HLW8112_RegData(REG_ANGLE_ADDR,2);
	U16_AngleData = b;
	
	if ( F_AC_PF < 55)	//线性频率50HZ
	{
		a = b;
		a = a * 0.0805;
		F_Angle = a;
	}
	else
	{
		//线性频率60HZ
		a = b;
		a = a * 0.0965;
		F_Angle = a;
	}
	
	if (F_AC_P < 0.5)		//功率小于0.5时，说明没有负载，相角为0
	{
		F_Angle = 0;
	}
	
	if (F_Angle < 90)
	{
		a = F_Angle;
		//printf("电流超前电压:%f\n " ,a);
	}
	else if (F_Angle < 180)
	{
		a = 180-F_Angle;
		//printf("电流滞后电压:%f\n " ,a);	
	}
	else if (F_Angle < 360)
	{
		a = 360 - F_Angle;
		//printf("电流滞后电压:%f\n " ,a);	
	}
	else
	{
			a = F_Angle -360;
			//printf("电流超前电压:%f\n " ,a);	
	}
	
}

/*=====================================================
 * Function : void Read_HLW8112_State(void)
 * Describe : 读8112 中断状态位
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2019/03/18
=====================================================*/
void Read_HLW8112_State(void)
{
   //读取过压状态位,必须读取RIF状态，才能进入下一次中断
 // U16_IFData = Read_HLW8112_RegData(REG_IF_ADDR,2);
 // U16_RIFData = Read_HLW8112_RegData(REG_RIF_ADDR,2);
	
	
	U16_IEData = Read_HLW8112_RegData(REG_IE_ADDR,2);  
	U16_INTData= Read_HLW8112_RegData(REG_INT_ADDR,2); 
	
	
	U16_SAGCYCData = Read_HLW8112_RegData(REG_SAGCYC_ADDR,2); 
	U16_SAGLVLData = Read_HLW8112_RegData(REG_SAGLVL_ADDR,2); 
	
	
	U16_SYSCONData = Read_HLW8112_RegData(REG_SYSCON_ADDR,2); 
	U16_EMUCONData = Read_HLW8112_RegData(REG_EMUCON_ADDR,2); 
	U16_EMUCON2Data = Read_HLW8112_RegData(REG_EMUCON2_ADDR,2); 
}
/*=====================================================
 * Function : void HLW8012_Measure(void);
 * Describe : 
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2018/05/10
=====================================================*/

void HLW8112_Measure(void)
{
	
	Check_WriteReg_Success();
	
	Read_HLW8112_PA_I();
	Read_HLW8112_U();
	Read_HLW8112_PA();
	Read_HLW8112_EA();    
  Read_HLW8112_Linefreq();
	Read_HLW8112_Angle();
  Read_HLW8112_State();
  Read_HLW8112_PF();   
  Read_HLW8112_PB_I();
	Read_HLW8112_PB();
	Read_HLW8112_EB();
	
	
//	printf("交流测量-SPI通讯 \r\n");
//	
//	printf("\r\n\r\n");//插入换行
//	printf("A通道电能参数\r\n");
//	printf("F_AC_V = %f V \n " ,F_AC_V);		//电压
//	printf("F_AC_I = %f A \n " ,F_AC_I);		//A通道电流
//	printf("F_AC_P = %f W \n " ,F_AC_P);		//A通道功率
//	printf("F_AC_E = %f KWH \n " ,F_AC_E);		//A通道电量


//	printf("\r\n\r\n");//插入换行
//	printf("B通道电能参数\r\n");
//	printf("F_AC_I_B = %f A \n " ,F_AC_I_B);		//B通道电流
//	printf("F_AC_P_B = %f W \n " ,F_AC_P_B);		//B通道功率
//	printf("F_AC_E_B = %f KWH \n " ,F_AC_E_B);		//B通道电量
//	
//	printf("\r\n\r\n");//插入换行
//	printf("F_AC_PF = %f\n " ,F_AC_PF);		//A通道功率因素
//	printf("F_AC_LINE_Freq = %f Hz \n " ,F_AC_LINE_Freq);		//F_AC_LINE_Freq	
//	printf("F_Angle = %f\n " ,F_Angle);

//	
//	printf("\r\n\r\n");//插入换行
//	printf("电能参数\r\n");
//	printf("U32_RMSIA_RegData = %x\n " ,U32_RMSIA_RegData);
//	printf("U32_RMSIB_RegData = %x\n " ,U32_RMSIB_RegData);
//	printf("U32_RMSU_RegData = %x\n " ,U32_RMSU_RegData);
//	printf("U32_POWERPA_RegData = %x\n " ,U32_POWERPA_RegData);
//	printf("U32_POWERPB_RegData = %x\n " ,U32_POWERPB_RegData);
//	printf("U16_AngleData = %x\n " ,U16_AngleData);
//	printf("U16_PFData = %x\n " ,U16_PFData);
//	printf("U16_LineFData = %x\n " ,U16_LineFData);
//	printf("U16_IFData = %x\n " ,U16_IFData);
//	printf("U16_RIFData = %x\n " ,U16_RIFData);
//	printf("U16_IEData = %x\n " ,U16_IEData);
//	printf("U16_INTData = %x\n " ,U16_INTData);
//	printf("U16_SAGCYCData = %x\n " ,U16_SAGCYCData);
//	printf("U16_SAGLVLData = %x\n " ,U16_SAGLVLData);
//	
//	printf("U16_SYSCONData = %x\n " ,U16_SYSCONData);
//	printf("U16_EMUCONData = %x\n " ,U16_EMUCONData);
//	printf("U16_EMUCON2Data = %x\n " ,U16_EMUCON2Data);
//	
//	
//	
//	printf("\r\n\r\n");//插入换行
//	printf("A通道电流转换系数:%x\n " ,U16_RMSIAC_RegData);
//	printf("B通道电流转换系数:%x\n " ,U16_RMSIBC_RegData);
//	printf("电压通道转换系数:%x\n " ,U16_RMSUC_RegData);
//	printf("A通道功率转换系数:%x\n " ,U16_PowerPAC_RegData);
//	printf("B通道功率转换系数:%x\n " ,U16_PowerPBC_RegData);
//	printf("A通道电量转换系数:%x\n " ,U16_EnergyAC_RegData);
//	printf("B通道电量转换系数:%x\n " ,U16_EnergyBC_RegData);
//	printf("U16_CheckSUM_RegData = %x\n " ,U16_CheckSUM_RegData);
//	printf("U16_CheckSUM_Data = %x\n " ,U16_CheckSUM_Data);
//	


//	printf("----------------------------------------------\r\n");	
//	printf("----------------------------------------------\r\n");	
}


#endif



























