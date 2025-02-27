/*****************************************Copyright(C)******************************************
****************************************************************************************
*---------------------------------------------------------------------------------------
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
#include "Frame66.h"
#include "common.h"
#include "dwin_com_pro.h"
#include "main.h"
#include "chtask.h"
#include "dlt645_port.h"
#include "utils_hmac.h"


/* secretKey 设备秘钥 */
#define DEVICESECRE          	"VkkWMX0UmJT7Ask8"
/* secretKey len */
#define DEVICESECRE_LEN      	strlen(DEVICESECRE)

#define SEND_66_FRAME_LEN   3
typedef enum
{
    CMD_SEND_REGISTER = 0,  	//注册
    CMD_SEND_HEARTBEAT,			//心跳
    CMD_SEND_DATA,				//实时数据
    CMD_SEND_START_ACK,			//启动应答
    CMD_SEND_POWERCTL_ACK,		//功率调节应答
    CMD_SEND_STOP_ACK,			//停止应答
    CMD_SEND_ORDER,				//上传订单
    CMD_SEND_OORDERQ_ACK,		//订单查询应答
    CMD_SEND_RESET_ACK,			//软件复位应答
    CMD_SEND_SETTIME_ACK,		//时间同步应答
    CMD_SEND_SETQR_ACK,			//设置二维码应答
    CMD_SEND_MAX,
} _66_SEND_CMD;

typedef enum
{
    CMD_RECV_REGISTER = 0,  	//注册应答
    CMD_RECV_HEARTBEAT,			//心跳应答
    CMD_RECV_START,				//启动应答
    CMD_RECV_POWERCTL,			//功率调节应答
    CMD_RECV_STOP,				//停止应答
    CMD_RECV_ORDER,				//上传订单应答
    CMD_RECV_OORDERQ,			//订单查询应答
    CMD_RECV_RESET,				//软件复位
    CMD_RECV_SETTIME,			//时间同步
    CMD_RECV_SETQR,				//设置二维码
    CMD_RECV_OTA,				//远程升级
    CMD_RECV_MAX,
} _66_RECV_CMD;


#define sw64(A) ((uint64_t)(\
				(((uint64_t)(A)& (uint64_t)0x00000000000000ffULL) << 56) | \
				(((uint64_t)(A)& (uint64_t)0x000000000000ff00ULL) << 40) | \
				(((uint64_t)(A)& (uint64_t)0x0000000000ff0000ULL) << 24) | \
				(((uint64_t)(A)& (uint64_t)0x00000000ff000000ULL) << 8) | \
				(((uint64_t)(A)& (uint64_t)0x000000ff00000000ULL) >> 8) | \
				(((uint64_t)(A)& (uint64_t)0x0000ff0000000000ULL) >> 24) | \
				(((uint64_t)(A)& (uint64_t)0x00ff000000000000ULL) >> 40) | \
				(((uint64_t)(A)& (uint64_t)0xff00000000000000ULL) >> 56) ))

typedef struct
{
    uint8_t CmdCode;
    uint8_t FunctionCode;
} _66_CMD;

_66_CMD Send66CmdTable[CMD_SEND_MAX] =
{

    {0x01,0x01},  		//注册
    {0x01,0x02},		//心跳
    {0x02,0x01},		//实时数据
    {0x02,0x02},		//启动应答
    {0x02,0x03},		//功率调节应答
    {0x02,0x04},		//停止应答
    {0x02,0x05},		//上传订单
    {0x02,0x06},		//订单查询应答
    {0x03,0x01},		//软件复位应答
    {0x03,0x02},		//时间同步应答
    {0x03,0x03},		//设置二维码应答
};


_66_CMD Recv66CmdTable[CMD_RECV_MAX] =
{

    {0x81,0x01},    		//注册应答
    {0x81,0x02},			//心跳应答
    {0x82,0x02},			//启动应答
    {0x82,0x03},			//功率调节应答
    {0x82,0x04},			//停止应答
    {0x82,0x05},			//上传订单应答
    {0x82,0x06},			//订单查询应答
    {0x83,0x01},			//软件复位
    {0x83,0x02},			//时间同步
    {0x83,0x03},			//设置二维码
    {0x83,0x04},			//远程升级
};


//登录
__packed typedef struct
{
    uint8_t Username[32];    	//BCD  不足7位补充0
    uint8_t Password[32];		//枪号，从1开始
    uint8_t SoftVer[16];		//枪状态
    uint8_t HardwareVer[16];	//硬件版本
    uint8_t MAC[32];
    uint8_t	SIM[20];
} _66_SEND_REGISTER;

__packed typedef struct
{
    /* 1-成功 2-拒绝， 设备不存在 3-拒绝， 设备未激活 4-拒绝， 设备暂停服务 5-拒绝， 鉴权失败 */
    uint8_t Result;
} _66_RECV_REGISTER;

//心跳
__packed typedef struct
{
    uint64_t SysTime;    		//系统时间 当前设备的时间戳(毫秒)
} _66_SEND_HEARTBEAT;



//实时数据
//充电桩需周期性的上报当前实时数据。充电中 60 秒上报一次，非充电中 180 秒上报一
//次。在指定的字段中，若标注为变化上报，则当字段数值发生变化时，立即上报。数据类型
__packed typedef struct
{
    uint8_t gun;    	//系统时间 当前设备的时间戳(毫秒)
    uint8_t OrderNumber[30];	//订单号
    uint64_t StartTime;			//开始充电时间  时间戳 (毫秒)
    uint8_t WorkState;			//工作状体 0-空闲中 10-插枪未开始充电  30-充电进行中  40-充电完成未拔枪 60-故障
    int8_t scRssi;
    /* 6、告警状态 Bit0-过压告警 0-正常,1-异常 Bit1-欠压告警 0-正常,1-异常 Bit2-过流告警 0-正常,1-异常 Bit3-短路告警 0-正常,1-异常
                   Bit4-漏电告警 0-正常,1-异常 Bit5-枪头过温告警 0-正常,1-异常 Bit7-急停告警 0-正常,1-异常 Bit8-继电器粘连 0-正常,1-异常(变化上传) */
    __packed union
    {
        uint16_t ssWarningState;
        __packed struct
        {
            uint8_t		Bit0_OverVolt			:1;
            uint8_t		Bit1_LowVolt			:1;
            uint8_t		Bit2_OverCurrent	    :1;
            uint8_t		Bit3_SHORT_CIRCUIT	    :1;
            uint8_t		Bit4_LEAKAGE	        :1;
            uint8_t		Bit5_CONNECTOR_OVERHEAT	:1;
            uint8_t		Bit6_NONE	            :1;
            uint8_t		Bit7_EMERGENCY_STOP		:1;
            uint8_t		Bit8_RELAY_ERR			:1;
            uint8_t		res			:7;
        } TwoByte;
    } u1;
    /* 7、A 相电压 单位 0.1V */
    int16_t UaVolt;
    /* 8、A 相电流 单位 0.01A */
    int16_t Currenta;
    /* 9、B 相电压 单位 0.1V */
    int16_t UbVolt;
    /* 10、B 相电流 单位 0.01A */
    int16_t Currentb;
    /* 11、C 相电压 单位 0.1V */
    int16_t UcVolt;
    /* 12、C 相电流 单位 0.01A */
    int16_t Currentc;
    /* 13、CP 引导电压 单位 0.01V */
    int16_t CpVolt;
    /* 14、PWM 占空比 0.01%/位,0 偏移量,数据范围 0～100% */
    int16_t PwmPuls;
    uint8_t RFID[18];
    /* 9、计费策略 1 电量计费 2 时长计费 */
    uint8_t	ChargeFeeType;
    uint8_t StartSeg;  //开始时间段 N
    uint8_t SegNum;  //时间段个数
    uint16_t  SegEnergy[48];  //20、电量N 单位 0.001 度,精确到小数点后三位
} _66_SEND_DATA;

