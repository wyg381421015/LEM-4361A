#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include <stdio.h>
#include <global.h>
#include <monitor.h>
#include <stdlib.h>

#define DEVICE_NAME			"pcf8563"//串口5设备名

#define THREAD_MONITOR_PRIORITY     17
#define THREAD_MONITOR_STACK_SIZE   1024
#define THREAD_MONITOR_TIMESLICE    6
static rt_uint8_t monitor_stack[THREAD_MONITOR_STACK_SIZE];//线程堆栈

static struct rt_thread monitor;
static rt_device_t pcf8563;


///* 定义邮箱控制块 */
//extern rt_mailbox_t m_save_mail;

STR_SYSTEM_TIME System_Time_STR;

static void monitor_thread_entry(void *parameter)
{
	rt_err_t ret = RT_EOK;
  rt_uint32_t* r_str;
	
	pcf8563 = (rt_device_t)rt_device_find(DEVICE_NAME);
	
	if(pcf8563 != RT_NULL)
	{
//		rt_lprintf("[Monitor]:pcf8563 device find sucess!\r\n");
		
		if (rt_device_open(pcf8563, 0) == RT_EOK)
		{
			rt_lprintf("[Monitor]:Open pcf8563 device sucess!\r\n");
		}
	}
	else
	{
		rt_lprintf("[Monitor]:Not find device pcf8563!\r\n");
		return;
	}
	
	rt_device_control(pcf8563,RT_DEVICE_CTRL_RTC_GET_TIME,&System_Time_STR);
	
	rt_thread_mdelay(100);
	
	while (1)
	{
	
		rt_device_control(pcf8563,RT_DEVICE_CTRL_RTC_GET_TIME,&System_Time_STR);
		
		rt_lprintf("[Monitor]:Systerm time is %02X-%02X-%02X-%02X-%02X-%02X!\r\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day\
								,System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second);
		
		rt_thread_mdelay(1000);
	}
}

int monitor_thread_init(void)
{
	rt_err_t res;
	
	res=rt_thread_init(&monitor,
											"monitor",
											monitor_thread_entry,
											RT_NULL,
											monitor_stack,
											THREAD_MONITOR_STACK_SIZE,
											THREAD_MONITOR_PRIORITY,
											THREAD_MONITOR_TIMESLICE);
	if (res == RT_EOK) 
	{
			rt_thread_startup(&monitor);
	}
	return res;
}


#if defined (RT_MONITOR_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(monitor_thread_init);
#endif
MSH_CMD_EXPORT(monitor_thread_init, monitor thread run);

rt_err_t Set_rtc_time(STR_SYSTEM_TIME *time)
{
	rt_err_t result;
	
	result = rt_device_control(pcf8563,RT_DEVICE_CTRL_RTC_SET_TIME,time);
	
	return result;
}


void WrTime(int argc, char**argv)//WrTime 19 08 04 08 12 59 00
{
	rt_uint8_t i;
	rt_uint8_t	time[7],tmp[7];
	rt_uint32_t level;
	char *buf;
	
	level = rt_hw_interrupt_disable();
	
	for(i = 0;i < 7;i++)
	{
		tmp[6-i] = strtol(argv[1+i],&buf,10);
	}
	rt_kprintf("set rtc time is %s\n",buf);
	
	Int_toBCD(time,tmp,7);

	if((time[0]>0x60)||(time[1]>0x60)||(time[2]>0x24)||(time[3]>0x31)||(time[4]>0x07)||(time[5]>0x12)||(time[6]>0x99))
	{
		rt_hw_interrupt_enable(level);
		rt_kprintf("time is err\n");
		return;
	}
	rt_kprintf("[Monitor]:set rtc time is %02X-%02X-%02X-%02X-%02X-%02X!\r\n",time[6],time[5],time[3],time[4],time[2],time[1],time[0]);
	
	rt_device_control(pcf8563,RT_DEVICE_CTRL_RTC_SET_TIME,&time);
	
	rt_hw_interrupt_enable(level);
}
MSH_CMD_EXPORT(WrTime, set time);


void RdTime(void)
{
	rt_device_control(pcf8563,RT_DEVICE_CTRL_RTC_GET_TIME,&System_Time_STR);
	rt_kprintf("[Monitor]:Systerm time is %02X-%02X-%02X-%02X-%02X-%02X!\r\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day\
								,System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second);
}
MSH_CMD_EXPORT(RdTime, read time);

