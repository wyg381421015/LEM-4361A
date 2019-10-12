#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <rtthread.h>
#include <rtdevice.h>

/***********能源路由器参数配置结构体******************************/
typedef struct
{
	char cAssetNum[23];//路由器资产编号 字符串 maxlen=22
	
}ScmRouterPata_Msg;//路由器参数
extern ScmRouterPata_Msg  stRouterPata_msg;
/****************************************************************/

typedef struct
{
	char cPlieAddr[17];//充电桩编号  字符串 maxlen=16
	unsigned char ucInterfaceNum;//充电接口标识
	char cInstallation[40];//充电桩安装地址  maxlen = 40
	unsigned long ulMinimunPow;//最小充电功率
	unsigned long ulRatePow;//额定功率
	unsigned char ucWake;//是否支持唤醒
}ScmChargePilePara;//充电桩参数




typedef enum {
	Cmd_MeterNumWr=0,                   //0
	Cmd_MeterNumRd,
	Cmd_MeterPowerWr,                   //0
	Cmd_MeterPowerRd,
	Cmd_MeterGJFModeWr,                 //0
	Cmd_MeterGJFModeRd,	
	Cmd_MeterHalfPowerWr,               //0
	Cmd_MeterHalfPowerRd,
	Cmd_MeterAnalogWr,                  //0
	Cmd_MeterAnalogRd,
	Cmd_HistoryRecordWr,                /*充电订单事件记录单元*/
	Cmd_HistoryRecordRd,	
	Cmd_OrderChargeWr,                  /*有序充电事件记录单元*/
	Cmd_OrderChargeRd,		
	Cmd_PlanOfferWr,                    /*充电计划上报记录单元*/
	Cmd_PlanOfferRd,
	Cmd_PlanFailWr,                     /*充电计划生成失败记录单元*/
	Cmd_PlanFailRd,	
	Cmd_OnlineStateWr,                  /*表计在线状态*/
	Cmd_OnlineStateRd,		
	
	
	
	End_Sto_cmdListNum,
}STORAGE_CMD_ENUM;
#define STORAGE_CMD_ENUM rt_uint32_t


extern int GetStorageData(STORAGE_CMD_ENUM cmd,void *STO_GetPara,rt_uint32_t datalen);
extern int SetStorageData(STORAGE_CMD_ENUM cmd,void *STO_SetPara,rt_uint32_t datalen);


#endif

