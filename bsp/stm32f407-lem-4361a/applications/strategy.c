#include <rtthread.h>
#include <rtdevice.h>
#include "strategy.h"
#include "chargepile.h"
#include "698.h"
#include "meter.h"
#include "energycon.h"
#include <board.h>

#ifndef FALSE
#define FALSE         0
#endif
#ifndef TRUE
#define TRUE          1
#endif
#define THREAD_STRATEGY_PRIORITY     8
#define THREAD_STRATEGY_STACK_SIZE   1024
#define THREAD_STRATEGY_TIMESLICE    5

#ifndef SUCCESSFUL
#define SUCCESSFUL    0
#endif
#ifndef FAILED
#define FAILED        1
#endif

#define RELAYA_PIN    GET_PIN(F, 2)
#define RELAYB_PIN    GET_PIN(F, 3)


rt_uint8_t DeviceState;
rt_uint8_t ChgpileState;
rt_bool_t DeviceFauFlag;
///////////////////////////////////////////////////////////////////
//定义故障原因
static char *err_strfault[] = 
{
           "                       ",            /* ERR_OK          0  */
           "终端主板内存故障！   ",               /* ERR             1  */
           "时钟故障！         ",                 /* ERR             2  */
           "主板通信故障！	       ",            /* ERR             3  */
           "485抄表故障！         ",              /* ERR             4  */
           "显示板故障！         ",               /* ERR             5  */
           "载波通道异常！       ",               /* ERR             6  */
           "NandFLASH初始化错误！       ",        /* ERR             7  */
           "ESAM错误！         ",                 /* ERR             8  */
           "蓝牙模块故障！         ",             /* ERR             9  */
           "电源模块故障！         ",             /* ERR             10 */
           "充电桩通信故障！        ",            /* ERR             11 */
           "充电桩设备故障！         ",           /* ERR             12 */
	       "本地订单记录满！       ",             /* ERR             13 */
};




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

//extern struct rt_event PowerCtrlEvent;
//extern struct rt_event HplcEvent;



//超时结果
static rt_timer_t StartChgResp;
static rt_timer_t StopChgResp;
static rt_timer_t AdjustPowerResp;
	
static rt_uint8_t strategy_stack[THREAD_STRATEGY_STACK_SIZE];//线程堆栈
static struct rt_thread strategy;

ChargPilePara_TypeDef ChargePilePara;
CHARGE_STRATEGY Chg_Strategy;
CHARGE_STRATEGY_RSP Chg_StrategyRsp;
CHARGE_STRATEGY Adj_Chg_Strategy;
CHARGE_STRATEGY_RSP Adj_Chg_StrategyRsp;
CHARGE_EXE_STATE Chg_ExeState;
CTL_CHARGE Ctrl_StartRsp;
CTL_CHARGE Ctrl_StopRsp;


unsigned char PileWorkState;
rt_bool_t startchg_flag;
rt_bool_t stopchg_flag;
/**************************************************************
 * 函数名称: StartChgResp_Timeout 
 * 参    数: 
 * 返 回 值: 
 * 描    述: 启动充电超时函数
 **************************************************************/
static void StartChgResp_Timeout(void *parameter)
{
    rt_lprintf("StartChgResp event is timeout!\n");
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
    rt_lprintf("StopChgResp event is timeout!\n");
	ChargepileDataGetSet(Cmd_ChargeStopResp,0);
}
/**************************************************************
 * 函数名称: timer_create_init 
 * 参    数: 
 * 返 回 值: 
 * 描    述: 定时器
 **************************************************************/
void timer_create_init()
{
    /* 创建启机回复定时器 */
	 StartChgResp = rt_timer_create("StartChgResp",  /* 定时器名字是 StartChgResp */
									StartChgResp_Timeout, /* 超时时回调的处理函数 */
									RT_NULL, /* 超时函数的入口参数 */
									5000, /* 定时长度，以OS Tick为单位，即5000个OS Tick */
									RT_TIMER_FLAG_ONE_SHOT); /* 一次性定时器 */
	/* 创建停机回复定时器 */
	 StopChgResp = rt_timer_create("StopChgResp",  /* 定时器名字是 StartChgResp */
									StopChgResp_Timeout, /* 超时时回调的处理函数 */
									RT_NULL, /* 超时函数的入口参数 */
									5000, /* 定时长度，以OS Tick为单位，即5000个OS Tick */
									RT_TIMER_FLAG_ONE_SHOT); /* 一次性定时器 */
}
/*  */

