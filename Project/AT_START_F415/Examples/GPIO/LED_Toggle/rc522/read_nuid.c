/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/greedyhao/rc522_rtt
 *
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the rt-thread SPI interface.
 *
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 */

#include "ch_port.h"
#include "common.h"
#include "dwin_com_pro.h"
#include "DispShowStatus.h"
#include "chtask.h"
#include "MenuDisp .h"
#include "string.h"
#include "DispKey.h"
#include "DispkeyFunction.h"
#include "4GMain.h"
#include "DwinProtocol.h"
#include "main.h"
#include "rc522.h"

#define ENABLE_BUZZER              70                  //蜂鸣器

#define CARD_LOCK   0xff   //启动读卡时 给卡上锁
#define CARD_UNLOCK  0     //停止结算后 给卡解锁

#define BLOCK_POSSWORD			4				//密码块
#define BLOCK_STATUS			5				//卡状态块
#define BLOCK_STARTMONEY	10				//开始充电的时候写入一次。  等待下一次充电判断金额是否变化，若金额发生了变化，则判断为卡已经解锁
#define BLOCK_MONEY				6				//金额块
#define BLOCK_CardType 12   //卡类型是在3扇区块0
#define Free_onecardonepile 0x4230  //免费卡：一卡一桩B0转化成ASCII=0x4230
#define Free_onecardmanypile 0x4231  //免费卡：一卡多桩B1 转化成ASCII=0x4231
#define Money_onecardonepile 0x4330  //收费卡：一卡一桩C0 转化成ASCII=0x4330
#define Money_onecardmanypile 0x4331  //收费卡：一卡多桩C1 转化成ASCII=0x4331
#define S_Card 0x5300 //计算费率卡
#define BLOCK_CardMCUID 13   //卡类型是在3扇区块1:存储设备ID号




_m1_card_info m1_card_info;		 //M1卡相关信息

extern CH_TASK_T stChTcb;

extern SYSTEM_RTCTIME gs_SysTime;
_m1_control M1Control = {0};			//M1卡控制
uint8_t Card_ID[10];   //当前读到的卡ID


