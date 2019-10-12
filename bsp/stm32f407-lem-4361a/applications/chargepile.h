#ifndef __CHARGEPILE_H__
#define __CHARGEPILE_H__
#include <meter.h>

/* 充电控制-事件控制块 */
extern struct rt_event ChargePileEvent;
/****************宏定义**********************************/
//定义事件类型
typedef enum {
	NO_EVENT                  =0x00000000,
	ChargeStartOK_EVENT       =0x00000001,        //启动成功事件		      
	ChargeStartER_EVENT       =0x00000002,        //启动失败事件			      
	ChargeStopOK_EVENT        =0x00000004,        //停止成功事件		
	ChargeStopER_EVENT        =0x00000008,        //停止失败事件	
	SetPowerOK_EVENT		  =0x00000010,		  //功率下发成功事件
	SetPowerER_EVENT		  =0x00000020,		  //功率下发失败事件	
	PileComFau_EVENT      	  =0x00000040,        //通信中断事件
				
} PILE_EVENT_TYPE;
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
typedef struct
{
	ScmMeter_Analog PileMeter;
	uint32_t  PWM_Duty;        // 占空比 1位小数
	uint32_t StartReson;
	uint32_t StopReson;

}ChargPilePara_TypeDef;

typedef enum {
	Cmd_ChargeStart=0,                  //0
	Cmd_ChargeStartResp,
	Cmd_ChargeStop,                     //
	Cmd_ChargeStopResp,
	Cmd_SetPower,                       //
	Cmd_GetPower,
	Cmd_SetPowerResp, 
	Cmd_RdVertion,                      //
	Cmd_RdVoltCurrPara,                 //
	
	End_cmdListNum,
}COMM_CMD_P;
#define COMM_CMD_P rt_uint32_t





extern rt_uint8_t ChargepileDataGetSet(COMM_CMD_P cmd,void *STR_SetPara);



















#endif