//启动充电
__packed typedef struct
{
    /* 1、枪号 充电枪口号,默认值为 1 */
    int8_t	gun;
    /* 订单号 */
    uint8_t OrderNumber[30];	//订单号
    /* 6、充电结束条件 1:按电量充电 2:按时间充电 3:按金额充电 4:充满为止 */
    int8_t	ChargeEndCondition;
    /* 7、充电参数 按电量充电,此数据为充电电量,单位 0.01 度 按时间充电,此数据为充电时长,单位 1 分钟 按金额充电,此数据为充电金额,单位 0.01 元 按充满为止,此数据为 0 */
    int32_t ChargePara;
    /* 8、充电功率限额 默认为 0,表示不限制绝对值,数据分辨率： 0.1kWh/位； 偏移量： -1000.0kWh； 数据范围：-1000.0kW ~ +1000.0kWh(正表示充电,负是放电) */
    int16_t ChargePowerMax;
    /* 9、计费策略 1 电量计费 2 时长计费 */
    int8_t	ChargeFeeType;
    /* 10、时间段数N 时间段个数,最大值 48 */
    int8_t SegNum;
    /*单位 0.01 元/度，精确到小数点后两位，如果为时长计费策略， 单 位 0.01元/分钟*/
    int16_t Price[48];

} _66_RECV_STARTCHARGE;


//启动应答
__packed typedef struct
{
    /* 1、枪号 充电枪口号,默认值为 1 */
    int8_t	gun;
    /* 2、成功标志 1-成功 4100-失败,其它原因 4101-失败,未检测到枪 4201-失败,系统故障 4202-失败,暂停服务 4203-失败,电表通信中断 */
    int16_t ssStartSuccessFlag;
    int8_t OrderNumber[30];
} _66_SEND_CHARGE_ACK;



/* 2.7.4 功率调节(82,03) */
__packed typedef struct
{
    /* 1、枪号 充电枪口号,默认值为 1 */
    int8_t	gun;
    /* 2、充电功率限额 默认为 0,表示不限制绝对值,数据分辨率:0.1kWh/位;偏移量:-1000.0 kWh;数据范围:-1000.0kW ~ +1000.0kWh(正表示充电,负是放电) */
    int16_t ChargePowerMax;

} _66_RECV_POWERCTL;

/* 2.7.5 功率调节应答(02,03) */
__packed typedef struct
{
    /* 1、枪号 充电枪口号,默认值为 1 */
    int8_t	gun;
    /* 2、结果 1-执行成功 2-失败,设备不支持 3-失败,其它原 */
    int8_t PowerFlag;

} _66_SEND_POWERCTL_ACK;


/* 2.7.6 停止充电(82,04) */
__packed typedef struct
{
    /* 1、枪号 充电枪口号,默认值为 1 */
    int8_t	gun;
    /* 2、停止原因 1001-用户APP主动停止 1002-余额不足停机 1006-客服主动停止充电 1007-客服强制停止充电 1011-充电金额超阀值后台主停止充电 */
    int16_t	ChargeSyopReason;
    /* 3、订单号 最大 32 */
    uint8_t OrderNumber[30];	//订单号

} _66_RECV_STOP;

/* 2.7.7 停止充电应答(02,04) */
__packed typedef struct
{
    /* 1、枪号 充电枪口号,默认值为 1 */
    int8_t	gun;
    /* 2、结果 1-执行成功 2-订单号不匹配 5100-其它原因停止 5101-设备没有充电 */
    int16_t ChargeStopFlag;

} _66_SEND_STOP_ACK;




/* 2.7.8 订单信息上报(02,05) */
__packed typedef struct
{
    /* 3、订单号 最大 30 */
    uint8_t OrderNumber[30];	//订单号
    /* 4、终止充电原因 参见附录终止充电原因 */
    uint16_t StopReason;
    /* 5、开始充电时间 时间戳(1970年到开始时间的毫秒数) */
    uint64_t StartChargeTime;
    /* 6、结束充电时间 时间戳(1970年到结束时间的毫秒数) */
    uint64_t StopChargeTime;
    /* 7、充电时长 单位秒 */
    uint32_t ChargeTime;
    /* 8、充电总电量 单位0.001KW,精确到小数点后四位 */
    uint32_t TotalChargePower;
    /* 9、计费策略 1 电量计费 2 时长计费 */
    uint8_t	ChargeFeeType;
    /* 10、充电总费用 单位0.0001元,精确到小数点后两位 */
    uint32_t TotalFee;

    /* 11、开始时间段 N */
    uint8_t StartSegNum;
    /* 11、时间段个数 N */
    uint8_t SegNum;

    /* 14、时间段 N 充电量 单位0.001KW,精确到小数点后三位 */
    uint16_t SegEnergy[48];
} _66_SEND_ORDER;


/* 2.7.9 订单信息应答(82,05) */
__packed typedef struct
{
    /* 3、订单号 最大 30 */
    uint8_t OrderNumber[30];	//订单号
    /* 2、结果 1-成功 2-失败 */
    int8_t TradeInfoFlag;
} _66_RECV_ORDER_ACK;

/* 订单查询 */
__packed typedef struct
{
    /* 1、枪号 充电枪口号,默认值为 1 */
    int8_t	gun;
    /* 3、订单号 最大 30 */
    uint8_t OrderNumber[30];	//订单号
} _66_RECV_ORDER_QUERY;


/* 2.7.11 订单远程查询应答  */
__packed typedef struct
{
    /* 1、结果 1-成功 2-订单不存在 */
    uint8_t CheckTradeInfoFlag;
    //如果订单不存在，后续字段内容不需要传

    /* 3、订单号 最大 30 */
    uint8_t OrderNumber[30];	//订单号
    /* 4、终止充电原因 参见附录终止充电原因 */
    uint16_t StopReason;
    /* 5、开始充电时间 时间戳(1970年到开始时间的毫秒数) */
    uint64_t StartChargeTime;
    /* 6、结束充电时间 时间戳(1970年到结束时间的毫秒数) */
    uint64_t StopChargeTime;
    /* 7、充电时长 单位秒 */
    uint32_t ChargeTime;
    /* 8、充电总电量 单位0.001KW,精确到小数点后四位 */
    uint32_t TotalChargePower;
    /* 9、计费策略 1 电量计费 2 时长计费 */
    uint8_t	ChargeFeeType;
    /* 10、充电总费用 单位0.0001元,精确到小数点后两位 */
    uint32_t TotalFee;

    /* 11、开始时间段 N */
    uint8_t StartSegNum;
    /* 11、时间段个数 N */
    uint8_t SegNum;

    /* 14、时间段 N 充电量 单位0.001KW,精确到小数点后三位 */
    uint16_t SegEnergy[48];
} _66_SNED_ORDER_QUERYACK;


/*软件复位应答( (0 0 3,01)*/
__packed typedef struct
{
    /*1-成功，2-设备充电中，拒绝复位*/
    int8_t	Flag;
} _66_SNED_RESET_ACK;



/*时钟同步*/
__packed typedef struct
{
    /*时间戳  1970 年到当前时间的毫*/
    uint64_t	Time;
} _66_RECV_TIME;

/*时钟同步*/
__packed typedef struct
{
    /*成功标志 1-成功 2-设备充电中，拒绝校时*/
    int8_t	Flag;
} _66_SEND_TIME_ACK;


/*设置二维码*/
__packed typedef struct
{
    int8_t gun;		//枪号
    int8_t	QR[80]; //二维码
} _66_RECV_SETQR;

/*设置二维码应答*/
__packed typedef struct
{
    int8_t gun;		//枪号
//	1-成功，2-失败，设备不支持 3-失败，其他错误
    int8_t	Flag;
} _66_SEND_SETQR_ACK;

/*远程升级*/
__packed typedef struct
{
    int32_t FileLen;		//文件大小
    int8_t	URL[128]; 		//下载地址
    int8_t	MD5[32];		//文件校验
} _66_RECV_UPDATA;


__packed typedef struct
{
    uint16_t 	StartCharge;	//启动充电
    uint16_t 	SetPower;		//功率调节
    uint16_t	StopCharge;		//停止充电
    uint16_t	SelBill;		//查询订单
    uint16_t	ResSet;			//软件复位
    uint16_t	SetTime;		//时间同步
    uint16_t	SetQ;			//设置二维码
    uint16_t	Updata;			//远程升级
} _66_RECV_COUNT;			//接收报文标识符，应答时需要和接收对应上