uint8_t CIDBuffer[4];
/**
 * @brief  读取卡号
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t app_read_nuid(void)
{
    uint8_t  Card_addr, Card_Data[20],pcd_err;

    while (1)
    {
        // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle. And if present, select one.
        pcd_err =RC522_PcdRequest(0x52,CIDBuffer);//返回值为0表示寻卡成功  把卡片类型存到RxBuffer
        if(pcd_err != 0)
        {
            RC522_Reset();
            return 0;
        }
        pcd_err = RC522_PcdAnticoll(CIDBuffer); //防冲撞 完成这步 可以简单的读取卡号
        if(pcd_err == 0)
        {
           memcpy((char*)Card_ID,CIDBuffer,4);
            return 1;
        }
    }
    return 0;
}

/**
 * @brief  读取锁状态
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t read_lock_state(void)
{
    uint8_t status;
    uint8_t buffer[18] = {0};
    uint8_t size = sizeof(buffer);
    if(RC522_ReadCard(BLOCK_STATUS,buffer) == 0)
    {
        m1_card_info.lockstate = buffer[0];
    }
    else
    {
        return 0;
    }
    return 1;
}

/**
 * @brief  写锁状态
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t writer_lock_state(uint8_t lockstate)
{
    uint8_t status;
    uint8_t buffer[16] = {0};
    buffer[0] = lockstate;
    uint8_t size = sizeof(buffer);


    if(RC522_WriteCard(BLOCK_STATUS,buffer) == 0)
    {
        m1_card_info.lockstate = buffer[0];
    }
    else
    {
        return 0;
    }
    if(m1_card_info.lockstate  != lockstate)
    {
        return 0;
    }
    return 1;
}


/**
 * @brief  读取卡余额
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t read_card_money(void)
{
    uint32_t status;
    uint8_t buffer[16] = {0};
    uint8_t size = sizeof(buffer);

    if(RC522_ReadCard(BLOCK_MONEY,buffer) == 0)
    {
        __NOP();
    }
    else
    {
        return 0;
    }

    m1_card_info.balance = ((buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24));

    return 1;
}


/**
 * @brief  设置卡余额
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
uint8_t set_card_money(uint32_t money)
{
    uint8_t status;
    if( RC522_SetValue(BLOCK_MONEY,money) == 0)
    {
        return 1;
    }
    return 0;

}

uint8_t read_card_type(void) //读出卡类型0和1字节
{
    uint8_t status;
    uint8_t buffer[16] = {0};
    if(RC522_ReadCard(BLOCK_CardType,buffer) == 0)
    {
        __NOP();
    }
    else
    {
        return 0;
    }

    m1_card_info.CardType = (buffer[0]&0x00FF)<<8 | buffer[1]; //卡类型
    memcpy(m1_card_info.Card_MCUID,&buffer[3],12); //把卡上面的ID号数据赋值到结构体中
    //Storage_MCUID
    memcpy(&m1_card_info.Storage_MCUID[0],&buffer[0],2);
    memcpy(&m1_card_info.Storage_MCUID[3],&m1_card_info.MCUID[0],12);//第3位到14位，还有3位==141516
    return 1;
}


uint8_t set_CardMCUID(uint8_t * butter)
{
    uint8_t setvalue[16];
    uint8_t status;
    memcpy(setvalue,butter,16); //赋值里面

    if(RC522_WriteCard(BLOCK_CardType,setvalue) == 0)
    {
        __NOP();
    }
    else
    {
        return 0;
    }
    return 1;
}




/*****************************************************************************
* Function     :Unlock_settlement
* Description  :主页面单独解锁按钮
* Input        :
* Output       :None
* Return       :
* Note(s)      :
* author       :hycsh
* Contributor  :2022年3月
*****************************************************************************/
uint8_t Unlock_settlement(void)
{
    uint32_t num; //查询当前第几条记录（从最前面开始查找）
    uint32_t Cardmoney;
    uint8_t pdata[10];

    if(app_read_nuid() == 1)  //[1]读取卡号
    {
        //读出卡类型和卡内余额
        if((read_card_type()==1)&&(read_card_money()==1)&&((DisSysConfigInfo.standaloneornet == DISP_CARD)||(DisSysConfigInfo.standaloneornet == DISP_CARD_mode))) //读取卡类型和余额
        {
            if(m1_card_info.CardType==S_Card) //如果是s费率卡
            {
                Set_judge_rete_info((m1_card_info.balance)*10);
                DisplayCommonMenu(&HYMenu36,&HYMenu1);//费率设置成功页面
                return 0;
            }
        }


        if(read_lock_state() == 1)
        {
            if(m1_card_info.lockstate == CARD_UNLOCK) //未锁状态
            {
                DisplayCommonMenu(&HYMenu35,&HYMenu1);//显示卡未锁界面
                return 0;
            }
        }

        memcpy(m1_card_info.uidByte,Card_ID,sizeof(Card_ID));  //赋值卡号
        for(num=RECODECONTROL.RecodeCurNum; num<RECODECONTROL.RecodeCurNum+1; num--)
        {
            Recordqueryinfo_WR(RECODE_DISPOSE1(num%1000),FLASH_ORDER_READ,(uint8_t *)&SaveRecordinfo);//依次读出每条信息的值

            if((SaveRecordinfo.CardNum)==((m1_card_info.uidByte[0]<<24)|(m1_card_info.uidByte[1] << 16)|(m1_card_info.uidByte[2] << 8)|(m1_card_info.uidByte[3])))  //卡号比对相等
            {
                Dis_ShowCopy(pdata,SHOW_NOTBIL);	 //比对已结算SHOW_BILL或者未结算=SHOW_NOTBIL
                if(strncmp((char*)SaveRecordinfo.BillState,(char*)pdata,6) == 0)
                {
                    if(read_card_money() == 1)  //【2】读取卡内余额
                    {
                        Cardmoney=(m1_card_info.balance)*100;
                        if(Cardmoney<SaveRecordinfo.chargemoney) //解锁时卡内余额为0或者余额不足，也会返回
                        {
                            mq_service_card_send_disp(0,(uint32_t)&HYMenu11,0,NULL); //解锁时切换到卡内余额不足界面
                            return 0;
                        }
                    }

                    //解锁时当前卡内余额减去消费金额，等于设置的金额
                    if(set_card_money((Cardmoney-SaveRecordinfo.chargemoney)/100) == 0)  //解锁时设置卡内余额
                    {
                        return 0;
                    }

                    if(writer_lock_state(CARD_UNLOCK) == 0)  //解锁时卡解锁
                    {
                        return 0;
                    }
                    DisplayCommonMenu(&HYMenu34,&HYMenu1);//卡已解锁界面
                    Unlock_settlementrecord(SHOW_BILL,num);//解锁时记录写入(正常的结算，未结算修改成已结算)
                    return 1;
                }
                DisplayCommonMenu(&HYMenu7,&HYMenu1);//已结算：卡锁请原桩解锁
                return 1; //比对已结算和未结算
            }
        }
        DisplayCommonMenu(&HYMenu7,&HYMenu1);//卡号不一致：卡锁请原桩解锁
        return 0;
    }
    return 0;
}


