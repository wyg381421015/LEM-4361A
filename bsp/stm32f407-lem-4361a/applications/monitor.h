#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <rtthread.h>
#include <global.h>



//typedef struct 				
//{
//	unsigned char  Second;        // 秒
//	unsigned char  Minute;        // 分
//	unsigned char  Hour;          // 时
//	unsigned char  Week;          //星期
//	
//	unsigned char  Day;           // 日 
//	unsigned char  Month;         // 月
//	unsigned char  Year;          // 年 后两位
//}STR_SYSTEM_TIME;

//extern STR_SYSTEM_TIME System_Time_STR;

/***********模拟量YC结构体******************************/
//typedef struct        //模拟量测量结果结构变量
//{
//	union
//	{	
//		unsigned long  ChargVa;		    //充电电压  1位小数
//		unsigned long  ChargIa;       //充电电流  3位小数
//		unsigned long  V_cp;          //导引电压  1位小数
//		unsigned long  Tempa;         //温度1     1位小数
//		unsigned long  Tempb;         //温度1     1位小数
//		
//		unsigned long lResult[5];
//	}Analog;
//}YC_Analog_TypeDef;



/////**********用户调用函数****************************************/
//extern YC_Analog_TypeDef  STR_YC_Analog_A;

extern rt_err_t Set_rtc_time(STR_SYSTEM_TIME *time);


///**********内部函数****************************************/


#endif

