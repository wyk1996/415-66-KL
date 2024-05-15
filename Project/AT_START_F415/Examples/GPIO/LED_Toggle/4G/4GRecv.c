/*****************************************Copyright(C)******************************************
***************************************************************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: GPRSMain.c
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
#include "4GMain.h"
#include "4GRecv.h"
#include <string.h>
#include "common.h"
#include "dwin_com_pro.h"
#include "main.h"
/* Private define-----------------------------------------------------------------------------*/

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
_RECV_DATA_CONTROL RecvDataControl[LINK_NET_NUM];
OS_Q Recv4GMq;

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
//专门做GPRS接收处理


/*****************************************************************************
* Function     : APP_RecvDataControl
* Description  : 
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日       
*****************************************************************************/
_RECV_DATA_CONTROL	* APP_RecvDataControl(uint8_t num)
{
	if(num >= NetConfigInfo[NET_YX_SELCT].NetNum)
	{
		return NULL;
	}
	return &RecvDataControl[num];
}

//专门做GPRS接收处理
uint8_t recvbuf[URART_4GRECV_LEN];
/*****************************************************************************
* Function     : TaskGPRSRecv
* Description  : 串口测试任务
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月16日        
*****************************************************************************/
void AppTask4GRecv(void *pdata)
{
	OS_ERR ERR;
	uint8_t err,i;  
	int iRxLen = 0;
	    MQ_MSG_T stMsg = {0};

	//连续10次未收到数据，则主动读取数据,连续40次则重启
	static uint32_t NetRset[LINK_NET_NUM] = {0};		

	
	static uint32_t lastSysTime[LINK_NET_NUM] = {0}; 
	static uint32_t nowSysTime[LINK_NET_NUM] = {0};		
	 OSTimeDly(2000, OS_OPT_TIME_PERIODIC, &timeerr);
	UART14Ginit();	
	OSQCreate (&Recv4GMq,
                 "4G send mq",
                 20,
                 &ERR);
	if(ERR != OS_ERR_NONE)
	{
		printf("OSQCreate %s Fail", "4G send mq");
		return;
	}
    if(DisSysConfigInfo.standaloneornet != DISP_NET)
	{
		return;
	}
	for(i = 0; i < NetConfigInfo[NET_YX_SELCT].NetNum;i++)
	{
		nowSysTime[i] = OSTimeGet(&timeerr);
		lastSysTime[i] = nowSysTime[i];
	}
	while(1)
    {
        //从串口读取一个消息GPRSTempBuf[GPRS_TEMP_BUF_LEN]
		for(i = 0; i < NetConfigInfo[NET_YX_SELCT].NetNum;i++)
		{
			nowSysTime[i] = OSTimeGet(&timeerr);
		}
		if(NetConfigInfo[NET_YX_SELCT].NetNum  == 0)
		{
			iRxLen = 0;
		}
		
		
		if(mq_service_recv_msg(&Recv4GMq,&stMsg,recvbuf,sizeof(recvbuf),CM_TIME_1_SEC) == 0 )
		{
			 if(stMsg.uiSrcMoudleId == CM_4GUARTRECV_MODULE_ID)
			 {
				//如果已经连接上服务器
				if (NetConfigInfo[NET_YX_SELCT].NetNum == 1)
				{
					if(APP_GetModuleConnectState(0) == STATE_OK) //已经连接上后台了
					{
	//					if((GPRSTempBuf[0] == 0x0d) && (GPRSTempBuf[1] == 0x0a) )
	//					{
	//						APP_SetNetNotConect(0);   //调试的时候出现0d 0a
	//					}
	//					
						#if(UPDATA_STATE)   
						if(APP_GetSIM7600Mode() == MODE_HTTP)  
						{
							SIM7600_RecvDesposeCmd(&recvbuf[0],stMsg.uiLoadLen); //未连接上服务器，AT指令处理
//						GPRSTempBuf[2000] = '\0';  //防止无线打印
//						rt_kprintf("rx %s\r\n",GPRSTempBuf);
						}
						else
						{
						
							if(_4G_RecvFrameDispose(&recvbuf[0],stMsg.uiLoadLen))  //数据透传
							{
									lastSysTime[0] = nowSysTime[0];
							}
						}
						#else
						if(_4G_RecvFrameDispose(&recvbuf[0],stMsg.uiLoadLen))  //数据透传
						{
								lastSysTime[0] = nowSysTime[0];
						}
						#endif
//						printf("rx len:%d,rx data:",stMsg.uiLoadLen);
//						for(i = 0; i < stMsg.uiLoadLen; i++)
//						{
//							printf("%02x ",recvbuf[i]);
//						}
//						printf("\r\n");
						memset(recvbuf,0,sizeof(recvbuf));
						__NOP();
					}
					else
					{
						SIM7600_RecvDesposeCmd(&recvbuf[0],stMsg.uiLoadLen); //未连接上服务器，AT指令处理
//						
//						recvbuf[URART_4GRECV_LEN - 1] = '\0';  //防止无线打印
//						printf("rx %s\r\n",recvbuf);
						
					}
				}
				else
				{
					//未连接上服务器，AT指令处理
					SIM7600_RecvDesposeCmd(&recvbuf[0],stMsg.uiLoadLen);
					for(i = 0;i < NetConfigInfo[NET_YX_SELCT].NetNum;i++)
					{
						if(RecvDataControl[i].RecvStatus == RECV_FOUND_DATA)
						{
							RecvDataControl[i].RecvStatus = RECV_NOT_DATA;
							//接收数据处理
							
							//临时接收什么发送什么
							//ModuleSIM7600_SendData(i,RecvDataControl[i].DataBuf,RecvDataControl[i].len);
							if(i == 0)
							{
								if(_4G_RecvFrameDispose(RecvDataControl[i].DataBuf,RecvDataControl[i].len))
								{
										lastSysTime[i] = nowSysTime[i];			
								}
							}
							else
							{
								if(RecvDataControl[i].DataBuf[0] == 0x68)    //简单判读下
								{
									//政府平台,有数据返回就是注册成功
									APP_SetAppRegisterState(1,STATE_OK);
										lastSysTime[i] = nowSysTime[i];
								}
							}
							NetRset[i] = 0;
						}
					}
				}
			}
		}
				
		for(i = 0;i < NetConfigInfo[NET_YX_SELCT].NetNum;i++)
		{
			if(APP_GetModuleConnectState(i) == STATE_OK) //已经连接上后台了
			{
				if (NetConfigInfo[NET_YX_SELCT].NetNum > 1)
				{
					if(++NetRset[i] >= 13)  // 15s
					{
						NetRset[i] = 0;
						if(i == 0)
						{
							mq_service_send_to_4gsend(BSP_4G_SENDNET1,GUN_A ,0 ,NULL);
						}else{
							
							mq_service_send_to_4gsend(BSP_4G_SENDNET2,GUN_B ,0 ,NULL);
						}
					}
				}
				
				//重启时间根据周期性发送数据再放余量
				#if(NET_YX_SELCT == XY_AP)
				{
					if((nowSysTime[i] >= lastSysTime[i]) ? ((nowSysTime[i] - lastSysTime[i]) >= CM_TIME_60_SEC) : \
					((nowSysTime[i] + (0xffffffff - lastSysTime[i])) >= CM_TIME_60_SEC))
					{
						lastSysTime[i] = nowSysTime[i];
						APP_SetNetNotConect(i);
					}
				}
				#else
				{
					if((nowSysTime[i] >= lastSysTime[i]) ? ((nowSysTime[i] - lastSysTime[i]) >= CM_TIME_90_SEC) : \
					((nowSysTime[i] + (0xffffffff - lastSysTime[i])) >= CM_TIME_90_SEC))
					{
						lastSysTime[i] = nowSysTime[i];
						APP_SetNetNotConect(i);
					}
				}
				#endif
	
			}
			else
			{
				lastSysTime[i] = nowSysTime[i];
			}
		}
	}	
	
}



/************************(C)COPYRIGHT *****END OF FILE****************************/