/*****************************************************************************
* Function     :startchargUnlock(void)
* Description  ://开始启动充电的过程中——解锁
* Input        :
* Output       :None
* Return       :
* Note(s)      :
* author       :hycsh
* Contributor  :2022年3月
*****************************************************************************/
uint8_t startchargUnlock(void)
{
    uint32_t num; //查询当前第几条记录（从最前面开始查找）
    uint32_t Cardmoney;
    uint8_t pdata[10];

    memcpy(m1_card_info.uidByte,Card_ID,sizeof(Card_ID));  //赋值卡号
    for(num=RECODECONTROL.RecodeCurNum; num<RECODECONTROL.RecodeCurNum+1; num--)
    {
        Recordqueryinfo_WR(RECODE_DISPOSE1(num%1000),FLASH_ORDER_READ,(uint8_t *)&SaveRecordinfo);//依次读出每条信息的值

        if((SaveRecordinfo.CardNum)==((m1_card_info.uidByte[0]<<24)|(m1_card_info.uidByte[1] << 16)|(m1_card_info.uidByte[2] << 8)|(m1_card_info.uidByte[3])))  //卡号对比相等
        {
            Dis_ShowCopy(pdata,SHOW_NOTBIL);	 //比对已结算SHOW_BILL或者未结算=SHOW_NOTBIL
            if(strncmp((char*)SaveRecordinfo.BillState,(char*)pdata,6) == 0)
            {
                if(read_card_money() == 1)  //【2】读取卡内余额
                {
                    Cardmoney=(m1_card_info.balance)*100;
                    if(Cardmoney<SaveRecordinfo.chargemoney) //解锁时卡内余额为0或者余额不足，也会返回
                    {
                        mq_service_card_send_disp(0,(uint32_t)&HYMenu11,0,NULL); //解锁时切换到卡内余额不足界面
                        return 0;
                    }
                }

                //解锁时当前卡内余额减去消费金额，等于设置的金额
                if(set_card_money((Cardmoney-SaveRecordinfo.chargemoney)/100) == 0)  //解锁时设置卡内余额
                {
                    return 0;
                }

                if(writer_lock_state(CARD_UNLOCK) == 0)  //解锁时卡解锁
                {
                    return 0;
                }
                Unlock_settlementrecord(SHOW_BILL,num);//解锁时记录写入(正常的结算，未结算修改成已结算)
                return 1;
            }
        }
    }
    //DisplayCommonMenu(&HYMenu7,&HYMenu1);//卡已锁,请到原桩解锁
    mq_service_card_send_disp(0,(uint32_t)&HYMenu7,0,NULL); //卡已锁,请到原桩解锁
    BUZZER_ON;
    OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
    BUZZER_OFF;
    OSTimeDly(3000, OS_OPT_TIME_PERIODIC, &timeerr);
    return 0;

}