extern RATE_T	           stChRate;                 /* 充电费率  */
extern _dlt645_info dlt645_info;
extern CH_TASK_T stChTcb;
_66_RECV_COUNT RecvCount = {0};  	//接收应答报文标识符
uint16_t		SendCount = 1;		//报文标识符
uint8_t IfAPPStop = 0;		//是否app主动停止
uint8_t Send66buf[300];
uint8_t Recv66buf[300];
_66_RECV_STARTCHARGE _66StartCharge;
_66_RECV_SETQR _66_QR = {0};   //66二维码
_4G_START_TYPE _66_StartType =  _4G_APP_START;					//0表示App启动 1表示刷卡启动

uint8_t   _66_SendRegisterCmd(void);
uint8_t   _66_SendHearGunACmd(void);
uint8_t   _66_SendSJDataGunACmd(void);

static uint8_t   _66_RecvRegisterAck(uint8_t *pdata,uint16_t len);
static uint8_t   _66_RecvHeartAck(uint8_t *pdata,uint16_t len);
static uint8_t   _66_RecvStart(uint8_t *pdata,uint16_t len);
static uint8_t   _66_RecvSetPower(uint8_t *pdata,uint16_t len);
static uint8_t   _66_RecvStop(uint8_t *pdata,uint16_t len);
static uint8_t   _66_RecvBillAck(uint8_t *pdata,uint16_t len);
static uint8_t   _66_RecvBillQ(uint8_t *pdata,uint16_t len);
static uint8_t   _66_RecvReset(uint8_t *pdata,uint16_t len);
static uint8_t   _66_RecvSetTime(uint8_t *pdata,uint16_t len);
static uint8_t   _66_RecvSetQR(uint8_t *pdata,uint16_t len);
static uint8_t   _66_RecvOTA(uint8_t *pdata,uint16_t len);
_4G_SEND_TABLE _66SendTable[SEND_66_FRAME_LEN] = {
    {0,    0,    CM_TIME_10_SEC, 	_66_SendRegisterCmd		},  //发送注册帧

    {0,    0,    CM_TIME_20_SEC, 	_66_SendHearGunACmd		},  //发送枪心跳状态

    {0,    0,    CM_TIME_60_SEC, 	_66_SendSJDataGunACmd		},  //充电中 60 秒上报一次，非充电中 180 秒上报一次

};


_4G_RECV_TABLE _66_RecvTable[CMD_RECV_MAX] = {

    {CMD_RECV_REGISTER 			,	_66_RecvRegisterAck		}, 		//注册

    {CMD_RECV_HEARTBEAT			,	_66_RecvHeartAck		}, 		//心跳

    {CMD_RECV_START				,	_66_RecvStart			}, 		//启动充电

    {CMD_RECV_POWERCTL			,	_66_RecvSetPower		}, 		//功率调节

    {CMD_RECV_STOP				,	_66_RecvStop			}, 		//停止应答

    {CMD_RECV_ORDER				,	_66_RecvBillAck			}, 		//订单上传

    {CMD_RECV_OORDERQ			,	_66_RecvBillQ			}, 		//订单查询

    {CMD_RECV_RESET				,	_66_RecvReset			}, 		//软件复位

    {CMD_RECV_SETTIME			,	_66_RecvSetTime			}, 		//时间同步

    {CMD_RECV_SETQR				,	_66_RecvSetQR			}, 		//设置二维码

    {CMD_RECV_OTA				,	_66_RecvOTA			}, 		//远程升级

};
int StringToHex(char *str, char *out, unsigned int *outlen)
{
    char *p = str;
    char high = 0, low = 0;
    int tmplen = strlen(p), cnt = 0;
    tmplen = strlen(p);
    while(cnt < (tmplen / 2))
    {
        high = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
        low = (*(++ p) > '9' && ((*p <= 'F') || (*p <= 'f'))) ? *(p) - 48 - 7 : *(p) - 48;
        out[cnt] = ((high & 0x0f) << 4 | (low & 0x0f));
        p ++;
        cnt ++;
    }
    if(tmplen % 2 != 0) out[cnt] = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;

    if(outlen != NULL) *outlen = tmplen / 2 + tmplen % 2;
    return tmplen / 2 + tmplen % 2;
}
/*****************************************************************************
* Function     : _66_GetBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
uint8_t *  _66_GetBatchNum(uint8_t gun)
{
    return _66StartCharge.OrderNumber;
}

/*****************************************************************************
* Function     : APP_Get66StartType
* Description  :获取启动方式
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   APP_Get66StartType(uint8_t gun)
{
    if(gun != GUN_A)
    {
        return _4G_APP_START;
    }
    return (uint8_t)_66_StartType;
}

/*****************************************************************************
* Function     : APP_SetAPStartType
* Description  : 设置安培快充启动方式
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   APP_Set66StartType(uint8_t gun ,_4G_START_TYPE  type)
{
    if((type >=  _4G_APP_MAX) || (gun != GUN_A))
    {
        return FALSE;
    }

    _66_StartType = type;
    return TRUE;
}

/**
 * @brief 将uint16_t类型调换高低字节并返回
 * @param[in] usSrc 待转化数据
 * @param[out]
 * @return uiret 转化后的数据
 * @note
 */
uint8_t app_get66_qr(uint8_t * pdata,uint8_t len)
{
    if(pdata == NULL || !len)
    {
        return FALSE;
    }
    memcpy(pdata,_66_QR.QR,MIN(len,sizeof(_66_QR.QR)));
    return TRUE;
}

/**
 * @brief 将uint16_t类型调换高低字节并返回
 * @param[in] usSrc 待转化数据
 * @param[out]
 * @return uiret 转化后的数据
 * @note
 */
uint16_t uint16_H_to_L(uint16_t usSrc)
{
    uint16_t usret = 0;

    usret = (usSrc&0x00FF);
    usret <<= 8;
    usret |= (uint8_t)(usSrc>>8);
    return usret;
}

/**
 * @brief 将uint32_t类型低两字节和高两字节分别调换高低字节，然后将低两字节和高两字节调换高低字节，得到一个新的uint32_t并返回
 * @param[in] uiSrc 待转化数据
 * @param[out]
 * @return uiret 转化后的数据
 * @note 例如0x00100100-->0x10000001-->0x00011000
 */
uint32_t uint32split_H_to_L(uint32_t uiSrc)
{
    uint16_t Hdata;
    uint16_t Ldata;
    uint32_t uiret = 0;


    Ldata = (uint16_t)(uiSrc & 0xFFFF);
    Ldata = uint16_H_to_L( Ldata );

    Hdata = (uint16_t)(uiSrc >> 16);
    Hdata = uint16_H_to_L( Hdata );

    uiret = (Ldata << 16) | Hdata;

    return uiret;

}



/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t  get_xor_check_sum(uint8_t *pucData, int iDataLen)
{
    uint8_t ucXor = 0;
    int i = 0;

    for(i = 0; i < iDataLen; i++)
    {
        ucXor  ^= pucData[i];
    }

    return ucXor;
}

/*****************************************************************************
* Function     : Fream66Send
* Description  : 66帧发送
* Input        :
count 报文标识符  主动发送 报文标识符自动+1；从1开始，应答应与后台发送过来的标识符对应上
				ucCmdCode 指令码
 *            	ucFunctionCode 功能码
 *            	pdata 数据
				len 数据长度
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10 月18日
*****************************************************************************/
uint8_t   Fream66Send(uint16_t count, uint8_t CmdCode, uint8_t FunctionCode,uint8_t *pdata, uint16_t len)
{
    //根据66协议，如下发送
    uint16_t crc,i,devlen;
    uint8_t Sendbuf[300] = {0};
    if((pdata == NULL) || (!len)  || (len > 250))
    {
        return FALSE;
    }
//    DisSysConfigInfo.DevNum[15] = 0;  //防止长度越界
//    if(strlen((char*)DisSysConfigInfo.DevNum) > 16)
//    {
//        return FALSE;
//    }
    if(count == 0)
    {
        count = 1;
    }
    //1字节帧投
    Sendbuf[0] = 0x68;
    //版本号，固定为3
    Sendbuf[1] = 3;
    //2字节序列号
    Sendbuf[2] = CmdCode;
    Sendbuf[3] = FunctionCode;
    //报文标识符  大端
    Sendbuf[5] = count & 0x00ff;
    Sendbuf[4] = (count >> 8) & 0x00ff;

//    //DisSysConfigInfo.DevNum[15] = 0;  //防止长度越界
//    Sendbuf[6] =strlen((char*)DisSysConfigInfo.DevNum);			//桩编码固定为
//    devlen	= strlen((char*)DisSysConfigInfo.DevNum);
		
		Sendbuf[6] =sizeof(DisSysConfigInfo.DevNum);			//桩编码固定为
    devlen	= sizeof(DisSysConfigInfo.DevNum);
		
    //充电桩编号
    memcpy(&Sendbuf[7],(char*)DisSysConfigInfo.DevNum,devlen);
    //数据域长度 大端
    Sendbuf[8+devlen] = len & 0x00ff;
    Sendbuf[7+devlen] = (len >> 8) & 0x00ff;
    //数据域
    memcpy(&Sendbuf[9+devlen],pdata,len);
    //2字节crc
    Sendbuf[9+devlen+len] = get_xor_check_sum(Sendbuf,9+devlen+len);
    Sendbuf[10+devlen+len] = 0x48;

    count++;

    ModuleSIM7600_SendData(0, Sendbuf,(11+devlen+len)); //发送数据

	printf("tx len:%d,tx data:",11+devlen+len);
    for(i = 0; i < (11+devlen+len); i++)
    {
        printf("%02x ",Sendbuf[i]);
    }
    printf("\r\n");
    OSTimeDly(500, OS_OPT_TIME_PERIODIC, &timeerr);
    return TRUE;
}




