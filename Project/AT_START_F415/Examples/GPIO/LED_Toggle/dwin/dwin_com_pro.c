/*************************************************
 Copyright (c) 2019
 All rights reserved.
 File name:     dwin_com_pro.c
 Description:   dwin 移植&使用例程文件
 History:
 1. Version:
    Date:
    Author:
    Modify:
*************************************************/
#include "string.h"
#include "dwin_com_pro.h"
#include "Dispkey.h"
#include "DwinProtocol.h"
#include "MenuDisp .h"
#include "DispKeyFunction.h"
#include "DispShowStatus.h"
#include "ch_port.h"
#include "RC522.h"
#include "chtask.h"
#include "common.h"
#include "4GMain.h"
#include "ch_port.h"
#include "flashdispos.h"
#include "dlt645_port.h"
#include "Frame66.h"

extern CH_TASK_T stChTcb;
extern _m1_card_info m1_card_info;		 //M1卡相关信息
extern _m1_control M1Control;

extern SYSTEM_RTCTIME gs_SysTime;


//显示数据数据地址从0x1000开始
#define DIS_ADD(id,add)      					    ( ( ( (id) & 0x00ff) << 7 ) | ( (add) & (0x007f) ) + 0x1000 )
#define DIS_ADD_KEY(id,add)      			        ( ( ( (id) & 0x00ff) << 7 ) | ( (add) & (0x007f) ) )

#define INPUT_MENU6_CODE		0x1600    //密码输入界面地址
typedef struct
{
    uint16_t variaddr;                                      //变量地址
    uint8_t (*Fun)(uint16_t addr, uint8_t *pvalue,uint8_t len);    //对应的处理函数
} _DISP_ADDR_FRAME;




//屏幕需要显示得数据

//03充电界面显示信息
typedef struct
{
    uint16_t  chargevol;			//充电电压 0.1v
    uint16_t  chargecur;			//充电电流 0.01a
    uint16_t  chargepwer;			//充电电量 0.01
    uint16_t  chargemoney;			//充电金额 0.01元
    uint32_t   balance;				//卡内余额
} _CHARGE_INFO;



//记录界面显示(正式函数)
//typedef struct
//{
//    uint8_t	SerialNum[20];			//交易流水号
//    uint8_t	  BillStopRes[10];		//停止原因
//    uint16_t  chargepwer;			//充电电量 0.01
//    uint8_t	  StartTime[20];		//开始充电时间
//    uint8_t	  StopTime[20];			//结束充电时间
//    uint8_t   ChargeType[10];		//充电方式
//    uint8_t   BillState[10];		//结算状态
//    uint16_t  chargemoney;			//充电金额 0.01元
//} _RECORD_INFO;







////记录查询页面测试
//typedef struct
//{
//    //=======hycsh start
//    uint32_t CardNum; //卡号4字节
//    uint8_t Gunnum[10];	//接口号，A或B枪
//    uint32_t Record_balance;   //扣款后卡内余额
//    //=======hycsh end

//    uint32_t  chargepwer;			//充电电量 0.01
//    uint32_t  chargemoney;			//消费金额 0.01元
//    uint8_t	SerialNum[20];			//交易流水号
//    uint8_t	  BillStopRes[10];		//停止原因
//    uint8_t	  StartTime[20];		//开始充电时间
//    uint8_t	  StopTime[20];			//结束充电时间
//    uint8_t   ChargeType[10];		//充电方式
//    uint8_t   BillState[10];		//结算状态
//} _RECORD_INFO;


////======================hycsh显示记录信息30，31页面对应的结构体[1]
//typedef struct
//{
//    uint32_t Record_balance;   //扣款后卡内余额
//    uint8_t  Record_BillState[10];  //结算状态
//    uint8_t	 Record_SerialNum[20]; //交易流水号
//    uint32_t Record_chargepwer; //充电电量
//    uint8_t	 Record_BillStopRes[10]; //停止原因

//    uint32_t CardNum; //卡号4字节
//    uint8_t  Record_ChargeType[10];	//充电方式
//    uint8_t Gunnum[10];	//接口号，A或B枪
//    uint32_t Record_chargemoney; //消费金额
//    uint8_t	 Record_StartTime[20];		//开始充电时间
//    uint8_t	 Record_StopTime[20];			//结束充电时间
//} _Record_displayinfo;


//typedef struct
//{
//    uint16_t RecodeCurNum;			   //交易记录条数
//    uint16_t CurReadRecodeNun;		//当前读到的交易记录的偏移条数（查询使用）
//    uint16_t NextReadRecodeNun;		//下一条读到的交易记录的偏移条数（查询使用）
//    uint16_t UpReadRecodeNun;			//上一条读到的交易记录的偏移条数（查询使用）
//    uint16_t CurNun;					   //当前记录页码
//    _RECORD_INFO CurRecode;	//存放当前记录
//    _RECORD_INFO NextRecode;	//存放下一条记录
//    _RECORD_INFO UpRecode;	//存放上一条记录
//} _RECODE_CONTROL;

//_RECODE_CONTROL RECODECONTROL;
//_Record_displayinfo Recorddisplay_info;
//_Record_displayinfo contrast_CardNum;
//_RECORD_INFO	RecordInfo;		//显示交易记录信息
//_RECORD_INFO  SaveRecordinfo; //hycsh存储交易记录
_RECODE_CONTROL RECODECONTROL;
_Record_displayinfo Recorddisplay_info;
_Record_displayinfo contrast_CardNum;
_RECORD_INFO	RecordInfo;		//显示交易记录信息
_RECORD_INFO  SaveRecordinfo; //hycsh存储交易记录


_DISP_CONTROL DispControl;		//显示控制结构体
_CHARGE_INFO ChargeInfo;		//充电信息

_DIS_SYS_CONFIG_INFO DisSysConfigInfo;	//显示界面系统配置信息

uint8_t Billbuf[GUN_MAX][200] = {0};			//缓冲
#if(WLCARD_STATE)
uint8_t FlashCardVinWLBuf[2100] = {0};		//卡Vin白名单chuli
#endif
#define PARA_OFFLINEBILL_FLLEN   	(300)   //单个长度
#define FLASH_WR_LEN (1024)  //单个存放记录的长度4096字节=4K



RATE_T	           stChRate = {0};                 /* 充电费率  */
RATE_T	           stChRateA = {0};                 /* 充电费率  */
RATE_T	           stChRateB = {0};                 /* 充电费率  */



#if(WLCARD_STATE)
/**
* @brief    读取白名单卡
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t* APP_GetCARDWL(void)
{
    fal_partition_read(CARD_WL,0,FlashCardVinWLBuf,sizeof(FlashCardVinWLBuf));
    return FlashCardVinWLBuf;
}
#endif

/**
 * @brief    获取离线交易记录个数
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t APP_GetNetOFFLineRecodeNum(void)
{
    uint8_t num;
    if(fal_partition_read(OFFLINE_BILL,0,&num,1) < 0)  //读取费率信息
    {
        printf("Partition read error! Flash device(%d) read error!", OFFLINE_BILL);
    }
    //最多100个
    if(num > 100)
    {
        num = 0;
        fal_partition_write(OFFLINE_BILL,0,&num,1);
        return 0;
    }
    return num;
}

/*****************************************************************************
* Function     : APP_GetBillInfo
* Description  : 获取枪订单信息
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
uint8_t *APP_GetBillInfo(_GUN_NUM gun)
{
    if(gun >= GUN_MAX)
    {
        return NULL;
    }
    return &Billbuf[gun][1];
}

/*****************************************************************************
* Function     : APP_GetNetOFFLineRecodeNum
* Description  : 读写离线交易记录
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
uint8_t APP_SetNetOFFLineRecodeNum(uint8_t num)
{

    //最多100个
    if(num > 100)
    {
        return FALSE;
    }
    fal_partition_write(OFFLINE_BILL,0,&num,1);

    return TRUE;;
}

/**
 * @brief    写离线交易记录
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t WriterFmBill(_GUN_NUM gun,uint8_t billflag)
{
//第一个字节表示时候已经结算     				  billflag:0表示已经结算			billflag：表示未结算
    if(gun >= GUN_MAX)
    {
        return FALSE;
    }
    Billbuf[gun][0] = billflag;
    if(billflag != 0)
    {
        Pre4GBill(gun,&Billbuf[gun][1]);
    }

    //读取所有配置信息
    if(gun == GUN_A )
    {
        if(billflag == 0)
        {
            fal_partition_write(ONLINE_BILL,0,&Billbuf[GUN_A][0],1);
        }
        else
        {
            fal_partition_write(ONLINE_BILL,0,&Billbuf[GUN_A][0],200);
        }
    }
    else
    {
        if(billflag == 0)
        {
            fal_partition_write(ONLINE_BILL,4*1024,&Billbuf[GUN_B][0],1);
        }
        else
        {
            fal_partition_write(ONLINE_BILL,4*1024,&Billbuf[GUN_B][0],200);
        }
    }

    return TRUE;
}
/*****************************************************************************
* Function     : ReadFmBill
* Description  : 读取订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
uint8_t ReadFmBill(void)
{
    if(fal_partition_read(ONLINE_BILL,0,&Billbuf[GUN_A][0],200) < 0)
    {
        printf("Partition read error! Flash device(%d) read error!", ONLINE_BILL);
    }
    if(fal_partition_read(ONLINE_BILL,4*1024,&Billbuf[GUN_B][0],200) < 0)
    {
        printf("Partition read error! Flash device(%d) read error!", ONLINE_BILL);
    }
    if(Billbuf[GUN_A][0] == 1)
    {
        APP_SetResendBillState(GUN_A,1);
    }
    else {

        APP_SetResendBillState(GUN_A,0);
    }
    if(Billbuf[GUN_B][0] == 1)
    {
        APP_SetResendBillState(GUN_B,1);
    }
    else {

        APP_SetResendBillState(GUN_B,0);
    }
    return TRUE;
}

/*****************************************************************************
* Function     : APP_RWCardWl
* Description  : 读写网络离线交易记录
* Input        :读写白名单卡
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
uint8_t APP_RWCardWl(_FLASH_ORDER RWChoose,uint8_t  * pdata,uint16_t len)
{
    if(len > 4096)
    {
        return FALSE;
    }
    if(RWChoose == FLASH_ORDER_WRITE)
    {
        fal_partition_write(CARD_WL,0,pdata,len);
    }
    else
    {
        fal_partition_read(CARD_WL,0,pdata,len);
    }
    return TRUE;
}

/*****************************************************************************
* Function     : APP_RWOFFLineRe离线交易记录
* Description  : 读写网络离线交易记录
* Input        :
				count  读写在第几条 1 - 100条
                 RWChoose  读写命令
                 precode 缓冲区地址
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
uint8_t APP_RWNetOFFLineRecode(uint16_t count,_FLASH_ORDER RWChoose,uint8_t  * pdata)
{

    if((count > 100) || (pdata == NULL) )
    {
        return FALSE;
    }
    if(RWChoose == FLASH_ORDER_WRITE)
    {
        fal_partition_write(OFFLINE_BILL,FLASH_WR_LEN*(count+1),pdata,PARA_OFFLINEBILL_FLLEN);
    }
    else
    {
        fal_partition_read(OFFLINE_BILL,FLASH_WR_LEN*(count+1),pdata,PARA_OFFLINEBILL_FLLEN);
    }
    return TRUE;
}

/*****************************************************************************
* Function     : APP_RWOFFLineRe离线交易记录
* Description  : 读写网络离线交易记录
* Input        :
				count  读写在第几条 1 - 100条
                 RWChoose  读写命令
                 precode 缓冲区地址
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
uint8_t APP_RWNetFSOFFLineRecode(uint16_t count,_FLASH_ORDER RWChoose,uint8_t  * pdata)
{

    if((count > 100) || (pdata == NULL) )
    {
        return FALSE;
    }
    if(RWChoose == FLASH_ORDER_WRITE)
    {
        fal_partition_write(OFFFSLINE_BILL,FLASH_WR_LEN*(count+1),pdata,PARA_OFFLINEBILL_FLLEN);
    }
    else
    {
        fal_partition_read(OFFFSLINE_BILL,FLASH_WR_LEN*(count+1),pdata,PARA_OFFLINEBILL_FLLEN);
    }
    return TRUE;
}






/*****************************************************************************
* Function     :Recordqueryinfo_WR
* Description  :记录信息 读——写函数
* Input        :
				          count  读写在第几条 1 - 100条
                 RWChoose  读写命令
                 precode 缓冲区地址
* Output       :None
* Return       :static
* Note(s)      :
* author       :hycsh
* Contributor  :2022年3月
*****************************************************************************/
uint8_t Recordqueryinfo_WR(uint16_t count,_FLASH_ORDER RWChoose,uint8_t * pdata)
{
    if((count > 1000) || (pdata == NULL) )
    {
        return FALSE;
    }
    if(RWChoose == FLASH_ORDER_WRITE)
    {
        fal_partition_write(RECORD_QUERY, FLASH_WR_LEN*count, pdata, sizeof(_RECORD_INFO));
    }
    else
    {
        fal_partition_read(RECORD_QUERY,FLASH_WR_LEN*count,pdata,sizeof(_RECORD_INFO));  //读的时候就读结构体字节大小
    }
    return TRUE;
}