/*****************************************************************************
* Function     :start_change_card(void)
* Description  :开始充电时步骤流程
* Input        :
* Output       :None
* Return       :
* Note(s)      :
* author       :hycsh
* Contributor  :2022年3月
*****************************************************************************/
uint8_t start_change_card(void)
{
    uint8_t StrID[12]= {0};
    if(app_read_nuid() == 0)  //【1】读取卡号
    {
        return 0;
    }

    if(read_card_type()==1)  //读取卡类型
    {
        //不在卡类型中，不能充电:卡类型有4个B0和B1  C0的C1
        if((m1_card_info.CardType==Free_onecardonepile)||(m1_card_info.CardType==Free_onecardmanypile)|| \
                (m1_card_info.CardType==Money_onecardmanypile)||(m1_card_info.CardType==Money_onecardonepile))
        {
            if((m1_card_info.CardType==Free_onecardonepile)||(m1_card_info.CardType==Money_onecardonepile)) //一卡一桩（收费和免费） B0和C0
            {
//                if(read_CardMCUID()==1)
//                {
                if(strncmp((char*)m1_card_info.MCUID,(char*)m1_card_info.Card_MCUID,sizeof(m1_card_info.MCUID))==0)
                {
                    __NOP;
                }
                else if(strncmp((char*)m1_card_info.Card_MCUID,(char*)StrID,sizeof(m1_card_info.Card_MCUID))==0)  //初次新卡就是就是0，直接写入
                {
                    //m1_card_info.MCUID
                    set_CardMCUID(m1_card_info.Storage_MCUID);   //如果读取的值，如果是全部是00,就是写入ID卡里面
                    BUZZER_ON;
                    OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
                    BUZZER_OFF;
                    OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
                    BUZZER_ON;
                    OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
                    BUZZER_OFF;
                    OSTimeDly(1000, OS_OPT_TIME_PERIODIC, &timeerr);
                    return 0;
                }
                else  //则设备号不一致就是不启动
                {
                    return 0;
                }
//                }
            }
        }
        else
        {
            mq_service_card_send_disp(0,(uint32_t)&HYMenu10,0,NULL); //切换到非发行方卡界面
            OSTimeDly(1000, OS_OPT_TIME_PERIODIC, &timeerr);
            return 0;
        }
    }
    if(read_lock_state() == 0)  //【2】读取锁状态
    {
        mq_service_card_send_disp(0,(uint32_t)&HYMenu10,0,NULL); //切换到非发行方卡界面
        BUZZER_ON;
        OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
        BUZZER_OFF;
        OSTimeDly(1000, OS_OPT_TIME_PERIODIC, &timeerr);
        mq_service_card_send_disp(0,(uint32_t)&HYMenu1,0,NULL); //跳转到主界面
        //DispControl.CurSysState = DIP_STATE_NORMAL;
        return 0;
    }

    //锁卡状态时，会处理解锁卡,失败会返回0，成功会返回1
    if(m1_card_info.lockstate == CARD_LOCK)
    {
        if(startchargUnlock()==0)
        {
            return 0;
        }
    }


    if(read_card_money() == 1)  //【3】读取卡内余额
    {
        if((m1_card_info.CardType==Money_onecardmanypile)||(m1_card_info.CardType==Money_onecardonepile))//收费卡才会判断余额为0,停止充电
        {
            if(m1_card_info.balance == 0) //余额为0,停止充电
            {
                mq_service_card_send_disp(0,(uint32_t)&HYMenu11,0,NULL); //切换到卡内余额不足界面
                BUZZER_ON;
                OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
                BUZZER_OFF;
                OSTimeDly(3000, OS_OPT_TIME_PERIODIC, &timeerr);
                return 0;
            }
        }
    }

    if((m1_card_info.CardType==Money_onecardmanypile)||(m1_card_info.CardType==Money_onecardonepile))
    {
        if(writer_lock_state(CARD_LOCK) == 0)  //给卡上锁
        {
            return 0;
        }
    }
	

		
		
    memcpy(m1_card_info.uidByte,Card_ID,sizeof(Card_ID));  //赋值卡号
//在锁卡的时候写未结算记录，主要是为了解决掉电单机无法结算 20220805
    M1Control.m1_if_balance = 1;		//此卡未结算
	StoreRecodeCurNum();//启动后，记录总条数+1;
	stChTcb.stCh.uiChStoptTime = gs_SysTime;	//停止充电时间
	 stChTcb.stCh.uiChStartTime = gs_SysTime;
	stChTcb.stCh.uiAllEnergy = 0;
	stChTcb.stCh.uiChargeEnergy = 0;
	APPTransactionrecord(END_CONDITION,SHOW_START_CARD,SHOW_GUNA,RECODECONTROL.RecodeCurNum);//交易记录写入(未结算)
		

    //4种模式
    if((DisSysConfigInfo.standaloneornet == DISP_CARD_mode)&&(stChTcb.stChCtl.ucChMode != 0))
    {
        send_ch_ctl_msg(1,0,stChTcb.stChCtl.ucChMode,stChTcb.stChCtl.uiStopParam);
        DispControl.CurSysState = DIP_STATE_NORMAL;  //自动恢复到正常的界面

        if(stChTcb.stChCtl.ucChMode==5)  //模式5:定时充时，才会显示预约模式
        {
            DispControl.CurSysState = DIP_CARD_SHOW;  //无需界面再次切换
            DisplayCommonMenu(&HYMenu21,NULL);  /*跳转到预约界面*/
        }
    }


    if((DisSysConfigInfo.standaloneornet == DISP_CARD_mode)&&(stChTcb.stChCtl.ucChMode == 0))  	//直接刷卡自动充满
    {
        send_ch_ctl_msg(1,0,4,0);   //直接开始充电充满
    }


    if(DisSysConfigInfo.standaloneornet == DISP_CARD)  /*单机正常模式*/
    {
        send_ch_ctl_msg(1,0,4,0);   //直接开始充电充满
    }
    return 1;
}


