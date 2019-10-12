#ifndef __GLOBAL_H
#define __GLOBAL_H	

#include <rtthread.h>
#include  <rtconfig.h>
#include  <string.h>
#include  <stdarg.h>

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//LEM_4242A Board
//系统时钟初始化	
//鲁能智能@LNINT
//技术论坛:www.openedv.com
//创建日期:2014/5/2
//版本：V1.0
//版权所有，盗必究。
//Copyright(C) 山东鲁能智能技术有限公司 2017-2099
//All rights reserved
//********************************************************************************
//修改说明
//无
//////////////////////////////////////////////////////////////////////////////////


#define CCMRAM __attribute__((section("ccmram")))

#define MY_HEX	1
#define MY_CHAR 2

extern unsigned char DEBUG_MSH;

typedef struct{
	rt_uint8_t DataRx_len;
	rt_uint8_t Rx_data[1024];
	rt_uint8_t DataTx_len;
	rt_uint8_t Tx_data[1024];

}ScmUart_Comm;

//////////////////////////////////////////////////////////////////////////////////
typedef struct 				
{
	unsigned char  Second;        // 秒
	unsigned char  Minute;        // 分
	unsigned char  Hour;          // 时
	unsigned char  Day;           // 日
	
	unsigned char  Week;          //星期
	
	unsigned char  Month;         // 月
	unsigned char  Year;          // 年 后两位
}STR_SYSTEM_TIME;
extern STR_SYSTEM_TIME System_Time_STR;




typedef union 
{
	rt_uint32_t ALARM;
	struct
	{
		rt_uint32_t BLE_COMM:1;           //bit0//蓝牙通信故障
		rt_uint32_t METER_COMM:1;         //bit1//计量模块通信故障
		rt_uint32_t METER_VOL_HIGH:1;     //bit2//电压过高
		rt_uint32_t METER_VOL_LOW:1;      //bit3//电压过低
		rt_uint32_t METER_CUR_HIGH:1;     //bit4//过流
		rt_uint32_t STORAGE_NAND:1;       //bit5//flash故障
		rt_uint32_t RTC_COMM:1;          	//bit6//rtc通信故障
		rt_uint32_t ESAM_COMM:1;      		//bit7//esam通信故障
		rt_uint32_t HPLC_COMM:1;          //bit8//hplc通信故障
	}
	B;
}ROUTER_ALARM;//路由器故障信息


typedef union 
{
	rt_uint32_t ALARM;
	struct
	{
		rt_uint32_t  ConnState:1;         //车辆连接状态  	    0 连接 1未连接
		rt_uint32_t  StopEctFau:1;        //急停动作故障        0 正常 1异常
		rt_uint32_t	ArresterFau:1;       //避雷器故障	        0 正常 1异常
		rt_uint32_t  GunOut:1;            //充电枪未归位		0 正常 1异常
		rt_uint32_t  ChgTemptOVFau:1;     //充电桩过温故障		
		rt_uint32_t  VOVWarn:1;           //输出电压过压
		rt_uint32_t  VLVWarn:1;           //输入电压欠压
		rt_uint32_t  OutConState:1;       //输出接触器状态      0正常，1异常
		rt_uint32_t  PilotFau:1;		   //充电中车辆控制导引故障        
		rt_uint32_t  ACConFau:1;		   //交流接触器故障	    0正常，1异常
		rt_uint32_t  OutCurrAla:1;		   //输出过流告警		            
		rt_uint32_t  OverCurrFau:1;	   //输出过流故障	    0正常，1保护
		rt_uint32_t  CurrentOutFlag:1;    //输出过流延时标识	TRUE开始计数   FALSE停止计数
		rt_uint32_t  CurrentOutCount:1;   //输出过流延时计时	
		rt_uint32_t  CCFau:1;	  		   //充电中枪头异常断开			 							
		rt_uint32_t  ACCirFau:1;		   //交流断路器故障
		rt_uint32_t  LockState:1;		   //充电接口电子锁状态
		rt_uint32_t  LockFauState:1;	   //充电接口电子锁故障状态
		rt_uint32_t  GunTemptOVFau:1;     //充电接口过温故障				
		rt_uint32_t  CC:1;				   //充电连接状态CC检测点4    1枪归位桩  0枪离开桩
		rt_uint32_t  CP:2;				   //充电控制状态CP检测点1
		rt_uint32_t  PEFau:1;			   //PE断线故障
		rt_uint32_t  DooropenFau:1;	   //柜门打开故障
		rt_uint32_t  ChgTemptOVAla:1;	   //充电桩过温告警				
		rt_uint32_t  GunTemptOVAla:1;	   //充电枪过温告警				
		rt_uint32_t  Contactoradjoin:1;   //接触器粘连
		rt_uint32_t  OthWarnVal:1;        //其他报警值
	}
	B;
}CHARGEPILE_ALARM;//充电桩故障信息

typedef struct 
{
	rt_uint32_t WorkState;    //  01忙碌    10故障   11 暂停//系统工作状态
	
	ROUTER_ALARM ScmRouter_Alarm;
	CHARGEPILE_ALARM ScmChargePile_Alarm;
}Systerm_State;


//////////////////////////////////////////////////////////////////////////////////


extern unsigned long timebin2long(unsigned char *buf);
CCMRAM extern char Printf_Buffer[1024];
CCMRAM extern char Sprintf_Buffer[1024];


extern const char ProgramVersion[8]; // 版本号
extern void Kick_Dog(void);

extern unsigned char str2bcd(char*src,unsigned char *dest);
extern void BCD_toInt(unsigned char *data1,unsigned char *data2,unsigned char len);
extern void Int_toBCD(unsigned char *data1,unsigned char *data2,unsigned char len);
extern unsigned char bcd2str(unsigned char *src,char *dest,unsigned char count);
extern unsigned char CRC7(unsigned char *ptr,unsigned int cnt);
//extern unsigned short CRC16_CCITT_ISO(unsigned char *ptr, unsigned int count);
extern unsigned int CRC_16(unsigned char *ptr, unsigned int nComDataBufSize);
extern unsigned char CRC7(unsigned char *ptr,unsigned int count);
extern unsigned char XOR_Check(unsigned char *pData, unsigned int Len);


extern void my_printf(char* buf,rt_uint32_t datalenth,rt_uint8_t type,rt_uint8_t cmd,char* function);
////////////////////////////////////////////////////////////////////////////////// 

#endif