/*****************************************************************************
* Function     :APP_SelectCurChargeRecode
* Description  :显示交流记录总函数，里面包含显示页面的函数
* Input        :
                _RECORD_INFO * precode,ST_Menu *pMenu
								注意一个大小端转化，先发低位再发高位（小端）
* Output       :None
* Return       :static
* Note(s)      :
* author       :hycsh
* Contributor  :2022年3月
*****************************************************************************/
static uint8_t DispShow_Recode(_RECORD_INFO * precode,ST_Menu *pMenu)
{
    _Record_displayinfo recorddisplay;
    memset(&Recorddisplay_info,0,sizeof(Recorddisplay_info)); //第一步把结构体清零

    //正式赋值
    Recorddisplay_info.Record_balance=(precode->Record_balance&0x000000FF)<<24|(precode->Record_balance&0x0000FF00)<<8|\
                                      (precode->Record_balance&0x00FF0000)>>8|(precode->Record_balance&0xFF000000)>>24;  //【1】卡内余额(4字节)
    memcpy(Recorddisplay_info.Record_BillState,precode->BillState,sizeof(precode->BillState));  //【2】结算状态(10字节)
    memcpy(Recorddisplay_info.Record_SerialNum,precode->SerialNum,sizeof(precode->SerialNum)); //【3】交易流水号Record_SerialNum[20];
    Recorddisplay_info.Record_chargepwer=((precode->chargepwer&0x000000FF)<<24|(precode->chargepwer&0x0000FF00)<<8|\
                                          (precode->chargepwer&0x00FF0000)>>8|(precode->chargepwer&0xFF000000)>>24);
    memcpy(Recorddisplay_info.Record_BillStopRes,precode->BillStopRes,sizeof(precode->BillStopRes)); //【5】停止原因(10字节)


    Recorddisplay_info.CardNum=precode->CardNum; //【6】卡号(4字节)
    memcpy(Recorddisplay_info.Record_ChargeType,precode->ChargeType,sizeof(precode->ChargeType)); //【7】充电方式
    memcpy(Recorddisplay_info.Gunnum,precode->Gunnum,sizeof(precode->Gunnum));//【8】接口号，A或B枪暂无
    Recorddisplay_info.Record_chargemoney=((precode->chargemoney/100&0x000000FF)<<24|(precode->chargemoney/100&0x0000FF00)<<8|\
                                           (precode->chargemoney/100&0x00FF0000)>>8|(precode->chargemoney/100&0xFF000000)>>24);//【9】消费金额（4字节）

    memcpy(Recorddisplay_info.Record_StartTime,precode->StartTime,sizeof(precode->StartTime)); //开始充电时间
    memcpy(Recorddisplay_info.Record_StopTime,precode->StopTime,sizeof(precode->StopTime));  //结束充电时间

    //数据显示
    PrintStr(DIS_ADD(pMenu->FrameID,1),(uint8_t *)&Recorddisplay_info,34);  //【1-3】34字节
    PrintStr(DIS_ADD(pMenu->FrameID,18),(uint8_t *)&Recorddisplay_info.Record_chargepwer,4);  //【4】充电电量
    PrintStr(DIS_ADD(pMenu->FrameID,20),(uint8_t *)&Recorddisplay_info.Record_BillStopRes,10); //【5】停止原因(10字节)
    PrintStr(DIS_ADD(pMenu->FrameID,25),(uint8_t *)&Recorddisplay_info.CardNum,4); //卡号 (4字节)
    PrintStr(DIS_ADD(pMenu->FrameID,27),(uint8_t *)&Recorddisplay_info.Record_ChargeType,10); //充电方式(10字节)
    PrintStr(DIS_ADD(pMenu->FrameID,32),(uint8_t *)&Recorddisplay_info.Gunnum,14); //接口号和消费金额

    PrintStr(DIS_ADD(pMenu->FrameID,80),(uint8_t *)Recorddisplay_info.Record_StartTime,40);  //【10-11】开始和结束时间
    return TRUE;
}




/*****************************************************************************
* Function     :StoreRecodeCurNum
* Description  :存储当前的条数_单独一个区域存储
* Input        :
* Output       :None
* Return       :static
* Note(s)      :
* author       :hycsh
* Contributor  :2022-3-25
*****************************************************************************/
uint8_t StoreRecodeCurNum(void)
{
    RECODECONTROL.RecodeCurNum++; //当前的存储的第几条数
	    fal_partition_write(RECORD_QUERY,0,(uint8_t *)&RECODECONTROL.RecodeCurNum,sizeof(RECODECONTROL.RecodeCurNum));
    return 1;
}
uint8_t Clear_record(void)
{
    RECODECONTROL.RecodeCurNum=0;
    fal_partition_write(RECORD_QUERY, 0,  (uint8_t*)&RECODECONTROL.RecodeCurNum, sizeof(RECODECONTROL.RecodeCurNum));
    return 1;
}


void StrToHex(char *pbDest, char *pbSrc, int nLen)
{
    char ddl,ddh;
    int i;

    for (i=0; i<nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        pbDest[i*2] = ddh;
        pbDest[i*2+1] = ddl;
    }

    pbDest[nLen*2] = '\0';
}

/*****************************************************************************
* Function     :APP_Transactionrecord
* Description  :交易记录信息写入flash函数
* Input        :GUNnum=A枪或B枪
                BILL_status=结算状态：已结算或未结算
                Stop_reason=停止原因
                Charg_mode=充电模式
                CurNum=当前的条数
* Output       :None
* Return       :static
* Note(s)      :
* author       :hycsh
* Contributor  :2022年3月18号_SHOW_NUM num
*****************************************************************************/
uint8_t  APPTransactionrecord(STOP_REASON Stop_reason,_SHOW_NUM Charg_mode,_SHOW_NUM GUNnum,uint32_t CurNum)
{
    int length;
    uint8_t * pbuf;
    uint8_t starttime[20] = {0};
    uint8_t stoptime[20] = {0};
    memset(&SaveRecordinfo,0,sizeof(SaveRecordinfo));
    SaveRecordinfo.Record_balance=m1_card_info.balance;  //【1】扣款前卡内余额(4字节)

    if(DisSysConfigInfo.standaloneornet == DISP_NET)
    {
        Dis_ShowCopy(SaveRecordinfo.BillState,SHOW_BILL); //网络状态都已结算
    }
    else
    {
        if(M1Control.m1_if_balance == 1) //未结算
        {
            Dis_ShowCopy(SaveRecordinfo.BillState,SHOW_NOTBIL); //单击未结算时，显示未结算
        }
        else
        {
            if(stChTcb.stCh.reason==UNPLUG)
            {
                Dis_ShowCopy(SaveRecordinfo.BillState,SHOW_NOTBIL); //单击未结算时，显示未结算
            }
            else
            {
                Dis_ShowCopy(SaveRecordinfo.BillState,SHOW_BILL); //已结算
            }
        }
    }

    if(DisSysConfigInfo.standaloneornet == DISP_NET)
    {
        pbuf  = APP_GetBatchNum(GUN_A);
        if(pbuf == NULL)
        {
            return FALSE;
        }
        StrToHex((char*)SaveRecordinfo.SerialNum,(char*)&pbuf[11],5);
    }
    SaveRecordinfo.chargepwer = stChTcb.stCh.uiChargeEnergy /10;
    //SaveRecordinfo.chargepwer=((stChTcb.stCh.uiChargeEnergy /10)&0x00ff)<<8|((stChTcb.stCh.uiChargeEnergy /10) & 0xff00)>>8; //【4】充电电量0.01（4字节）
    Dis_Showstop_reason(SaveRecordinfo.BillStopRes,Stop_reason); //【5】停止原因:正常停止（10字节）

    if(DisSysConfigInfo.standaloneornet == DISP_NET)
    {
        if(_4G_GetStartType(GUN_A) == _4G_APP_CARD)
        {
            SaveRecordinfo.CardNum = (m1_card_info.uidByte[0]<<24) | (m1_card_info.uidByte[1] << 16) |\
                                     (m1_card_info.uidByte[2] << 8) | (m1_card_info.uidByte[3]);  //【6】复制卡号（4字节）
        }
    }
    else
    {
        SaveRecordinfo.CardNum = (m1_card_info.uidByte[0]<<24) | (m1_card_info.uidByte[1] << 16) |\
                                 (m1_card_info.uidByte[2] << 8) | (m1_card_info.uidByte[3]);  //【6】复制卡号（4字节）
    }
    Dis_ShowCopy(SaveRecordinfo.ChargeType,Charg_mode);		//【7】充电方式= APP启动或者刷卡启动（10字节）
    Dis_ShowCopy(SaveRecordinfo.Gunnum,GUNnum);//【8】接口号（10字节）
    SaveRecordinfo.chargemoney=stChTcb.stCh.uiAllEnergy;//【9】消费金额 0.01（4字节）

    length = snprintf((char *)starttime, sizeof(starttime), "20%02d-%02d-%02d-%02d-%02d-%02d",stChTcb.stCh.uiChStartTime.ucYear - 100 \
                      ,stChTcb.stCh.uiChStartTime.ucMonth,stChTcb.stCh.uiChStartTime.ucDay,stChTcb.stCh.uiChStartTime.ucHour,stChTcb.stCh.uiChStartTime.ucMin \
                      ,stChTcb.stCh.uiChStartTime.ucSec);

    if (length == -1)
    {
        printf("snprintf error, the len is %d", length);
        return FALSE;
    }

    length = snprintf((char *)stoptime, sizeof(stoptime), "20%02d-%02d-%02d-%02d-%02d-%02d",stChTcb.stCh.uiChStoptTime.ucYear - 100 \
                      ,stChTcb.stCh.uiChStoptTime.ucMonth,stChTcb.stCh.uiChStoptTime.ucDay,stChTcb.stCh.uiChStoptTime.ucHour,stChTcb.stCh.uiChStoptTime.ucMin \
                      ,stChTcb.stCh.uiChStoptTime.ucSec);

    if (length == -1)
    {
        printf("snprintf error, the len is %d", length);
        return FALSE;
    }
    memcpy(SaveRecordinfo.StartTime,starttime,20);  //开始时间(20字节)
    memcpy(SaveRecordinfo.StopTime,stoptime,20);  //停止时间(20字节)
    SaveRecordinfo.StopTime[19] = ' ';
    SaveRecordinfo.StartTime[19] = ' ';
    Recordqueryinfo_WR(RECODE_DISPOSE1(CurNum%1000),FLASH_ORDER_WRITE,(uint8_t *)&SaveRecordinfo); //写函数
    return TRUE;
}



/*****************************************************************************
* Function     :Unlock_settlementrecord
* Description  :解锁卡时写入的函数
* Input        :BILL_status=结算状态：已结算和未结算(10字节)
                Stop_reason=停止原因
                Charg_mode=充电方式
                GUNnum=枪口号
                CurNum=第几条记录
* Output       :None
* Return       :static
* Note(s)      :
* author       :hycsh
* Contributor  :2022-3-24
*****************************************************************************/
uint8_t  Unlock_settlementrecord(_SHOW_NUM BILL_status,uint32_t CurNum)
{
    SaveRecordinfo.Record_balance=m1_card_info.balance;  //【1】扣款前卡内余额(4字节)
    Dis_ShowCopy(SaveRecordinfo.BillState,BILL_status); //【2】结算状态：已结算和未结算(10字节)
    //memcpy(SaveRecordinfo.SerialNum,"20221565896321999999",20);  //【3】交易流水号(20字节)
    //SaveRecordinfo.chargepwer=((stChTcb.stCh.uiChargeEnergy /10)&0x00ff)<<8|((stChTcb.stCh.uiChargeEnergy /10) & 0xff00)>>8; //【4】充电电量0.01（4字节）
    //Dis_ShowCopy(SaveRecordinfo.BillStopRes,Stop_reason);	//【5】停止原因:正常停止（10字节）
    //SaveRecordinfo.CardNum = (m1_card_info.uidByte[0]<<24) | (m1_card_info.uidByte[1] << 16) |\
    (m1_card_info.uidByte[2] << 8) | (m1_card_info.uidByte[3]);  //【6】复制卡号（4字节）
    //Dis_ShowCopy(SaveRecordinfo.ChargeType,Charg_mode);		//【7】充电方式= APP启动或者刷卡启动（10字节）
    //Dis_ShowCopy(SaveRecordinfo.Gunnum,GUNnum);//【8】接口号（10字节）
    //SaveRecordinfo.chargemoney=stChTcb.stCh.uiAllEnergy;//【9】消费金额 0.01（4字节）
    //memcpy(SaveRecordinfo.StartTime,"2021-01-02-12-6-21",20);  //开始时间(20字节)
    //memcpy(SaveRecordinfo.StopTime,"2021-01-02-12-8-25",20);  //停止时间(20字节)

    Recordqueryinfo_WR(RECODE_DISPOSE1(CurNum%1000),FLASH_ORDER_WRITE,(uint8_t *)&SaveRecordinfo); //写函数
    return TRUE;
}