/*****************************************************************************
* Function     :stop_change_card(uint32_t money)
* Description  :停止充电时步骤流程
* Input        :money=充电过程中消费的实时金额
* Output       :None
* Return       :
* Note(s)      :
* author       :hycsh
* Contributor  :2022年3月
*****************************************************************************/
uint8_t stop_change_card(uint32_t money)
{
    uint32_t setmoney;
//    //首先判断充电金额超出卡内余额
//    if(m1_card_info.balance <= money)
//    {
//        if((stChTcb.ucState == CHARGING)&&(m1_card_info.CardType==Money_onecardmanypile||m1_card_info.CardType==Money_onecardonepile)) //如果现在在充电中，发送充电结束
//        {
//            send_ch_ctl_msg(2,0,4,0);   	//在充电中发送结束充电
//            return 0;
//        }
//    }

    ST_Menu* CurMenu = GetCurMenu();  //获取当前界面
    if(app_read_nuid() == 1)  //读取卡号,正常的结算流程
    {
        if(strncmp((char*)m1_card_info.uidByte,(char*)Card_ID,sizeof(Card_ID)) != 0)
        {
            mq_service_card_send_disp(0,(uint32_t)&HYMenu8,0,NULL); //卡号不一致
            return 0;
        }

        if(m1_card_info.balance >= money)   /*余额 > 充电金额 */
        {
            setmoney = m1_card_info.balance - money;
        }
        else
        {
            setmoney = 0;
        }
        if(read_card_type()==1)
        {
            if(m1_card_info.CardType==Money_onecardonepile||m1_card_info.CardType==Money_onecardmanypile) //收费卡才会写入剩余金额
            {
                if(set_card_money(setmoney) == 0)  //设置卡内余额
                {
                    return 0;
                }
            }
        }

        if(writer_lock_state(CARD_UNLOCK) == 0)  //卡解锁
        {
            return 0;
        }

        if(stChTcb.ucState == CHARGING)
        {
            send_ch_ctl_msg(2,0,4,0);   	//在充电中发送结束充电
        }

        if(CurMenu == &HYMenu9)  //非正常停止时(例：金额、达到条件、急停等)，记录重新写入：已结算
        {
            Unlock_settlementrecord(SHOW_BILL,RECODECONTROL.RecodeCurNum);  //写入当前的条数
        }

        if((stChTcb.stChCtl.ucChMode == 5)&&(CurMenu == &HYMenu21))  /*模式5 在预约界面*/
        {
            memset(&stChTcb.stChCtl,0,sizeof(stChTcb.stChCtl));  //启停结构体清零
        }
        M1Control.m1_if_balance = 0;	//已结算
        DispControl.CurSysState = DIP_STATE_NORMAL;
        return 1;
    }
    return 0;
}