/********************************************************************  
*	函 数 名: CtrlData_RecProcess()
*	功能说明: 控制器数据接收处理函数
*	形    参: 无
*	返 回 值: 无
********************************************************************/ 
static void CtrlData_RecProcess(void)
{
	rt_uint8_t c_rst;
	rt_uint32_t chgplanIssue,chgplanIssueAdj,startchg,stopchg;

	
	//收到充电计划
//	if(rt_event_recv(&PowerCtrlEvent, ChgPlanIssue_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &chgplanIssue) == RT_EOK)
	if(strategy_event_get()!=0)
	{
		c_rst = CtrlUnit_RecResp(Cmd_ChgPlanIssue,&Chg_Strategy,0);//取值
		if((Chg_Strategy.ucChargeMode == 1)&&(Chg_Strategy.ucDecType == 1))
			rt_sem_release(&rx_sem_energycon);
		
		memcpy(&Chg_StrategyRsp,&Chg_Strategy,40);
		Chg_StrategyRsp.cSucIdle = 0;
		
		c_rst = CtrlUnit_RecResp(Cmd_ChgPlanIssueAck,&Chg_StrategyRsp,0);//回复		
	}
	
	//收到充电计划调整
//	if(rt_event_recv(&PowerCtrlEvent, ChgPlanAdjust_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &chgplanIssueAdj) == RT_EOK)
	if(strategy_event_get()!=0)
	{
		c_rst = CtrlUnit_RecResp(Cmd_ChgPlanAdjust,&Adj_Chg_Strategy,0);//取值	
		if((Chg_Strategy.ucChargeMode == 1)&&(Chg_Strategy.ucDecType == 2))
			rt_sem_release(&rx_sem_energycon);
		
		memcpy(&Adj_Chg_StrategyRsp,&Adj_Chg_Strategy,40);
		Chg_StrategyRsp.cSucIdle = 0;
		
		c_rst = CtrlUnit_RecResp(Cmd_ChgPlanAdjustAck,&Adj_Chg_StrategyRsp,0);//回复		
	}
	
	//收到启动充电命令
//	if(rt_event_recv(&PowerCtrlEvent, StartChg_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &startchg) == RT_EOK)
	if(strategy_event_get()!=0)
	{
		startchg_flag = TRUE;
		rt_lprintf("[hplc]  (%s)  收到启动充电命令  \n",__func__);
		
		if(DeviceFauFlag != TRUE)
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
			Ctrl_StartRsp.cSucIdle = FAILED;
			c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Ctrl_StartRsp,0);//回复	
		}		   	
	}
	//收到停止充电命令
//	if(rt_event_recv(&PowerCtrlEvent, StopChg_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &stopchg) == RT_EOK)
	if(strategy_event_get()!=0)
	{
		stopchg_flag = TRUE;		
		rt_lprintf("[hplc]  (%s)  收到停止充电命令  \n",__func__);  
			
		if(DeviceFauFlag != TRUE)
		{
			ChargepileDataGetSet(Cmd_ChargeStop,0);	
			
			/* 开始停机回复计时 */
			if (StopChgResp != RT_NULL)
				rt_timer_start(StopChgResp);
			else
				rt_lprintf("StartChgResp timer create error\n");
		}
		else
		{
			Ctrl_StopRsp.cSucIdle = FAILED;
			c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&Ctrl_StopRsp,0);//回复
		}
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
	rt_uint32_t start_result,stop_result;
	
	if(startchg_flag == TRUE)
	{
		if(rt_event_recv(&ChargePileEvent, ChargeStartOK_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &start_result) == RT_EOK)	//启动成功
		{
			rt_timer_stop(StartChgResp);
			c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,0,0);
			
			if(c_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				startchg_flag = FALSE;//清位
				rt_lprintf("start charge successful!\n");
			}
			
			rt_lprintf("chargepile:ChargePileEvent 0x%02X\n", start_result);	
		}
		else if(rt_event_recv(&ChargePileEvent, ChargeStartER_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &start_result) == RT_EOK)	//启动失败
		{
			rt_timer_stop(StartChgResp);
			p_rst = ChargepileDataGetSet(Cmd_ChargeStartResp,&ChargePilePara);//获取失败原因
			
			if(p_rst != SUCCESSFUL)
			{
				
			}
			else
			{
				startchg_flag = FALSE;//清位
				rt_lprintf("start charge failed,reason:%d!\n",ChargePilePara.StartReson);
			}
			
			c_rst = CtrlUnit_RecResp(Cmd_StartChgAck,&Chg_Strategy,0);
			
			rt_lprintf("chargepile:ChargePileEvent 0x%02X\n", start_result);
		}
	}
	
	if(stopchg_flag == TRUE)
	{
		if(rt_event_recv(&ChargePileEvent, ChargeStopOK_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &stop_result) == RT_EOK)		//停机成功
		{
			rt_timer_stop(StopChgResp);
			c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,&ChargePilePara,0);
			stopchg_flag = FALSE;
			rt_lprintf("chargepile:ChargePileEvent 0x%02X\n", stop_result);	
		}
		else if(rt_event_recv(&ChargePileEvent, ChargeStartER_EVENT,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,100, &stop_result) == RT_EOK)		//停机失败
		{
			ChargepileDataGetSet(Cmd_ChargeStartResp,&ChargePilePara);//获取失败原因
			c_rst = CtrlUnit_RecResp(Cmd_StopChgAck,0,0);
			rt_lprintf("chargepile:ChargePileEvent 0x%02X\n", stop_result);
		}
	}
}