/*****************************************************************************
* Function     :APP_ClearRecodeInfo
* Description  :显示清除上下和当前页偏移量
* Input        :
* Output       :None
* Return       :static
* Note(s)      :
* author       :hycsh
* Contributor  :2022年3月
*****************************************************************************/
uint8_t APP_ClearRecodeInfo(void)
{
    RECODECONTROL.CurReadRecodeNun=0;
    RECODECONTROL.NextReadRecodeNun=0;
    RECODECONTROL.UpReadRecodeNun=0;
    RECODECONTROL.CurNun=0; //当前的页面编号

    memset(&RECODECONTROL.CurRecode,0,sizeof(_RECORD_INFO));
    memset(&RECODECONTROL.NextRecode,0,sizeof(_RECORD_INFO));
    memset(&RECODECONTROL.UpRecode,0,sizeof(_RECORD_INFO));
    return TRUE;
}


/*****************************************************************************
* Function     :APP_SelectCurChargeRecode
* Description  :点击记录查询时，第一次进入
* Input        :
* Output       :None
* Return       :static
* Note(s)      :
* author       :hycsh
* Contributor  :2022年3月
*****************************************************************************/
uint8_t APP_SelectCurChargeRecode(void)
{
    RECODECONTROL.CurNun++;  //左上角编号
    RECODECONTROL.UpReadRecodeNun=65535; //点击第一次进入时，重要是返回主页使用
    RECODECONTROL.CurReadRecodeNun=RECODECONTROL.RecodeCurNum;//总条数赋值当前的偏移量
    RECODECONTROL.NextReadRecodeNun=RECODECONTROL.CurReadRecodeNun-1; //下一条的偏移量

    Recordqueryinfo_WR(RECODE_DISPOSE1(RECODECONTROL.CurReadRecodeNun%1000),FLASH_ORDER_READ,(uint8_t *)&RECODECONTROL.CurRecode); //读取当前页的数据
    memcpy(&RECODECONTROL.UpRecode,&RECODECONTROL.CurRecode,sizeof(_RECORD_INFO)); //拷贝当前的数据能上一页

    if(RECODECONTROL.NextReadRecodeNun>0)
    {
        Recordqueryinfo_WR(RECODE_DISPOSE1(RECODECONTROL.NextReadRecodeNun%1000),FLASH_ORDER_READ,(uint8_t *)&RECODECONTROL.NextRecode); //读取下一页页面显示倒数第二个的数据
    }

    if(RECODECONTROL.CurReadRecodeNun==0)
    {
        DisplayCommonMenu(&HYMenu32,NULL); 		//当前的偏移等于0时，就是无充电记录
    }
    else if(RECODECONTROL.CurReadRecodeNun==1)
    {
        DispShow_Recode(&RECODECONTROL.CurRecode,&HYMenu31);//显示数据
        PrintNum16uVariable(0x1F80,RECODECONTROL.CurNun);  //显示页码
        DisplayCommonMenu(&HYMenu31,NULL); //最后1条
    } else
    {
        DispShow_Recode(&RECODECONTROL.CurRecode,&HYMenu30);//显示数据
        PrintNum16uVariable(0x1F00,RECODECONTROL.CurNun);  //显示页码
        DisplayCommonMenu(&HYMenu30,NULL);  //多条
    }
    return TRUE;
}


/*****************************************************************************
* Function     :APP_SelectCurChargeRecode
* Description  :第一次进入记录信息后，点击下一条或者下N条
* Input        :num：代表第几条
* Output       :None
* Return       :static
* Note(s)      :
* author       :hycsh
* Contributor  :2022年3月
*****************************************************************************/
uint8_t APP_SelectNextNChargeRecode(uint8_t num)
{
    while(num--)
    {
        RECODECONTROL.CurNun++;  //页码
        RECODECONTROL.UpReadRecodeNun=RECODECONTROL.CurReadRecodeNun; //当前偏移量赋上一条
        RECODECONTROL.CurReadRecodeNun=RECODECONTROL.NextReadRecodeNun;//下一条的值赋给当前
        RECODECONTROL.NextReadRecodeNun--; //执行1次减1次

        memcpy(&RECODECONTROL.UpRecode,&RECODECONTROL.CurRecode,sizeof(_RECORD_INFO)); //把第一次进入读取的数据，当前数据赋值到上一条。
        memcpy(&RECODECONTROL.CurRecode,&RECODECONTROL.NextRecode,sizeof(_RECORD_INFO)); //下一条数据，先赋值于当前数据
        if((RECODECONTROL.NextReadRecodeNun>0)&&(RECODECONTROL.CurNun<1000))
        {
            Recordqueryinfo_WR(RECODE_DISPOSE1(RECODECONTROL.NextReadRecodeNun%1000),FLASH_ORDER_READ,(uint8_t *)&RECODECONTROL.NextRecode); //读取下一页页面显示倒数第二个的数据
        }
        else
        {
            break;  //当下一页为不大于0时，退出循环直接跳出
        }
    }

    if((RECODECONTROL.NextReadRecodeNun==0)||(RECODECONTROL.CurNun>=1000))   //等于0或者编号>=1000时，没有下一页
    {
        DispShow_Recode(&RECODECONTROL.CurRecode,&HYMenu31);//显示数据
        PrintNum16uVariable(0x1F80,RECODECONTROL.CurNun);  //显示页码
        DisplayCommonMenu(&HYMenu31,NULL); //1条
    }
    else
    {
        DispShow_Recode(&RECODECONTROL.CurRecode,&HYMenu30);//显示数据
        PrintNum16uVariable(0x1F00,RECODECONTROL.CurNun);  //显示页码
        DisplayCommonMenu(&HYMenu30,NULL);  //多条
    }
    return TRUE;
}


/*****************************************************************************
* Function     :APP_SelectUpNChargeRecode
* Description  :上1条或者上N条
* Input        :num：代表第几条
* Output       :None
* Return       :static
* Note(s)      :
* author       :hycsh
* Contributor  :2022年3月
*****************************************************************************/
uint8_t APP_SelectUpNChargeRecode(uint8_t num)
{
    static uint8_t Entone=0; //返回上10条时，10条不够时，自动返回第1条标志位
    while(num--)
    {
        RECODECONTROL.CurNun--; //页码就是-1
        RECODECONTROL.NextReadRecodeNun=RECODECONTROL.CurReadRecodeNun;  //当前赋下一条
        RECODECONTROL.CurReadRecodeNun=RECODECONTROL.UpReadRecodeNun;    //上一条赋当前

        memcpy(&RECODECONTROL.NextRecode,&RECODECONTROL.CurRecode,sizeof(_RECORD_INFO));
        memcpy(&RECODECONTROL.CurRecode,&RECODECONTROL.UpRecode,sizeof(_RECORD_INFO));  //把上一条数据赋值给当前

        if((0<RECODECONTROL.UpReadRecodeNun)&&(RECODECONTROL.UpReadRecodeNun<=RECODECONTROL.RecodeCurNum)) //上一页大于或者等于时当前的总条数时，就必须显示多条
        {
            RECODECONTROL.UpReadRecodeNun++;  //上一页就等于+1
            Recordqueryinfo_WR(RECODE_DISPOSE1(RECODECONTROL.UpReadRecodeNun%1000),FLASH_ORDER_READ,(uint8_t*)&RECODECONTROL.UpRecode); //读取下一页页面显示倒数第二个的数据
        }

        //这个是判断是第一页(例如上10条时，10条不够时，最终显示第1条，
        if((RECODECONTROL.UpReadRecodeNun==0)||(RECODECONTROL.UpReadRecodeNun>RECODECONTROL.RecodeCurNum))
        {
            Entone++;
            RECODECONTROL.UpReadRecodeNun=0;
            RECODECONTROL.CurNun=1; //页号
            break;
        }

    }

    if((Entone==2)&&(RECODECONTROL.UpReadRecodeNun==0))
    {
        DispControl.CurSysState = DIP_STATE_NORMAL;
        DisplayCommonMenu(&HYMenu1,NULL);  //显示第1页
        Entone=0;
    }
    else if(RECODECONTROL.CurReadRecodeNun==65535)
    {
        DispControl.CurSysState = DIP_STATE_NORMAL;
        DisplayCommonMenu(&HYMenu1,NULL);  //显示第1页
    }
    else
    {
        DispShow_Recode(&RECODECONTROL.CurRecode,&HYMenu30);//显示数据
        PrintNum16uVariable(0x1F00,RECODECONTROL.CurNun);  //显示页码
        DisplayCommonMenu(&HYMenu30,NULL);  //多
    }
    return TRUE;
}









/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */

//======hycsh  显示桩编号、IP和端口，协议
uint8_t Munu13_ShowSysInfo(void)
{
    uint16_t port;
    uint16_t NetNum;
    uint16_t IP[4];
    uint8_t i;
    uint8_t show_buf[10];
    memset(show_buf,0,sizeof(show_buf));

    PrintStr(0x1680,DisSysConfigInfo.DevNum,sizeof(DisSysConfigInfo.DevNum)); //显示桩编号

    if(NET_YX_SELCT  == XY_HY)  //判断一下点击后,是第几个协议
    {
        Dis_ShowStatus(DIS_ADD(HYMenu13.FrameID,18),SHOW_XY_HY);  //汇誉协议
    }
    else if(NET_YX_SELCT==1)
    {
        Dis_ShowStatus(DIS_ADD(HYMenu13.FrameID,18),SHOW_XY_YKC); //YKC协议
    }
    else if(NET_YX_SELCT==2)
    {
        Dis_ShowStatus(DIS_ADD(HYMenu13.FrameID,18),SHOW_XY_AP); //安培协议
    }
    else if(NET_YX_SELCT==3)
    {
        Dis_ShowStatus(DIS_ADD(HYMenu13.FrameID,18),SHOW_XY_YL1); //预留1
    }

    port = (DisSysConfigInfo.Port & 0x00ff) << 8 | (DisSysConfigInfo.Port & 0xff00) >> 8;
    for(i = 0; i < 4; i++)
    {
        IP[i] = (DisSysConfigInfo.IP[i] & 0x00ff) << 8 | 0;
    }
    memcpy(&show_buf[0],IP,8);
    memcpy(&show_buf[8],&port,sizeof(port));
    PrintStr(DIS_ADD(HYMenu13.FrameID,9),show_buf,sizeof(show_buf));


    if(DisSysConfigInfo.standaloneornet == DISP_NET)  //网络模式
    {
        PrintIcon(0x169E,1); //网络版亮
        PrintIcon(0x1715,0); //单机正常灰
        PrintIcon(0x1716,0);//单机预约灰
    } else if(DisSysConfigInfo.standaloneornet == DISP_CARD) //单机正常
    {
        PrintIcon(0x169E,0); //网络版灰
        PrintIcon(0x1715,1); //单机正常亮
        PrintIcon(0x1716,0);//单机预约灰

    } else if(DisSysConfigInfo.standaloneornet == DISP_CARD_mode) //单机预约模式
    {
        PrintIcon(0x169E,0); //网络版灰
        PrintIcon(0x1715,0); //单机正常亮
        PrintIcon(0x1716,1);//单机预约灰
    }



    //使用内部电表时状态显示
    if(DisSysConfigInfo.energymeter == USERN8209)
    {
        PrintIcon(0x16A0,1); //内部亮
        PrintIcon(0x16A1,0);//外部灰
        //DisSysConfigInfo.energymeter=USERN8209;
    }
    else if(DisSysConfigInfo.energymeter == NOUSERN8209)
    {
        PrintIcon(0x16A0,0); //内部灰
        PrintIcon(0x16A1,1);//外部亮
        //DisSysConfigInfo.energymeter=NOUSERN8209;
    }


    //卡类型
    if(DisSysConfigInfo.cardtype == B0card)
    {
        PrintIcon(0x16A2,1);
        PrintIcon(0x16A3,0);
        PrintIcon(0x16A4,0);
        PrintIcon(0x16A5,0);
    } else if(DisSysConfigInfo.cardtype == B1card)
    {
        PrintIcon(0x16A2,0);
        PrintIcon(0x16A3,1);
        PrintIcon(0x16A4,0);
        PrintIcon(0x16A5,0);
    } else if(DisSysConfigInfo.cardtype == C0card)
    {
        PrintIcon(0x16A2,0);
        PrintIcon(0x16A3,0);
        PrintIcon(0x16A4,1);
        PrintIcon(0x16A5,0);
    } else if(DisSysConfigInfo.cardtype == C1card)
    {
        PrintIcon(0x16A2,0);
        PrintIcon(0x16A3,0);
        PrintIcon(0x16A4,0);
        PrintIcon(0x16A5,1);
    }


    fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));
}



