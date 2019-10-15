#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include <stdio.h>
#include <global.h>
#include "energycon.h"
#include "chargepile.h"
#include "strategy.h"
#include "698.h"
#include "meter.h"
#include "storage.h"
#include <board.h>


#define THREAD_ENERGYCON_PRIORITY     18
#define THREAD_ENERGYCON_STACK_SIZE   1024
#define THREAD_ENERGYCON_TIMESLICE    20

#define RELAYA_PIN    GET_PIN(F, 2)
#define RELAYB_PIN    GET_PIN(F, 3)

#ifndef FALSE
#define FALSE         0
#endif
#ifndef TRUE
#define TRUE          1
#endif
#ifndef SUCCESSFUL
#define SUCCESSFUL    0
#endif
#ifndef FAILED
#define FAILED        1
#endif
#ifndef ORTHERS
#define ORTHERS       255
#endif

static rt_uint8_t energycon_stack[THREAD_ENERGYCON_STACK_SIZE];//线程堆栈

struct rt_thread energycon;
struct rt_semaphore rx_sem_energycon;     //用于接收数据的信号量

CTL_CHARGE Ctrl_Start;
CTL_CHARGE Ctrl_Stop;
CTL_CHARGE Ctrl_PowerAdj;
CTL_CHARGE_EVENT CtrlCharge_Event;

//指令标志
static rt_bool_t startchg_flag;
static rt_bool_t stopchg_flag;
static rt_bool_t adjpower_flag;

//超时结果
static rt_timer_t StartChgResp;
static rt_timer_t StopChgResp;
static rt_timer_t PowerAdjResp;

static unsigned char count;
static unsigned char SetPowerFinishFlag[50];
static char cRequestNO_Old[17];
static char cRequestNO_New[17];



void RELAY_ON(void)//吸合继电器
{
	rt_pin_write(RELAYA_PIN, PIN_LOW);
	rt_pin_write(RELAYB_PIN, PIN_HIGH);
}

void RELAY_OFF(void)//断开继电器
{
	rt_pin_write(RELAYB_PIN, PIN_LOW);
	rt_pin_write(RELAYA_PIN, PIN_HIGH);
}
/**************************************************************
 * 函数名称: StartChgResp_Timeout 
 * 参    数: 
 * 返 回 值: 
 * 描    述: 启动充电超时函数
 **************************************************************/
static void StartChgResp_Timeout(void *parameter)
{
    rt_lprintf("[strategy] : StartChgResp event is timeout!\n");
	ChargepileDataGetSet(Cmd_ChargeStartResp,0);
}
/**************************************************************
 * 函数名称: StopChgResp_Timeout 
 * 参    数: 
 * 返 回 值: 
 * 描    述: 停止充电超时函数
 **************************************************************/
static void StopChgResp_Timeout(void *parameter)
{
    rt_lprintf("[strategy] : StopChgResp event is timeout!\n");
	ChargepileDataGetSet(Cmd_ChargeStopResp,0);
}
/**************************************************************
 * 函数名称: PowAdjResp_Timeout 
 * 参    数: 
 * 返 回 值: 
 * 描    述: 调整功率超时函数
 **************************************************************/
static void PowAdjResp_Timeout(void *parameter)
{
    rt_lprintf("[strategy] : PowerAdjResp event is timeout!\n");
	ChargepileDataGetSet(Cmd_SetPowerResp,0);
}
/**************************************************************
 * 函数名称: timer_create_init 
 * 参    数: 
 * 返 回 值: 
 * 描    述: 定时器
 **************************************************************/
static void timer_create_init()
{
    /* 创建启机回复定时器 */
	 StartChgResp = rt_timer_create("StartChgResp",  /* 定时器名字是 StartChgResp */
									StartChgResp_Timeout, /* 超时时回调的处理函数 */
									RT_NULL, /* 超时函数的入口参数 */
									5000, /* 定时长度，以OS Tick为单位，即5000个OS Tick */
									RT_TIMER_FLAG_ONE_SHOT); /* 一次性定时器 */
	/* 创建停机回复定时器 */
	 StopChgResp = rt_timer_create("StopChgResp",  /* 定时器名字是 StopChgResp */
									StopChgResp_Timeout, /* 超时时回调的处理函数 */
									RT_NULL, /* 超时函数的入口参数 */
									5000, /* 定时长度，以OS Tick为单位，即5000个OS Tick */
									RT_TIMER_FLAG_ONE_SHOT); /* 一次性定时器 */
	/* 创建调整功率回复定时器 */
	 PowerAdjResp = rt_timer_create("PowerAdjResp",  /* 定时器名字是 PowerAdjResp */
									PowAdjResp_Timeout, /* 超时时回调的处理函数 */
									RT_NULL, /* 超时函数的入口参数 */
									5000, /* 定时长度，以OS Tick为单位，即5000个OS Tick */
									RT_TIMER_FLAG_ONE_SHOT); /* 一次性定时器 */
}
/*  */