/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */

//未结算且已拔枪-清空标志位
//卡类型 （特殊卡直接变位结算）
//标准卡  拔枪（条件）变为已经结算

extern uint8_t ch_is_cp_off(CH_TASK_T *pstChTcb);  //是否插枪，1插枪  0不插枪

uint8_t Clear_flag(void)
{
    //未结算和非法拔枪（只针对非法拔枪）
    if((M1Control.m1_if_balance==1)&&(ch_is_cp_off(&stChTcb) == 1)) //未结算且已拔枪和非法拔枪
    {
        OSTimeDly(200, OS_OPT_TIME_PERIODIC, &timeerr);     //保证显示任务先写记录
        M1Control.SlotCardControl = SLOT_CARD_STOP;
        M1Control.m1_if_balance=0;
        if(DispControl.CurSysState ==  DIP_CARD_SHOW)
        {
            DispControl.CurSysState =  DIP_STATE_NORMAL;
        }
    }
    return 1;
}






//===================停止时如果未结算,跳转到结算页面================
uint8_t  show_Notcalculated(void)
{
    if(M1Control.m1_if_balance==1)//未结算
    {
        DispControl.CurSysState = DIP_CARD_SHOW;  //无需界面再次切换
        mq_service_card_send_disp(0,(uint32_t)&HYMenu9,0,NULL); //切换的刷卡结算界面
        //DisplayCommonMenu((struct st_menu*)stMsg.uiMsgVar,CurMenu);
    }
}

/*在预约模式中，即为在预约页面时，拔枪时，记录写入*/
uint8_t mode5(void)
{
    if(stChTcb.stChCtl.ucChMode == 5)
    {
    //    StoreRecodeCurNum();//启动后，记录总条数+1;
        stChTcb.stCh.uiChStoptTime = gs_SysTime;	//停止充电时间
        APPTransactionrecord(END_CONDITION,SHOW_START_CARD,SHOW_GUNA,RECODECONTROL.RecodeCurNum);//交易记录写入(未结算)
    }
}




