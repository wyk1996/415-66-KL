/*****************************************Copyright(C)******************************************
****************************************************************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			:ModuleYouren.c
* Author			: 
* Date First Issued	: 
* Version			: 
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "ModuleA7680C.h"
#include "4GMain.h"
#include "dwin_com_pro.h"
#include "main.h"
#include "flashdispos.h"
#include "4GMain.h"
#if(UPDATA_STATE)
#define DIM7600_RECV_TABLE_LEN        (23u)
#else
#define DIM7600_RECV_TABLE_LEN        (17u)
#endif
/* Private define-----------------------------------------------------------------------------*/
__packed typedef struct
{
    _4G_STATE SIM_ExistState;                      	//模块是否存在
	_4G_STATE SIM_ConectState[2];					//是否连接到服务器(最大可连接10路)
	_4G_STATE SIM_Regiset[2];						//是否注册成功(最大可连接10路)
	#if(UPDATA_STATE)
	_4G_MODE  Mode;									//4G模式还是HTTP模式
	_4G_STATE HTTP_Start;							//HTTP开始返回
	_4G_STATE HTTP_ConectState;						//HTTP连接状态
	_4G_STATE HTTP_PasswordState;					//HTTP密码登录状态
	_4G_STATE HTTP_Get;								//HTTP获取
	_4G_STATE HTTP_Read;							//HTTP读取
	_4G_STATE HTTP_ReadDone;						//读取完成
	#endif
	
	__packed union
    {
        uint8_t OneByteBits;
        __packed struct
        {
			uint8_t ATCMD_CIMI   	  : 1;			 //读取卡号返回
			uint8_t ATCMD_CSQ  : 1;			 	 //读取信号强度返回
			
			uint8_t ATCMD_CGDCONT   : 1;			 //配置PDP返回
			uint8_t ATCMD_CIPMODE   : 1;			 //数据透传返回
			uint8_t ATCMD_NETOPEN   : 1;			 //激活PDP返回
			uint8_t ATCMD_SETCIPRXGET  : 1;         //接收手动获取
            uint8_t ATCMD_OK        : 1;           //切换倒指令模式
			uint8_t ATCMD_EGRM        : 1;           //读取IMEI
        }OneByte;
	}State0;
	uint8_t ATCMD_CIPOPEN[2];				//连接服务器返回状态
	uint8_t ATCMD_SENDACK[2];				//发送数据确认
}_SIM7600_CMD_STATE;


static uint8_t Send_AT_EnterATCmd(void);
static uint8_t Send_AT_CIPCLOSE(uint8_t num);
static uint8_t Send_AT_NETCLOSE(void);

/* Private typedef----------------------------------------------------------------------------*/
/*static */_SIM7600_CMD_STATE   SIM7600CmdState;		
uint8_t CSQNum = 0;				//信号强度数值