/*******************************周期性发送数据*******************************/
/*****************************************************************************
* Function     : _66_SendRegisterCmd
* Description  : 发送注册帧
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日完成
*****************************************************************************/
uint8_t   _66_SendRegisterCmd(void)
{
    static uint32_t i = 0;
    _66_SEND_REGISTER * pRegister = NULL;
    char Passward_temp[128] = {0};
    char base64para[128] = {0};
    int  Passward_temp_len = 0;
    unsigned int outlen = 0;
//	uint8_t * pid;
		uint8_t buf[17];
		memcpy(buf,DisSysConfigInfo.DevNum,16);
		buf[16] = 0;
    if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
    {
        return FALSE;
    }
    memset(Send66buf,0,sizeof(Send66buf));



    pRegister = (_66_SEND_REGISTER *)Send66buf;
    //DisSysConfigInfo.DevNum[15] = 0;  //防止长度越界
    snprintf((char*)pRegister->Username, sizeof(pRegister->Username), "%s&%d",buf,cp56time(NULL));
    /* 以DeviceSecret为秘钥对temp中的明文,进行hmacsha1加密,结果就是密码,并保存到缓冲区中 */
    utils_hmac_sha1((char*)pRegister->Username,strlen((char*)pRegister->Username),Passward_temp,DEVICESECRE,DEVICESECRE_LEN);
    //utils_hmac_sha1("AC750054&1666934389176",strlen("AC750054&1666934389176"),Passward_temp,DEVICESECRE,DEVICESECRE_LEN);
    Passward_temp_len = strlen(Passward_temp);
    StringToHex(Passward_temp,base64para,&outlen);
    base64_encode(base64para, (char*)pRegister->Password, outlen);


    memcpy(pRegister->SoftVer,"V1.0",strlen("V1.0"));
    memcpy(pRegister->HardwareVer,"V1.0",strlen("V1.0"));
    memset(pRegister->SIM,0,sizeof(pRegister->SIM));
    memcpy(pRegister->SIM,SIMNum,MIN(sizeof(pRegister->SIM),sizeof(SIMNum)));
    memset(pRegister->MAC,0,sizeof(pRegister->MAC));
    memcpy(pRegister->MAC,_4GIMEI,MIN(sizeof(pRegister->MAC),sizeof(_4GIMEI)));



    return Fream66Send(SendCount++,Send66CmdTable[CMD_SEND_REGISTER].CmdCode,Send66CmdTable[CMD_SEND_REGISTER].FunctionCode,Send66buf,sizeof(_66_SEND_REGISTER));
}


