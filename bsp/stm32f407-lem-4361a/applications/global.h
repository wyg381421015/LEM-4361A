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

/******************************** 路由器相关信息 ***********************************/	//zcx190710
typedef enum {
	DevSt_Starting=0,			// 开机中
	DevSt_StandbyOK,          	// 待机正常
	DevSt_InCharging,           // 充电中
	DevSt_DisCharging,          // 放电中
	DevSt_Charged,				// 充放电完成（3秒回待机）
	DevSt_Fault,           		// 故障
	DevSt_Update,				// 升级中
}DEVICE_WORKSTATE;/*路由器状态*/


typedef struct
{
	char UserID[65];   			//用户id  visible-string（SIZE(64)）
	char Token[39];   			//用户登录令牌  visible-string（SIZE(38)）
	unsigned char AccountState;	//账户状态 {0：正常，1：欠费}
}WHITE_LIST;/*路由器白名单*/

typedef struct
{
	char kAddress[17];   		//通讯地址	visible-string（SIZE(16)）
	char MeterNum[9];   		//表号  visible-string（SIZE(8)）
	char KeyNum[9];				//密钥信息	visible-string（SIZE(8)）	
}KEYINFO_UNIT;/*路由器密钥信息单元*/



/******************************** 充电桩相关信息 ***********************************/	//zcx190807
typedef enum {
	ChgSt_Standby=0,            //正常待机
	ChgSt_InCharging,           //充电中
	ChgSt_Fault,            	//故障
}CHGPILE_WORKSTATE;/*充电桩状态*/

/******************************** 故障信息 ***********************************/		//zcx190710
typedef enum 
{
//	NO_FAULT			=0x00000000,
//	Memory_FAULT		=0x00000001,		//	终端主板内存故障（0）
//	Clock_FAULT			=0x00000002,		//  时钟故障        （1）
//	Board_FAULT			=0x00000004,		//  主板通信故障    （2）
//	MeterCom_FAULT		=0x00000008,		//  485抄表故障     （3）
//	Screen_FAULT		=0x00000010,		//  显示板故障      （4）
//	CarrierChl_FAULT	=0x00000020,		//  载波通道异常    （5）
//	NandF_FAULT			=0x00000040,		//	NandFLASH初始化错误  （6）
//	ESAM_FAULT			=0x00000080,		//  ESAM错误        （7）
//	Bluetooth_FAULT		=0x00000100,		//	蓝牙模块故障     （8）
//	Battery_FAULT		=0x00000200,		//	电源模块故障 	（9）
//	CanCom_FAULT		=0x00000400,		//  充电桩通信故障 （10）
//	ChgPile_FAULT		=0x00000800,		//  充电桩设备故障 （11）
//	OrdRecord_FAULT		=0x00001000, 		//	本地订单记录满 （12）
	NO_FAULT=0,
	Memory_FAULT,		//	终端主板内存故障（0）
	Clock_FAULT,		//  时钟故障        （1）
	Board_FAULT,		//  主板通信故障    （2）
	MeterCom_FAULT,		//  485抄表故障     （3）
	Screen_FAULT,		//  显示板故障      （4）
	CarrierChl_FAULT,	//  载波通道异常    （5）
	NandF_FAULT,		//	NandFLASH初始化错误  （6）
	ESAM_FAULT,			//  ESAM错误        （7）
	Bluetooth_FAULT,	//	蓝牙模块故障     （8）
	Battery_FAULT,		//	电源模块故障 	（9）
	CanCom_FAULT,		//  充电桩通信故障 （10）
	ChgPile_FAULT,		//  充电桩设备故障 （11）
	OrdRecord_FAULT, 	//	本地订单记录满 （12）
}DEVICE_FAULT;/*路由器故障类型*/


typedef enum 
{
	PILE_NOFAULT        =0x00000000,
	PILE_Memory_FAULT	=0x00000001,	//	终端主板内存故障（0）
	PILE_Clock_FAULT	=0x00000002,	//  时钟故障        （1）
	PILE_Board_FAULT	=0x00000004,	//  主板通信故障    （2）
	PILE_MeterCom_FAULT	=0x00000008,	//  485抄表故障    （3）
	PILE_Screen_FAULT	=0x00000010,	//  显示板故障      （4）
	CardOffLine_FAULT	=0x00000020,	//	读卡器通讯中断  （5）
	PILE_ESAM_FAULT		=0x00000040,	//  ESAM错误        （6）
	StopEct_FAULT		=0x00000080,	//  急停按钮动作故障（7）
	Arrester_FAULT		=0x00000100,	//	避雷器故障		（8）
	GunHoming_FAULT		=0x00000200,	//	充电枪未归位		（9）
	OverV_FAULT			=0x00000400,	//	输入过压告警		（10）
	UnderV_FAULT		=0x00000800,	//	输入欠压告警		（11）
	Pilot_FAULT			=0x00001000,	//	充电中车辆控制导引告警（12）
	Connect_FAULT		=0x00002000,	//	交流接触器故障	（13）
	OverI_Warning		=0x00004000,	//	输出过流告警		（14）
	OverI_FAULT			=0x00008000,	//	输出过流保护动作	（15）
	ACCir_FAULT			=0x00010000,	//	交流断路器故障	（16）
	GunLock_FAULT		=0x00020000,	//	充电接口电子锁故障（17）
	GunOverTemp_FAULT	=0x00040000,	//	充电接口过温故障	（18）
	CC_FAULT			=0x00080000,	//	充电连接状态CC异常（19）
	CP_FAULT			=0x00100000,	//	充电控制状态CP异常（20）
	PE_FAULT			=0x00200000,	//	PE断线故障		（21）
	Dooropen_FAULT		=0x00400000,	//	柜门打开故障		(22)
	Other_FAULT			=0x00800000,	//	充电机其他故障	（23）
}PILE_FAULT;/*充电桩故障类型*/


//typedef union 
//{
//	rt_uint32_t ALARM;
//	struct
//	{
//		rt_uint32_t BLE_COMM:1;           //bit0//蓝牙通信故障
//		rt_uint32_t METER_COMM:1;         //bit1//计量模块通信故障
//		rt_uint32_t METER_VOL_HIGH:1;     //bit2//电压过高
//		rt_uint32_t METER_VOL_LOW:1;      //bit3//电压过低
//		rt_uint32_t METER_CUR_HIGH:1;     //bit4//过流
//		rt_uint32_t STORAGE_NAND:1;       //bit5//flash故障
//		rt_uint32_t RTC_COMM:1;          	//bit6//rtc通信故障
//		rt_uint32_t ESAM_COMM:1;      		//bit7//esam通信故障
//		rt_uint32_t HPLC_COMM:1;          //bit8//hplc通信故障
//	}
//	B;
//} 
//Systerm_Alarm;


//////////////////////////////////////////////////////////////////////////////////


extern unsigned long timebin2long(unsigned char *buf);
CCMRAM extern char Printf_Buffer[1024];
CCMRAM extern char Srintf_Buffer[1024];


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