/********************************************************************  
*	函 数 名: CtrlData_RecProcess()
*	功能说明: 控制类数据接收处理函数
*	形    参: 无
*	返 回 值: 无
********************************************************************/ 
static void CtrlData_RecProcess(void)
{
	rt_uint8_t c_rst;
	rt_uint32_t chgplanIssue,chgplanIssueAdj,startchg,stopchg;
	rt_uint32_t EventCmd;
	EventCmd = strategy_event_get();
	
	switch(EventCmd)
	{	
		//收到启动充电命令
		case StartChg_EVENT:
		{
			startchg_flag = TRUE;
			c_rst = CtrlUnit_RecResp(Cmd_StartChg,&Ctrl_Start,0);//取值
			rt_lprintf("[strategy]  (%s)  收到启动充电命令  \n",__func__);
			memcpy(&CtrlCharge_Event,&Ctrl_Start,41);
			CtrlCharge_Event.CtrlType = CTRL_START;
			
			if(Fault.Total != TRUE)
			{
				if(memcmp(&RouterIfo.AssetNum,&Ctrl_Start.cAssetNO,22) == 0)//校验资产一致性
				{
					ChargepileDataGetSet(Cmd_ChargeStart,0);	
				
					/* 开始启动回复计时 */
					if (StartChgResp != RT_NULL)
						rt_timer_start(StartChgResp);
					else
						rt_lprintf("StartChgResp timer create error\n");							
				}
				else
				{
					Ctrl_Start.cSucIdle = ORTHERS;
					c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Ctrl_Start,0);//回复
				}			
			}
			else
			{
				Ctrl_Start.cSucIdle = FAILED;	
				c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Ctrl_Start,0);//回复			
			}		
			CtrlCharge_Event.cSucIdle = Ctrl_Start.cSucIdle;			
			break;
		}
		//收到停止充电命令
		case StopChg_EVENT:
		{
			stopchg_flag = TRUE;	
			c_rst = CtrlUnit_RecResp(Cmd_StopChg,&Ctrl_Stop,0);//取值			
			rt_lprintf("[strategy]  (%s)  收到停止充电命令  \n",__func__); 
			memcpy(&CtrlCharge_Event,&Ctrl_Stop,41);			
			CtrlCharge_Event.CtrlType = CTRL_STOP;
			
			if(Fault.Total != TRUE)
			{
				if((memcmp(&RouterIfo.AssetNum,&Ctrl_Stop.cAssetNO,22) == 0)//校验资产一致性
					||(memcmp(&Ctrl_Start.OrderSn,&Ctrl_Stop.OrderSn,16) == 0))//校验启停单号一致性		
				{
					ChargepileDataGetSet(Cmd_ChargeStop,0);	
				
					/* 开始停机回复计时 */
					if (StopChgResp != RT_NULL)
						rt_timer_start(StopChgResp);
					else
						rt_lprintf("StopChgResp timer create error\n");
				}
				else
				{
					Ctrl_Stop.cSucIdle = ORTHERS;
					c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&Ctrl_Stop,0);//回复
				}
			}
			else
			{
				Ctrl_Stop.cSucIdle = FAILED;
				c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&Ctrl_Stop,0);//回复
			}
			CtrlCharge_Event.cSucIdle = Ctrl_Stop.cSucIdle;
			break;
		}
		//收到调整功率命令
		case PowerAdj_EVENT:
		{
			adjpower_flag = TRUE;
			c_rst = CtrlUnit_RecResp(Cmd_PowerAdj,&Ctrl_PowerAdj,0);//取值	
			rt_lprintf("[strategy]  (%s)  收到调整功率命令  \n",__func__);  
			memcpy(&CtrlCharge_Event,&Ctrl_PowerAdj,41);
			CtrlCharge_Event.CtrlType = CTRL_ADJPOW;
			
			if(Fault.Total != TRUE)
			{
				if(memcmp(&RouterIfo.AssetNum,&Ctrl_PowerAdj.cAssetNO,22) == 0)//校验资产一致性
				{
					ChargePilePara_Set.PWM_Duty = Ctrl_PowerAdj.SetPower*10/132;//功率换算: D(含一位小数)=I/60*1000=P/(60*220)*1000
					ChargepileDataGetSet(Cmd_SetPower,&ChargePilePara_Set);	
					
					/* 开始功率调整回复计时 */
					if (PowerAdjResp != RT_NULL)
						rt_timer_start(PowerAdjResp);
					else
						rt_lprintf("[strategy] : PowerAdjResp timer create error\n");
				}
				else
				{
					Ctrl_PowerAdj.cSucIdle = ORTHERS;
					c_rst = CtrlUnit_RecResp(Cmd_PowerAdjAck,&Ctrl_PowerAdj,0);//回复
				}
			}
			else
			{
				Ctrl_PowerAdj.cSucIdle = FAILED;
				c_rst = CtrlUnit_RecResp(Cmd_PowerAdjAck,&Ctrl_PowerAdj,0);//回复
			}
			Ctrl_Stop.cSucIdle = Ctrl_PowerAdj.cSucIdle;
			break;
		}

		default:
			break;
	}
}
/********************************************************************  
*	函 数 名: PileData_RecProcess()
*	功能说明: 电桩数据接收处理函数
*	形    参: 无
*	返 回 值: 无
********************************************************************/ 
static void PileData_RecProcess(void)
{
	rt_uint8_t c_rst,p_rst;
	rt_uint32_t start_result,stop_result,adjpow_result;
	
	if(startchg_flag == TRUE)
	{
		//启动成功
		if(rt_event_recv(&ChargePileEvent, ChargeStartOK_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &start_result) == RT_EOK)	
		{
			rt_timer_stop(StartChgResp);
			
			Ctrl_Start.cSucIdle = SUCCESSFUL;
			c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Ctrl_Start,0);
			
			if(c_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				startchg_flag = FALSE;//清位
				rt_lprintf("[strategy] : start charge successful!\n");
			}
		}
		//启动失败
		else if(rt_event_recv(&ChargePileEvent, ChargeStartER_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &start_result) == RT_EOK)	
		{
			rt_timer_stop(StartChgResp);
			
			Ctrl_Start.cSucIdle = FAILED;
			p_rst = ChargepileDataGetSet(Cmd_ChargeStartResp,&ChargePilePara_Get);//获取失败原因
			
			if(p_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				startchg_flag = FALSE;//清位
				rt_lprintf("[strategy] : start charge failed,reason:%d!\n",ChargePilePara_Get.StartReson);
			}
			
			c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Ctrl_Start,0);		
		}
		rt_lprintf("[strategy] : ChargePileEvent 0x%02X\n", start_result);
	}
	
	if(stopchg_flag == TRUE)
	{
		//停机成功
		if(rt_event_recv(&ChargePileEvent, ChargeStopOK_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &stop_result) == RT_EOK)		
		{
			rt_timer_stop(StopChgResp);
			
			Ctrl_Stop.cSucIdle = SUCCESSFUL;
			c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&Ctrl_Stop,0);
						
			if(c_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				stopchg_flag = FALSE;//清位
				rt_lprintf("[strategy] : stop charge successful!\n");
			}
		}
		//停机失败
		else if(rt_event_recv(&ChargePileEvent, ChargeStopER_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &stop_result) == RT_EOK)		
		{
			rt_timer_stop(StopChgResp);
			
			Ctrl_Start.cSucIdle = FAILED;
			p_rst = ChargepileDataGetSet(Cmd_ChargeStartResp,&ChargePilePara_Get);//获取失败原因
			
			if(p_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				stopchg_flag = FALSE;//清位
				rt_lprintf("[strategy] : stop charge failed,reason:%d!\n",ChargePilePara_Get.StopReson);
			}
			
			c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&Ctrl_Stop,0);			
		}
		rt_lprintf("[strategy] : ChargePileEvent 0x%02X\n", stop_result);
	}
	
	if(adjpower_flag == TRUE)
	{
		//调整功率成功
		if(rt_event_recv(&ChargePileEvent, SetPowerOK_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &adjpow_result) == RT_EOK)	
		{
			rt_timer_stop(PowerAdjResp);
			
			Ctrl_PowerAdj.cSucIdle = SUCCESSFUL;
			c_rst = CtrlUnit_RecResp(Cmd_PowerAdjAck,&Ctrl_PowerAdj,0);
			
			if(c_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				adjpower_flag = FALSE;//清位
				rt_lprintf("[strategy] : start charge successful!\n");
			}
		}
		//调整功率失败
		else if(rt_event_recv(&ChargePileEvent, SetPowerER_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &adjpow_result) == RT_EOK)	
		{
			rt_timer_stop(PowerAdjResp);
			
			Ctrl_PowerAdj.cSucIdle = FAILED;
			p_rst = ChargepileDataGetSet(Cmd_SetPowerResp,&ChargePilePara_Get);//获取失败原因
			
			if(p_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				adjpower_flag = FALSE;//清位
//				rt_lprintf("[strategy] : adjust power failed,reason:%d!\n",ChargePilePara_Get.AdjPowerReson);
			}
			
			c_rst = CtrlUnit_RecResp(Cmd_PowerAdjAck,&Ctrl_PowerAdj,0);		
		}
		rt_lprintf("[strategy] : chargepile:ChargePileEvent 0x%02X\n", adjpow_result);
	}
}
/********************************************************************  
*	函 数 名: TimeSolt_PilePowerCtrl()
*	功能说明: 分时段进行电桩功率控制
*	形    参: 无
*	返 回 值: 无
********************************************************************/ 
static void TimeSolt_PilePowerCtrl(void)
{
	count = 0;
	memset(&SetPowerFinishFlag,0,50);
	while(1)
	{
		if((Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Year >= System_Time_STR.Year)&& 
		(Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Month >= System_Time_STR.Month)&& 
		(Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Day >= System_Time_STR.Day)&& 
		(Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Hour >= System_Time_STR.Hour)&& 
		(Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Minute >= System_Time_STR.Minute)&& 
		(Chg_Strategy.strChargeTimeSolts[count].strDecStartTime.Second >= System_Time_STR.Second))	//定位当前所属计划单起始时间段
		{
			Chg_ExeState.ucPlanPower = Chg_Strategy.strChargeTimeSolts[count].ulChargePow;
			
			ChargePilePara_Set.PWM_Duty = Chg_ExeState.ucPlanPower*10/132;//功率换算
			ChargepileDataGetSet(Cmd_SetPower,&ChargePilePara_Set);
			
			Chg_ExeState.exeState = EXE_ING;
			memcpy(cRequestNO_Old,cRequestNO_New,sizeof(cRequestNO_Old));
			break;
		}
		
		count++;
	}
}