/*****************************************************************************
* Function     :Munu27_ShowSysInfo
* Description  :显示费率详情，包含时间段、费率、服务费
* Input        :entrn 0时，第一次进入，1是上一页，2是下一页
* Output       :None
* Return       :static
* Note(s)      :
* author       :hycsh
* Contributor  :2022年2月
*****************************************************************************/
uint8_t Munu27_ShowSysInfo(uint8_t entrn)
{
    uint8_t i,num=0;  //num为分组
    uint8_t index[48]= {0};  //下标,后期记录停止时间
    uint8_t page;    //总页数
    static uint8_t total=0; //当前的页数

    uint16_t stophour[110],stopminute[110];  //其中停止的时间就是开始的时间
    memset(stophour,0,sizeof(stophour));
    memset(stopminute,0,sizeof(stopminute));


    for(i=0; i<47; i++)
    {
        if(stChRate.ucSegNum[i]!=stChRate.ucSegNum[i+1])
        {
            index[num]=i;
            num++;
        }

        if(i+1==47)
        {
            index[num]=47;
            num++;
        }
    }


    if(num%4==0)
    {
        page=(num/4);
    }
    else
    {
        page=(num/4)+1; //一共多少页
    }


    if(entrn==0)  //entrn 0时，第一次进入，1是上一页，2是下一页
    {
        total=1;
    } else if(entrn==1)
    {
        total--;

    } else
    {
        total++;
    }


    ST_Menu * CurMenu = GetCurMenu();
    if(total<=0)
    {
        DisplayCommonMenu(CurMenu->Menu_PrePage,NULL);
    }
    else if(total==page)
    {
        if((CurMenu == &HYMenu13) || (CurMenu == &HYMenu1))
        {
            DisplayCommonMenu(&HYMenu28,CurMenu);  //只有一页。不显示下一页按钮界面
        }
        else
        {
            DisplayCommonMenu(&HYMenu28,NULL);  //只有一页。不显示下一页按钮界面
        }
    }
    else
    {
        if((CurMenu == &HYMenu13) || (CurMenu == &HYMenu1))
        {
            DisplayCommonMenu(&HYMenu27,CurMenu);  //只有一页。不显示下一页按钮界面
        }
        else
        {
            DisplayCommonMenu(&HYMenu27,NULL);  //只有一页。不显示下一页按钮界面
        }
    }




    //显示时间和分钟
    for(i=0; i<num; i++)
    {
        stophour[i*2+1]= (index[i]+1)*30/60;
        if(stophour[i*2+1]==24)
        {
            stophour[i*2+1]=0;
        }
        stopminute[i*2+1]=(index[i]+1)*30%60;
    }
    if(total==1)  //第一页时显示0时0分
    {
        PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,0),0);  //显示的小时
        PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,1),0);  //显示的分钟
        PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,0),0);  //显示的小时
        PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,1),0);  //显示的分钟
    }
    else   //第二页的第一个时间：显示最后前一页最后一个时间
    {
        PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,0),stophour[(total-2)*8+7]);  //显示的小时
        PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,1),stopminute[(total-2)*8+7]);  //显示的分钟
        PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,0),stophour[(total-2)*8+7]);  //显示的小时
        PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,1),stopminute[(total-2)*8+7]);  //显示的分钟
    }
    //显示27和28的时间
    PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,2),stophour[(total-1)*8+1]);  //显示的小时
    PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,3),stopminute[(total-1)*8+1]);  //显示的分钟
    PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,4),stophour[(total-1)*8+3]);  //显示的小时
    PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,5),stopminute[(total-1)*8+3]);  //显示的分钟
    PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,6),stophour[(total-1)*8+5]);  //显示的小时
    PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,7),stopminute[(total-1)*8+5]);  //显示的分钟
    PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,8),stophour[(total-1)*8+7]);  //显示的小时
    PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,9),stopminute[(total-1)*8+7]);  //显示的分钟

    PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,2),stophour[(total-1)*8+1]);  //显示的小时
    PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,3),stopminute[(total-1)*8+1]);  //显示的分钟
    PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,4),stophour[(total-1)*8+3]);  //显示的小时
    PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,5),stopminute[(total-1)*8+3]);  //显示的分钟
    PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,6),stophour[(total-1)*8+5]);  //显示的小时
    PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,7),stopminute[(total-1)*8+5]);  //显示的分钟
    PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,8),stophour[(total-1)*8+7]);  //显示的小时
    PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,9),stopminute[(total-1)*8+7]);  //显示的分钟

//========显示的单价和服务费用：第2种方法
    for(i=(total-1)*4; i<total*4; i++)
    {
        if(total<page && page!=1 && i<num)  //有下一页 num为总行数
        {
            if((i%4)==0)
            {
                PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,10),stChRate.Prices[stChRate.ucSegNum[index[i]]]/1000);
                PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,14),stChRate.fwPrices[stChRate.ucSegNum[index[i]]]/1000);
            } else if((i%4)==1)
            {
                PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,11),stChRate.Prices[stChRate.ucSegNum[index[i]]]/1000);
                PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,15),stChRate.fwPrices[stChRate.ucSegNum[index[i]]]/1000);
            } else if((i%4)==2)
            {
                PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,12),stChRate.Prices[stChRate.ucSegNum[index[i]]]/1000);
                PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,16),stChRate.fwPrices[stChRate.ucSegNum[index[i]]]/1000);
            } else if((i%4)==3)
            {
                PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,13),stChRate.Prices[stChRate.ucSegNum[index[i]]]/1000);
                PrintNum16uVariable(DIS_ADD(HYMenu27.FrameID,17),stChRate.fwPrices[stChRate.ucSegNum[index[i]]]/1000);
            }
        } else if(total==page && i<num) //有无下一页
        {
            if((i%4)==0)
            {
                PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,10),stChRate.Prices[stChRate.ucSegNum[index[i]]]/1000);
                PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,14),stChRate.fwPrices[stChRate.ucSegNum[index[i]]]/1000);
            } else if((i%4)==1)
            {
                PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,11),stChRate.Prices[stChRate.ucSegNum[index[i]]]/1000);
                PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,15),stChRate.fwPrices[stChRate.ucSegNum[index[i]]]/1000);
            } else if((i%4)==2)
            {
                PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,12),stChRate.Prices[stChRate.ucSegNum[index[i]]]/1000);
                PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,16),stChRate.fwPrices[stChRate.ucSegNum[index[i]]]/1000);
            } else if((i%4)==3)
            {
                PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,13),stChRate.Prices[stChRate.ucSegNum[index[i]]]/1000);
                PrintNum16uVariable(DIS_ADD(HYMenu28.FrameID,17),stChRate.fwPrices[stChRate.ucSegNum[index[i]]]/1000);
            }
        }
    }
    return TRUE;
}









/**
 * @brief dwin 管理员密码下发
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t Munu12_CodeDispose(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    uint8_t i;
    uint32_t code;
#warning"汇誉管理员密码临时固定"
    uint32_t admin_code = 888888;

    if((pvalue == NULL) || (len != 4) )
    {
        return 0;
    }
    code = (pvalue[0] << 24) | (pvalue[1] << 16) | (pvalue[2] << 8) | pvalue[3];

    if(code == admin_code)   //密码正确，跳转到管理员界面
    {

        DisplayCommonMenu(&HYMenu13,HYMenu12.Menu_PrePage);  			//跳转到系统配置界面
        Munu13_ShowSysInfo();
    }
    else
    {
        DisplayCommonMenu(&HYMenu26,HYMenu12.Menu_PrePage);  //密码错误
    }
    return 1;
}

/**
 * @brief dwin 桩编号设置
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t Munu13_DevnumDispose(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    uint8_t i;

    if(pvalue == NULL)
    {
        return 0;
    }
    for (i = 0; i <16; i++)
    {
        DisSysConfigInfo.DevNum[i]  = 0x30;
    }
    if(len > 16)
    {
        len = 16;		//ASICC输入时，可能存在len大于16
    }
    for(i = 0; i < 16; i++)
    {
        if(pvalue[i] >= 0x30 && pvalue[i] <= 0x39)
        {
            DisSysConfigInfo.DevNum[i] = pvalue[i];
        }
        else
        {
            break;
        }
    }

    fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));
    Munu13_ShowSysInfo();
    OSTimeDly(5, OS_OPT_TIME_PERIODIC, &timeerr);
    PrintStr(0x1050,DisSysConfigInfo.DevNum,16);
    return 1;
}



/**
 * @brief dwin 端口设置
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t Munu13_PortSert(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    uint16_t temp; //记录缓存
    if((pvalue == NULL) || (len != 2) )
    {
        return 0;
    }

    temp = (pvalue[0] << 8) | (pvalue[1]);
    DisSysConfigInfo.Port=temp; //这个主要是存储在结构体中
    NetConfigInfo[NET_YX_SELCT].port= temp; //==hycsh当前某一个协议，对应的端口

    fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));
    Munu13_ShowSysInfo();
    return 1;
}


/**
 * @brief dwin IP设置
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t Munu13_IP1Sert(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    //uint8_t temp; //记录缓存
    if((pvalue == NULL) || (len != 2) )
    {
        return 0;
    }
    uint8_t temp; //记录缓存
    temp=pvalue[1];
    DisSysConfigInfo.IP[0]=temp;
    snprintf(NetConfigInfo[NET_YX_SELCT].pIp,sizeof(NetConfigInfo[NET_YX_SELCT].pIp),"%d.%d.%d.%d",\
             DisSysConfigInfo.IP[0],DisSysConfigInfo.IP[1],DisSysConfigInfo.IP[2],DisSysConfigInfo.IP[3]);

    fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));
    Munu13_ShowSysInfo();
    return 1;
}


/**
 * @brief dwin IP设置
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t Munu13_IP2Sert(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    uint8_t i;
    if((pvalue == NULL) || (len != 2) )
    {
        return 0;
    }
    uint8_t temp; //记录缓存
    temp=pvalue[1];
    DisSysConfigInfo.IP[1]=temp;
    snprintf(NetConfigInfo[NET_YX_SELCT].pIp,sizeof(NetConfigInfo[NET_YX_SELCT].pIp),"%d.%d.%d.%d",\
             DisSysConfigInfo.IP[0],DisSysConfigInfo.IP[1],DisSysConfigInfo.IP[2],DisSysConfigInfo.IP[3]);

    fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));
    Munu13_ShowSysInfo();
    return 1;
}

/**
 * @brief dwin IP设置
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t Munu13_IP3Sert(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    if((pvalue == NULL) || (len != 2) )
    {
        return 0;
    }
    uint8_t temp; //记录缓存
    temp=pvalue[1];
    DisSysConfigInfo.IP[2]=temp;
    snprintf(NetConfigInfo[NET_YX_SELCT].pIp,sizeof(NetConfigInfo[NET_YX_SELCT].pIp),"%d.%d.%d.%d",\
             DisSysConfigInfo.IP[0],DisSysConfigInfo.IP[1],DisSysConfigInfo.IP[2],DisSysConfigInfo.IP[3]);
    fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));
    Munu13_ShowSysInfo();
    return 1;
}

/**
 * @brief dwin IP设置
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t Munu13_IP4Sert(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    if((pvalue == NULL) || (len != 2) )
    {
        return 0;
    }
    uint8_t temp; //记录缓存
    temp=pvalue[1];
    DisSysConfigInfo.IP[3]=temp;
    snprintf(NetConfigInfo[NET_YX_SELCT].pIp,sizeof(NetConfigInfo[NET_YX_SELCT].pIp),"%d.%d.%d.%d",\
             DisSysConfigInfo.IP[0],DisSysConfigInfo.IP[1],DisSysConfigInfo.IP[2],DisSysConfigInfo.IP[3]);

    fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));
    Munu13_ShowSysInfo();
    return 1;
}



/*****************************************************************************
* Function     :timeSert
* Description  :系统设置2：时间显示
* Input        :addr=屏幕地址  pvalue=数据指针  len=数据长度
* Output       :None
* Return       :static
* Note(s)      :
* author       :hycsh
* Contributor  :2022年5月
*****************************************************************************/
static uint8_t timeSert(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    uint32_t buf[6];
    if(pvalue == NULL)
    {
        return 0;
    }
    //ASCII码值转化成数字，用10进制或者16进制表示
    buf[0]=((pvalue[0]-'0')*1000 + (pvalue[1]-'0')*100+(pvalue[2]-'0')*10+ (pvalue[3]-'0'));
	if(pvalue[7]==0xFF)
  {
     pvalue[7]=pvalue[6];
    pvalue[6]=0x30;
  }
  if(pvalue[11]==0xFF)
  {
     pvalue[11]=pvalue[10];
    pvalue[10]=0x30;
  }
  if(pvalue[15]==0xFF)
  {
     pvalue[15]=pvalue[14];
    pvalue[14]=0x30;
  }
  
  if(pvalue[19]==0xFF)
  {
     pvalue[19]=pvalue[18];
    pvalue[18]=0x30;
  }
  if(pvalue[23]==0xFF)
  {
     pvalue[23]=pvalue[22];
    pvalue[22]=0x30;
  }
    buf[1]=((pvalue[6]-'0')*10+ (pvalue[7]-'0'));
    buf[2]=((pvalue[10]-'0')*10+ (pvalue[11]-'0'));
    buf[3]=((pvalue[14]-'0')*10+ (pvalue[15]-'0'));
    buf[4]=((pvalue[18]-'0')*10+ (pvalue[19]-'0'));
    buf[5]=((pvalue[22]-'0')*10+ (pvalue[23]-'0'));
    //设置时间

    set_date(buf[0], buf[1], buf[2]);
    set_time(buf[3], buf[4], buf[5]);

    return 1;
}










