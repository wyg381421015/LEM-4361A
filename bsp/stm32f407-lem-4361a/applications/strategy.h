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
/******************************** 路由器相关信息 ***********************************/	//zcx190710
typedef enum {
	DevSt_StandbyOK=0,          // 待机正常
	DevSt_InCharging,           // 充电中
	DevSt_DisCharging,          // 放电中
	DevSt_Fault,           		// 故障
	DevSt_Update,				// 升级中
}DEVICE_WORKSTATE;/*路由器状态*/

typedef struct
{
	char UserID[65];   			//用户id  visible-string（SIZE(64)）
	char Token[39];   			//用户登录令牌  visible-string（SIZE(38)）
	unsigned char AccountState;	//账户状态 {0：正常，1：欠费}
}WHITE_LIST;/*路由器白名单*/

typedef struct
{
	char kAddress[17];   		//通讯地址	visible-string（SIZE(16)）
	char MeterNum[9];   		//表号  visible-string（SIZE(8)）
	char KeyNum[9];				//密钥信息	visible-string（SIZE(8)）	
}KEYINFO_UNIT;/*路由器密钥信息单元*/

/******************************** 充电桩相关信息 ***********************************/	//zcx190807
typedef enum {
	ChgSt_Standby=0,            //正常待机
	ChgSt_InCharging,           //充电中
	ChgSt_Fault,            	//故障
}CHGPILE_WORKSTATE;/*充电桩状态*/

/******************************** 故障信息 ***********************************/		//zcx190710
typedef enum 
{
//	NO_FAULT			=0x00000000,
//	Memory_FAULT		=0x00000001,		//	终端主板内存故障（0）
//	Clock_FAULT			=0x00000002,		//  时钟故障        （1）
//	Board_FAULT			=0x00000004,		//  主板通信故障    （2）
//	MeterCom_FAULT		=0x00000008,		//  485抄表故障     （3）
//	Screen_FAULT		=0x00000010,		//  显示板故障      （4）
//	CarrierChl_FAULT	=0x00000020,		//  载波通道异常    （5）
//	NandF_FAULT			=0x00000040,		//	NandFLASH初始化错误  （6）
//	ESAM_FAULT			=0x00000080,		//  ESAM错误        （7）
//	Bluetooth_FAULT		=0x00000100,		//	蓝牙模块故障     （8）
//	Battery_FAULT		=0x00000200,		//	电源模块故障 	（9）
//	CanCom_FAULT		=0x00000400,		//  充电桩通信故障 （10）
//	ChgPile_FAULT		=0x00000800,		//  充电桩设备故障 （11）
//	OrdRecord_FAULT		=0x00001000, 		//	本地订单记录满 （12）
	NO_FAULT=0,
	Memory_FAULT,		//	终端主板内存故障（0）
	Clock_FAULT,		//  时钟故障        （1）
	Board_FAULT,		//  主板通信故障    （2）
	MeterCom_FAULT,		//  485抄表故障     （3）
	Screen_FAULT,		//  显示板故障      （4）
	CarrierChl_FAULT,	//  载波通道异常    （5）
	NandF_FAULT,		//	NandFLASH初始化错误  （6）
	ESAM_FAULT,			//  ESAM错误        （7）
	Bluetooth_FAULT,	//	蓝牙模块故障     （8）
	Battery_FAULT,		//	电源模块故障 	（9）
	CanCom_FAULT,		//  充电桩通信故障 （10）
	ChgPile_FAULT,		//  充电桩设备故障 （11）
	OrdRecord_FAULT, 	//	本地订单记录满 （12）
}DEVICE_FAULT;/*设备故障类型*/