//心跳
/*****************************************************************************
* Function     : YKC_SendHearGunACmd3
* Description  : 云快充发送A枪心跳状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
uint8_t   _66_SendHearGunACmd(void)
{
    _66_SEND_HEARTBEAT *pheart = NULL;
    if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
    {
        return FALSE;		//注册未成功，无需发送
    }
    memset(Send66buf,0,sizeof(Send66buf));
    pheart = (_66_SEND_HEARTBEAT *)Send66buf;

    pheart->SysTime =sw64(cp56time(NULL) *(uint64_t)1000);

    return Fream66Send(SendCount++,Send66CmdTable[CMD_SEND_HEARTBEAT].CmdCode,Send66CmdTable[CMD_SEND_HEARTBEAT].FunctionCode,Send66buf,sizeof(_66_SEND_HEARTBEAT));
}


/*****************************************************************************
* Function     : _66_SendSJDataGunACmd13
* Description  : 充电中 60 秒上报一次，非充电中 180 秒上报一次
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
uint8_t    _66_SendSJDataGunACmd(void)
{
    uint8_t num;
    uint8_t i = 0;
    static uint8_t count = 0;

    _66_SEND_DATA *pSJData = NULL;
    if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
    {
        return FALSE;		//注册未成功，无需发送
    }


    //充电中 60 秒上报一次，非充电中 180 秒上报一次
    if( CHARGING != stChTcb.ucState)
    {
        count++;
        if(count < CM_TIME_3_MIN/CM_TIME_60_SEC)
        {
            return TRUE;
        }

    }
    count = 0;


    memset(Send66buf,0,sizeof(Send66buf));
    pSJData = (_66_SEND_DATA *)Send66buf;

    pSJData->gun = 1;
    if( CHARGING == stChTcb.ucState)
    {
        memcpy(pSJData->OrderNumber,_66StartCharge.OrderNumber,sizeof(_66StartCharge.OrderNumber)); //订单号
        pSJData->StartTime = sw64(stChTcb.stCh.timestart *(uint64_t)1000);			//开始充电时间
        if(DisSysConfigInfo.energymeter == 1)
        {
            pSJData->UaVolt = uint16_H_to_L(stChTcb.stHLW8112.usVolt);			//电压		//输出电压  0.1
            pSJData->Currenta = uint16_H_to_L(stChTcb.stHLW8112.usCurrent);		//输出电流	0.01
        }
        else
        {
            pSJData->UaVolt = uint16_H_to_L(dlt645_info.out_vol);			//电压		//输出电压  0.1
            pSJData->Currenta = uint16_H_to_L(dlt645_info.out_cur/10);		//输出电流	0.1
        }




        pSJData->PwmPuls = uint16_H_to_L(5300);  //占空比
        pSJData->StartSeg =  ((stChTcb.stCh.timestart + BEIJI_TIMESTAMP_OFFSET)    % CH_DAT_SEC) / CH_HALF_HOUR_SEC + 1;  //开始时间段
        num =  ((cp56time(NULL) + BEIJI_TIMESTAMP_OFFSET)% CH_DAT_SEC) / CH_HALF_HOUR_SEC + 1;
        pSJData->SegNum = num >= pSJData->StartSeg ? (num - pSJData->StartSeg + 1):((48 - pSJData->StartSeg) + num + 1);
        if(pSJData->SegNum > 48)   //时间段个数大于48
        {
            return FALSE;
        }
        for(i = 0; i < pSJData->SegNum; i++)
        {
            if(pSJData->StartSeg + i <= 48)
            {
                pSJData->SegEnergy[i] = uint16_H_to_L(stChTcb.stCh.Curkwh[pSJData->StartSeg - 1 + i]);   //电量
            }
            else
            {
                pSJData->SegEnergy[i] = uint16_H_to_L(stChTcb.stCh.Curkwh[pSJData->StartSeg + i - 48 - 1]);   //电量
            }
        }
        pSJData->ChargeFeeType = _66StartCharge.ChargeFeeType;	//充电方式

    }
    else
    {
        pSJData->ChargeFeeType = 1;
        pSJData->SegNum = 0;
    }

    if(stChTcb.ucState == CHARGING)
    {
        pSJData->WorkState = 30;
    }
    else if(stChTcb.ucState == CHARGER_FAULT)
    {
        pSJData->WorkState  = 60; //故障
    }
    else if(stChTcb.stGunStat.ucCpStat != CP_12V)
    {
        pSJData->WorkState = 10;    //插枪
    }
    else
    {
        pSJData->WorkState = 0; //待机
    }


    pSJData->scRssi = APP_GetCSQNum();	//信号强度
    if(stChTcb.stOverVolt.ucFlg)
    {
        pSJData->u1.TwoByte.Bit0_OverVolt = 1;
    }

    if(stChTcb.stOverVolt.ucFlg)
    {
        pSJData->u1.TwoByte.Bit1_LowVolt = 1;
    }

    if(stChTcb.stOverVolt.ucFlg)
    {
        pSJData->u1.TwoByte.Bit2_OverCurrent = 1;
    }
    pSJData->CpVolt = uint16_H_to_L(stChTcb.stGunStat.uiCpVolt/10);


    return Fream66Send(SendCount++,Send66CmdTable[CMD_SEND_DATA].CmdCode,Send66CmdTable[CMD_SEND_DATA].FunctionCode,Send66buf,sizeof(_66_SEND_DATA) - 2*(48 - pSJData->SegNum));
}


/*******************************非周期性发送数据*******************************/
/*****************************************************************************
* Function     : _66_SendDataGunACmd
* Description  : 状态发生变化发送
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
uint8_t    _66_SendDataGunACmd(void)
{
    uint8_t num;
    uint8_t i = 0;

    _66_SEND_DATA *pSJData = NULL;
    if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
    {
        return FALSE;		//注册未成功，无需发送
    }




    memset(Send66buf,0,sizeof(Send66buf));
    pSJData = (_66_SEND_DATA *)Send66buf;

    pSJData->gun = 1;
    if( CHARGING == stChTcb.ucState)
    {
        memcpy(pSJData->OrderNumber,_66StartCharge.OrderNumber,sizeof(_66StartCharge.OrderNumber)); //订单号
        pSJData->StartTime = sw64(stChTcb.stCh.timestart *(uint64_t)1000);			//开始充电时间
        if(DisSysConfigInfo.energymeter == 1)
        {
            pSJData->UaVolt = uint16_H_to_L(stChTcb.stHLW8112.usVolt);			//电压		//输出电压  0.1
            pSJData->Currenta = uint16_H_to_L(stChTcb.stHLW8112.usCurrent);		//输出电流	0.1
        }
        else
        {
            pSJData->UaVolt = uint16_H_to_L(dlt645_info.out_vol);			//电压		//输出电压  0.1
            pSJData->Currenta = uint16_H_to_L(dlt645_info.out_cur/10);		//输出电流	0.1
        }
        pSJData->PwmPuls = uint16_H_to_L(5300);  //占空比

        pSJData->StartSeg =  ((stChTcb.stCh.timestart+ BEIJI_TIMESTAMP_OFFSET) % CH_DAT_SEC) / CH_HALF_HOUR_SEC + 1;  //开始时间段
        num =  ((cp56time(NULL)+ BEIJI_TIMESTAMP_OFFSET) % CH_DAT_SEC) / CH_HALF_HOUR_SEC + 1;
        pSJData->SegNum = num >= pSJData->StartSeg ? (num - pSJData->StartSeg + 1):((48 - pSJData->StartSeg) + num + 1);
        for(i = 0; i < 48; i++)
        {
            pSJData->SegEnergy[i] = uint16_H_to_L(stChTcb.stCh.Curkwh[i]);   //电量
        }
        pSJData->ChargeFeeType = _66StartCharge.ChargeFeeType;	//充电方式

    }
    else
    {
        pSJData->SegNum = 0;
        pSJData->ChargeFeeType = 1;	//充电方式
    }

    if(stChTcb.ucState == CHARGING)
    {
        pSJData->WorkState = 30;
    }
    else if(stChTcb.ucState == CHARGER_FAULT)
    {
        pSJData->WorkState  = 60; //故障
    }
    else if(stChTcb.stGunStat.ucCpStat != CP_12V)
    {
        pSJData->WorkState = 10;    //插枪
    }
    else
    {
        pSJData->WorkState = 0; //待机
    }

    pSJData->scRssi = APP_GetCSQNum();	//信号强度
    if(stChTcb.stOverVolt.ucFlg)
    {
        pSJData->u1.TwoByte.Bit0_OverVolt = 1;
    }

    if(stChTcb.stOverVolt.ucFlg)
    {
        pSJData->u1.TwoByte.Bit1_LowVolt = 1;
    }

    if(stChTcb.stOverVolt.ucFlg)
    {
        pSJData->u1.TwoByte.Bit2_OverCurrent = 1;
    }
    pSJData->CpVolt = uint16_H_to_L(stChTcb.stGunStat.uiCpVolt/10);

	
    return Fream66Send(SendCount++,Send66CmdTable[CMD_SEND_DATA].CmdCode,Send66CmdTable[CMD_SEND_DATA].FunctionCode,Send66buf,sizeof(_66_SEND_DATA) - 2*(48 - pSJData->SegNum));
}


/*****************************************************************************
* Function     : _66_SendStartAck
* Description  : 启动应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   _66_SendStartAck(_GUN_NUM gun)
{

    _66_SEND_CHARGE_ACK *pChargeAck = NULL;
    if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
    {
        return FALSE;		//注册未成功，无需发送
    }

    if(gun >= GUN_MAX)
    {
        return FALSE;
    }
    memset(Send66buf,0,sizeof(Send66buf));
    pChargeAck = (_66_SEND_CHARGE_ACK *)Send66buf;

    pChargeAck->gun = 1;

    if( CHARGING == stChTcb.ucState)
    {
        pChargeAck->ssStartSuccessFlag = uint16_H_to_L(1);
    }
    else
    {
        if(stChTcb.stGunStat.ucCpStat == CP_12V)
        {
            pChargeAck->ssStartSuccessFlag  = uint16_H_to_L(4101);  //未插枪
        }
        else
        {
            pChargeAck->ssStartSuccessFlag  = uint16_H_to_L(4100);
        }
    }
    memcpy(pChargeAck->OrderNumber,_66StartCharge.OrderNumber,sizeof(_66StartCharge.OrderNumber)); //订单号
    return Fream66Send(RecvCount.StartCharge,Send66CmdTable[CMD_SEND_START_ACK].CmdCode,Send66CmdTable[CMD_SEND_START_ACK].FunctionCode,Send66buf,sizeof(_66_SEND_CHARGE_ACK));
}


/*****************************************************************************
* Function     : _66_SendPowerCTLAck
* Description  : 功率调节控制
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   _66_SendPowerCTLAck(_GUN_NUM gun)
{

    _66_SEND_POWERCTL_ACK *pPowerCtl = NULL;
    if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
    {
        return FALSE;		//注册未成功，无需发送
    }

    if(gun >= GUN_MAX)
    {
        return FALSE;
    }
    memset(Send66buf,0,sizeof(Send66buf));
    pPowerCtl = (_66_SEND_POWERCTL_ACK *)Send66buf;

    pPowerCtl->gun = 1;
    pPowerCtl->PowerFlag = 1;		//功率调节成功

    return Fream66Send(RecvCount.SetPower,Send66CmdTable[CMD_SEND_POWERCTL_ACK].CmdCode,Send66CmdTable[CMD_SEND_POWERCTL_ACK].FunctionCode,Send66buf,sizeof(_66_SEND_POWERCTL_ACK));
}


/*****************************************************************************
* Function     : _66_SendStopAck
* Description  : 停止充电应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t   _66_SendStopAck(_GUN_NUM gun)
{

    _66_SEND_STOP_ACK *pStop = NULL;
    if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
    {
        return FALSE;		//注册未成功，无需发送
    }

    if(gun >= GUN_MAX)
    {
        return FALSE;
    }
    memset(Send66buf,0,sizeof(Send66buf));
    pStop = (_66_SEND_STOP_ACK *)Send66buf;

    pStop->gun = 1;
    pStop->ChargeStopFlag = 1;		//功率调节成功

    return Fream66Send(RecvCount.StopCharge,Send66CmdTable[CMD_SEND_STOP_ACK].CmdCode,Send66CmdTable[CMD_SEND_STOP_ACK].FunctionCode,Send66buf,sizeof(_66_SEND_STOP_ACK));
}


/*****************************************************************************
* Function     : _66_SendBill
* Description  : 发送订单
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日
*****************************************************************************/
uint8_t  _66_SendBill(_GUN_NUM gun)
{
    uint8_t i,num;
    uint8_t errcode;

    _66_SEND_ORDER *pBill	= NULL;
    if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
    {
        return FALSE;		//注册未成功，无需发送
    }

    if(gun >= GUN_MAX)
    {
        return FALSE;
    }
    memset(Send66buf,0,sizeof(Send66buf));
    pBill = (_66_SEND_ORDER *)Send66buf;
    memcpy(pBill->OrderNumber,_66StartCharge.OrderNumber,sizeof(_66StartCharge.OrderNumber)); //订单号





    // 停止原因
    if((stChTcb.stCh.reason == NORMAL) || (stChTcb.stCh.reason == END_CONDITION) || \
            (stChTcb.stCh.reason == NO_CURRENT) ||( stChTcb.stCh.reason == STOP_CHARGEFULL) )
    {
        pBill->StopReason = uint16_H_to_L(3501);  //充满停止
    }
    else if(stChTcb.stCh.reason == E_END_BY_APP)
    {
        pBill->StopReason = uint16_H_to_L(1001); //APP主动停止
    }
    else if(stChTcb.stCh.reason == E_END_BY_CARD)
    {
        pBill->StopReason = uint16_H_to_L(3503); //刷卡停止
    }
    else if(stChTcb.stCh.reason == UNDER_VOLT)
    {
        pBill->StopReason = uint16_H_to_L(3155); //欠压
    }
    else if(stChTcb.stCh.reason == OVER_VOLT)
    {
        pBill->StopReason = uint16_H_to_L(3154); //过压
    }
    else if(stChTcb.stCh.reason == OVER_CURRENT)
    {
        pBill->StopReason = uint16_H_to_L(3152); //过流
    }
    else if(stChTcb.stCh.reason == UNPLUG)
    {
        pBill->StopReason = uint16_H_to_L(3144); //非法拔枪
    }
    else if(stChTcb.stCh.reason == EM_STOP)
    {
        pBill->StopReason = uint16_H_to_L(3122);  //急停
    }
    else if(stChTcb.stCh.reason == E_END_APP_BALANC)
    {
        pBill->StopReason = uint16_H_to_L(1003);  //余额不足
    }
    else
    {
        pBill->StopReason = uint16_H_to_L(3502);  //其他都认为是小电流停止
    }
    pBill->StartChargeTime = sw64((uint64_t)stChTcb.stCh.timestart * (uint64_t)1000);  //开始充电时间
    pBill->StopChargeTime = sw64((uint64_t)stChTcb.stCh.timestop * (uint64_t)1000);		//结束充电时间
    pBill->ChargeTime = uint32split_H_to_L(cp56time(NULL) - stChTcb.stCh.timestart);	//充电时间
    pBill->TotalChargePower = uint32split_H_to_L(stChTcb.stCh.uiChargeEnergy);  //充电总电量
    pBill->ChargeFeeType = _66StartCharge.ChargeFeeType;	//充电方式
    pBill->TotalFee = uint32split_H_to_L(stChTcb.stCh.uiAllEnergy); //充电总费用
    pBill->StartSegNum =  ((stChTcb.stCh.timestart + BEIJI_TIMESTAMP_OFFSET)% CH_DAT_SEC) / CH_HALF_HOUR_SEC + 1;  //开始时间段
    num =  ((stChTcb.stCh.timestop+ BEIJI_TIMESTAMP_OFFSET) % CH_DAT_SEC) / CH_HALF_HOUR_SEC + 1;



    pBill->SegNum = num >= pBill->StartSegNum ? (num - pBill->StartSegNum + 1):((48 - pBill->StartSegNum) + num + 1);
    if(pBill->SegNum > 48)   //时间段个数大于48
    {
        return FALSE;
    }
    for(i = 0; i < pBill->SegNum; i++)
    {
        if(pBill->StartSegNum + i <= 48)
        {
            pBill->SegEnergy[i] = uint16_H_to_L(stChTcb.stCh.Curkwh[pBill->StartSegNum - 1 + i]);   //电量
        }
        else
        {
            pBill->SegEnergy[i] = uint16_H_to_L(stChTcb.stCh.Curkwh[pBill->StartSegNum + i - 48 - 1]);   //电量
        }
    }

    WriterFmBill(gun,1);
    ResendBillControl[gun].CurTime = OSTimeGet(&timeerr);;
    ResendBillControl[gun].CurTime = ResendBillControl[gun].LastTime;
    return Fream66Send(SendCount++,Send66CmdTable[CMD_SEND_ORDER].CmdCode,Send66CmdTable[CMD_SEND_ORDER].FunctionCode,Send66buf,sizeof(_66_SEND_ORDER) - 2*(48 - pBill->SegNum) );

}