//======模式1：按电量充电====
static uint8_t Electric_charing(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    if(pvalue==NULL)
    {
        return 0;
    }
    stChTcb.stChCtl.ucChMode = 1;
    stChTcb.stChCtl.uiStopParam = ((pvalue[0]&0x00FF)<<8 | pvalue[1]);
    DisplayCommonMenu(&HYMenu25,NULL);  /*跳转到刷卡开始充电界面，这时候就是可以刷卡了*/
    return 1;
}




/* 模式3：按金额充电*/
static uint8_t Amount_charging(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    if(pvalue==NULL)
    {
        return 0;
    }
    stChTcb.stChCtl.ucChMode = 3;
    stChTcb.stChCtl.uiStopParam = ((pvalue[0]&0x00FF)<<8 | pvalue[1]);  //金额
    DisplayCommonMenu(&HYMenu25,NULL);  /*跳转到刷卡开始充电界面，这时候就是可以刷卡了*/
    return 1;
}



/* 模式2：按时间充电 */
static uint8_t Time_charging(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    if(pvalue==NULL)
    {
        return 0;
    }
    stChTcb.stChCtl.ucChMode = 2;
    stChTcb.stChCtl.uiStopParam = ((pvalue[0]&0x00FF)<<8 | pvalue[1]);   //时间(分钟计算)
    DisplayCommonMenu(&HYMenu25,NULL);  /*跳转到刷卡开始充电界面，这时候就是可以刷卡了*/
    return 1;
}


/* 模式5：定时充电 */
static uint8_t Timing_charging(uint16_t addr,uint8_t *pvalue,uint8_t len)
{
    stChTcb.stCh.Reservetime = cp56time(NULL); //记录开始预约充电时的时间（距1970年秒数）

    if(pvalue==NULL)
    {
        return 0;
    }
    stChTcb.stChCtl.ucChMode = 5;
    stChTcb.stChCtl.uiStopParam = ((pvalue[0]&0x00FF)<<8 | pvalue[1]);   //时间(分钟计算)



    stChTcb.stCh.uiChStartTime = gs_SysTime;

    //判断一下输入的时间如果等于当前的时间，就返回主页面，清空结构体
    if(stChTcb.stCh.uiChStartTime.ucHour == stChTcb.stChCtl.uiStopParam)
    {
        memset(&stChTcb.stChCtl,0,sizeof(stChTcb.stChCtl));  //启停结构体清零
        DispControl.CurSysState = DIP_STATE_NORMAL;  //无需界面再次切换
        return 0;
    }
    else
    {
        DispControl.CurSysState = DIP_CARD_SHOW;  //无需界面再次切换
        DisplayCommonMenu(&HYMenu25,NULL);  /*跳转到刷卡开始充电界面，这时候就是可以刷卡了*/
    }



    return 1;
}






const _DISP_ADDR_FRAME Disp_RecvFrameTable[] =
{
    /********************************汇誉屏幕*************************/
    {INPUT_MENU6_CODE, Munu12_CodeDispose },  // 界面21输入卡密码地址
    {DIS_ADD(13,0),	Munu13_DevnumDispose }, //桩编号处理
    {DIS_ADD(13,0x0D),	Munu13_PortSert	},	//端口设置
    {DIS_ADD(13,0x09),	Munu13_IP1Sert	},	//ip设置
    {DIS_ADD(13,0x0A),	Munu13_IP2Sert	},	//ip设置
    {DIS_ADD(13,0x0B),	Munu13_IP3Sert	},	//ip设置
    {DIS_ADD(13,0x0C),	Munu13_IP4Sert	},	//ip设置
    {0x1705			,	timeSert	},			//时间设置
    {DIS_ADD(17,0x0D), Electric_charing}, //按电量充
    {DIS_ADD(18,0x0D),Amount_charging},  //按金额充
    {DIS_ADD(19,0x0D),Time_charging},  //按时间充
    {DIS_ADD(20,0x0D),Timing_charging},  //定时充
};





/*****************************************************************************
* Function     : Period_WriterFmRecode
* Description  : 在充电中周期性储存记录交易记录
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
static uint8_t Period_WriterFmBill(uint32_t time)
{
    static uint16_t count= 0;


    if(stChTcb.ucState == CHARGING)
    {
        if(++count >= ((CM_TIME_5_MIN)/time) )   //之前为30s存一下，flash用不了多久，目前临时改成5分钟一次 20210623
            //if(++count >= ((CM_TIME_10_SEC*4)/time) )   //之前为30s存一下，flash用不了多久，目前临时改成5分钟一次 20210623
        {

            count = 0;
            //写入A枪记录
#if(NET_YX_SELCT == XY_AP)
            {
                if(APP_GetStartNetState(GUN_A) == NET_STATE_ONLINE)
                {
                    WriterFmBill(GUN_A,1);			//在线保存
                }
            }
#else
            {
                WriterFmBill(GUN_A,1);
            }
#endif

        }
    }
    else
    {
        count = 0;
    }
    return TRUE;

}


/**
 * @brief 迪文显示屏线程
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void rt_dwin_period_entry(void)
{
    static uint32_t curtick = 0;
    static uint32_t lasttick = 0;

    curtick = OSTimeGet(&timeerr);

    /* 大致为1s中时间 */
    if((curtick - lasttick) > CM_TIME_1_SEC)
    {
        lasttick = curtick;

        if(DispControl.CountDown >= 1)
        {
            DispControl.CountDown--;
        }
        if(DisSysConfigInfo.standaloneornet == DISP_NET)
        {
            Period_WriterFmBill(CM_TIME_1_SEC);    //网络状态下周期性存储数据
        }
    }

}


/**
 * @brief 变量处理
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t DealWithVari(uint16_t addr, uint8_t *pvalue,uint8_t len)
{
    if (addr ==NULL ||pvalue ==NULL || !len)
    {
        return 0;
    }
    for (uint8_t i = 0; i < sizeof(Disp_RecvFrameTable)/sizeof(_DISP_ADDR_FRAME); i++)
    {
        if (Disp_RecvFrameTable[i].variaddr == addr)                    //查找地址
        {
            if (Disp_RecvFrameTable[i].Fun)                             //找到相同变量地址
            {
                return Disp_RecvFrameTable[i].Fun(addr, pvalue, len);   //变量处理
            }
        }
    }
    return 1;
}


/**
 * @brief 数据接收解析
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t APP_DisplayRecvDataAnalyze(uint8_t *pdata, uint8_t len)
{
    ST_Menu* CurMenu = GetCurMenu();

    _LCD_KEYVALUE keyval = (_LCD_KEYVALUE)0;                //取键值

    uint16_t KeyID = 0;       								//按键的ID与页面的ID对应上

    if  ( (pdata == NULL) || (len < 6) || CurMenu == NULL)                       //长度数据做保护
    {
        return 0;
    }

    uint8_t  datalen = pdata[2];                              //数据长度 = 帧长度-帧头（2byte）-自身占用空间（1byte）
    uint8_t  cmd     = pdata[3];                              //命令
    uint16_t lcdhead = ((pdata[0]<<8) | pdata[1]);
	
	//临时配置
	if(len > strlen("set pileId:"))
	{
		if(strncmp((char*)pdata,"set pileId:",strlen("set pileId:")) == 0)
		{
			//设置设备号
			if((len - strlen("set pileId:")) > 16)  //最大为16
			{
				UART2SENDBUF((uint8 *)"set pileId fail\r\n",strlen("set pileId fail\r\n"));
				return 0;
			}
			memset(DisSysConfigInfo.DevNum,0,sizeof(DisSysConfigInfo.DevNum));
			memcpy(DisSysConfigInfo.DevNum,&pdata[strlen("set pileId:")],len - strlen("set pileId:"));
			 fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));
			UART2SENDBUF((uint8 *)"set pileId success\r\n",strlen("set pileId success\r\n"));
			
		}
		if(strncmp((char*)pdata,"set ZFpileId:",strlen("set ZFpileId:")) == 0)
		{
			//设置设备号
			if((len - strlen("set ZFpileId:")) > 16)  //最大为16
			{
				UART2SENDBUF((uint8 *)"set ZFpileId fail\r\n",strlen("set ZFpileId fail\r\n"));
				return 0;
			}
			memset(DisSysConfigInfo.ZFDevNum,0,sizeof(DisSysConfigInfo.ZFDevNum));
			memcpy(DisSysConfigInfo.ZFDevNum,&pdata[strlen("set ZFpileId:")],len - strlen("set ZFpileId:"));
			 fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));
			UART2SENDBUF((uint8 *)"set ZFpileId success\r\n",strlen("set ZFpileId success\r\n"));
			
		}
	}
	
    if (lcdhead == DWIN_LCD_HEAD)            //判断帧头帧尾
    {
        if ( (datalen + 3 != len) )
        {
            return 0;
        }
        if(cmd == VARIABLE_READ)                             //读变量地址返回数据
        {
            uint16_t variaddr  = ((pdata[4]<<8) | pdata[5]);  //提取变量地址
            uint8_t  varilen   = pdata[6] * 2;                //提取变量数据长度(这里转换成字节)
            uint8_t *varivalue = &pdata[7];                   //提取变量值开始地址

            if(variaddr == KEY_VARI_ADDR)                   //所有的按键地址都是0x0000 只是用键值去区分
            {
                KeyID = (pdata[datalen+1]<<8) | pdata[datalen+2];	   //取按键ID
                keyval = (_LCD_KEYVALUE)(pdata[datalen+2] & 0x7f);     //取键值,取低7位
                if((DIS_ADD_KEY(CurMenu->FrameID,keyval) + 0x1000) == KeyID)			//只有在当前界面上的按键才有效
                {
                    /* 按键动作 */
                    DealWithKey(&keyval);
                }
            }
            else                                            //变量数据返回
            {
                DealWithVari(variaddr,varivalue,varilen);   //变量数据处理
            }

        }

    }
    return 1;
}
/**
 * @brief 显示信号强度
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t DispShow_NetState(void)
{
    static uint8_t state[2] = {0},laststate[2] = {0xff},num=1;   //变化了才执行

    //CSQ 0~31 99
    //分5个等级
    if(DisSysConfigInfo.standaloneornet == DISP_NET) //等于网络时才会显示，不等于网络时，就是默认
    {
        if(	APP_GetAppRegisterState(0))   //网络在线
        {
            state[0] = 0;
            if(state[0] != laststate[0])
            {
                laststate[0] = state[0];
                PrintIcon(0x2060,1);
            }
        }
        else
        {
            state[0] = 1;
            if(state[0] != laststate[0])
            {
                laststate[0] = state[0];
                PrintIcon(0x2060,0);
            }
        }



        //第一次上电判断状态等于0，显示第一个灰色
        if((APP_GetCSQNum()== 0)&&(num==1))
        {
            PrintIcon(0x2050,0);
            num=0;
        }
        else if((APP_GetCSQNum()== 0) || (APP_GetCSQNum() == 99) )  //中间断网时，
        {
            state[1] = 0;
            if(state[1] != laststate[1])
            {
                laststate[1] = state[1];
                PrintIcon(0x2050,0);
            }
        }
        else if(APP_GetCSQNum() == 1)
        {
            state[1] = 1;
            if(state[1] != laststate[1])
            {
                laststate[1] = state[1];
                PrintIcon(0x2050,1);
            }
        }
        else if(APP_GetCSQNum()< 20)
        {
            state[1] = 2;
            if(state[1]!= laststate[1])
            {
                laststate[1] = state[1];
                PrintIcon(0x2050,2);
            }
        }
        else if(APP_GetCSQNum() < 25)
        {
            state[1] = 3;
            if(state[1] != laststate[1])
            {
                laststate[1] = state[1];
                PrintIcon(0x2050,3);
            }
        }
        else
        {
            state[1] = 4;
            if(state[1] != laststate[1])
            {
                laststate[1] = state[1];
                PrintIcon(0x2050,4);
            }
        }
    }
    return 1;
}

/**
 * @brief 迪文显示初始化,显示固定不变的,只显示一次
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void dwin_show_state(void)
{
    /* 只显示一次 */
    static uint8_t show = 1;

    static char s_qr_buf[100] = {0};
   char * net = "https://api.huichongchongdian.com/cpile/coffee/";
	// char * net = "https://cdz.greenxiaoling.com/cpile/coffee/";  
    char * apnet = "https://zjec.evshine.cn/scan/scan/scanTransfer?gunNo=";
    char * ykcnet = "http://www.ykccn.com/MPAGE/index.html?pNum=";
    uint8_t len;
    uint8_t code[17];
    uint8_t apcode[18];
    uint8_t buf[100]  = {0};
    char vison[8] = {0};
