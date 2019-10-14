#ifndef __STRATEGY_H__
#define __STRATEGY_H__

#include <string.h>
#include <stdio.h>
#include "global.h"
#include "chargepile.h"

extern ChargPilePara_TypeDef ChargePilePara;
extern rt_err_t DevState_Judge(void);

extern struct rt_semaphore rt_sem_bluetoothfau;

extern rt_uint8_t DeviceState;
extern rt_uint8_t ChgpileState;
extern rt_bool_t DeviceFauFlag;



/******************************** 与控制器交互信息 ***********************************/
typedef enum
{
	GUN_A=1,
	GUN_B,
}GUN_NUM;/*枪序号 {A枪（1）、B枪（2）}*/

typedef enum
{
	DISORDER=0,
	ORDER,
}CHARGE_MODE;/*充电模式 {正常（0），有序（1）}*/

typedef enum
{
	EXE_ING=1,
	EXE_END,
	EXE_FAILED,
}EXESTATE;/*执行状态 {1：正常执行 2：执行结束 3：执行失败}*/

typedef enum
{
	STANDBY=1,
	CHARGING,
	FAULT,
}PILESTATE;/*充电桩状态（1：待机 2：工作 3：故障）*/

typedef enum
{
	SEV_ENABLE=0,
	SEV_DISABLE,
}PILE_SERVICE;/*桩充电服务 {可用（0），停用（1）}*/

typedef enum
{
	CONNECT=0,
	DISCONNECT,
}PILE_COM_STATE;/*与桩通信状态 {正常（0），异常（1）}*/

typedef enum
{
	CTRL_START=1,
	CTRL_STOP,
	CTRL_ADJPOW,
}CTRL_TYPE;/*{1：启动  2：停止  3：调整功率}*/



/******************************* 启停充电 *************************************/
			
typedef struct
{
	char cAssetNO[23];		//路由器资产编号  visible-string（SIZE(22)）
	GUN_NUM gunAB;		//充电功率设定值（单位：kW，换算：-4）
}StartStopChg;
/******************************* 充电计划 *************************************/
typedef struct
{
	STR_SYSTEM_TIME strDecStartTime;//起始时间
	STR_SYSTEM_TIME strDecStopTime;	//结束时间
	unsigned long ulChargePow;		//充电功率设定值（单位：kW，换算：-4）
}CHARGE_TIMESOLT;/*时段负荷单元*/

typedef struct
{
	char cRequestNO[17];			//申请单号  octet-string（SIZE(16)）
	char cUserID[65];   			//用户id  visible-string（SIZE(64)）
	unsigned char ucDecMaker;		//决策者  {主站（1）、控制器（2）}
	unsigned char ucDecType; 		//决策类型{生成（1） 、调整（2）}
	STR_SYSTEM_TIME strDecTime;		//决策时间
	char cAssetNO[23];				//路由器资产编号  visible-string（SIZE(22)）
	unsigned char GunNum;			//枪序号	{A枪（1）、B枪（2）}
	unsigned long ulChargeReqEle;	//充电需求电量（单位：kWh，换算：-2）
	unsigned long ulChargeRatePow;	//充电额定功率 （单位：kW，换算：-4）
	unsigned char ucChargeMode;		//充电模式 {正常（0），有序（1）}
	unsigned char ucTimeSlotNum;	//时间段数量
	CHARGE_TIMESOLT strChargeTimeSolts[50];//时间段内容，最大50段

}CHARGE_STRATEGY;/*充电计划单*/
extern CHARGE_STRATEGY Chg_Strategy;//下发计划单
extern CHARGE_STRATEGY Adj_Chg_Strategy;//变更计划单

typedef struct
{
	char cRequestNO[17];	//申请单号  octet-string（SIZE(16)）
	char cAssetNO[23];		//路由器资产编号  visible-string（SIZE(22)）
	unsigned char cSucIdle;	//成功或失败原因:{0：成功 1：失败 255：其他}
}CHARGE_STRATEGY_RSP;/*充电计划单响应*/

typedef struct
{
	char cRequestNO[17];			//申请单号  octet-string（SIZE(16)）
	char cAssetNO[23];				//路由器资产编号  visible-string（SIZE(22)）
	unsigned char exeState;			//执行状态 {1：正常执行 2：执行结束 3：执行失败}
	unsigned char ucTimeSlotNum;	//时间段数量
										//尖峰平谷
	unsigned long ulEleBottomValue[5]; 	//电能示值底值（充电首次执行时示值）（单位：kWh，换算：-2）
	unsigned long ulEleActualValue[5]; 	//当前电能示值（单位：kWh，换算：-2）
	unsigned long ucChargeEle[5];		//已充电量（单位：kWh，换算：-2）
	unsigned long ucChargeTime;		//已充时间（单位：s）
	unsigned long ucPlanPower;		//计划充电功率（单位：W，换算：-1）
	unsigned long ucActualPower;	//当前充电功率（单位：W，换算：-1）
	unsigned short ucVoltage;		//当前充电电压（单位：V，换算：-1）
	unsigned int ucCurrent;			//当前充电电流（单位：A，换算：-3）
	unsigned char ChgPileState;		//充电桩状态（1：待机 2：工作 3：故障）
}CHARGE_EXE_STATE;/*路由器工作状态  即 充电计划单执行状态*/
extern CHARGE_EXE_STATE Chg_ExeState;

