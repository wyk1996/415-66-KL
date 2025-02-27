/*******************************************************************************
 * @file
 * @note
 * @brief
 *
 * @author
 * @date     2021-05-02
 * @version  V1.0.0
 *
 * @Description
 *
 * @note History:
 * @note     <author>   <time>    <version >   <desc>
 * @note
 * @warning
 *******************************************************************************/
#include "chtask.h"
#include "ch_port.h"
#include "common.h"
#include "dwin_com_pro.h"
#include "main.h"
#include "HLW8110.h"
#include "4GMain.h"


CH_TASK_T stChTcb = {0};
CH_TASK_T *pstCh;
OS_Q ChTaskMq;
extern SYSTEM_RTCTIME gs_SysTime;

/**
 * @brief 充电任务线程数据初始化
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ch_task_init(void)
{
    OS_ERR err;
    //  mq_service_bind(CM_CHTASK_MODULE_ID,"ch task mq");

    OSQCreate (&ChTaskMq,
               "ch task mq",
               20,
               &err);
    if(err != OS_ERR_NONE)
    {
        printf("OSQCreate %s Fail", "ch task mq");
        return;
    }

    ch_ctl_disable();

    OSTimeDly(200, OS_OPT_TIME_PERIODIC, &timeerr);

    return;
}

/**
 * @brief 获取cp状态信息(12v,9v,6v)
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ch_get_cp_state_info(void)
{
    ch_get_cp_volt(&stChTcb.stGunStat.uiCpVolt);

    if((CP_12V_MAX >= stChTcb.stGunStat.uiCpVolt)
            && (CP_12V_MIN  <= stChTcb.stGunStat.uiCpVolt))          /* 在12V 范围 */
    {
        stChTcb.stGunStat.ucCpStat = CP_12V;
        return;
    }

    if((CP_9V_MAX >= stChTcb.stGunStat.uiCpVolt)
            && (CP_9V_MIN <= stChTcb.stGunStat.uiCpVolt))              /* 在9V 范围 */
    {
        stChTcb.stGunStat.ucCpStat = CP_9V;
        return;
    }

    if((CP_6V_MAX >= stChTcb.stGunStat.uiCpVolt)
            && (CP_6V_MIN <= stChTcb.stGunStat.uiCpVolt))             /* 在6V 范围 */
    {
        stChTcb.stGunStat.ucCpStat = CP_6V;
        return;
    }
    stChTcb.stGunStat.ucCpStat = CP_ABNORMAL;                    /* 异常 */


//stChTcb.stGunStat.ucCpStat = CP_6V;

}


extern SYSTEM_RTCTIME gs_SysTime;

/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t ch_from_ctl_msg(CH_CTL_T *pstChCtl)
{
    if(CH_START_CTL == pstChCtl->ucCtl)                                             /* 充电启动 */
    {
        if(stChTcb.ucState == CHARGING)
        {
            return 0;
        }
        if(stChTcb.ucState == INSERT)                                               /* 在插枪状态可以启动 */
        {
            stChTcb.stChCtl.ucCtl = CH_START_CTL;
            stChTcb.stChCtl.ucChStartMode = pstChCtl->ucChStartMode;
            stChTcb.stChCtl.ucChMode = pstChCtl->ucChMode;
            stChTcb.stChCtl.uiStopParam = pstChCtl->uiStopParam;

        }
        else
        {
            memset(&stChTcb.stCh,0,sizeof(CH_T));

            stChTcb.stCh.uiChStartTime = gs_SysTime;
            stChTcb.stCh.timestart = cp56time(NULL);


            mq_service_ch_send_dip(CH_TO_DIP_STARTFAIL ,UNPLUG ,0,NULL);  //枪未插

        }
    }
    else
    {
        if((CH_START_CTL == stChTcb.stChCtl.ucCtl) )
        {
            stChTcb.stChCtl.ucCtl        = CH_STOP_CTL;
        }
    }
    //mq_service_ch_send_msg_to_tcu(CH_TO_TCU_CHCTL_ACK,0,sizeof(CHCTL_ACK_T),(uint8_t *)&stChCtlAckData);
    return 1;

}