static void energycon_thread_entry(void *parameter)
{
	rt_err_t res;
	rt_err_t ret = RT_EOK;
	rt_uint32_t* r_str;
	unsigned char i = 0;
	
	rt_pin_mode(RELAYA_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(RELAYB_PIN, PIN_MODE_OUTPUT);
	RELAY_ON();//上电吸合继电器
	
	rt_thread_mdelay(100);
	
	while (1)
	{
//		if((res = rt_sem_take(&rx_sem_energycon, 1000)) == RT_EOK)
//		{	
//			memcpy(cRequestNO_New,Chg_Strategy.cRequestNO,sizeof(cRequestNO_New));		
//		}	
		PileData_RecProcess();	
		CtrlData_RecProcess();
		
		if(memcmp(cRequestNO_Old,cRequestNO_New,sizeof(cRequestNO_Old)) != 0)
		{
			TimeSolt_PilePowerCtrl(); 
		}
		
		for(i=count;i<50;i++)
		{
			if((Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Year >= System_Time_STR.Year)&& 
			(Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Month >= System_Time_STR.Month)&& 
			(Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Day >= System_Time_STR.Day)&& 
			(Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Hour >= System_Time_STR.Hour)&& 
			(Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Minute >= System_Time_STR.Minute)&& 
			(Chg_Strategy.strChargeTimeSolts[i].strDecStartTime.Second >= System_Time_STR.Second))	//定位当前所属计划单起始时间段
			{
				ChargePilePara_Set.PWM_Duty = Chg_Strategy.strChargeTimeSolts[i].ulChargePow;
				if(SetPowerFinishFlag[i] == 0)//限制发送一次
				{
					ChargepileDataGetSet(Cmd_SetPower,&ChargePilePara_Set);
					SetPowerFinishFlag[i] = 1;
				}
				break;
			}
		}
		
		rt_thread_mdelay(1000);
	}
}

int energycon_thread_init(void)
{
	rt_err_t res;
	
	/* 初始化定时器 */
    timer_create_init();
	
	/*初始化信号量*/
	rt_sem_init(&rx_sem_energycon, "rx_sem_energycon", 0, RT_IPC_FLAG_FIFO);
	
	res=rt_thread_init(&energycon,
											"energycon",
											energycon_thread_entry,
											RT_NULL,
											energycon_stack,
											THREAD_ENERGYCON_STACK_SIZE,
											THREAD_ENERGYCON_PRIORITY,
											THREAD_ENERGYCON_TIMESLICE);
	if (res == RT_EOK) 
	{
		rt_thread_startup(&energycon);
	}
	return res;
}


#if defined (RT_ENERGYCON_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(energycon_thread_init);
#endif
MSH_CMD_EXPORT(energycon_thread_init, energycon thread run);