uint8_t SIMNum[15];
uint8_t _4GIMEI[15];
#if(UPDATA_STATE) 
/*****************************************************************************
* Function     :Send_AT_HTTPINIT
* Description  :开始HTTP
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_HTTPINIT(void)
{
	
	if (UART_4GWrite((uint8_t*)"AT+HTTPINIT\r\n", strlen("AT+HTTPINIT\r\n") ) == strlen("AT+HTTPINIT\r\n") )
    {
		return TRUE;	
    }
	return FALSE;
}	


/*****************************************************************************
* Function     :Send_AT_HTTPTERM
* Description  :结束HTTP
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_HTTPTERM(void)
{
	
	if (UART_4GWrite((uint8_t*)"AT+HTTPTERM\r\n", strlen("AT+HTTPTERM\r\n") ) == strlen("AT+HTTPTERM\r\n") )
    {
		return TRUE;	
    }
	return FALSE;
}


/*****************************************************************************
* Function     :Send_AT_HTTPPAR
* Description  :连接HTTP服务器
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_HTTPPAR(char *http_add)
{
	uint8_t buf[100];
	int length;
	length = snprintf((char *)buf, sizeof(buf),"AT+HTTPPARA=\"URL\",\"%s\"\r\n",http_add);
	if (length == -1)
	{
        printf("snprintf error, the len is %d", length);
		return FALSE;
	}
	if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
		return TRUE;	
    }
	return FALSE;
}	

/*****************************************************************************
* Function     :Send_AT_HTTPINIT
* Description  :开始HTTP
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_ReadIMEI(void)
{
	
	if (UART_4GWrite((uint8_t*)"AT+SIMEI?\r\n", strlen("AT+SIMEI?\r\n") ) == strlen("AT+SIMEI?\r\n") )
	//if (UART_4GWrite((uint8_t*)"AT+EGRM=2,7\r\n", strlen("AT+EGRM=2,7\r\n") ) == strlen("AT+EGRM=2,7\r\n") )
    {
		return TRUE;	
    }
	return FALSE;
}	


/*****************************************************************************
* Function     :Send_AT_GET
* Description  :获取数据
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_GET(void)
{
	if (UART_4GWrite((uint8_t*)"AT+HTTPACTION=0\r\n", strlen("AT+HTTPACTION=0\r\n") ) == strlen("AT+HTTPACTION=1\r\n") )
    {
		return TRUE;	
    }
	return FALSE;
}	


/*****************************************************************************
* Function     :Send_AT_READ
* Description  :读取数据
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_READ(uint32_t len)
{
	uint8_t buf[60];
     int length;
	

	length = snprintf((char *)buf, sizeof(buf), "AT+HTTPREAD=0,%d\r\n",len);
	if (length == -1)
	{
        printf("snprintf error, the len is %d", length);
		return FALSE;
	}
	if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
		return TRUE;	
    }
	return FALSE;
}

/*****************************************************************************
* Function     :Send_AT_HTTPPAR_PASSWORD
* Description  :密码登录
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_HTTPPAR_PASSWORD(char *http_pass)
{
	uint8_t buf[100];
	int length;
	length = snprintf((char *)buf, sizeof(buf),"AT+HTTPPARA=\"USERDATA\",\"%s:Basic bWFyczpsb28=\"\r\n",http_pass);
	if (length == -1)
	{
        printf("snprintf error, the len is %d", length);
		return FALSE;
	}
	if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
		return TRUE;	
    }
	return FALSE;
}	

/*****************************************************************************
* Function     :Module_HTTPDownload
* Description  :HTTP下载
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
  uint8_t Module_HTTPDownload(_HTTP_INFO *info)
{
	_RECV_DATA_CONTROL* pcontrol = APP_RecvDataControl(0);
	static uint32_t i = 0;
	static uint32_t downtime = 0,lastdowntime;
	uint32_t  down_len = 0;    //下载的长度
	uint8_t  down_state = 0x55;		//表示升级成功
	
	if(info == NULL)
	{
		return FALSE;
	}
	if(info->ServerAdd[0] == 0)
	{
		return FALSE;
	}
	OSTimeDly(CM_TIME_2_SEC, OS_OPT_TIME_PERIODIC, &timeerr);
	Send_AT_EnterATCmd();
	OSTimeDly(CM_TIME_2_SEC, OS_OPT_TIME_PERIODIC, &timeerr);
	Send_AT_HTTPTERM();
	OSTimeDly(CM_TIME_1_SEC, OS_OPT_TIME_PERIODIC, &timeerr);
	
		//开始http
	Send_AT_HTTPINIT();
	//等待2s回复
	i = 0;
	while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		OSTimeDly(CM_TIME_100_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);
		//等待AT指令被回复
		if (SIM7600CmdState.HTTP_Start== STATE_OK)
		{
			SIM7600CmdState.HTTP_Start = STATE_ERR;
			break;
		}
		i++;
	}
		  //判断是否回复超时
	if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		printf("respond cmd:HTTPINIT timeout");
		return FALSE;
	}
	
	//连接http服务器
	Send_AT_HTTPPAR(info->ServerAdd);
	//等待2s回复
	i = 0;
	while (i < CM_TIME_2_SEC / CM_TIME_50_MSEC)
	{
		OSTimeDly(CM_TIME_50_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);
		//等待AT指令被回复
		if (SIM7600CmdState.HTTP_ConectState== STATE_OK)
		{
			SIM7600CmdState.HTTP_ConectState = STATE_ERR;
			break;
		}
		i++;
	}
		  //判断是否回复超时
	if (i == CM_TIME_2_SEC / CM_TIME_50_MSEC)
	{
		printf("respond cmd:HTTPINIT timeout");
		return FALSE;
	}
	
	//密码登录
	if(info->ServerPassword[0] != 0)   //说明有密码
	{
		//密码验证
		Send_AT_HTTPPAR_PASSWORD(info->ServerPassword);
		i = 0;
		while (i < CM_TIME_2_SEC / CM_TIME_50_MSEC)
		{
			OSTimeDly(CM_TIME_50_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);
			//等待AT指令被回复
			if (SIM7600CmdState.HTTP_PasswordState== STATE_OK)
			{
				SIM7600CmdState.HTTP_PasswordState = STATE_ERR;
				break;
			}
			i++;
		}
			  //判断是否回复超时
		if (i == CM_TIME_2_SEC / CM_TIME_50_MSEC)
		{
			printf("respond cmd:HTTPINIT timeout");
			return FALSE;
		}
	}
	
		//Get
	Send_AT_GET();
	//等待2s回复
	i = 0;
	while (i < CM_TIME_2_SEC / CM_TIME_50_MSEC)
	{
		OSTimeDly(CM_TIME_50_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);
		//等待AT指令被回复
		if (SIM7600CmdState.HTTP_Get== STATE_OK)
		{
			SIM7600CmdState.HTTP_Get = STATE_ERR;
			break;
		}
		i++;
	}
		  //判断是否回复超时
	if (i == CM_TIME_2_SEC / CM_TIME_50_MSEC)
	{
		printf("respond cmd:HTTP GET timeout");
		return FALSE;
	}
	
	if( pcontrol->AllLen > (1024*200) )   //数据总长度不能操作200k
	{
		printf("http data too len");
		return FALSE;
	}
		
	downtime =OSTimeGet(&timeerr);
	lastdowntime = downtime;
	//read
	while(!SIM7600CmdState.HTTP_ReadDone)
	{
		if(down_len > pcontrol->AllLen)
		{
			return FALSE; 
		}
		if(down_len == pcontrol->AllLen)     
		{
			#warning "校验   YXY 20220531"
			#warning "存储   YXY 20220531"

			fal_partition_write(APP_CODE,1,(uint8_t*)&down_len,sizeof(down_len));
			SIM7600CmdState.HTTP_ReadDone = STATE_OK;    //说明下载成功
			fal_partition_write(APP_CODE,0,&down_state,sizeof(down_state));
			break;
		}
		if((pcontrol->AllLen - down_len) > 500)
		{
			Send_AT_READ(500);   //一次一次读取
		}
		else
		{
			Send_AT_READ(pcontrol->AllLen - down_len);   //一次一次读取
		}
		downtime =OSTimeGet(&timeerr);
	
		if(SIM7600CmdState.HTTP_ReadDone== STATE_OK)
		{
			//校验，返回下载成功
			if(down_len == pcontrol->AllLen)     
			{
				fal_partition_write(APP_CODE,1,(uint8_t*)&down_len,sizeof(down_len));
				SIM7600CmdState.HTTP_ReadDone = STATE_OK;    //说明下载成功
				fal_partition_write(APP_CODE,0,&down_state,sizeof(down_state));
			}
			break;
		}
		if((downtime - lastdowntime)  > (CM_TIME_60_SEC * 2))   //2分钟没又下载完成说明超时了
		{
			printf("respond cmd:HTTP READ timeout");
			return FALSE;
		}
		
		
		//等待2s回复
		i = 0;
		while (i < CM_TIME_2_SEC / CM_TIME_50_MSEC)
		{
			OSTimeDly(CM_TIME_50_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);
			//等待AT指令被回复
			if (SIM7600CmdState.HTTP_Read== STATE_OK)
			{
				SIM7600CmdState.HTTP_Read = STATE_ERR;
				
				//数据存储
				fal_partition_write(APP_CODE,down_len + 5,pcontrol->DataBuf,pcontrol->DownPackLen);
				down_len += pcontrol->DownPackLen;
				break;
			}
			i++;
		}
			  //判断是否回复超时
		if (i >= (CM_TIME_2_SEC / CM_TIME_50_MSEC) )
		{
			printf("respond cmd:HTTP READ timeout");
			return FALSE;
		}
	}
	Send_AT_HTTPTERM();
	OSTimeDly(CM_TIME_1_SEC, OS_OPT_TIME_PERIODIC, &timeerr);
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetSIM7600Mode
* Description  : 获取4g当前模式
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
uint8_t APP_SetSIM7600Mode(_4G_MODE mode)
{
	SIM7600CmdState.Mode = mode;
	return TRUE;
}


/*****************************************************************************
* Function     : APP_GetSIM7600Mode
* Description  : 获取4g当前模式
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
_4G_MODE APP_GetSIM7600Mode(void)
{
	return SIM7600CmdState.Mode;
}
#endif



/*****************************************************************************
* Function     : APP_GetCSQNum
* Description  : 获取型号强度值
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t APP_GetCSQNum(void)
{
    return CSQNum;
}




//以下是AT指令发送处理
/*****************************************************************************
* Function     : SIM7600Reset
* Description  : 模块复位
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t SIM7600Reset()
{
	uint8_t i;
	memset(&SIM7600CmdState,0,sizeof(SIM7600CmdState));	
	for(i = 0;i < NetConfigInfo[NET_YX_SELCT].NetNum;i++)
	{
		Send_AT_CIPCLOSE(i);
	}
	Send_AT_NETCLOSE();
	
	printf("4G reset \r\n");
	
	_4G_POWER_OFF;
	 OSTimeDly(5000, OS_OPT_TIME_PERIODIC, &timeerr);
	_4G_POWER_ON;
	 OSTimeDly(200, OS_OPT_TIME_PERIODIC, &timeerr);
	
	_4G_PWKEY_OFF;
	_4G_RET_OFF;
	OSTimeDly(CM_TIME_250_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
	//开机
	_4G_PWKEY_ON;
	OSTimeDly(CM_TIME_500_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
	_4G_PWKEY_OFF;
	OSTimeDly(CM_TIME_500_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
	//复位
	_4G_RET_ON;
	OSTimeDly(CM_TIME_3_SEC, OS_OPT_TIME_PERIODIC, &timeerr);	
	_4G_RET_OFF;
	OSTimeDly(CM_TIME_500_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
//	
	OSTimeDly(CM_TIME_10_SEC, OS_OPT_TIME_PERIODIC, &timeerr);	
	OSTimeDly(2000, OS_OPT_TIME_PERIODIC, &timeerr);
	return FALSE;
}

/*****************************************************************************
* Function     : SIM7600CloseNet
* Description  : 关闭网络
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
uint8_t SIM7600CloseNet(uint8_t num)
{
	if(num > 10)
	{
		return FALSE;
	}		
	
	Send_AT_CIPCLOSE(num);
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetSIM7600Status
* Description  : 获取模块是否存在
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
_4G_STATE APP_GetSIM7600Status(void)
{
	return SIM7600CmdState.SIM_ExistState;
}



/*****************************************************************************
* Function     : APP_GetModuleConnectState
* Description  : 连接服务器状态,最大可连接10路
* Input        :     
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_STATE APP_GetModuleConnectState(uint8_t num)
{
	if(num > 10)
	{
		return STATE_ERR;
	}
	return SIM7600CmdState.SIM_ConectState[num];
}

/*****************************************************************************
* Function     : APP_GetAppRegisterState
* Description  : 注册是否成功,最大可连接10路
* Input        :     
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_STATE APP_GetAppRegisterState(uint8_t num)
{
	if(num > 10)
	{
		return STATE_ERR;
	}
	return SIM7600CmdState.SIM_Regiset[num];
}

/*****************************************************************************
* Function     : APP_GetAppRegisterState
* Description  : 注册是否成功,最大可连接10路
* Input        :     
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t  APP_SetAppRegisterState(uint8_t num,_4G_STATE state)
{
	if(num > 10)
	{
		return FALSE;
	}
	 SIM7600CmdState.SIM_Regiset[num] = state;
}


/*****************************************************************************
* Function     : Send_AT_CIPCLOSE
* Description  : 关闭服务器
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_CIPCLOSE(uint8_t num)
{
	uint8_t buf[60];
	int length;
	 if(num> 10)
	 {
		 return FALSE;
	 }

	length = snprintf((char *)buf, sizeof(buf), "AT+CIPCLOSE=%d\r\n",num);
	if (length == -1)
	{
        printf("snprintf error, the len is %d", length);
		return FALSE;
	}
	if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {	
		return TRUE;
    }
	
    return FALSE;
}	


/*****************************************************************************
* Function     : Send_AT_NETCLOSE
* Description  : 去除pdp
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_NETCLOSE(void)
{
	if (UART_4GWrite((uint8_t*)"AT+NETCLOSE\r\n", strlen("AT+NETCLOSE\r\n") ) == strlen("AT+NETCLOSE\r\n") )
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : Send_AT_EnterATCmd
* Description  : 进入AT配置指令1
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_EnterATCmd(void)
{
	if (UART_4GWrite((uint8_t*)"+++", strlen("+++") ) == strlen("+++") )
    {	
        return TRUE;
    }
    return FALSE;
}
/*****************************************************************************
* Function     : Send_AT_CSQ
* Description  : 读取信号强度
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Send_AT_CSQ(void)
{
	if (UART_4GWrite((uint8_t*)"AT+CSQ\r\n", strlen("AT+CSQ\r\n") ) == strlen("AT+CSQ\r\n") )
    {
        return TRUE;
    }
    return FALSE;
}	

/*****************************************************************************
* Function     : Send_AT_CIMI
* Description  : 读取卡号
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_CIMI(void)
{
	if (UART_4GWrite((uint8_t*)"AT+CIMI\r\n", strlen("AT+CIMI\r\n") ) == strlen("AT+CIMI\r\n") )
	//if(BSP_UARTWrite(GPRS_UART,(uint8_t*)"AT+CIMI\r\n",strlen("AT+CIMI\r\n")))
    {
        return TRUE;
    }
    return FALSE;
}	
/*****************************************************************************
* Function     : Send_AT_CGDCONT
* Description  : 配置PDP
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_CGDCONT(void)
{
	if (UART_4GWrite((uint8_t*)"AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n", strlen("AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n") ) == strlen("AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n") )
    {
        return TRUE;
    }
    return FALSE;
}	
/*****************************************************************************
* Function     : Send_AT_CIPMODE
* Description  : 数据透传
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Send_AT_CIPMODE(void)
{
	if (UART_4GWrite((uint8_t*)"AT+CIPMODE=1\r\n", strlen("AT+CIPMODE=1\r\n") ) == strlen("AT+CIPMODE=1\r\n") )
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : Send_AT_NOTCIPMODE
* Description  : 非数据透传
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_NOTCIPMODE(void)
{
	if (UART_4GWrite((uint8_t*)"AT+CIPMODE=0\r\n", strlen("AT+CIPMODE=0\r\n") ) == strlen("AT+CIPMODE=0\r\n") )
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : Send_AT_CIPRXGET
* Description  : 接收手动获取
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_SetCIPRXGET(void)
{
	if (UART_4GWrite((uint8_t*)"AT+CIPRXGET=1\r\n", strlen("AT+CIPRXGET=1\r\n") ) == strlen("AT+CIPMODE=0\r\n") )
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : Send_AT_NETOPEN
* Description  : 激活PDP
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_NETOPEN(void)
{
	if (UART_4GWrite((uint8_t*)"AT+NETOPEN\r\n", strlen("AT+NETOPEN\r\n") ) == strlen("AT+NETOPEN\r\n") )
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : Send_AT_CIPSEND
* Description  : 发送数据请求
* Input        :num  哪个socket 
				len   数据长度
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_CIPSEND(uint8_t num,uint8_t len)
{
	uint8_t buf[60];
	int length;

	length = snprintf((char *)buf, sizeof(buf), "AT+CIPSEND=%d,%d\r\n",num,len);
	if (length == -1)
	{
        printf("snprintf error, the len is %d", length);
		return FALSE;
	}
	if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
		return TRUE;
    }
	return FALSE;
}

/*****************************************************************************
* Function     : Send_AT_CIPRXGET
* Description  : 读取数据请求
* Input        :num  哪个socket 
				len   数据长度
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Send_AT_CIPRXGET(uint8_t num)
{
	uint8_t buf[60];
	int length;

	length = snprintf((char *)buf, sizeof(buf), "AT+CIPRXGET=2,%d\r\n",num);
	if (length == -1)
	{
        printf("snprintf error, the len is %d", length);
		return FALSE;
	}
	if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
		return TRUE;
    }
	return FALSE;
}

/*****************************************************************************
* Function     :Module_ConnectServer
* Description  :连接服务器
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Module_ConnectServer(uint8_t num,uint8_t* pIP,uint16_t port)
{
	uint8_t buf[60];
     int length;
	 if((pIP == NULL) || (num> 10))
	 {
		 return FALSE;
	 }

	length = snprintf((char *)buf, sizeof(buf), "AT+CIPOPEN=%d,\"TCP\",\"%s\",%d\r\n",num,pIP,port);
	 //length = snprintf((char *)buf, sizeof(buf), "AT+CIPOPEN=%d,\"TCP\",www.baidu.com\r\n",num);
	if (length == -1)
	{
        printf("snprintf error, the len is %d", length);
		return FALSE;
	}
	if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
		return TRUE;	
    }
	return FALSE;
}	



//以下是AT指令接收数据处理
/*****************************************************************************
* Function     : Recv_AT_EnterATCmd_Ack
* Description  : 进入AT配置指令返回
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_EnterATCmd_Ack(uint8_t *pdata, uint16_t len)
{
	if (UART_4GWrite((uint8_t*)"+++", strlen("+++") ) == strlen("+++") )
    {	
        return TRUE;
    }
    return FALSE;
}	


/*****************************************************************************
* Function     : Recv_AT_CSQ_Ack
* Description  : 读取信号强度返回
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_CSQ_Ack(uint8_t *pdata, uint16_t len)
{
	
	if ( (pdata == NULL) || len < 16)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
	if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
	} 
	
	CSQNum = atoi((const char *)&pdata[14]);
	if((CSQNum == 99) || (CSQNum == 0) )
	{
		//无信号
		return FALSE;
	}
	
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.State0.OneByte.ATCMD_CSQ = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}	

/*****************************************************************************
* Function     : Recv_AT_IMEI_Ack
* Description  : 读取卡号返回
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_IMEI_Ack(uint8_t *pdata, uint16_t len)
{
	char * poffset;
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
	if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
    } 
	
	poffset = strtok((char *)pdata," ");
	if(poffset == NULL)
	{
		return FALSE;
	}
	if(len < 35)
	{
		return FALSE;
	}
	memcpy(_4GIMEI,&pdata[20],15);
//	memcpy(SIMNum,&pdata[10],15);
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.State0.OneByte.ATCMD_EGRM = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}	


/*****************************************************************************
* Function     : Recv_AT_CIMI_Ack
* Description  : 读取卡号返回
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_CIMI_Ack(uint8_t *pdata, uint16_t len)
{
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
	if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
    } 
	if(len < 25)
	{
		return FALSE;
	}
	memcpy(SIMNum,&pdata[10],15);
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.State0.OneByte.ATCMD_CIMI = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}	

/*****************************************************************************
* Function     : Recv_AT_CGDCONT_Ack
* Description  : 配置PDP返回
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_CGDCONT_Ack(uint8_t *pdata, uint16_t len)
{
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
//	if (strstr((char *)pdata, "OK") == NULL)
//    {
//        return FALSE;
//    } 
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.State0.OneByte.ATCMD_CGDCONT = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}	
/*****************************************************************************
* Function     : Recv_AT_CIPMODE_Ack
* Description  : 数据透传返回
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_CIPMODE_Ack(uint8_t *pdata, uint16_t len)
{
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
//	if (strstr((char *)pdata, "OK") == NULL)
//    {
//        return FALSE;
//    } 
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.State0.OneByte.ATCMD_CIPMODE = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}

/*****************************************************************************
* Function     : Recv_AT_NETOPEN_Ack
* Description  : 激活PDP返回
* Input        : uint8_t *pdata  
                 uint16_t len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_NETOPEN_Ack(uint8_t *pdata, uint16_t len)
{
	
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
//	if (strstr((char *)pdata, "OK") == NULL)
//    {
//        return FALSE;
//    } 
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.State0.OneByte.ATCMD_NETOPEN = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}


/*****************************************************************************
* Function     :Recv_AT_ConnectServer_Ack
* Description  :连接服务器返回
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ConnectServer0_Ack(uint8_t *pdata, uint16_t len)
{
//	AT+CIPOPEN=0,"TCP","122.114.122.174",33870
//	OK

//	+CIPOPEN: 0,0

	uint8_t res;
	char *poffset;
	
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
	if (strstr((char *)pdata, "OK") != NULL)
    {
        OSSchedLock(&ERRSTATE);
		SIM7600CmdState.ATCMD_CIPOPEN[0] = _4G_RESPOND_OK;
		OSSchedUnlock(&ERRSTATE);
		return TRUE;
    } 
	
	poffset = strtok((char *)pdata, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	res = atoi(poffset);
	//0表示新连接上，4表示意见连接上了
	if((res != 0) || (res != 4))
	{
		return FALSE;
	}
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.ATCMD_CIPOPEN[0] = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}	

/*****************************************************************************
* Function     :Recv_AT_ConnectServer_Ack
* Description  :连接服务器返回
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ConnectServer1_Ack(uint8_t *pdata, uint16_t len)
{
	//	AT+CIPOPEN=0,"TCP","122.114.122.174",33870
//	OK

//	+CIPOPEN: 0,0

	uint8_t res;
	char *poffset;
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
	if (strstr((char *)pdata, "OK") != NULL)
    {
        OSSchedLock(&ERRSTATE);
		SIM7600CmdState.ATCMD_CIPOPEN[1] = _4G_RESPOND_OK;
		OSSchedUnlock(&ERRSTATE);
		return TRUE;
    } 
	
	poffset = strtok((char *)pdata, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	res = atoi(poffset);
	//0表示新连接上，4表示意见连接上了
	if((res != 0) || (res != 4))
	{
		return FALSE;
	}
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.ATCMD_CIPOPEN[1] = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}	


/*****************************************************************************
* Function     :Recv_AT_CmdConnectServer0_Ack
* Description  :连接服务器指令返回
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ConnectServer0Cmd_Ack(uint8_t *pdata, uint16_t len)
{
	//	AT+CIPOPEN=0,"TCP","122.114.122.174",33870
//	OK

//	+CIPOPEN: 0,0

	uint8_t res;
	char *poffset;
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}

	pdata[len] = '\0';  //结束符
	if(NetConfigInfo[NET_YX_SELCT].NetNum == 1)
	{
		{
			if (strstr((char *)pdata, "ERROR") == NULL)  //没有错误就返回注册成功
			{
				OSSchedLock(&ERRSTATE);
				SIM7600CmdState.ATCMD_CIPOPEN[0] = _4G_RESPOND_OK;
				OSSchedUnlock(&ERRSTATE);
				return TRUE;
			} 
			return FALSE;
		}
	}
	else
	{
		if (strstr((char *)pdata, "ERROR") != NULL)
		{
			return FALSE;
		} 
		
		if (strstr((char *)pdata, "OK") != NULL)
		{
			OSSchedLock(&ERRSTATE);
			SIM7600CmdState.ATCMD_CIPOPEN[0] = _4G_RESPOND_OK;
			OSSchedUnlock(&ERRSTATE);
			return TRUE;
		} 
		
		poffset = strtok((char *)pdata, ",");
		if(poffset == NULL)
		{
			return FALSE;
		}
		
		poffset = strtok(NULL, ",");
		if(poffset == NULL)
		{
			return FALSE;
		}
		
		poffset = strtok(NULL, ",");
		if(poffset == NULL)
		{
			return FALSE;
		}
		
		poffset = strtok(NULL, ",");
		if(poffset == NULL)
		{
			return FALSE;
		}
		res = atoi(poffset);
		//0表示新连接上，4表示意见连接上了
		if((res != 0) || (res != 4))
		{
			return FALSE;
		}
		OSSchedLock(&ERRSTATE);
		SIM7600CmdState.ATCMD_CIPOPEN[0] = _4G_RESPOND_OK;
		OSSchedUnlock(&ERRSTATE);
	}
	return TRUE;
}

/*****************************************************************************
* Function     :Recv_AT_CmdConnectServer0_Ack
* Description  :连接服务器指令返回
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ConnectServer1Cmd_Ack(uint8_t *pdata, uint16_t len)
{
		//	AT+CIPOPEN=0,"TCP","122.114.122.174",33870
//	OK

//	+CIPOPEN: 0,0


	uint8_t res;
	char *poffset;
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
	if (strstr((char *)pdata, "OK") != NULL)
    {
        OSSchedLock(&ERRSTATE);
		SIM7600CmdState.ATCMD_CIPOPEN[1] = _4G_RESPOND_OK;
		OSSchedUnlock(&ERRSTATE);
		return TRUE;
    } 
	
	if (strstr((char *)pdata, "ERROR") != NULL)
    {
		return FALSE;
    } 
	
	poffset = strtok((char *)pdata, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	res = atoi(poffset);
	//0表示新连接上，4表示意见连接上了
	if((res != 0) || (res != 4))
	{
		return FALSE;
	}
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.ATCMD_CIPOPEN[1] = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}
/*****************************************************************************
* Function     :Recv_AT_SetReAct_Ack
* Description  :设置接收主动获取
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_SetReAct_Ack(uint8_t *pdata, uint16_t len)
{
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
//	if (strstr((char *)pdata, "OK") == NULL)
//    {
//        return FALSE;
//    } 
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.State0.OneByte.ATCMD_SETCIPRXGET = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}


/*****************************************************************************
* Function     :Recv_AT_SendAck0Cmd_Ack
* Description  :发送数据确认
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_SendAck0Cmd_Ack(uint8_t *pdata, uint16_t len)
{
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	//
	pdata[len] = '\0';  //结束符
//	if ((strstr((char *)pdata, ">") == NULL) )
//    {
//		OSSchedLock(ERRSTATE)();
//		SIM7600CmdState.ATCMD_SENDACK[0] = _4G_RESPOND_AGAIN;
//		OSSchedUnlock();
//        return FALSE;
//    } 
	if ((strstr((char *)pdata, "ERROR") != NULL) )
    {
		OSSchedLock(&ERRSTATE);
		SIM7600CmdState.ATCMD_SENDACK[0] = _4G_RESPOND_AGAIN;
		OSSchedUnlock(&ERRSTATE);
        return FALSE;
    } 
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.ATCMD_SENDACK[0] = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}

/*****************************************************************************
* Function     :Recv_AT_SendAck0Cmd_Ack
* Description  :发送数据确认
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_SendAck1Cmd_Ack(uint8_t *pdata, uint16_t len)
{
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	//
	pdata[len] = '\0';  //结束符
	//	if ((strstr((char *)pdata, ">") == NULL) )
//    {
//		OSSchedLock(ERRSTATE)();
//		SIM7600CmdState.ATCMD_SENDACK[1] = _4G_RESPOND_AGAIN;
//		OSSchedUnlock(ERRSTATE)();
//        return FALSE;
//    } 
	if ((strstr((char *)pdata, "ERROR") != NULL) )
    {
		OSSchedLock(&ERRSTATE);
		SIM7600CmdState.ATCMD_SENDACK[1] = _4G_RESPOND_AGAIN;
		OSSchedUnlock(&ERRSTATE);
        return FALSE;
    } 
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.ATCMD_SENDACK[1] = _4G_RESPOND_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}



/*****************************************************************************
* Function     :Recv_AT_SendAck0Cmd_Ack
* Description  :发送数据确认
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ReRecv0Cmd_Ack(uint8_t *pdata, uint16_t len)
{

	_RECV_DATA_CONTROL* pcontrol = APP_RecvDataControl(0);
	if(pcontrol == NULL)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
	if ((strstr((char *)pdata, "+IP ERROR: No data") != NULL) )
    {
		//表示无数据
        return FALSE;
    } 
//	AT+CIPRXGET=2,0
//	+CIPRXGET: 2,0,6,0
//	123456
	
//	OK
//	if ((strstr((char *)pdata, "OK") == NULL) )
//   {
//		//表示无数据
//       return FALSE;
//   } 
	char *poffset = strtok((char *)pdata + 18, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");  //为数据长度
	if(poffset == NULL)
	{
		return FALSE;
	}
	pcontrol->len = atoi(poffset);
	if(pcontrol->len > 500)
	{
		return FALSE;
	}
	//数据拷贝
	if(pcontrol->len < 10)
	{
		memcpy(pcontrol->DataBuf,poffset+5,pcontrol->len);
	}
	else if(pcontrol->len < 100)
	{
		memcpy(pcontrol->DataBuf,poffset+6,pcontrol->len);
	}
	else
	{
		memcpy(pcontrol->DataBuf,poffset+7,MIN(pcontrol->len,sizeof(pcontrol->DataBuf)));
	}
	pcontrol->RecvStatus = RECV_FOUND_DATA;  //有数据了，因为这个变量再同一个任务，故不要加锁
	return TRUE;
}

/*****************************************************************************
* Function     :Recv_AT_SendAck0Cmd_Ack
* Description  :发送数据确认
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ReRecv1Cmd_Ack(uint8_t *pdata, uint16_t len)
{
	_RECV_DATA_CONTROL* pcontrol = APP_RecvDataControl(1);
	if(pcontrol == NULL)
	{
		return FALSE;
	}
	pdata[len] = '\0';  //结束符
	if ((strstr((char *)pdata, "+IP ERROR: No data") != NULL) )
    {
		//表示无数据
        return FALSE;
    } 
//	AT+CIPRXGET=2,0
//	+CIPRXGET: 2,0,6,0
//	123456
	
//	OK
//	if ((strstr((char *)pdata, "OK") == NULL) )
//   {
//		//表示无数据
//       return FALSE;
//   } 
	char *poffset = strtok((char *)pdata + 18, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");  //为数据长度
	if(poffset == NULL)
	{
		return FALSE;
	}
	pcontrol->len = atoi(poffset);
	if(pcontrol->len > 500)
	{
		return FALSE;
	}
	//数据拷贝
	if(pcontrol->len < 10)
	{
		memcpy(pcontrol->DataBuf,poffset+5,pcontrol->len);
	}
	else if(pcontrol->len < 100)
	{
		memcpy(pcontrol->DataBuf,poffset+6,pcontrol->len);
	}
	else
	{
		memcpy(pcontrol->DataBuf,poffset+7,MIN(pcontrol->len,sizeof(pcontrol->DataBuf)));
	}
	pcontrol->RecvStatus = RECV_FOUND_DATA;  //有数据了，因为这个变量再同一个任务，故不要加锁
	return TRUE;
}



/*****************************************************************************
* Function     :Recv_ActRecv0_Ack
* Description  ：有数据接收
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_ActRecv0_Ack(uint8_t *pdata, uint16_t len)
{
	pdata = pdata;
	len = len;
	
	Send_AT_CIPRXGET(0);		//去读取数据
	return TRUE;
}

/*****************************************************************************
* Function     :Recv_ActRecv1_Ack
* Description  ：有数据接收
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_ActRecv1_Ack(uint8_t *pdata, uint16_t len)
{
	pdata = pdata;
	len = len;
	
	Send_AT_CIPRXGET(1);		//去读取数据
    return TRUE;
}
#if(UPDATA_STATE)
/*****************************************************************************
* Function     :Recv_ActRecv1_Ack
* Description  ：有数据接收
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_HttpStart_Ack(uint8_t *pdata, uint16_t len)
{	
	
//[09:13:46.037]发→◇AT+HTTPINIT
//□
//[09:13:46.048]收←◆AT+HTTPINIT
//OK

	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	//
	pdata[len] = '\0';  //结束符
	if ((strstr((char *)pdata, "OK") == NULL) )
	{
		//表示不成功
		return FALSE;
	}

    OSSchedLock(&ERRSTATE);
	SIM7600CmdState.HTTP_Start = STATE_OK;
    OSSchedUnlock(&ERRSTATE);
	
	return TRUE;
}

/*****************************************************************************
* Function     :Recv_HttpConect_Ack
* Description  ：连接返回
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_HttpConect_Ack(uint8_t *pdata, uint16_t len)
{	
	
//[09:14:48.781]发→◇AT+HTTPPARA="URL","http://14.205.92.144/dowload/project.hex"
//□
//[09:14:48.798]收←◆AT+HTTPPARA="URL","http://14.205.92.144/dowload/project.hex"
//OK
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	//
	pdata[len] = '\0';  //结束符
	if ((strstr((char *)pdata, "OK") == NULL) )
	{
		//表示不成功
		return FALSE;
	} 
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.HTTP_ConectState = STATE_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}

/*****************************************************************************
* Function     :Recv_HttpConect_Ack
* Description  ：连接返回
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_HttpGet_Ack(uint8_t *pdata, uint16_t len)
{	
//[09:15:18.284]发→◇AT+HTTPACTION=0
//□
//[09:15:18.291]收←◆AT+HTTPACTION=0
//OK

//[09:15:18.654]收←◆
//+HTTPACTION: 0,200,440836
	_RECV_DATA_CONTROL* pcontrol = APP_RecvDataControl(0);
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	//
	pdata[len] = '\0';  //结束符
	
//	if ((strstr((char *)pdata, "OK") == NULL) )
//	{
//		//表示不成功
//		return FALSE;
//	} 
	
	char *poffset = strtok((char *)pdata, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	poffset = strtok(NULL, ",");
	if(poffset == NULL)
	{
		return FALSE;
	}
	pcontrol->AllLen = atoi((char*)poffset);
	if(pcontrol->AllLen == 0)
	{
		return FALSE;
	}
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.HTTP_Get = STATE_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}



/*****************************************************************************
* Function     :Recv_ReadData_Ack
* Description  ：读取数据
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_ReadData_Ack(uint8_t *pdata, uint16_t len)
{	
//AT+HTTPREAD=0,1000
//OK

//+HTTPREAD: 1000
	char * pchar  = "+HTTPREAD:";
	uint16_t datalen;
	_RECV_DATA_CONTROL* pcontrol = APP_RecvDataControl(0);
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	//
	pdata[len] = '\0';  //结束符
	
	if ((strstr((char *)pdata, "OK") == NULL) )
	{
		//表示不成功
		return FALSE;
	} 
	char *poffset;
	
	
	//38
	poffset = pdata + 14;
	
	
	datalen = atoi((char*)poffset);
	if((datalen == 0) || (datalen > 1024) )
	{
		return FALSE;
	}
	if (datalen >= 1000)
	{
		poffset += 14;
	}
	else if (datalen >= 100)
	{
		poffset += 13;
	}
	else if (datalen >= 10)
	{
		poffset += 12;
	}
	else
	{
		poffset += 11;
	}
	
	
	
	
	
	
	
	datalen = 0;
	
	
	poffset += strlen(pchar);

	datalen= atoi((char*)poffset);
	if((datalen == 0) || (datalen > 1024) )
	{
		return FALSE;
	}
	pcontrol->len = datalen;
	if (pcontrol->len >= 1000)
	{
		poffset += 6;
	}
	else if (pcontrol->len >= 100)
	{
		poffset += 5;
	}
	else if (pcontrol->len >= 10)
	{
		poffset += 4;
	}
	else
	{
		poffset += 3;
	}
	
	if (((uint32_t)poffset - (uint32_t)pdata + pcontrol->len + 16) != len)
	{
		return FALSE;
	}
	memcpy(pcontrol->DataBuf,poffset,pcontrol->len);   //拷贝数据
	pcontrol->DownPackLen = pcontrol->len;           //单包的长度
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.HTTP_Read = STATE_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}

/*****************************************************************************
* Function     :Recv_HttpConect_Ack
* Description  ：连接返回
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_ReadDone_Ack(uint8_t *pdata, uint16_t len)
{	
//[15:51:43.864]收←◆
//+HTTPREAD: 0
	OSSchedLock(&ERRSTATE);
	SIM7600CmdState.HTTP_ReadDone = STATE_OK;
	OSSchedUnlock(&ERRSTATE);
	return TRUE;
}
#endif
//GPRS接收表
static const _4G_AT_FRAME 	TIM7600_RecvFrameTable[DIM7600_RECV_TABLE_LEN] = 
{

	{"AT+CSQ"						,      Recv_AT_CSQ_Ack							},		//读取信号强度返回
	
	{"AT+CIMI"         			    ,  	   Recv_AT_CIMI_Ack							},		//读取卡号返回
	
	{"AT+SIMEI"						,      Recv_AT_IMEI_Ack							},		//读取IMEI
	
	{"AT+CGDCONT"                   ,      Recv_AT_CGDCONT_Ack                    	},		//配置PDP返回
	
	{"AT+CIPMODE"                 	,      Recv_AT_CIPMODE_Ack                       	},		//数据透传返回
	
	{"AT+NETOPEN"                 	,      Recv_AT_NETOPEN_Ack                       	},		//激活PDP
	
	{"AT+CIPOPEN=0"                 ,      Recv_AT_ConnectServer0Cmd_Ack               },		//连接服务器指令返回
	
	{"AT+CIPOPEN=1"                 ,      Recv_AT_ConnectServer1Cmd_Ack               },		//连接服务器指令返回
	
	
	{"+CIPOPEN: 0"                 ,      Recv_AT_ConnectServer0_Ack               },		//连接服务器返回
	
	{"+CIPOPEN: 1"                 ,      Recv_AT_ConnectServer1_Ack               },		//连接服务器返回
	
	{"OK"							,      Recv_AT_EnterATCmd_Ack					},		//进入AT配置指令返回
	
	{"AT+CIPSEND=0"					,      Recv_AT_SendAck0Cmd_Ack					},		//发送数据确认
	
	{"AT+CIPSEND=1"					,      Recv_AT_SendAck1Cmd_Ack					},		//发送数据确认
	
	{"AT+CIPRXGET=2,0"				,      Recv_AT_ReRecv0Cmd_Ack					},		//接收返回
	
	{"AT+CIPRXGET=2,1"				,      Recv_AT_ReRecv1Cmd_Ack					},		//接收返回
	
	{"AT+CIPRXGET=1"				,      Recv_AT_SetReAct_Ack						},		//设置接收主动获取
	
	{"\r\n+CIPRXGET: 1,0"			,	   Recv_ActRecv0_Ack						},		//0号主动接收数据
	
	{"\r\n+CIPRXGET: 1,1"			,	   Recv_ActRecv1_Ack						},		//1号主动接收数据
	#if(UPDATA_STATE)
		//HTTP
	{"AT+HTTPINIT"					,		Recv_HttpStart_Ack						},		//开始返回
	
	{"AT+HTTPPARA=\"URL\""			,		Recv_HttpConect_Ack						},		//注册返回
	
	{"\r\n+HTTPACTION:"				,		Recv_HttpGet_Ack						},		//读取数据返回长度
	
	{"\r\n+HTTPREAD: 0"				,		Recv_ReadDone_Ack							},		//读取完成
	
	{"AT+HTTPREAD=0"				,		Recv_ReadData_Ack							},		//读取数据
	#endif
};



/*****************************************************************************
* Function     :Module_SIM7600Test
* Description  :模块是否存在
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Module_SIM7600Test(void)
{
    static uint8_t i = 0;
	uint8_t err;
	static uint8_t count = 1;
    
	//读取卡号
	Send_AT_CIMI();
	//等待2s回复
	i = 0;
	while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		OSTimeDly(CM_TIME_100_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
		//等待AT指令被回复
		if (SIM7600CmdState.State0.OneByte.ATCMD_CIMI == _4G_RESPOND_OK)
		{
			SIM7600CmdState.State0.OneByte.ATCMD_CIMI = _4G_RESPOND_ERROR;
			break;
		}
		i++;
	}
	  //判断是否回复超时
	if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		printf("respond cmd:ATCMD_CIMI timeout");
		return FALSE;
	}
	//读取IMEI
	Send_AT_ReadIMEI();
	//等待2s回复
	i = 0;
	while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		OSTimeDly(CM_TIME_100_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
		//等待AT指令被回复
		if (SIM7600CmdState.State0.OneByte.ATCMD_EGRM == _4G_RESPOND_OK)
		{
			SIM7600CmdState.State0.OneByte.ATCMD_EGRM = _4G_RESPOND_ERROR;
			break;
		}
		i++;
	}
	  //判断是否回复超时
	if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		printf("respond cmd:ATCMD_CIMI timeout");
		return FALSE;
	}
	
	
	
	//读取信号强度
	Send_AT_CSQ();
	//等待2s回复
	i = 0;
	while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		OSTimeDly(CM_TIME_100_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
		//等待AT指令被回复
		if (SIM7600CmdState.State0.OneByte.ATCMD_CSQ == _4G_RESPOND_OK)
		{
			SIM7600CmdState.State0.OneByte.ATCMD_CSQ = _4G_RESPOND_ERROR;
			break;
		}
		i++;
	}
	//判断是否回复超时
	if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		printf("respond cmd:ATCMD_CSQ timeout");
		return FALSE;
	}
	
	//配置pdp
	Send_AT_CGDCONT();
		//等待2s回复
	i = 0;
	while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		OSTimeDly(CM_TIME_100_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
		//等待AT指令被回复
		if (SIM7600CmdState.State0.OneByte.ATCMD_CGDCONT == _4G_RESPOND_OK)
		{
			SIM7600CmdState.State0.OneByte.ATCMD_CGDCONT = _4G_RESPOND_ERROR;
			break;
		}
		i++;
	}
	  //判断是否回复超时
	if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		printf("respond cmd:ATCMD_CGDCONT timeout");
		return FALSE;
	}
	
	//设置为透传模式
	if(NetConfigInfo[NET_YX_SELCT].NetNum == 1)
	{
	
		Send_AT_CIPMODE();
	}
	else
	{
		//设置数据为非透传
		Send_AT_NOTCIPMODE();
	}
	OSTimeDly(CM_TIME_500_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
	i = 0;
	while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		OSTimeDly(CM_TIME_100_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
		//等待AT指令被回复
		if (SIM7600CmdState.State0.OneByte.ATCMD_CIPMODE == _4G_RESPOND_OK)
		{
			SIM7600CmdState.State0.OneByte.ATCMD_CIPMODE = _4G_RESPOND_ERROR;
			break;
		}
		i++;
	}
	  //判断是否回复超时
	if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
	{
		printf("respond cmd:ATCMD_CIPMODE timeout");
		return FALSE;
	}

	if(NetConfigInfo[NET_YX_SELCT].NetNum > 1)
	{
		Send_AT_SetCIPRXGET(); //接收手动获取
		i = 0;
		while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
		{
			OSTimeDly(CM_TIME_100_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
			//等待AT指令被回复
			if (SIM7600CmdState.State0.OneByte.ATCMD_SETCIPRXGET == _4G_RESPOND_OK)
			{
				SIM7600CmdState.State0.OneByte.ATCMD_SETCIPRXGET = _4G_RESPOND_ERROR;
				break;
			}
			i++;
		}
		  //判断是否回复超时
		if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
		{
			printf("respond cmd:ATCMD_CIPMODE timeout");
			return FALSE;
		}
	}
	
	SIM7600CmdState.SIM_ExistState = STATE_OK;		//模块存在

	return TRUE;
}

/*****************************************************************************
* Function     :ModuleSIM7600_ConnectServer
* Description  :连接服务器
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t ModuleSIM7600_ConnectServer(uint8_t num,uint8_t* pIP,uint16_t port)
{
	static uint8_t i = 0;
	//激活PDP
	Send_AT_NETOPEN();		
	i = 0;
	while (i < (CM_TIME_2_SEC / CM_TIME_100_MSEC) )
	{
		OSTimeDly(CM_TIME_100_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);	
		//等待AT指令被回复
		if (SIM7600CmdState.State0.OneByte.ATCMD_NETOPEN == _4G_RESPOND_OK)
		{
			SIM7600CmdState.State0.OneByte.ATCMD_NETOPEN = _4G_RESPOND_ERROR;
			break;
		}
		i++;
	}
	  //判断是否回复超时
	if (i >= (CM_TIME_2_SEC / CM_TIME_100_MSEC) )
	{
		printf("respond cmd:ATCMD_NETOPEN timeout");
		return FALSE;
	}
	
	
	Module_ConnectServer(num,pIP,port);		
	i = 0;
	while (i < (CM_TIME_5_SEC / CM_TIME_100_MSEC) )
	{
		
		OSTimeDly(CM_TIME_100_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);
		//等待AT指令被回复
		if (SIM7600CmdState.ATCMD_CIPOPEN[num] == _4G_RESPOND_OK)
		{
			SIM7600CmdState.ATCMD_CIPOPEN[num] = _4G_RESPOND_ERROR;
			break;
		}
		i++;
	}
	  //判断是否回复超时
	if (i >= (CM_TIME_5_SEC / CM_TIME_100_MSEC))
	{
		printf("respond cmd:ATCMD_CIPOPEN timeout");
		return FALSE;
	}
	//rt_thread_mdelay(CM_TIME_10_SEC);
	SIM7600CmdState.SIM_ConectState[num] = STATE_OK;		//已经连接上了服务器
	return TRUE;
}	

extern OS_MUTEX  sendmutex;
/*****************************************************************************
* Function     :ModuleSIM7600_SendData
* Description  :发送数据
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t ModuleSIM7600_SendData(uint8_t num,uint8_t* pdata,uint16_t len)
{
	uint8_t i = 0;
	OS_ERR ERR;

	
	i = 0;
	uint8_t buf[60] = {0};
	int32_t length;

	length = snprintf((char *)buf, sizeof(buf), "AT+CIPSEND=%d,%d\r\n",num,len);
	if (length == -1)
	{
        printf("snprintf error, the len is %d", length);
		return FALSE;
	}
	
	OSMutexPend(&sendmutex,0,OS_OPT_PEND_BLOCKING,NULL,&ERR); //获取锁
	if(NetConfigInfo[NET_YX_SELCT].NetNum > 1)
	{
		if((num > 10) || (pdata == NULL) )
		{
			OSMutexPost(&sendmutex,OS_OPT_POST_NONE,&ERR); //释放锁
			return FALSE;
		}
		
		UART1SENDBUF(buf, strlen((char*)buf) );
		OSTimeDly(CM_TIME_50_MSEC, OS_OPT_TIME_PERIODIC, &ERR);
		
		while (i < CM_TIME_5_SEC / CM_TIME_50_MSEC)
		{
			OSTimeDly(CM_TIME_50_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);
			//等待AT指令被回复
			if (SIM7600CmdState.ATCMD_SENDACK[num] == _4G_RESPOND_OK)
			{
				SIM7600CmdState.ATCMD_SENDACK[num] = _4G_RESPOND_ERROR;
				break;
			}
			i++;
//			if((i % 20) == 0) //1s钟没回复 就继续发送
//			{
//				Send_AT_CIPSEND(num,len);       //多发比发不出去好！
//			}
			
		}
		if (i == CM_TIME_5_SEC / CM_TIME_50_MSEC)
		{
			//APP_SetNetNotConect(num);
			//SIM7600CmdState.SIM_ConectState[num] = STATE_ERR;  //等待2s还没发送数据，则重新连接网络
			printf("respond cmd:ATCMD_SENDACK timeout");
			OSMutexPost(&sendmutex,OS_OPT_POST_NONE,&ERR); //释放锁
			return FALSE;
		}
	}
	//真正发送数据
	UART1SENDBUF(pdata,len);
	OSMutexPost(&sendmutex,OS_OPT_POST_NONE,&ERR); //释放锁
	
	OSTimeDly(CM_TIME_1_SEC, OS_OPT_TIME_PERIODIC, &timeerr);
	return TRUE;
	
}

/*****************************************************************************
* Function     :ModuleSIM7600_ReadData
* Description  :读取数据请求
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t ModuleSIM7600_ReadData(uint8_t num)
{
	if(num > 10)
	{
		return FALSE;
	}
	Send_AT_CIPRXGET(num);
	return TRUE;
}

/*****************************************************************************
* Function     :SIM7600_RecvDesposeCmd
* Description  :命令模式下模块接收处理
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t SIM7600_RecvDesposeCmd(uint8_t *pdata,uint32_t len)
{
	uint8_t i;
	
	if((pdata == NULL) || (!len))
	{
		return FALSE;
	}
	for(i = 0;i < DIM7600_RECV_TABLE_LEN;i++)
	{
		
		if (!strncmp( (char *)pdata, TIM7600_RecvFrameTable[i].ATCode, MIN(strlen(TIM7600_RecvFrameTable[i].ATCode),len) ) )     
		{
			TIM7600_RecvFrameTable[i].Fun(pdata,len);
			return TRUE;
		}
	}
	return FALSE;
	
}

/*****************************************************************************
* Function     :APP_SetNetNotConect
* Description  :设置网络未连接
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t APP_SetNetNotConect(uint8_t num)
{
	if(num >= NetConfigInfo[NET_YX_SELCT].NetNum)
	{
		return FALSE;
	}
	if(NetConfigInfo[NET_YX_SELCT].NetNum == 1)
	{
		SIM7600CmdState.SIM_ExistState = STATE_ERR;   //只有一个模块的时候需要重新启动
		CSQNum = 0;			//信号强度为0
	}
	SIM7600CmdState.SIM_ConectState[num] = STATE_ERR;
	SIM7600CmdState.SIM_Regiset[num] = STATE_ERR;
	SIM7600CmdState.ATCMD_CIPOPEN[num] = STATE_ERR;
	return TRUE;
}


/* Private macro------------------------------------------------------------------------------*/	

/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/

/************************(C)COPYRIGHT *****END OF FILE****************************/

