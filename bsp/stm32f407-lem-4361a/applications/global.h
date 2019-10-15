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
	RtSt_Starting=0,			// 开机中
	RtSt_StandbyOK,          	// 待机正常
	RtSt_InCharging,           	// 充电中
	RtSt_DisCharging,          	// 放电中
	RtSt_Charged,				// 充放电完成（3秒回待机）
	RtSt_Fault,           		// 故障
	RtSt_Update,				// 升级中
}ROUTER_WORKSTATE;/*路由器状态*/


typedef struct
{
	char AssetNum[23];				//路由器资产编号 字符串 maxlen=22
	rt_uint8_t WorkState;			//路由器运行状态
}ROUTER_IFO_UNIT;/*路由器信息单元*/
extern ROUTER_IFO_UNIT RouterIfo;
	
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
}KEY_INFO_UNIT;/*路由器密钥信息单元*/



/******************************** 充电桩相关信息 ***********************************/	//zcx190807
typedef enum 
{
	ChgSt_Standby=0,            //正常待机
	ChgSt_InCharging,           //充电中
	ChgSt_Fault,            	//故障
}PILE_WORKSTATE;/*充电桩状态*/

typedef enum
{
	GUN_A=1,
	GUN_B,
}GUN_NUM;/*枪序号 {A枪（1）、B枪（2）}*/

typedef enum
{
	SEV_ENABLE=0,
	SEV_DISABLE,
}PILE_SERVICE;/*桩充电服务 {可用（0），停用（1）}*/

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

typedef struct
{
	unsigned char PileNum[17];			//充电桩编号         visible-string（SIZE(16)），
	rt_uint8_t PileIdent;       		//充电接口标识(A/B)
	unsigned char PileInstallAddr[41];	//充电桩的安装地址   visible-string，
	unsigned long minChargePow;			//充电桩最低充电功率 double-long（单位：W，换算：-1），
	unsigned long ulChargeRatePow;		//充电额定功率额定功率 double-long（单位：W，换算：-1）
	rt_uint8_t AwakeSupport;			//是否支持唤醒    {0:不支持 1：支持}
	rt_uint8_t WorkState;				//运行状态
}PILE_IFO_UNIT;/*充电桩信息单元*/

/******************************** 故障信息 ***********************************/		//zcx190710
typedef enum 
{
	NO_FAU=0,
	MEMORY_FAU,		//	终端主板内存故障（0）
	CLOCK_FAU,		//  时钟故障        （1）
	BOARD_FAU,		//  主板通信故障    （2）
	METER_FAU,		//  485抄表故障     （3）
	SCREEN_FAU,		//  显示板故障      （4）
	HPLC_FAU,		//  载波通道异常    （5）
	NANDFLSH_FAU,	//	NandFLASH初始化错误  （6）
	ESAM_FAU,		//  ESAM错误        （7）
	BLE_FAU,		//	蓝牙模块故障     （8）
	BATTERY_FAU,	//	电源模块故障 	（9）
	CANCOM_FAU,		//  充电桩通信故障 （10）
	CHGPILE_FAU,	//  充电桩设备故障 （11）
	ORDRECOED_FAU, 	//	本地订单记录满 （12）
	RTC_FAU,		//	RTC通信故障 （13）
}ROUTER_FAU;/*路由器故障类型*/

typedef union 
{
	rt_err_t Total;
	struct
	{
		rt_err_t Memory_Fau:1;		//	终端主板内存故障（0）
		rt_err_t Clock_Fau:1;		//  时钟故障        （1）
		rt_err_t Board_Fau:1;		//  主板通信故障    （2）
		rt_err_t MeterCom_Fau:1;	//  485抄表故障     （3）
		rt_err_t Screen_Fau:1;		//  显示板故障      （4）
		rt_err_t Hplc_Fau:1;		//  载波通道异常    （5）
		rt_err_t NandFlash_Fau:1;	//	NandFLASH初始化错误  （6）
		rt_err_t ESAM_Fau:1;		//  ESAM错误        （7）
		rt_err_t Ble_Fau:1;			//	蓝牙模块故障     （8）
		rt_err_t Battery_Fau:1;		//	电源模块故障 	（9）
		rt_err_t CanCom_Fau:1;		//  充电桩通信故障 （10）
		rt_err_t ChgPile_Fau:1;		//  充电桩设备故障 （11）
		rt_err_t OrdRecord_Fau:1; 	//	本地订单记录满 （12）
		rt_err_t RTC_Fau:1;         //	RTC通信故障  (13)	(自定义)
	}
	Bit;
}ROUTER_FAULT;
extern ROUTER_FAULT Fault;
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
