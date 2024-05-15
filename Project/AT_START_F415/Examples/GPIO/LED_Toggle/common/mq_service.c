/*******************************************************************************
 * @file
 * @note
 * @brief
 *
 * @author
 * @date     2021-10-23
 * @version  V1.0.0
 *
 * @Description  消息队列服务
 *
 * @note History:
 * @note     <author>   <time>    <version >   <desc>
 * @note
 * @warning
 *******************************************************************************/

#include <stdint.h>
#include <string.h>
#include "common.h"
#include "os.h"
#include "main.h"
/*
===============================================================================================================================
                                            ****** 类型定义/宏定义 ******
===============================================================================================================================
*/

#define        MQ_SERVICE_MSG_MAX_SIZE     sizeof(MQ_MSG_T)
#define        MQ_SERVICE_MSG_MAX_NUM      8

/**
 * @brief 功能模块id是否已经存在
 * @param[in] uiModuleId:功能模块ID
 * @param[out]
 * @return RT_NULL:功能模块ID没有绑定 非空:功能模块ID对应的节点指针
 * @note
 */
OS_Q * mq_service_moduleid_exist(uint32_t uiModuleId)
{
	if(CM_DISP_MODULE_ID == uiModuleId)
	{
		return &DwinMq;
	}
	else if(CM_CHTASK_MODULE_ID == uiModuleId)
	{
		return &ChTaskMq;
	}
	else if(CM_4GSEND_MODULE_ID == uiModuleId)
	{
		return &Send4GMq;
	}
	else if(CM_4GRECV_MODULE_ID == uiModuleId)
	{
		return &Recv4GMq;
	}


    return NULL;
}


#define MSQLEN     8
uint8_t pucBuf[MSQLEN][MSG_RECV_LEN] = {0};  //发送除4G以外得消息
MQ_MSG_T         stMsg[MSQLEN]    = {0};


#define MSQ4GLEN     5
uint8_t puc4GBuf[MSQ4GLEN][URART_4GRECV_LEN] = {0};  //发送4G得消息
MQ_MSG_T         st4GMsg[MSQ4GLEN]    = {0};
/**
 * @brief 发送消息到指定的功能模块
 * @param[in]  uiSrcMoudleId  ：消息源头的功能模块
 *             uiDestMoudleId ：需要发送到的功能模块
 *             uiMsgCode      ：消息码
 *             uiMsgVar       ：消息参数
 *             uiLoadLen      ：消息有效载荷长度
 *             pucLoad        ：指向需要发送的有效载荷地址
 * @param[out]
 * @return 0: 发送成功  非0: 发送失败
 * @note 内部调用
 */
uint8_t mq_service_send_msg(uint32_t uiSrcMoudleId, uint32_t  uiDestMoudleId, uint32_t  uiMsgCode,
                            uint32_t uiMsgVar, uint32_t  uiLoadLen, uint8_t  *pucLoad)
{
	
    OS_Q  *pstNode = NULL;
	OS_ERR   err;
	static uint8_t count = 0;
	static uint8_t count4G = 0;

	OSSchedLock(&ERRSTATE);
    pstNode = mq_service_moduleid_exist(uiDestMoudleId);

    if(pstNode == NULL)
    {
		OSSchedUnlock(&ERRSTATE);
        return 1;
    }
	if(pstNode == &Recv4GMq)
	{
		if(uiLoadLen > URART_4GRECV_LEN)
		{
			OSSchedUnlock(&ERRSTATE);
			return 1;
		}
		 if((pucLoad != NULL) && (uiLoadLen > 0))
		{
			memcpy(&puc4GBuf[count4G][0],pucLoad,uiLoadLen);
		}

		st4GMsg[count4G].uiSrcMoudleId    = uiSrcMoudleId;
		st4GMsg[count4G].uiDestMoudleId   = uiDestMoudleId;
		st4GMsg[count4G].uiMsgCode        = uiMsgCode;
		st4GMsg[count4G].uiMsgVar         = uiMsgVar;
		st4GMsg[count4G].uiLoadLen        = uiLoadLen;
		st4GMsg[count4G].pucLoad          = &puc4GBuf[count4G][0];

		OSQPost (pstNode,&st4GMsg[count4G],sizeof(MQ_MSG_T),OS_OPT_POST_FIFO,&err);	  
		count4G++;
		if(count4G >= MSQ4GLEN)   //一共有3个缓存发送
		{
			count4G = 0;
		}
	}
	else
	{
		if(uiLoadLen > MSG_RECV_LEN)
		{
			OSSchedUnlock(&ERRSTATE);
			return 1;
		}
		if((pucLoad != NULL) && (uiLoadLen > 0))
		{
			memcpy(&pucBuf[count][0],pucLoad,uiLoadLen);
		}

		stMsg[count].uiSrcMoudleId    = uiSrcMoudleId;
		stMsg[count].uiDestMoudleId   = uiDestMoudleId;
		stMsg[count].uiMsgCode        = uiMsgCode;
		stMsg[count].uiMsgVar         = uiMsgVar;
		stMsg[count].uiLoadLen        = uiLoadLen;
		stMsg[count].pucLoad          = &pucBuf[count][0];

		OSQPost (pstNode,&stMsg[count],sizeof(MQ_MSG_T),OS_OPT_POST_FIFO,&err);	  
		count++;
		if(count >= MSQLEN)   //一共有10个缓存发送
		{
			count = 0;
		}
	}
	
	if(err == OS_ERR_NONE)
	{
		OSSchedUnlock(&ERRSTATE);
		return 0;
	}
	OSSchedUnlock(&ERRSTATE);
	return 1;
}
/**
 * @brief   指定的功能模块接收消息
 * @param[in]  uiSrcMoudleId ：需要接收消息源头的功能模块
 *             uiBufSize     ：接收buf的尺寸
 *             rtTimeout     ：等待超时的时间
 * @param[out] pstMsg ： 指向存储接收到的消息缓冲区
*              pucMsgBuf：指向存储接收的消息的有效载荷
 * @return 0 ：发送成功  非0：发送失败
 * @note
 */