/**
 * @brief 处理tcu消息
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ch_task_tcu_msg_proc(MQ_MSG_T *pstMsg)
{
    switch(pstMsg->uiMsgCode)
    {
    case TCU_TO_CH_CHCLT:
        ch_from_ctl_msg((CH_CTL_T *)pstMsg->pucLoad);
        break;
    default:
        break;
    }
}


/**
 * @brief 消息处理
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void ch_msg_proc(MQ_MSG_T *pstMsg)
{
    switch(pstMsg->uiSrcMoudleId)
    {
        /* 目前TCU 模块和刷卡模块只向充电任务功能模块发送了充电控制消息 */
    case CM_TCUTASK_MODULE_ID:
        printf("msg src module id:%d, pstMsg->uiMsgCode = %d\n\r",pstMsg->uiSrcMoudleId,pstMsg->uiMsgCode);
        ch_task_tcu_msg_proc(pstMsg);
        break;
    default:
        printf("msg src module id:%d\n\r",pstMsg->uiSrcMoudleId);
        break;
    }
}

/**
 * @brief 充电任务等待消息
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void ch_wait_msg(void)
{
    MQ_MSG_T stMsg = {0};
    static uint8_t ucaMsgBuf[100];
    if(mq_service_recv_msg(&ChTaskMq,&stMsg,ucaMsgBuf,sizeof(ucaMsgBuf),CM_TIME_20_MSEC) == 0)
    {
        ch_msg_proc(&stMsg);
    }
}


uint32_t  uiTimeTest = 0;

/**
 * @brief 充电任务线程入口
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void HLW8112DataDispose(void)
{
    stChTcb.stHLW8112.uiE = F_AC_E * 1000;
    stChTcb.stHLW8112.usCurrent = F_AC_I * 100;
    stChTcb.stHLW8112.usVolt = F_AC_V * 10;
}


/**
 * @brief 周期性获取rn8112信息
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ch_cycle_get_8112(void)
{
    uint32_t uiTick = 0;

    uiTick = OSTimeGet(&timeerr);

    if(uiTick - stChTcb.stHLW8112.uiGetTime > CH_TIME_500_MSEC)
    {
        stChTcb.stHLW8112.uiGetTime = uiTick;
        OSSchedLock(&ERRSTATE);
        HLW8112_Measure();
        HLW8112DataDispose();
        OSSchedUnlock(&ERRSTATE);

        uiTimeTest  = cp56time(NULL);
    }
}





/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ch_task_cycle_proc(void)
{
    if(DisSysConfigInfo.energymeter == 1)
    {
        ch_cycle_get_8112();
    }
}


/**
 * @brief 是否设置故障
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t io_input_period(void)
{
    static uint32_t count;
    static uint8_t state = 1,laststate = 1;
    state = IO_YX1_READ;
    if(state != laststate)
    {
        if(++count > 3)
        {
            laststate = state;
            if(state)
            {
                stChTcb.stIOFault.uiFaultFlg= 0;
                stChTcb.stIOFault.ucFrist = 0;
            }
            else
            {
                stChTcb.stIOFault.uiFaultFlg= 1;
                stChTcb.stIOFault.ucFrist = 1;
            }
            count = 0;

        }
    }
    else
    {
        count = 0;
    }

    return 1;
}


/**
 * @brief 是否设置故障
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void HLW8112_Dispose(void)
{
	  static  uint32_t curtime = 0,lasttime = 0;

    curtime = OSTimeGet(&timeerr);

    if((stChTcb.stHLW8112.usCurrent > 600)  && (stChTcb.stCh.uiChargeEnergy == 0))   //电流大于6A
    {
		   if((curtime - lasttime) > CH_TIME_30_SEC)
		   {
			   Init_HLW8112();
			   curtime = lasttime;
		   }
	}
	else
	{
		lasttime = curtime;
	}
}


/**
 * @brief 充电任务线程入口
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void AppTaskCharge(void* parg)
{
    OS_ERR      err;
    ch_task_init();
    OSTimeDly(100, OS_OPT_TIME_PERIODIC, &err);
    cp_pwm_full();
    adc_init();
		
    while(1)
    {
		#if(UPDATA_STATE) 
		if(APP_GetSIM7600Mode() == MODE_HTTP)   //远程升级其他无关数据帧都不不要发送和处理
		{
			 OSTimeDly(1000, OS_OPT_TIME_PERIODIC, &timeerr);
			continue;
		}
		#endif
        io_input_period();
        ch_wait_msg();
		
        ch_get_cp_state_info();   	//CP电压检测
		
        ch_task_cycle_proc() ; 		//计量信息读取
        ch_loop_proc(&stChTcb);
        Clear_flag();     //未结算和已拔枪后清理
		HLW8112_Dispose();		//有电压  有电流  无电量处理
        OSTimeDly(1, OS_OPT_TIME_PERIODIC, &timeerr);
    }
}