//    uint32_t cpvolt;

//    //OSTimeDly(500, OS_OPT_TIME_PERIODIC, &timeerr);
//    DispShow_NetState(); //显示信号强度——只有网络状态下才会显示

//    //  LIB_Bcd2Asc(gs_AppCharge.ucDeviceID, sizeof(gs_AppCharge.ucDeviceID), ucDeviceID);

//    //=====以下是测试显示cp不停的去显示=============
//    cpvolt=stChTcb.stGunStat.uiCpVolt/10; //显示4位电压-小数点2位
//    PrintNum32uVariable(0x108A,cpvolt);


    if(show)
    {
        DisplayQRCode(0x5000,buf,100);
        snprintf(vison,sizeof(vison),"%s","V2.1");				//程序版本
        PrintStr(0x1080,(uint8_t*)vison,strlen(vison));  //显示版本号
        PrintStr(0x1050,DisSysConfigInfo.DevNum,16); //显示桩编号
        show = 0;  //原来是0
        if(NET_YX_SELCT == XY_AP)
        {
            len = strlen(apnet);
            if(len > 80)
            {
                return;
            }
            memcpy(buf,apnet,len);

            memcpy(&apcode,DisSysConfigInfo.DevNum,16);
            apcode[16] = '0';
            apcode[17] = '1';
            //memcpy(&buf[len],apcode,18);
            memcpy(&buf[len],apcode,16);     //单枪不需要+01
            DisplayQRCode(0x5000,buf,16 + len);
        }
        if((DisSysConfigInfo.standaloneornet == DISP_CARD) || (DisSysConfigInfo.standaloneornet == DISP_CARD_mode))  //等于单机时，才会显示图标
        {
            PrintIcon(0x6000,1);
        }
        else
        {
            PrintIcon(0x6000,200);
        }
    }




    if(NET_YX_SELCT == XY_HY)
    {
        len = strlen(net);
        if(len > 80)
        {
            return;
        }
        memcpy(buf,net,len);
        memcpy(code,DisSysConfigInfo.DevNum,sizeof(DisSysConfigInfo.DevNum));

        code[16] = '0';
        memcpy(&buf[len],code,17);
        //PrintStr(0x1050,buf,17+len);
//				PrintStr(0x1050,DisSysConfigInfo.DevNum,16);
        /* 长度或者内容不一致需要显示二维码 */
        if((strncmp(s_qr_buf,(char *)buf ,17+len)))
        {


            memcpy(s_qr_buf,buf,17+len);
            DisplayQRCode(0x5000,buf,17+len);
        }
    }

    if(NET_YX_SELCT == XY_YKC)
    {
        len = strlen(ykcnet);
        if(len > 80)
        {
            return;
        }
        memcpy(buf,ykcnet,len);
        memcpy(code,DisSysConfigInfo.DevNum,14);

        code[14] = '0';
        code[15] = '1';
        memcpy(&buf[len],code,16);
        if((strncmp(s_qr_buf,(char *)buf ,16+len)))
        {
            memcpy(s_qr_buf,buf,16+len);
            DisplayQRCode(0x5000,buf,16+len);
        }
    }
	if(NET_YX_SELCT == XY_JM)
	{
		app_get66_qr(buf,sizeof(buf));
		if((strncmp(s_qr_buf,(char *)buf,sizeof(buf))) )
        {
            memcpy(s_qr_buf,buf,sizeof(buf));
            DisplayQRCode(0x5000,buf,strlen((char*)buf));
        }
	}
//    /* 长度或者内容不一致需要显示二维码 */
//    if((strncmp(s_qr_buf,(char *)buf ,17+len)))
//    {


//        memcpy(s_qr_buf,buf,17+len);
//        DisplayQRCode(0x5000,buf,17+len);
//    }
}


/**
 * @brief 结算界面显示
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void dwin_show_record(void)
{
    memset((uint8_t*)&RecordInfo,0,sizeof(RecordInfo));

    memcpy(RecordInfo.SerialNum,"45646456456465",20);
    memcpy(RecordInfo.StartTime,"2021-01-02-12-6-30",20);
    memcpy(RecordInfo.StopTime,"2021-01-02-12-8-30",20);

    RecordInfo.chargepwer = 0x0100;
    RecordInfo.chargemoney = 0x0200;
    Dis_ShowCopy(RecordInfo.BillState,SHOW_BILL);  				   //已经结算
    Dis_ShowCopy(RecordInfo.BillStopRes,SHOW_STOP_ERR_NONE); //正常停止
    Dis_ShowCopy(RecordInfo.ChargeType,SHOW_START_APP);		  //APP启动
    PrintStr(DIS_ADD(HYMenu12.FrameID,0),(uint8_t*)&RecordInfo,sizeof(RecordInfo));
}








extern SYSTEM_RTCTIME gs_SysTime;

#define ADDR_TIME           	    (0x3000)		          	//时间变量地址 "xx:xx:xx"
/**
 * @brief 显示时间
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t Dis_ShowTime(uint16_t add_show ,SYSTEM_RTCTIME gRTC)
{
    char * buf[30];
    ST_Menu* CurMenu = GetCurMenu();
    if(CurMenu == NULL)
    {
        return 0;
    }
    memset(buf,0,sizeof(buf));

    snprintf((char *)buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",(gRTC.ucYear + 1900),(gRTC.ucMonth),(gRTC.ucDay),\
             (gRTC.ucHour),(gRTC.ucMin),(gRTC.ucSec));
    PrintStr(add_show,(uint8_t *)buf,30);   //显示状态
    SetVariColor((add_show & 0xFFC0 ) + 0x30,WHITE);
    return 1;
}


//======时间设置里面初始化时间
uint8_t Dis_SYSShowTime(ST_Menu *pMenu ,SYSTEM_RTCTIME gRTC)
{
    char * buf[30];
    ST_Menu* CurMenu = GetCurMenu();
    if(CurMenu == NULL)
    {
        return 0;
    }
    memset(buf,0,sizeof(buf));
    snprintf((char *)buf, sizeof(buf), "%04d--%02d--%02d--%02d::%02d::%02d",(gRTC.ucYear + 1900),(gRTC.ucMonth),(gRTC.ucDay),\
             (gRTC.ucHour),(gRTC.ucMin),(gRTC.ucSec));
    PrintStr(DIS_ADD(pMenu->FrameID,5),(uint8_t *)buf,30); //显示年月日
    return 1;
}

/**
 * @brief 显示RTC
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t Disp_ShowRTC(void)
{
    Dis_ShowTime(ADDR_TIME,gs_SysTime);
    return 0;
}
//只显示配置时间的页面
uint8_t Disp_Showsettime(void)
{
    Dis_SYSShowTime(&HYMenu14,gs_SysTime);
    return 0;
}











/**
 * @brief 迪文显示信息,在不同界面需要显示不同的信息
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void dwin_show(void)
{
    const ST_Menu* CurMenu = GetCurMenu();
    _CHARGE_INFO ShowChargeInfo;
    static uint32_t curtick = 0;
    static uint32_t lasttick = 0;
	uint32_t cpvolt;

    curtick = OSTimeGet(&timeerr);

    /* 大致为1s时间 */
    if((curtick - lasttick) > 1000)
    {
        lasttick = curtick;
    }
    else
    {
        return;
    }


    //OSTimeDly(500, OS_OPT_TIME_PERIODIC, &timeerr);
    DispShow_NetState(); //显示信号强度——只有网络状态下才会显示

    //  LIB_Bcd2Asc(gs_AppCharge.ucDeviceID, sizeof(gs_AppCharge.ucDeviceID), ucDeviceID);

    //=====以下是测试显示cp不停的去显示=============
    cpvolt=stChTcb.stGunStat.uiCpVolt/10; //显示4位电压-小数点2位
    PrintNum32uVariable(0x108A,cpvolt);
    /* 界面显示RTC */
    Disp_ShowRTC();


    /* 待机界面 */
    if(CurMenu == &HYMenu25)
    {
        dwin_show_record();
    }

    /* 记录查询界面 */ #warning 测试记录查询页面
    if(CurMenu == &HYMenu30||CurMenu == &HYMenu31)
    {
        // dwin_show_record1();
    }





    /* 在充电界面 */
    if(CurMenu == &HYMenu3)
    {
//#warning hycsh "模拟电流start"
//        stChTcb.HLW8112_T.usCurrent=0x07D0;
//#warning hycsh "模拟电流end"
//#warning hycsh "模拟金额和电量start"
//        static uint32_t i=0;
//        i+=100;
//        stChTcb.stCh.uiAllEnergy=10000+i;
//        stChTcb.stCh.uiChargeEnergy=100+i;
//#warning hycsh "模拟金额和电量end"

        if(DisSysConfigInfo.energymeter == 1)
        {
            ChargeInfo.chargecur = stChTcb.stHLW8112.usCurrent;
            ChargeInfo.chargevol = stChTcb.stHLW8112.usVolt;
        }
        else
        {
            ChargeInfo.chargecur = dlt645_info.out_cur*100;
            ChargeInfo.chargevol = dlt645_info.out_vol*10;
        }

        ChargeInfo.chargemoney = stChTcb.stCh.uiAllEnergy/100;   //充电金额
        ShowChargeInfo.chargemoney = (ChargeInfo.chargemoney & 0x00ff) << 8 |  (ChargeInfo.chargemoney & 0xff00) >> 8;

        ShowChargeInfo.chargecur = (ChargeInfo.chargecur & 0x00ff) << 8 |  (ChargeInfo.chargecur & 0xff00) >> 8;

        ShowChargeInfo.chargevol = (ChargeInfo.chargevol & 0x00ff) << 8 |  (ChargeInfo.chargevol & 0xff00) >> 8;

        ChargeInfo.chargepwer = stChTcb.stCh.uiChargeEnergy /10; //充电电量
        ShowChargeInfo.chargepwer = (ChargeInfo.chargepwer & 0x00ff) << 8 |  (ChargeInfo.chargepwer & 0xff00) >> 8;

        //卡内余额
        if(DisSysConfigInfo.standaloneornet == DISP_NET)
        {
            m1_card_info.balance = 0;
        }
        ShowChargeInfo.balance =  (m1_card_info.balance & 0x000000FF) << 24 | (m1_card_info.balance  & 0x0000FF00) << 8 | \
                                  (m1_card_info.balance & 0x00FF0000) >> 8 | (m1_card_info.balance & 0xFF000000) >> 24;

        PrintStr(DIS_ADD(HYMenu3.FrameID,0),(uint8_t*)&ShowChargeInfo,sizeof(ShowChargeInfo));
    }
    return;
}


///**
//* @brief 0表示灭   1~13 表示闪烁次数    0xaa表示常量
// * @param[in]
// * @param[out]
// * @return
// * @note
// */
//void LED_Control(uint8_t state)
//{
//	
//}

