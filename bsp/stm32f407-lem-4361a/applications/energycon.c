#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include <stdio.h>
#include <global.h>
#include "energycon.h"
#include "chargepile.h"
#include "strategy.h"
#include "698.h"


#define THREAD_ENERGYCON_PRIORITY     18
#define THREAD_ENERGYCON_STACK_SIZE   1024
#define THREAD_ENERGYCON_TIMESLICE    20
static rt_uint8_t energycon_stack[THREAD_ENERGYCON_STACK_SIZE];//线程堆栈

struct rt_thread energycon;
struct rt_semaphore rx_sem_energycon;     //用于接收数据的信号量

static unsigned char count;
static unsigned char SetPowerFinishFlag[50];
static char cRequestNO_Old[17];
static char cRequestNO_New[17];
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
			ChargePilePara.PWM_Duty = Chg_Strategy.strChargeTimeSolts[count].ulChargePow;
			ChargepileDataGetSet(Cmd_SetPower,&ChargePilePara);
			CtrlUnit_RecResp(Cmd_ChgPlanExeState,&Chg_ExeState,0);//上送充电计划执行状态
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
	

	rt_thread_mdelay(100);
	
	while (1)
	{
//		if((res = rt_sem_take(&rx_sem_energycon, 1000)) == RT_EOK)
//		{	
//			memcpy(cRequestNO_New,Chg_Strategy.cRequestNO,sizeof(cRequestNO_New));		
//		}	

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
				ChargePilePara.PWM_Duty = Chg_Strategy.strChargeTimeSolts[i].ulChargePow;
				if(SetPowerFinishFlag[i] == 0)//限制发送一次
				{
					ChargepileDataGetSet(Cmd_SetPower,&ChargePilePara);
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
	
		//初始化信号量
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