/*****************************************************************************
* Function     : _66_PreBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   _66_SendBillData(uint8_t * pdata,uint8_t len)
{
    _66_SEND_ORDER * pBill = NULL;
    if((pdata == NULL) || (len < sizeof(_66_SEND_ORDER)))
    {
        return FALSE;
    }
    pBill = (_66_SEND_ORDER*)pdata;
    return Fream66Send(SendCount++,Send66CmdTable[CMD_SEND_ORDER].CmdCode,Send66CmdTable[CMD_SEND_ORDER].FunctionCode,pdata,sizeof(_66_SEND_ORDER) - 2*(48 - pBill->SegNum));
}
/*****************************************************************************
* Function     : _66_PreBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   _66_PreBill(_GUN_NUM gun,uint8_t *pdata)
{
    uint8_t i,num;
    uint8_t errcode;

    _66_SEND_ORDER *pBill	= NULL;
    if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
    {
        return FALSE;		//注册未成功，无需发送
    }

    if(gun >= GUN_MAX)
    {
        return FALSE;
    }
    pBill = (_66_SEND_ORDER *)pdata;
    memcpy(pBill->OrderNumber,_66StartCharge.OrderNumber,sizeof(_66StartCharge.OrderNumber)); //订单号


    // 停止原因
    if((stChTcb.stCh.reason == NORMAL) || (stChTcb.stCh.reason == END_CONDITION) || \
            (stChTcb.stCh.reason == NO_CURRENT) ||( stChTcb.stCh.reason == STOP_CHARGEFULL) )
    {
        pBill->StopReason = uint16_H_to_L(3501);  //充满停止
    }
    else if(stChTcb.stCh.reason == E_END_BY_APP)
    {
        pBill->StopReason = uint16_H_to_L(1001); //APP主动停止
    }
    else if(stChTcb.stCh.reason == E_END_BY_CARD)
    {
        pBill->StopReason = uint16_H_to_L(3503); //刷卡停止
    }
    else if(stChTcb.stCh.reason == UNDER_VOLT)
    {
        pBill->StopReason = uint16_H_to_L(3155); //欠压
    }
    else if(stChTcb.stCh.reason == OVER_VOLT)
    {
        pBill->StopReason = uint16_H_to_L(3154); //过压
    }
    else if(stChTcb.stCh.reason == OVER_CURRENT)
    {
        pBill->StopReason = uint16_H_to_L(3152); //过流
    }
    else if(stChTcb.stCh.reason == UNPLUG)
    {
        pBill->StopReason = uint16_H_to_L(3144); //非法拔枪
    }
    else if(stChTcb.stCh.reason == EM_STOP)
    {
        pBill->StopReason = uint16_H_to_L(3122);  //急停
    }
    else if(stChTcb.stCh.reason == E_END_APP_BALANC)
    {
        pBill->StopReason = uint16_H_to_L(1003);  //余额不足
    }
    else
    {
        pBill->StopReason = uint16_H_to_L(3502);  //其他都认为是小电流停止
    }
    pBill->StartChargeTime = sw64((uint64_t)stChTcb.stCh.timestart * (uint64_t)1000);  //开始充电时间
    pBill->StopChargeTime = sw64((uint64_t)stChTcb.stCh.timestop * (uint64_t)1000);		//结束充电时间
    pBill->ChargeTime = uint32split_H_to_L(cp56time(NULL) - stChTcb.stCh.timestart);	//充电时间
    pBill->TotalChargePower = uint32split_H_to_L(stChTcb.stCh.uiChargeEnergy);  //充电总电量
    pBill->ChargeFeeType = _66StartCharge.ChargeFeeType;	//充电方式
    pBill->TotalFee = uint32split_H_to_L(stChTcb.stCh.uiAllEnergy); //充电总费用
    pBill->StartSegNum =  ((stChTcb.stCh.timestart+ BEIJI_TIMESTAMP_OFFSET) % CH_DAT_SEC) / CH_HALF_HOUR_SEC + 1;  //开始时间段
    num =  ((stChTcb.stCh.timestop+ BEIJI_TIMESTAMP_OFFSET) % CH_DAT_SEC) / CH_HALF_HOUR_SEC + 1;
    pBill->SegNum = num >= pBill->StartSegNum ? (num - pBill->StartSegNum + 1):((48 - pBill->StartSegNum) + num + 1);
    if(pBill->SegNum > 48)   //时间段个数大于48
    {
        return FALSE;
    }
    for(i = 0; i < pBill->SegNum; i++)
    {
        if(pBill->StartSegNum + i <= 48)
        {
            pBill->SegEnergy[i] = uint16_H_to_L(stChTcb.stCh.Curkwh[pBill->StartSegNum - 1 + i]);   //电量
        }
        else
        {
            pBill->SegEnergy[i] = uint16_H_to_L(stChTcb.stCh.Curkwh[pBill->StartSegNum + i - 48 - 1]);   //电量
        }
    }
    return TRUE;
}


/*****************************************************************************
* Function     : _66_RestAck
* Description  : 软件复位应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   _66_RestAck(_GUN_NUM gun)
{
    Send66buf[0] = 1; //成功

    Fream66Send(RecvCount.ResSet,Send66CmdTable[CMD_SEND_RESET_ACK].CmdCode,Send66CmdTable[CMD_SEND_RESET_ACK].FunctionCode,Send66buf,1);
    OSTimeDly(2000, OS_OPT_TIME_PERIODIC, &timeerr);
    SystemReset();			//软件复位
    return TRUE;
}

//uint8_t SetTimeAck = 0;
//uint8_t SetQRAck = 0;

/*****************************************************************************
* Function     : _66_SetTimeAck
* Description  : 设置时间应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   _66_SetTimeAck(void)
{
//	SetTimeAck = 1;
    Send66buf[0] = 1; //成功

    return Fream66Send(RecvCount.SetTime,Send66CmdTable[CMD_SEND_SETTIME_ACK].CmdCode,Send66CmdTable[CMD_SEND_SETTIME_ACK].FunctionCode,Send66buf,1);
}


/*****************************************************************************
* Function     : _66_SetQRAck
* Description  : 设置二维码应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
uint8_t   _66_SetQRAck(_GUN_NUM gun)
{
//	SetQRAck = 1;
    Send66buf[0] = 1; //枪号
    Send66buf[1] = 1; //成功
    return Fream66Send(RecvCount.SetQ,Send66CmdTable[CMD_SEND_SETQR_ACK].CmdCode,Send66CmdTable[CMD_SEND_SETQR_ACK].FunctionCode,Send66buf,2);
}

/*****************************************************************************
* Function     : JM_SendFrameDispose
* Description  : 接收数据处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
uint8_t   JM_SendFrameDispose(void)
{
    uint8_t i;

    for(i = 0; i < SEND_66_FRAME_LEN; i++)
    {
        if(_66SendTable[i].cycletime == 0)
        {
            continue;
        }
        _66SendTable[i].curtime = OSTimeGet(&timeerr);
        if((_66SendTable[i].curtime >= _66SendTable[i].lasttime) ? ((_66SendTable[i].curtime - _66SendTable[i].lasttime) >= _66SendTable[i].cycletime) : \
                ((_66SendTable[i].curtime + (0xFFFFFFFFu - _66SendTable[i].lasttime)) >= _66SendTable[i].cycletime))
        {
            _66SendTable[i].lasttime = _66SendTable[i].curtime;
            if(_66SendTable[i].Sendfunc != NULL)
            {
                _66SendTable[i].Sendfunc();
            }
        }

    }
    return TRUE;
}

/*******************************接收函数*******************************/
/*****************************************************************************
* Function     : UDIDCountDispose
* Description  : 接收标识符处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月26日
*****************************************************************************/
static uint8_t   UDIDCountDispose(_66_RECV_CMD cmd,uint16_t count)
{
    switch(cmd)
    {
    case CMD_RECV_START:
        RecvCount.StartCharge = count;
        break;
    case CMD_RECV_POWERCTL:
        RecvCount.SetPower = count;
        break;
    case CMD_RECV_STOP:
        RecvCount.StopCharge = count;
        break;
    case CMD_RECV_RESET:
        RecvCount.ResSet = count;
        break;
    case CMD_RECV_SETTIME:
        RecvCount.SetTime = count;
        break;
    case CMD_RECV_SETQR:
        RecvCount.SetQ = count;
        break;
    default:
        break;
    }
}