typedef enum 
{
	PILE_NOFAULT        =0x00000000,
	PILE_Memory_FAULT	=0x00000001,	//	终端主板内存故障（0）
	PILE_Clock_FAULT	=0x00000002,	//  时钟故障        （1）
	PILE_Board_FAULT	=0x00000004,	//  主板通信故障    （2）
	PILE_MeterCom_FAULT	=0x00000008,	//  485抄表故障    （3）
	PILE_Screen_FAULT	=0x00000010,	//  显示板故障      （4）
	CardOffLine_FAULT	=0x00000020,	//	读卡器通讯中断  （5）
	PILE_ESAM_FAULT		=0x00000040,	//  ESAM错误        （6）
	StopEct_FAULT		=0x00000080,	//  急停按钮动作故障（7）
	Arrester_FAULT		=0x00000100,	//	避雷器故障		（8）
	GunHoming_FAULT		=0x00000200,	//	充电枪未归位		（9）
	OverV_FAULT			=0x00000400,	//	输入过压告警		（10）
	UnderV_FAULT		=0x00000800,	//	输入欠压告警		（11）
	Pilot_FAULT			=0x00001000,	//	充电中车辆控制导引告警（12）
	Connect_FAULT		=0x00002000,	//	交流接触器故障	（13）
	OverI_Warning		=0x00004000,	//	输出过流告警		（14）
	OverI_FAULT			=0x00008000,	//	输出过流保护动作	（15）
	ACCir_FAULT			=0x00010000,	//	交流断路器故障	（16）
	GunLock_FAULT		=0x00020000,	//	充电接口电子锁故障（17）
	GunOverTemp_FAULT	=0x00040000,	//	充电接口过温故障	（18）
	CC_FAULT			=0x00080000,	//	充电连接状态CC异常（19）
	CP_FAULT			=0x00100000,	//	充电控制状态CP异常（20）
	PE_FAULT			=0x00200000,	//	PE断线故障		（21）
	Dooropen_FAULT		=0x00400000,	//	柜门打开故障		(22)
	Other_FAULT			=0x00800000,	//	充电机其他故障	（23）
}PILE_FAULT;/*充电桩故障类型*/


/******************************** 与控制器交互信息 ***********************************/
typedef struct
{
	char OrderSn[17];	//订单号  octet-string（SIZE(16)）
	unsigned char StartType;
	unsigned char StopType;
	unsigned char cSucIdle;	//成功或失败原因:{0：成功1：失败255：其他}
}CTL_CHARGE;/*控制器充电控制*/



typedef struct
{
	STR_SYSTEM_TIME strDecStartTime;//起始时间
	STR_SYSTEM_TIME strDecStopTime;	//结束时间
//	unsigned char PowDensity;		//功率密度（功率调节的颗粒度，例如15分钟，5分钟，1分钟等）
	unsigned long ulChargePow;		//充电功率设定值
}CHARGE_TIMESOLT;/*时段负荷单元*/

typedef struct
{
	char cRequestNO[17];			//申请单号  octet-string（SIZE(16)）
	char cUserID[65];   			//用户id  visible-string（SIZE(64)）
	unsigned char ucDecMaker;		//决策者  {主站（1）、控制器（2）}
	unsigned char ucDecType; 		//决策类型{生成（1） 、调整（2）}
	STR_SYSTEM_TIME strDecTime;		//决策时间
	char cAssetNO[23];				//路由器资产编号  visible-string（SIZE(22)）
	unsigned long ulChargeReqEle;	//充电需求电量（单位：kWh，换算：-2）
	unsigned long ulChargeRatePow;	//充电额定功率 （单位：kW，换算：-4）
	unsigned char ucChargeMode;		//充电模式 {正常（0），有序（1）}
	unsigned char ucTimeSlotNum;	//时间段数量
	CHARGE_TIMESOLT strChargeTimeSolts[50];//时间段内容，最大50段

}CHARGE_STRATEGY;/*充电计划单*/
extern CHARGE_STRATEGY Chg_Strategy;
extern CHARGE_STRATEGY Adj_Chg_Strategy;

typedef struct
{
	char cRequestNO[17];	//申请单号  octet-string（SIZE(16)）
	char cAssetNO[23];		//路由器资产编号  visible-string（SIZE(22)）
	unsigned char cSucIdle;	//成功或失败原因:{0：成功1：失败255：其他}
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
}CHARGE_EXE_STATE;/*充电计划单执行状态*/
extern CHARGE_EXE_STATE Chg_ExeState;


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