/********************************************************************  
*	函 数 名: DevState_Judge()
*	功能说明: 路由器状态判断
*	形    参: 无
*	返 回 值: 故障代码
********************************************************************/ 
rt_err_t DevState_Judge(void)
{
	rt_err_t fau;
	fau = 0;
	
	if((System_Time_STR.Year > 50)||(System_Time_STR.Month > 12)||(System_Time_STR.Day > 31)
		||(System_Time_STR.Hour > 23)||(System_Time_STR.Minute > 60))
	{
		fau |= (1<<(Clock_FAULT-1));				
		rt_lprintf("%s\r\n",(const char*)err_strfault[Clock_FAULT]);
	}
	
//	if(rt_device_find("lcd"))
//	{
//		fau |= (1<<(Screen_FAULT-1));				
//		rt_lprintf("%s\r\n",(const char*)err_strfault[Screen_FAULT]);
//	}
	
//	if(rt_sem_trytake(&rt_sem_meterfau) == RT_EOK)
//	{
//		fau |= (1<<(MeterCom_FAULT-1));
//		rt_lprintf("%s\r\n",(const char*)err_strfault[MeterCom_FAULT]);
//	}
//	
//	if(rt_sem_trytake(&rt_sem_nandfau) == RT_EOK)
//	{
//		fau |= (1<<(NandF_FAULT-1));		
//		rt_lprintf("%s\r\n",(const char*)err_strfault[NandF_FAULT]);
//	}
	
//	if(rt_sem_trytake(&rt_sem_bluetoothfau) == RT_EOK)
//	{
//		fau |= (1<<(Bluetooth_FAULT-1));
//		rt_lprintf("%s\r\n",(const char*)err_strfault[Bluetooth_FAULT]);
//	}
		
	return fau;
	
}

static void strategy_thread_entry(void *parameter)
{
	rt_err_t res,fau;
	
	rt_pin_mode(RELAYA_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(RELAYB_PIN, PIN_MODE_OUTPUT);
	RELAY_ON();//上电吸合继电器
	
	DeviceFauFlag = FALSE;
	rt_thread_mdelay(100);
	
	while (1)
	{
		fau = DevState_Judge();
		if(fau != 0)
		{
			DeviceFauFlag = TRUE;
		}
		else
		{
			DeviceFauFlag = FALSE;
		}
		
		PileData_RecProcess();
		
		CtrlData_RecProcess();
		
		
		
		
						
		rt_thread_mdelay(1000);
	}
}

int strategy_thread_init(void)
{
	rt_err_t res;
	
	/* 初始化定时器 */
    timer_create_init();
	
	res=rt_thread_init(&strategy,
											"strategy",
											strategy_thread_entry,
											RT_NULL,
											strategy_stack,
											THREAD_STRATEGY_STACK_SIZE,
											THREAD_STRATEGY_PRIORITY,
											THREAD_STRATEGY_TIMESLICE);
	if (res == RT_EOK) 
	{
		rt_thread_startup(&strategy);
	}
	return res;
}


#if defined (RT_STRATEGY_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(strategy_thread_init);
#endif
MSH_CMD_EXPORT(strategy_thread_init, strategy thread run);