uint8_t testbuf[1000];

/*****************************************************************************
* Function     : _66_RecvFrameDispose
* Description  : 接收函数处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月26日
*****************************************************************************/
uint8_t   _66_RecvFrameDispose(uint8_t * pdata,uint16_t len)
{

    //根据66协议
    uint8_t i = 0,devlen = 0;
    uint8_t CmdCode, FunctionCode,Cmd;
    uint16_t datalen;
    uint16_t count;

    if((pdata == NULL) || (len < 11) )
    {
        return FALSE;
    }
    memcpy(testbuf,pdata,len);
    //数据分帧
    while(len > 11)
    {
        if(pdata[0] != 0x68 )  //帧头帧尾判断
        {
            return FALSE;
        }
        count = pdata[5] | (pdata[4] << 8);  //接收标识符
        devlen = pdata[6];   //设备号长度
        if(devlen > 16)
        {
            return FALSE;
        }

        //提取数据长度
        datalen= pdata[8+devlen] | (pdata[7+devlen] << 8);
        if(datalen >250)
        {
            return FALSE;
        }
        CmdCode = pdata[2];			//命令码
        FunctionCode = pdata[3];		//功能码
        if(pdata[10+devlen+datalen] != 0x48)
        {
            return FALSE;
        }

        //提取数据
        if(datalen > 0)
        {
            memcpy(Recv66buf,&pdata[9+devlen],datalen);
        }
        Cmd = 0xaa;
        for(i = 0; i < CMD_RECV_MAX; i++)
        {
            if((Recv66CmdTable[i].CmdCode == CmdCode) && (Recv66CmdTable[i].FunctionCode == FunctionCode) )
            {
                Cmd = i;
                break;
            }
        }


        for(i = 0; i < CMD_RECV_MAX; i++)
        {
            if(_66_RecvTable[i].cmd == Cmd)
            {
                if(_66_RecvTable[i].recvfunction != NULL)
                {
                    UDIDCountDispose(_66_RecvTable[i].cmd,count);  //接收标识符赋值
                    _66_RecvTable[i].recvfunction(Recv66buf,datalen);
                }
                break;
            }
        }
        len =len -  datalen - devlen - 11;
        if(len > 0)
        {
            __NOP;
        }
        pdata = &pdata[datalen+devlen+11];
    }
    return TRUE;
}


