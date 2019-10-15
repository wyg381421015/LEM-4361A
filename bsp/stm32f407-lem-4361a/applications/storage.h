#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <rtthread.h>
#include <rtdevice.h>


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

