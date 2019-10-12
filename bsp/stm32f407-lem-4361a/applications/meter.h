#ifndef __METER_H__
#define __METER_H__

//#include "monitor.h"
#include "global.h"


//typedef struct{
//	unsigned char DataRx_len;
//	unsigned char Rx_data[100];
//	unsigned char DataTx_len;
//	unsigned char Tx_data[100];
//	
//	void (*SendBefor)(void); 	  /* 开始发送之前的回调函数指针（主要用于RS485切换到发送模式） */
//	unsigned char (*SendProtocal)(void); /* 发送完毕的回调函数指针（主要用于RS485将发送模式切换为接收模式） */
//	void (*RecProtocal)(void);	/* 串口收到数据的回调函数指针 */
//}ScmMeter;

//计费模型
typedef struct 
{
	unsigned char	uiJFmodID[8];

	STR_SYSTEM_TIME  EffectiveTime;	     // 生效时间
	STR_SYSTEM_TIME  unEffectiveTime;	  // 失效时间
	unsigned char state;               //执行状态
	unsigned char	style;				 				//计量类型
	unsigned char ulTimeNo[48];		     //48个费率号 0:00~24:00
	unsigned char count;			         //有效费率数
	unsigned long ulPriceNo[48];       // 48个电价
}ScmMeter_PriceModle;

//电度表模拟量
typedef struct 
{
	unsigned long ulVol;         // 交流电压
	unsigned long ulCur;         // 交流电流
	unsigned long ulAcPwr;         // 瞬时有功功率
	unsigned long ulMeterTotal;     //有功总电量
	unsigned long ulPwrFactor;     //功率因数
	unsigned long ulFrequency;     //频率
	
}ScmMeter_Analog;

typedef struct //电量结构体
{
	unsigned long ulPowerT;         // 总电量
	unsigned long ulPowerJ;         // 尖
	unsigned long ulPowerF;         // 峰
	unsigned long ulPowerP;         // 平
	unsigned long ulPowerG;         // 谷
}ScmMeter_Power;


//电度表历史电量信息
typedef struct 
{
	ScmMeter_Power	ulMeter_Day;			//每日尖峰平谷总电量
	ScmMeter_Power 	ulMeter_Month;     //每月尖峰平谷总电量
}ScmMeter_HisData;

enum cmEMMETER
{
	EMMETER_ANALOG = 0,//模拟量数据
	EMMETER_HISDATA,//电量历史数据
};

//**************用户调用函数*******************//
extern void cmMeter_get_data(unsigned char cmd,void* str_data);


//static rt_err_t rt_send_mq(rt_mq_t mq, char* name, ScmStorage_Msg msg, void* args);

#endif