/*****************************************************************************
* Function     : _66_RecvRegisterAck
* Description  : 注册应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
static uint8_t   _66_RecvRegisterAck(uint8_t *pdata,uint16_t len)
{
    _66_RECV_REGISTER * pRegister = NULL;

    if((len != sizeof(_66_RECV_REGISTER))  || (pdata ==NULL))
    {
        return FALSE;
    }
    pRegister = (_66_RECV_REGISTER *)pdata;


    if(pRegister->Result == 1)
    {
        BUZZER_ON;
        OSTimeDly(100, OS_OPT_TIME_PERIODIC, &timeerr);
        BUZZER_OFF;
        APP_SetAppRegisterState(LINK_NUM,STATE_OK);
        if(stChTcb.ucState != CHARGING)   //没在充电中读取
        {
            ReadFmBill();   //是否需要发送订单
        }
        //发送实时数据
        mq_service_send_to_4gsend(APP_SJDATA_QUERY,GUN_A ,0 ,NULL);

    }
    return TRUE;
}

/*****************************************************************************
* Function     : _66_RecvHeartAck
* Description  : 心跳应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
static uint8_t   _66_RecvHeartAck(uint8_t *pdata,uint16_t len)
{
    return TRUE;
}

/*****************************************************************************
* Function     : _66_RecvStart
* Description  : 启动
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
static uint8_t   _66_RecvStart(uint8_t *pdata,uint16_t len)
{

    uint8_t i;
    uint16_t cur,D,N; //0.1kw
    uint16_t MaxPower;
    if((len != sizeof(_66_RECV_STARTCHARGE))  || (pdata ==NULL))
    {
        return FALSE;
    }

    memcpy(&_66StartCharge,pdata,sizeof(_66_RECV_STARTCHARGE));
    \
    if(_66StartCharge.gun != 1)
    {
        return FALSE;
    }


    if((_66StartCharge.ChargeEndCondition!=1) && (_66StartCharge.ChargeEndCondition!=2) &&\
            (_66StartCharge.ChargeEndCondition!=3) && (_66StartCharge.ChargeEndCondition!=4))
    {
        return FALSE;
    }
    for(i = 0; i < 48; i++)
    {
        stChRate.SegPrices[i] = uint16_H_to_L(_66StartCharge.Price[i]) * 1000;  //电价为5位小数
    }
    //Imax = D×100×0.6    D = (666 - N) / 666
//   默认为 0，表示不限制绝对值，数据分辨率：0.1kW/位；偏移量：-1000.0kW；数据范围：-1000.0kW~ +1000.0kW（正表示充电，负是放电）
    MaxPower = uint16_H_to_L(_66StartCharge.ChargePowerMax);
    if(MaxPower <  10000)
    {
        cp_set_pwm_val(313);   //不限制的时候按照最大输出
    }
    else
    {
        if((MaxPower - 10000)  >= 70)
        {
            cp_set_pwm_val(313);   //不限制的时候按照最大输出
        }
        else
        {
            cur = MaxPower*1000/220;   //电流为小数点一位
            D=cur/6; 				//占空比
            N = 666 - (D*666/100);
            cp_set_pwm_val(N);
        }
    }


    //发送启动充电
    send_ch_ctl_msg(1,0,_66StartCharge.ChargeEndCondition,uint32split_H_to_L(_66StartCharge.ChargePara));
    ResendBillControl[GUN_A].ResendBillState = FALSE;	  //之前的订单无需发送了
    ResendBillControl[GUN_A].SendCount = 0;
    APP_SetStartNetState(GUN_A,NET_STATE_ONLINE);		//设置为在线充电
    _66_StartType =  _4G_APP_START;
//	mq_service_send_to_4gsend(APP_START_ACK,GUN_A ,0 ,NULL);

    return TRUE;
}

/*****************************************************************************
* Function     : _66_RecvSetPower
* Description  : 设置功率
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
static uint8_t   _66_RecvSetPower(uint8_t *pdata,uint16_t len)
{

    _66_RECV_POWERCTL * pSetpower = NULL;
    uint16_t power,cur,D,N; //0.1kw
    if((len != sizeof(_66_RECV_POWERCTL))  || (pdata ==NULL))
    {
        return FALSE;
    }
    pSetpower = (_66_RECV_POWERCTL *)pdata;

    if(pSetpower->gun != 1)
    {
        return FALSE;
    }

    //Imax = D×100×0.6    D = (666 - N) / 666
//   默认为 0，表示不限制绝对值，数据分辨率：0.1kW/位；偏移量：-1000.0kW；数据范围：-1000.0kW~ +1000.0kW（正表示充电，负是放电）
    power = uint16_H_to_L(pSetpower->ChargePowerMax);
    if(power <  10000)
    {
        cp_set_pwm_val(313);   //不限制的时候按照最大输出
    }
    else
    {
        if((power - 10000)  >= 70)
        {
            cp_set_pwm_val(313);   //不限制的时候按照最大输出
        }
        else
        {
            cur = power*1000/220;   //电流为小数点一位
            D=cur/6; 				//占空比
            N = 666 - (D*666/100);
            cp_set_pwm_val(N);
        }
    }

    return TRUE;
}

/*****************************************************************************
* Function     : _66_RecvStop
* Description  : 停止
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
static uint8_t   _66_RecvStop(uint8_t *pdata,uint16_t len)
{
    _66_RECV_STOP * pStop = NULL;

    if((len != sizeof(_66_RECV_STOP))  || (pdata ==NULL))
    {
        return FALSE;
    }
    pStop = (_66_RECV_STOP *)pdata;

    if(pStop->gun != 1)
    {
        return FALSE;
    }
    mq_service_send_to_4gsend(APP_STOP_ACK,GUN_A ,0 ,NULL);
    send_ch_ctl_msg(2,0,4,0);
    return TRUE;
}

/*****************************************************************************
* Function     : _66_RecvBillAck
* Description  : 订单应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
static uint8_t   _66_RecvBillAck(uint8_t *pdata,uint16_t len)
{
    _66_RECV_ORDER_ACK * pBill = NULL;

    if((len != sizeof(_66_RECV_ORDER_ACK))  || (pdata ==NULL))
    {
        return FALSE;
    }
    pBill = (_66_RECV_ORDER_ACK *)pdata;

    if(pBill->TradeInfoFlag == 1)
    {
        ResendBillControl[GUN_A].ResendBillState = FALSE;			//订单确认，不需要重发订单
        ResendBillControl[GUN_A].SendCount = 0;
        WriterFmBill(GUN_A,0);
    }
    return TRUE;
}

/*****************************************************************************
* Function     : _66_RecvBillQ
* Description  :查询订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
static uint8_t   _66_RecvBillQ(uint8_t *pdata,uint16_t len)
{
    return TRUE;
}

/*****************************************************************************
* Function     : _66_RecvReset
* Description  :复位
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
static uint8_t   _66_RecvReset(uint8_t *pdata,uint16_t len)
{
    SystemReset();
    return TRUE;
}

/*****************************************************************************
* Function     : _66_RecvSetTime
* Description  :设置时间
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
static uint8_t   _66_RecvSetTime(uint8_t *pdata,uint16_t len)
{
//	    uint16_t  usMsec;
//    uint8_t   ucMin : 6;
//    uint8_t   ucRes0: 1;
//    uint8_t   ucIV  : 1;
//    uint8_t   ucHour:5;
//    uint8_t   ucRes1:2;
//    uint8_t   ucSU  :1;
//    uint8_t   ucDay: 5;
//    uint8_t   ucWeek:3;
//    uint8_t   ucMon: 4;
//    uint8_t   ucRes2:4;
//    uint8_t   ucYear:7;
//    uint8_t   ucRes3:1;
    static uint32_t  time;
    _66_RECV_TIME * pTime = NULL;
    static CP56TIME2A_T cp56time;
    if((len != sizeof(_66_RECV_TIME))  || (pdata ==NULL))
    {
        return FALSE;
    }
    pTime = (_66_RECV_TIME*)pdata;
    time = sw64(pTime->Time) /1000;

    localtime_to_cp56time(time,&cp56time);

    set_date(cp56time.ucYear + 2000,cp56time.ucMon&0x0f,cp56time.ucDay&0x1f);
    set_time(cp56time.ucHour,cp56time.ucMin,cp56time.usMsec/1000);
    mq_service_send_to_4gsend(APP_STE_TIME,GUN_A ,0 ,NULL);
    return TRUE;
}

/*****************************************************************************
* Function     : _66_RecvSetQR
* Description  :设置二维码
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
static uint8_t   _66_RecvSetQR(uint8_t *pdata,uint16_t len)
{
    if((len != sizeof(_66_RECV_SETQR))  || (pdata ==NULL))
    {
        return FALSE;
    }
    if(pdata[0] != 1)
    {
        return FALSE;
    }
    memcpy(&_66_QR,pdata,sizeof(_66_RECV_SETQR));
    mq_service_send_to_4gsend(APP_SETQR_ACK,GUN_A ,0 ,NULL);
    return TRUE;
}

/*****************************************************************************
* Function     : _66_RecvOTA
* Description  :远程升级
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年10月25日   完成
*****************************************************************************/
static uint8_t   _66_RecvOTA(uint8_t *pdata,uint16_t len)
{
    _66_RECV_UPDATA * pupdata;
    if((len != sizeof(_66_RECV_UPDATA))  || (pdata ==NULL))
    {
        return FALSE;
    }
    pupdata  = (_66_RECV_UPDATA*)pdata;
    memcpy(HttpInfo.ServerAdd,pupdata->URL,MIN(sizeof(HttpInfo.ServerAdd),sizeof(pupdata->URL)));

    APP_SetSIM7600Mode(MODE_HTTP);
    return TRUE;
}

/************************(C)COPYRIGHT 2020*****END OF FILE****************************/