typedef struct
{
	char cRequestNO[17];			//申请单号  octet-string（SIZE(16)）
	char cUserID[65];   			//用户id  visible-string（SIZE(64)）
	char cAssetNO[23];				//路由器资产编号  visible-string（SIZE(22)）
	unsigned char GunNum;			//枪序号	{A枪（1）、B枪（2）}
	unsigned long ulChargeReqEle;	//充电需求电量（单位：kWh，换算：-2）
	STR_SYSTEM_TIME	PlanUnChg_TimeStamp;//	计划用车时间
	unsigned char ChargeMode;			//	充电模式 {正常（0），有序（1）}
	char Token[33];   					//	用户登录令牌  visible-string（SIZE(32)）
}CHARGE_APPLY;/*充电申请单(BLE)*/

/******************************* 充电控制 *************************************/
typedef struct
{
	char OrderSn[17];			//订单号  octet-string（SIZE(16)）
	unsigned char CtrlType;		//控制类型{1：启动  2：停止  3：调整功率}
	unsigned char StartType;	//启动类型{1：4G启动  2:蓝牙启动}
	unsigned char StopType;		//停机类型{1：4G停机  2:蓝牙停机}
	unsigned long SetPower;		//设定充电功率（单位：W，换算：-1）
	unsigned char cSucIdle;		//成功或失败原因:{0：成功 1：失败 255：其他}
}CTL_CHARGE;/*控制器充电控制*/



/******************************** 事件信息记录 ***********************************/
typedef struct
{
	unsigned long OrderNum;			//记录序号
	STR_SYSTEM_TIME OnlineTimestamp;		//上线时间
	STR_SYSTEM_TIME OfflineTimestamp;		//离线时间
	unsigned char ChannelState;				//通道状态
	unsigned char AutualState;				//状态变化 {上线（0）， 离线（1）}
	unsigned char OfflineReason;			//离线原因 {未知（0），停电（1），信道变化（2）}
}ONLINE_STATE;/*表计在线状态*/

typedef struct
{
	unsigned long OrderNum;			//	事件记录序号 
	STR_SYSTEM_TIME StartTimestamp;	//  事件发生时间  
	STR_SYSTEM_TIME FinishTimestamp;//  事件结束时间  
	unsigned char Reason;			//  事件发生原因     
	unsigned char ChannelState;		//  事件上报状态 = 通道上报状态
	char RequestNO[17];				//	充电申请单号   （SIZE(16)）
	char AssetNO[23];				//	路由器资产编号 visible-string（SIZE(22)）
	char Data[33];					//  第n个关联对象属性的数据 
}PLAN_FAIL_EVENT;/*充电计划生成失败记录单元*/

typedef struct
{
	unsigned long OrderNum;				//	事件记录序号 
	STR_SYSTEM_TIME StartTimestamp;		//  事件发生时间  
	STR_SYSTEM_TIME FinishTimestamp;	//  事件结束时间  
	unsigned char Reason;				//  事件发生原因     
	unsigned char ChannelState;			//  事件上报状态 = 通道上报状态
	char RequestNO[17];					//	充电申请单号   （SIZE(16)）
	char cUserID[65];   				//	用户id  visible-string（SIZE(64)）
	char AssetNO[23];					//	路由器资产编号 visible-string（SIZE(22)）
	unsigned long ChargeReqEle;			//	充电需求电量（单位：kWh，换算：-2）
	STR_SYSTEM_TIME RequestTimeStamp;	//	充电申请时间
	STR_SYSTEM_TIME	PlanUnChg_TimeStamp;//	计划用车时间
	unsigned char ChargeMode;			//	充电模式 {正常（0），有序（1）}
	char Token[39];   					//	用户登录令牌  visible-string（SIZE(38)）
	char UserAccount[10];				//  充电用户账号  visible-string（SIZE(9)） 
	char Data[33];						//  第n个关联对象属性的数据
}PLAN_OFFER_EVENT;/*充电计划上报记录单元*/

typedef struct
{
	unsigned long OrderNum;				//	事件记录序号 
	STR_SYSTEM_TIME StartTimestamp;		//  事件发生时间  
	STR_SYSTEM_TIME FinishTimestamp;	//  事件结束时间  
	unsigned char Reason;				//  事件发生原因     
	unsigned char ChannelState;			//  事件上报状态 = 通道上报状态
	unsigned int TotalFault;			//	总故障
	char Fau[32];						//	各项故障状态
}ORDER_CHG_EVENT;/*有序充电事件记录单元*/

typedef struct
{
	unsigned long OrderNum;				//	事件记录序号 
	STR_SYSTEM_TIME StartTimestamp;		//  事件发生时间  
	STR_SYSTEM_TIME FinishTimestamp;	//  事件结束时间  
	unsigned char Reason;				//  事件发生原因     
	unsigned char ChannelState;			//  事件上报状态 = 通道上报状态
	char RequestNO[17];					//	充电申请单号   （SIZE(16)）
	char cUserID[65];   				//	用户id  visible-string（SIZE(64)）
	char AssetNO[23];					//	路由器资产编号 visible-string（SIZE(22)）
	unsigned long ChargeReqEle;			//	充电需求电量（单位：kWh，换算：-2）
	STR_SYSTEM_TIME RequestTimeStamp;	//	充电申请时间
	STR_SYSTEM_TIME	PlanUnChg_TimeStamp;//	计划用车时间
	unsigned char ChargeMode;			//	充电模式 {正常（0），有序（1）}
	unsigned long StartMeterValue;		//	启动时电表表底值
	unsigned long StopMeterValue;		//	停止时电表表底值
	STR_SYSTEM_TIME	ChgStartTime;		//	充电启动时间
	STR_SYSTEM_TIME ChgStopTime;		//	充电停止时间
	unsigned long ucChargeEle;			//	已充电量（单位：kWh，换算：-2）
	unsigned long ucChargeTime;			//	已充时间（单位：s）
}CHG_ORDER_EVENT;/*充电订单事件记录单元*/

#endif