/**
 * @brief 迪文状态处理，主要是为了切换界面
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void rt_dwin_state_dispose(uint8_t state)
{
    ST_Menu* pcur = GetCurMenu();  //获取当前页面
    uint8_t updata_curpage;
	
	  static uint32_t curtick = 0,lasttick = 0;
    curtick = OSTimeGet(&timeerr);
	
	

    //系统升级
//	if(stWzGprsWork.stFileDown.ucDownWorkStat !=  WZGPRS_UPGRADE_IDIE)
//	{
//		//说明在升级程序
//		updata_curpage = gs_AppCharge.ucProgressPer / 10;
//		 if(gs_AppCharge.ucProgressPer > 95)
//        {
//           DisplayCommonMenu(&HYMenu24,NULL);
//        }
//		else
//		{
//			switch(updata_curpage)
//			{
//				case 0:
//					DisplayCommonMenu(&HYMenu14,NULL);
//					break;
//				case 1:
//					DisplayCommonMenu(&HYMenu15,NULL);
//					break;
//				case 2:
//					DisplayCommonMenu(&HYMenu16,NULL);
//					break;
//				case 3:
//					DisplayCommonMenu(&HYMenu17,NULL);
//					break;
//				case 4:
//					DisplayCommonMenu(&HYMenu18,NULL);
//					break;
//				case 5:
//					DisplayCommonMenu(&HYMenu19,NULL);
//					break;
//				case 6:
//					DisplayCommonMenu(&HYMenu20,NULL);
//					break;
//				case 7:
//					DisplayCommonMenu(&HYMenu21,NULL);
//					break;
//				case 8:
//					DisplayCommonMenu(&HYMenu22,NULL);
//					break;
//				case 9:
//					DisplayCommonMenu(&HYMenu23,NULL);
//					break;
//	//			case 10:
//	//				DisplayCommonMenu(&HYMenu24,NULL);
//	//				break;
//
//				defaule:
//					break;
//
//			}
//		}
//
//		return ;
//	}


    if(DispControl.CurSysState != DIP_STATE_NORMAL)   //配置阶段页面不自动进行切换
    {
        return;
    }

    switch(state)
    {
    case INSERT:  //无故障而且枪连接
        if(DisSysConfigInfo.standaloneornet == DISP_CARD_mode)   /*只有单机预约模式显示*/
        {
            DisplayCommonMenu(&HYMenu16,NULL);
        }
        else if(pcur != &HYMenu25)  /*其他模式只会选择插枪界面*/
        {
#if(NET_YX_SELCT == XY_YKC|| NET_YX_SELCT == XY_JM)
            if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
            {
                mq_service_send_to_4gsend(APP_SJDATA_QUERY,GUN_A ,0 ,NULL);
            }
#endif
            DisplayCommonMenu(&HYMenu25,NULL);
        }
		if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
		{
			KEY_LED3_ON;      //充电灯
			KEY_LED1_OFF;
			KEY_LED2_OFF
		}
		else
		{
			if((curtick - lasttick) > 1000)
			{
				lasttick = curtick;
				KEY_LED2_OFF;
			}
			else
			{
				KEY_LED2_ON;
			}	
			KEY_LED1_OFF;
			KEY_LED3_OFF;
		}
        break;

				
    case STANDBY:	   //枪未连接				 无故障而且枪未连接
        if(pcur != &HYMenu1)
        {
#if(NET_YX_SELCT == XY_YKC|| NET_YX_SELCT == XY_JM)
            if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
            {
                mq_service_send_to_4gsend(APP_SJDATA_QUERY,GUN_A ,0 ,NULL);
            }
#endif
            DisplayCommonMenu(&HYMenu1,NULL);
        }
        if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
		{
			KEY_LED1_ON;      //待机灯
			KEY_LED2_OFF;
			KEY_LED3_OFF
		}
		else
		{
			if((curtick - lasttick) > 1000)
			{
				lasttick = curtick;
				KEY_LED2_OFF;
			}
			else
			{
				KEY_LED2_ON;
			}	
			KEY_LED1_OFF;
			KEY_LED3_OFF;
		}
        break;

				
    case CHARGER_FAULT:		//故障
        if(pcur != &HYMenu2)
        {
#if(NET_YX_SELCT == XY_YKC|| NET_YX_SELCT == XY_JM)
            if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
            {
                mq_service_send_to_4gsend(APP_SJDATA_QUERY,GUN_A ,0 ,NULL);
            }
#endif
            DisplayCommonMenu(&HYMenu2,NULL);
        }
		if((curtick - lasttick) > 500)
		{
			lasttick = curtick;
			KEY_LED2_OFF;
		}
		else
		{
			KEY_LED2_ON;
		}	
		KEY_LED1_OFF;
		KEY_LED3_OFF;
        break;

				
    case WAIT_CAR_READY:	//启动中
        if(pcur != &HYMenu4)
        {
            DisplayCommonMenu(&HYMenu4,NULL);
        }
		if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
		{
			KEY_LED3_ON;      //充电灯
			KEY_LED1_OFF;
			KEY_LED2_OFF
		}
		else
		{
			if((curtick - lasttick) > 1000)
			{
				lasttick = curtick;
				KEY_LED2_OFF;
			}
			else
			{
				KEY_LED2_ON;
			}	
			KEY_LED1_OFF;
			KEY_LED3_OFF;
		}
        break;

    case WAIT_STOP:	//停止中
        DisplayCommonMenu(&HYMenu9,NULL);  //跳转到结算界面
		if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
		{
			KEY_LED3_ON;      //充电灯
			KEY_LED1_OFF;
			KEY_LED2_OFF
		}
		else
		{
			if((curtick - lasttick) > 1000)
			{
				lasttick = curtick;
				KEY_LED2_OFF;
			}
			else
			{
				KEY_LED2_ON;
			}	
			KEY_LED1_OFF;
			KEY_LED3_OFF;
		}
        break;

    case CHARGING:	//充电中
        if(pcur != &HYMenu3)			//不在停止刷卡界面
        {
#if(NET_YX_SELCT == XY_YKC || NET_YX_SELCT == XY_JM)
            if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
            {
                mq_service_send_to_4gsend(APP_SJDATA_QUERY,GUN_A ,0 ,NULL);
            }
#endif
            DisplayCommonMenu(&HYMenu3,NULL);
        }
        if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
		{
			if((curtick - lasttick) > 1500)
			{
				lasttick = curtick;
				KEY_LED3_OFF;
			}
			else
			{
				KEY_LED3_ON;
			}	
			KEY_LED1_OFF;
			KEY_LED2_OFF;
		}
		else
		{
			if((curtick - lasttick) > 1000)
			{
				lasttick = curtick;
				KEY_LED2_OFF;
			}
			else
			{
				KEY_LED2_ON;
			}	
			KEY_LED1_OFF;
			KEY_LED3_OFF;
		}
        break;


//		case GUN_STARTFAIL:	//启动失败
//			DisplayCommonMenu(&HYMenu6,NULL);  //跳转到结算界面
//			break;
    default:
        break;
    }
}

//===s卡设置费率
uint8_t Set_judge_rete_info(uint32_t price)
{
    uint8_t i=0;
    for(i=0; i<16; i++) //0-8点一个时间段
    {
        stChRate.ucSegNum[i]=0;
    }

    for(i=16; i<44; i++) //8-22点
    {
        stChRate.ucSegNum[i]=1;
    }

    for(i=44; i<48; i++) //22-0点
    {
        stChRate.ucSegNum[i]=2;
    }
    //memset(stChRate.ucSegNum,0,sizeof(stChRate.ucSegNum)); //当前分组只分一组 0-0
    stChRate.fwPrices[0] = 0;
    stChRate.fwPrices[1] = 0;
    stChRate.fwPrices[2] = 0;
    stChRate.fwPrices[3] = 0;
    stChRate.Prices[0] = price;
    stChRate.Prices[1] = price;
    stChRate.Prices[2] = price;
    stChRate.Prices[3] = price;

    fal_partition_write(CHARGE_RATE,0,(uint8_t*)&stChRate,sizeof(RATE_T));
    return 1;
}






/**
 * @brief 费率是否正常
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t judge_rete_info(void)
{
    uint8_t count = 0;

    for(count =0; count < sizeof(stChRate.ucSegNum); count++)
    {
        if(stChRate.ucSegNum[count] > 3)  //段数量为0 - 3
        {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief flash 存储参数初始化
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void flash_para_init(void)
{
    uint8_t len;
    if(fal_partition_read(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO)) < 0)  //读取屏幕设置信息
    {
        printf("Partition read error! Flash device(%d) read error!", DWIN_INFO);
    }
    if(fal_partition_read(CHARGE_RATE,0,(uint8_t*)&stChRate,sizeof(RATE_T)) < 0)  //读取费率信息
    {
        printf("Partition read error! Flash device(%d) read error!", CHARGE_RATE);
    }

    if((DisSysConfigInfo.IP[0] == 0xff) && (DisSysConfigInfo.IP[1] == 0xff) && \
            (DisSysConfigInfo.IP[2] == 0xff) && (DisSysConfigInfo.IP[3] == 0xff))
    {
        //说明flash没有设置过，为初始值
        DisSysConfigInfo.IP[0]= NetConfigInfo[XY_JM].IP[0];
        DisSysConfigInfo.IP[1]= NetConfigInfo[XY_JM].IP[1];
        DisSysConfigInfo.IP[2]= NetConfigInfo[XY_JM].IP[2];
        DisSysConfigInfo.IP[3]= NetConfigInfo[XY_JM].IP[3];
        //写
        fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));

    }
    else
    {
        snprintf(NetConfigInfo[NET_YX_SELCT].pIp,sizeof(NetConfigInfo[NET_YX_SELCT].pIp),"%d.%d.%d.%d",\
                 DisSysConfigInfo.IP[0],DisSysConfigInfo.IP[1],DisSysConfigInfo.IP[2],DisSysConfigInfo.IP[3]);
    }
    if(DisSysConfigInfo.Port == 0xffff)
    {
        //说明flash没有设置过，为初始值
        DisSysConfigInfo.Port = NetConfigInfo[XY_JM].port;
        fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));

    }
    else
    {
        NetConfigInfo[NET_YX_SELCT].port = DisSysConfigInfo.Port;
    }



    //第一次烧录默认是单机正常、内部电表
    if((DisSysConfigInfo.standaloneornet == 0xFF)&&(DisSysConfigInfo.energymeter == 0xFF))
    {
        DisSysConfigInfo.standaloneornet = DISP_CARD; //单机正常
        DisSysConfigInfo.energymeter = USERN8209;  //内部电表
        fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));
    }

    if(DisSysConfigInfo.GunNum == 0xff)
    {
        DisSysConfigInfo.GunNum = 1;
        fal_partition_write(DWIN_INFO,0,(uint8_t*)&DisSysConfigInfo,sizeof(_DIS_SYS_CONFIG_INFO));
    }
//    //上电读取一次flash值，这个不写应该也是可以的。
//    if(DisSysConfigInfo.standaloneornet == DISP_NET)
//    {
//        //DISP_VERSION=DISP_NET;
//    }
//    else if(DisSysConfigInfo.standaloneornet == DISP_CARD)
//    {
//        //DISP_VERSION=DISP_CARD;
//    }
//		else if(DisSysConfigInfo.standaloneornet == DISP_CARD_mode)
//		{
//		   //DISP_VERSION=DISP_CARD_mode;
//		}



    //fal_partition_erase(charge_rete_info, 0, sizeof(RATE_T)); //清空费率区域
    if(judge_rete_info()== 0)  //说明费率信息有问题
    {
        uint8_t i=0;
        stChRate.fwPrices[0] = 0;
        stChRate.fwPrices[1] = 0;
        stChRate.fwPrices[2] = 0;
        stChRate.fwPrices[3] = 0;
        stChRate.Prices[0] = 100000;
        stChRate.Prices[1] = 100000;
        stChRate.Prices[2] = 100000;
        stChRate.Prices[3] = 100000;

        memset(stChRate.ucSegNum,0,sizeof(stChRate.ucSegNum)); //当前分组只分一组 0-0
        for(i=0; i<24; i++) //0-12点一个时间段
        {
            stChRate.ucSegNum[i]=0;
        }
        for(i=24; i<48; i++) //12-0点
        {
            stChRate.ucSegNum[i]=1;
        }
//        for(i=22; i<26; i++) //11-13点
//        {
//            stChRate.ucSegNum[i]=2;
//        }
//        for(i=26; i<44; i++) //13-22点
//        {
//            stChRate.ucSegNum[i]=3;
//        }
//        for(i=44; i<48; i++) //22-0点
//        {
//            stChRate.ucSegNum[i]=0;
//        }

        fal_partition_write(CHARGE_RATE,0,(uint8_t*)&stChRate,sizeof(RATE_T));
    }

#if(NET_YX_SELCT == XY_YKC)
    //云快充  当充电中修改费率，A枪的费率可能和B枪的费率不一样
    memcpy(&stChRateA,&stChRate,sizeof(RATE_T));
    memcpy(&stChRateB,&stChRate,sizeof(RATE_T));
#endif

    //上电后读区一共多少条记录,如果是新flsah，就会读出0xFFFF,则赋值为0
    fal_partition_read(RECORD_QUERY,0,(uint8_t *)&RECODECONTROL.RecodeCurNum,sizeof(RECODECONTROL.RecodeCurNum));
    if(RECODECONTROL.RecodeCurNum==0xFFFFFFFF)
    {
        Clear_record();
//        RECODECONTROL.RecodeCurNum=0;
//        fal_partition_erase(Record_query_info, 0,  sizeof(RECODECONTROL.RecodeCurNum));
//        fal_partition_write(Record_query_info, 0,  (uint8_t*)&RECODECONTROL.RecodeCurNum, sizeof(RECODECONTROL.RecodeCurNum));

    }
    //白名单卡个数
	#if(WLCARD_STATE)
	{
		fal_partition_read(CARD_WL,0,FlashCardVinWLBuf,1);
		if(FlashCardVinWLBuf[0] == 0xff)
		{
			FlashCardVinWLBuf[0] = 0;
			fal_partition_write(CARD_WL, 0,  FlashCardVinWLBuf, 1);
		}
	}
	#endif

    //离线交易记录
    fal_partition_read(OFFFSLINE_BILL,0,&len,1);
    if(len > 100)
    {
        len = 0;
        fal_partition_write(OFFFSLINE_BILL,0,&len,1);
    }
    //
    //离线交易记录
    fal_partition_read(OFFLINE_BILL,0,&len,1);
    if(len > 100)
    {
        len = 0;
        fal_partition_write(OFFLINE_BILL,0,&len,1);
    }
	//66固定为网络版
	#if(NET_YX_SELCT == XY_JM)
	#warning "YXY 需要删除"
//	memset(DisSysConfigInfo.DevNum,0,sizeof(DisSysConfigInfo.DevNum));
	//memcpy(DisSysConfigInfo.DevNum,"AC650009",strlen("AC650009"));
		
//	uint8_t DevNumbuf[16]={0};
//	memcpy(DevNumbuf,DisSysConfigInfo.DevNum,8);
//	memcpy(DisSysConfigInfo.DevNum,DevNumbuf,16);
	
	memcpy(DisSysConfigInfo.ZFDevNum,"AC66012022660006",strlen("AC66012022660006"));

	
	DisSysConfigInfo.standaloneornet = DISP_NET; //网络
	DisSysConfigInfo.energymeter = USERN8209;  //内部电表
	
	//66测试
//	  snprintf(NetConfigInfo[NET_YX_SELCT].pIp,sizeof(NetConfigInfo[NET_YX_SELCT].pIp),"%d.%d.%d.%d",\
//                 39,170,80,51);
//        //说明flash没有设置过，为初始值
//      NetConfigInfo[NET_YX_SELCT].port = 18992;
	  
	  
//	//66正式  //说明flash没有设置过，为初始值
//	  snprintf(NetConfigInfo[NET_YX_SELCT].pIp,sizeof(NetConfigInfo[NET_YX_SELCT].pIp),"%d.%d.%d.%d",139,196,60,58);
//      NetConfigInfo[NET_YX_SELCT].port = 18051;

	#endif
}

OS_Q DwinMq;


/**
 * @brief 迪文状态处理，主要是为了切换界面
 * @param[in] gun_mode2  模式2充电    
 * @param[out]
 * @return
 * @note
 */
