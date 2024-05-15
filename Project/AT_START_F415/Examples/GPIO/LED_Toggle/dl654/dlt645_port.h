#ifndef __DLT645_PORT_H
#define __DLT645_PORT_H
#include "common.h"

typedef struct
{
	float out_cur;		//输出电流	0.001
	float out_vol;		//输出电压	0.1
	float out_power;		//输出功率	0.0001
	float cur_hwh;		//当前电表读数 0.01
}_dlt645_info;	

typedef struct
{
	uint16_t 	charge_kwh;		//充电电量 0.01
	uint16_t  	charge_money;		//充电金额 0.01
	uint16_t 	charge_time;		//充电时间分钟
	uint16_t    cur_time_pric;		//当前时间段费率
	uint8_t		ifbill;				//是否开始计费         0 表示无需计费  1开始开始计费（当状态为充电中，则开始计费）
	uint8_t     last_ifbill;			//上一次是否开始计费   0 表示无需计费  1开始开始计费（当状态为充电中，则开始计费）
	uint8_t    id_kwh_change;			//电能读数是否发送改变
}_charge_bill_info;
//对外提供环境声明
//645采集硬件层初始化
int dlt645_port_init(void);
extern _dlt645_info dlt645_info;				//电表信息
#endif