/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void slotcard_state(void)
{
    ST_Menu* CurMenu = GetCurMenu();  //获取当前界面

    if(CurMenu == &HYMenu9)   //【1】余额用完或者充满无电流，停留刷卡结算界面，未拔枪
    {
        M1Control.SlotCardControl = SLOT_CARD_STOP;
        if(stChTcb.stGunStat.ucCpStat == CP_12V) //说明已经拔枪了
        {
            //M1Control.m1_if_balance = 0;  //初始化
            OSTimeDly(2000, OS_OPT_TIME_PERIODIC, &timeerr);
            mq_service_card_send_disp(0,(uint32_t)&HYMenu1,0,NULL); //跳转到主界面
        }
        else
        {
            return;
        }
    }


    if(CurMenu == &HYMenu33)  //判断一下如果在解锁界面就是可以刷卡_解锁
    {
        if(Unlock_settlement()==1) //已经解锁
        {
            BUZZER_ON;
            OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
            BUZZER_OFF;
            OSTimeDly(3000, OS_OPT_TIME_PERIODIC, &timeerr);
            return;
        }
        return;
    }


    //主要是主页面s卡修改费率
    if(CurMenu == &HYMenu1)
    {
        if(app_read_nuid() == 1)  //[1]读取卡号
        {
            //读出卡类型和卡内余额
            if((read_card_type()==1)&&(read_card_money()==1)&&((DisSysConfigInfo.standaloneornet == DISP_CARD)||(DisSysConfigInfo.standaloneornet == DISP_CARD_mode))) //读取卡类型和余额
            {
                if(m1_card_info.CardType==S_Card) //如果是s费率卡
                {
                    Set_judge_rete_info((m1_card_info.balance)*10);
                    BUZZER_ON;
                    OSTimeDly(1000, OS_OPT_TIME_PERIODIC, &timeerr);
                    BUZZER_OFF;
                    DisplayCommonMenu(&HYMenu36,&HYMenu1);//费率设置成功页面
                    OSTimeDly(1000, OS_OPT_TIME_PERIODIC, &timeerr);
                }
            }
        }
    }



//	//主要是刷卡器读出
//    if(CurMenu == &HYMenu1)
//    {
//        if(app_read_nuid() == 1)
//        {
//            if((read_card_type()==1)&&(read_card_money()==1)) //读取卡类型和余额
//            {
//                    rt_pin_write(ENABLE_BUZZER, PIN_HIGH );
//                    rt_thread_mdelay(500);
//                    rt_pin_write(ENABLE_BUZZER, PIN_LOW);
//                    rt_thread_mdelay(500);
//            }
//        }
//    }



    if(stChTcb.ucState == CHARGING)
    {
        M1Control.SlotCardControl = SLOT_CARD_STOP;
        return;
    }
    if((stChTcb.ucState == INSERT) && (M1Control.m1_if_balance == 0))   //插枪&&已结算
    {
        M1Control.SlotCardControl = SLOT_CARD_START;
        return;
    }


    //插枪未结算===hycsh如果有一个订单没有结算就会一直等待结算，换成别人充电也会显示未结算
    if((stChTcb.ucState == INSERT) && (M1Control.m1_if_balance == 1))
    {
        M1Control.SlotCardControl  = SLOT_CARD_STOP;
        return;
    }
    M1Control.SlotCardControl = SLOT_CARD_UNDEFINE;
}