void gun_mode2(uint8_t state)
{
	static uint32_t curtick = 0,lasttick = 0;
	static uint8_t chargetate  = 1;    //刚上电的时候，或者重新插拔枪才能再次充电
	curtick = OSTimeGet(&timeerr);
	
	
	if(state == STANDBY)
	{
		chargetate = 1;
	}

	if((state == INSERT)  && chargetate)
	{
		if((curtick - lasttick) > 3000)
		{
			send_ch_ctl_msg(1,1,4,0);
			chargetate  = 0;
		}
	}
	else
	{
		lasttick = curtick;
	}
}

/**
 * @brief 迪文显示屏线程
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void AppTaskDwin(void *p_arg)
{
    ST_Menu* CurMenu = NULL;
    MQ_MSG_T stMsg = {0};

    _LCD_KEYVALUE key;

//    rt_err_t uwRet = RT_EOK;
    OSTimeDly(1000, OS_OPT_TIME_PERIODIC, &timeerr);
    static uint8_t uartbuf[50];
    UART2Dispinit();
    DisplayCommonMenu(&HYMenu15,NULL);





    flash_para_init();   //flash 参数初始化
    ReadFmBill();
    if(DisSysConfigInfo.standaloneornet == DISP_NET)   //网络
    {
        ReadFmBill();					//读取订单
    }
    OS_ERR err;
    //  mq_service_bind(CM_CHTASK_MODULE_ID,"ch task mq");

    OSQCreate (&DwinMq,
               "dwin task mq",
               20,
               &err);
    if(err != OS_ERR_NONE)
    {
        printf("OSQCreate %s Fail", "dwin task mq");
        return;
    }

    while(1)
    {

		#if(UPDATA_STATE) 
		if(APP_GetSIM7600Mode() == MODE_HTTP)   //远程升级其他无关数据帧都不不要发送和处理
		{
			 OSTimeDly(1000, OS_OPT_TIME_PERIODIC, &timeerr);
			continue;
		}
		#endif
        if(mq_service_recv_msg(&DwinMq,&stMsg,uartbuf,sizeof(uartbuf),CM_TIME_500_MSEC) == 0 )
        {
            switch(stMsg.uiSrcMoudleId)
            {
            case CM_DWINRECV_MODULE_ID:   //串口接收
                APP_DisplayRecvDataAnalyze(&uartbuf[0],stMsg.uiLoadLen);     //数据解析
                break;

            case CM_CARD_MODULE_ID:   //卡接收
                if(CurMenu == NULL)
                {
                    break;
                }
                if((stMsg.uiMsgVar == (uint32_t)&HYMenu7) || (stMsg.uiMsgVar == (uint32_t)&HYMenu8) || (stMsg.uiMsgVar == (uint32_t)&HYMenu9)\
                        ||(stMsg.uiMsgVar == (uint32_t)&HYMenu10) || (stMsg.uiMsgVar == (uint32_t)&HYMenu11)||(stMsg.uiMsgVar == (uint32_t)&HYMenu16))
                {
                    //卡提示界面
                    if((CurMenu != &HYMenu7) && (CurMenu != &HYMenu8) &&(CurMenu != &HYMenu9)  &&(CurMenu != &HYMenu10)  &&(CurMenu != &HYMenu11))
                    {
                        DispControl.CurSysState = DIP_CARD_SHOW;  //无需界面再次切换
                        DisplayCommonMenu((struct st_menu*)stMsg.uiMsgVar,CurMenu);
                    }
                }
                else
                {
                    DispControl.CurSysState = DIP_STATE_NORMAL;  //主界面可以切换页面
                    DisplayCommonMenu((struct st_menu*)stMsg.uiMsgVar,CurMenu);
                }
                break;

            case CM_CHTASK_MODULE_ID:   /* 充电任务模块ID */
                switch(stMsg.uiMsgCode)
                {
                case CH_TO_DIP_STARTSUCCESS: 	//启动成功
                    if(DisSysConfigInfo.standaloneornet == DISP_NET)
                    {
                        WriterFmBill(GUN_A,1);   //存储交易记录
                        mq_service_send_to_4gsend(APP_START_ACK,GUN_A ,0 ,NULL);
                    }
                    break;

                case CH_TO_DIP_STARTFAIL:    	//启动失败
                    stChTcb.stCh.uiChStoptTime = gs_SysTime;	//停止充电时间

                    stChTcb.stCh.timestop = stChTcb.stCh.timestart;
                    stChTcb.stCh.reason = stMsg.uiMsgVar;		//停止原因

                    if(DisSysConfigInfo.standaloneornet == DISP_NET)
                    {
							StoreRecodeCurNum();//启动后，记录总条数+1; 单机的时候在在锁卡的时候写了
						  APPTransactionrecord(stChTcb.stCh.reason,SHOW_START_APP,SHOW_GUNA,RECODECONTROL.RecodeCurNum);//交易记录写入(未结算和已结算全部写入)
						#if NET_YX_SELCT == XY_JM
                        mq_service_send_to_4gsend(APP_START_ACK,GUN_A ,0 ,NULL);
						#else
						 mq_service_send_to_4gsend(APP_STOP_BILL,GUN_A ,0 ,NULL);
						#endif
                    }
                    else
                    {
                        if(&HYMenu10 != CurMenu)
                        {
                            DispControl.CurSysState = DIP_CARD_SHOW;  //无需界面再次切换
                            DisplayCommonMenu(&HYMenu9,CurMenu); //切换到刷卡结算界面
                        }
						 APPTransactionrecord(stChTcb.stCh.reason,SHOW_START_CARD,SHOW_GUNA,RECODECONTROL.RecodeCurNum);//交易记录写入(未结算和已结算全部写入)
                    }
                    //跳转到刷卡结算界面
                    break;

                case CH_TO_DIP_STOP:			//停止时

                    stChTcb.stCh.uiChStoptTime = gs_SysTime;	//停止充电时间
                    stChTcb.stCh.timestop = cp56time(NULL);

                    stChTcb.stCh.reason = stMsg.uiMsgVar;		//停止原因
                   // StoreRecodeCurNum();//启动后，记录总条数+1;

                    if(DisSysConfigInfo.standaloneornet == DISP_NET)
                    {
											StoreRecodeCurNum();//启动后，记录总条数+1; 单机的时候在在锁卡的时候写了
                        mq_service_send_to_4gsend(APP_STOP_BILL,GUN_A ,0 ,NULL);
                        APPTransactionrecord(stChTcb.stCh.reason,SHOW_START_APP,SHOW_GUNA,RECODECONTROL.RecodeCurNum);//交易记录写入(未结算和已结算全部写入)
                    }
                    else
                    {
                        APPTransactionrecord(stChTcb.stCh.reason,SHOW_START_CARD,SHOW_GUNA,RECODECONTROL.RecodeCurNum);//交易记录写入(未结算和已结算全部写入)
                    }
                    show_Notcalculated();  //停止时如果未结算,跳转到结算页面

                    //rt_thread_mdelay(10000);
                    // Clear_flag();  //清空标志位
                    break;

                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

	//	#if(NET_YX_SELCT != XY_JM)
        CurMenu = GetCurMenu();                                     //获取当前界面
        if (CurMenu && CurMenu->function3)
        {
            CurMenu->function3();                                   //数据显示
        }

        if(	DispControl.CountDown == 1)								//倒计时减到1
        {
            key = LCD_KEY2;			                                //返回上一级界面
            DealWithKey(&key);
        }

        rt_dwin_state_dispose(stChTcb.ucState);	             //页面切换
        dwin_show();				                                //显示信息\当前显示页面
        dwin_show_state();
        rt_dwin_period_entry();
		//gun_mode2(stChTcb.ucState);	     //模式2
//		OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
	//	#endif

    }
}






/**
 * @brief 给充电线程发送充电控制消息
 * @param[in] ucCtl:充电启停控制 1:启动充电,2:停止充电
 *            ucChStartMode:充电启动方式 0:app启动,1:刷卡启动
 *            ucChMode:充电方式 1:按电量充电,2:按时间充电(单位 1 分钟),3:按金额充电(单位 0.01 元),4:自动充满 5：定时充
 *            uiStpPargm:充电停止参数 按电量充电(单位 0.01 度),按时间充电(单位 1 分钟),按金额充电时单位(单位 0.01 元),按充满为止,此数据为 0
 * @param[out]
 * @return
 * @note
 */
void send_ch_ctl_msg(uint8_t ucCtl,uint8_t ucChStartMode,uint8_t ucChMode,uint32_t uiStopParam)
{
    uint8_t	  ucTxBuf[32]  = {0};
    uint32_t  uiMoudleId   = 0;

    /* TCU 统一发送充电控制消息给充电任务 */
    uiMoudleId =  CM_TCUTASK_MODULE_ID;

    ((CH_CTL_T *)(ucTxBuf))->ucCtl         = ucCtl;
    ((CH_CTL_T *)(ucTxBuf))->ucChStartMode = ucChStartMode;
    ((CH_CTL_T *)(ucTxBuf))->ucChMode      = ucChMode;
    ((CH_CTL_T *)(ucTxBuf))->uiStopParam   = uiStopParam;
    //((CH_CTL_T *)(ucTxBuf))->uistartParam  = uistartParam;
    mq_service_xxx_send_msg_to_chtask(uiMoudleId,TCU_TO_CH_CHCLT,0,sizeof(CH_CTL_T),ucTxBuf);
}