uint8_t mq_service_recv_msg(OS_Q * pq,MQ_MSG_T *pstMsg,uint8_t *pucMsgBuf,uint32_t uiBufSize,uint32_t rtTimeout)
{
	OS_ERR err;
	uint16_t len = sizeof(MQ_MSG_T);
	MQ_MSG_T* pbuf = NULL;
	uint16_t uiRxValidLen = 0;


	pbuf = OSQPend (pq,rtTimeout,OS_OPT_PEND_BLOCKING,&len,NULL,&err);
	if(err != OS_ERR_NONE)
	{
		return 1;
	}
	if(pbuf == NULL)
	{
		return 1;
	}	
	memcpy(pstMsg,pbuf,sizeof(MQ_MSG_T));
	uiRxValidLen  = CM_DATA_GET_MIN(uiBufSize,pstMsg->uiLoadLen);
	memcpy(pucMsgBuf,pstMsg->pucLoad,uiRxValidLen);
	pstMsg->pucLoad    = pucMsgBuf;
	pstMsg->uiLoadLen  = uiRxValidLen;
	return 0;
}


/**
 * @brief   发送消息到充电任务模块
 * @param[in]  uiSrcMoudleId ：消息源头的功能模块
 *             uiMsgCode     ：消息码
 *             uiMsgVar      ：消息参数
 *             uiLoadLen     ：消息有效载荷长度
 *             pucLoad       ：指向需要发送的有效载荷地址
 * @param[out]
* @return 0:发送成功  非0:发送失败
 * @note
 */
uint8_t mq_service_xxx_send_msg_to_chtask(uint32_t uiSrcMoudleId, uint32_t  uiMsgCode,
        uint32_t uiMsgVar, uint32_t uiLoadLen, uint8_t  *pucLoad)
{
    return mq_service_send_msg(uiSrcMoudleId,CM_CHTASK_MODULE_ID,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad);
}



/**
 * @brief   刷卡任务发送消息到显示任务
 * @param[in]  uiMsgCode ：消息码
 *             uiMsgVar  ：消息参数
 *             uiLoadLen ：消息有效载荷长度
 *             pucLoad   ：指向需要发送的有效载荷地址
 * @param[out]
 * @return 0 ：发送成功  非0：发送失败
 * @note
 */
uint8_t mq_service_card_send_disp(uint32_t  uiMsgCode ,uint32_t uiMsgVar ,uint32_t uiLoadLen ,uint8_t  *pucLoad)
{
    return mq_service_send_msg(CM_CARD_MODULE_ID,CM_DISP_MODULE_ID,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad) ;
}



/**
* @brief   充电模块发送给显示任务
 * @param[in]  uiMsgCode ：停止原因
 *             uiMsgVar  ：消息参数
 *             uiLoadLen ：消息有效载荷长度
 *             pucLoad   ：指向需要发送的有效载荷地址
 * @param[out]
 * @return 0 ：发送成功  非0：发送失败
 * @note
 */
uint8_t mq_service_ch_send_dip(uint32_t  uiMsgCode ,uint32_t uiMsgVar ,uint32_t uiLoadLen ,uint8_t  *pucLoad)
{
    return mq_service_send_msg(CM_CHTASK_MODULE_ID,CM_DISP_MODULE_ID,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad) ;
}


/**
* @brief   发送给4G发送任务
 * @param[in]  uiMsgCode ：消息码
 *             uiMsgVar  ：消息参数
 *             uiLoadLen ：消息有效载荷长度
 *             pucLoad   ：指向需要发送的有效载荷地址
 * @param[out]
 * @return 0 ：发送成功  非0：发送失败
 * @note
 */
uint8_t mq_service_send_to_4gsend(uint32_t  uiMsgCode ,uint32_t uiMsgVar ,uint32_t uiLoadLen ,uint8_t  *pucLoad)
{
    return mq_service_send_msg(CM_UNDEFINE_ID,CM_4GSEND_MODULE_ID,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad) ;
}

/**
* @brief   迪文接收任务发送到显示任务
 * @param[in]  uiMsgCode ：消息码
 *             uiMsgVar  ：消息参数
 *             uiLoadLen ：消息有效载荷长度
 *             pucLoad   ：指向需要发送的有效载荷地址
 * @param[out]
 * @return 0 ：发送成功  非0：发送失败
 * @note
 */
uint8_t mq_service_dwinrecv_send_disp(uint32_t  uiMsgCode ,uint32_t uiMsgVar ,uint32_t uiLoadLen ,uint8_t  *pucLoad)
{
    return mq_service_send_msg(CM_DWINRECV_MODULE_ID,CM_DISP_MODULE_ID,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad) ;
}


/**
* @brief   4G接收发送给接收任务
 * @param[in]  uiMsgCode ：消息码
 *             uiMsgVar  ：消息参数
 *             uiLoadLen ：消息有效载荷长度
 *             pucLoad   ：指向需要发送的有效载荷地址
 * @param[out]
 * @return 0 ：发送成功  非0：发送失败
 * @note
 */
uint8_t mq_service_4GUart_send_recv(uint32_t  uiMsgCode ,uint32_t uiMsgVar ,uint32_t uiLoadLen ,uint8_t  *pucLoad)
{
    return mq_service_send_msg(CM_4GUARTRECV_MODULE_ID,CM_4GRECV_MODULE_ID,uiMsgCode,uiMsgVar,uiLoadLen,pucLoad) ;
}