/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void AppTaskCard(void *p_arg)
{
    static uint8_t buf[30] = {0};
    uint32_t CardNum;
    uint8_t Card[8] = {0};
    uint8_t i = 0,num = 0;
    MQ_MSG_T stMsg = {0};
    uint8_t * pdata;

    RC522_Init();
    RC522_PcdReset();
    RC522_M500PcdConfigISOType('A');//设置RC632的工作方式

    OSTimeDly(1000, OS_OPT_TIME_PERIODIC, &timeerr);
    //send_ch_ctl_msg(1,0,4,0);
    while(1)
    {
		#if(UPDATA_STATE) 
		if(APP_GetSIM7600Mode() == MODE_HTTP)   //远程升级其他无关数据帧都不不要发送和处理
		{
			OSTimeDly(1000, OS_OPT_TIME_PERIODIC, &timeerr);
			continue;
		}
		#endif
        ST_Menu* CurMenu = GetCurMenu();  //获取当前界面

        OSTimeDly(1000, OS_OPT_TIME_PERIODIC, &timeerr);
        slotcard_state();		//状态处理
        if((M1Control.SlotCardControl == SLOT_CARD_START)&&((CurMenu == &HYMenu25) || (CurMenu == &HYMenu16) ))  //插枪且在25页面，才会开始运行
        {
            if(DisSysConfigInfo.standaloneornet == DISP_NET)   //网络刷卡
            {
                if(app_read_nuid())			//读取卡号发送卡鉴权
                {
                    memcpy(m1_card_info.uidByte,Card_ID,sizeof(Card_ID));
                    //发送卡鉴权
                    if(APP_GetAppRegisterState(0) == STATE_OK)
                    {
                        mq_service_send_to_4gsend(APP_CARD_INFO,GUN_A ,0 ,NULL);
                    }
                    else
                    {
#if NET_YX_SELCT == XY_AP
                        //判断是否为白名单
                        pdata = APP_GetCARDWL();
                        if(pdata != NULL)
                        {
                            num = pdata[0];	//卡白名单个数

                            CardNum = (m1_card_info.uidByte[0]) | (m1_card_info.uidByte[1] << 8) |\
                                      (m1_card_info.uidByte[2] << 16) | (m1_card_info.uidByte[3] << 24);

                            Card[0] = HEXtoBCD(CardNum / 100000000);
                            Card[1] = HEXtoBCD(CardNum % 100000000 /1000000);
                            Card[2] = HEXtoBCD(CardNum % 1000000 /10000);
                            Card[3] = HEXtoBCD(CardNum % 10000 /100);
                            Card[4] = HEXtoBCD(CardNum % 100 /1);
                            for(i = 0; i < num; i++)
                            {
                                if(CmpNBuf(&pdata[1+i*8],Card,8) )
                                {
                                    _4G_SetStartType(GUN_A,_4G_APP_CARD);			//设置为卡启动
                                    APP_SetStartNetState(GUN_A,NET_STATE_OFFLINE);	//离线充电
                                    send_ch_ctl_msg(1,0,4,0);   //开始充电
                                    break;
                                }
                            }
                        }
#endif
                        __NOP();
                    }
                    BUZZER_ON;
                    OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
                    BUZZER_OFF;
                    OSTimeDly(3000, OS_OPT_TIME_PERIODIC, &timeerr);
                }
            }
            else  //单机时
            {
                //返回1时，响一下，开始充电状态;
                if(start_change_card())
                {
                    BUZZER_ON;
                    OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
                    BUZZER_OFF;
                    OSTimeDly(3000, OS_OPT_TIME_PERIODIC, &timeerr);
                }
            }
        }


        //==停止时
        if(M1Control.SlotCardControl == SLOT_CARD_STOP)
        {
            if(DisSysConfigInfo.standaloneornet == DISP_NET )   //网络刷卡
            {
                if(_4G_GetStartType(GUN_A) == _4G_APP_CARD)    //刷卡启动的时候才执行
                {
                    if(app_read_nuid())			//读取卡号发送卡鉴权
                    {
                        if(!strncmp((char*)m1_card_info.uidByte,(char*)Card_ID,sizeof(Card_ID)) ) //判断卡号是否一致
                        {
                            send_ch_ctl_msg(2,0,4,0);   	//在充电中发送结束充电
                        }
                        else
                        {
                            mq_service_card_send_disp(0,(uint32_t)&HYMenu8,0,NULL); //卡号不一致
                        }
                        BUZZER_ON;
                        OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
                        BUZZER_OFF;
                        OSTimeDly(3000, OS_OPT_TIME_PERIODIC, &timeerr);
                    }
                }
            }
            else
            {
                if(stop_change_card(stChTcb.stCh.uiAllEnergy / 100))   //下一次充电才清掉
                {
                    if(M1Control.m1_if_balance == 0)  //已结算
                    {
                        BUZZER_ON;
                        OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
                        BUZZER_OFF;
                        OSTimeDly(3000, OS_OPT_TIME_PERIODIC, &timeerr);
                    }
                    else  //未结算
                    {
                        mq_service_card_send_disp(0,(uint32_t)&HYMenu9,0,NULL);  //请刷卡结算
                    }


                }
            }
        }
//        Clear_flag();  //一直检测是不是插枪状态和未结算状态(如果未结算且已拔枪，2s后清空结算状态)
    }
}

