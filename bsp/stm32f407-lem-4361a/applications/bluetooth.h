#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include <rtdevice.h>


#define  GET_REQUEST 						5
#define  SET_REQUEST 						6	
#define  ACTION_REQUEST			 	7
#define  REPORT_RESPONSE			  8
#define  PROXY_REQUEST				  9
#define  SECURITY_REQUEST     	16

#define GET_REQUEST_NOMAL			1	
#define ACTION_REQUEST_NOMAL			1

#define PLAINTEXT							0
#define CIPHERTEXT						1


#define ADDR_MAX_LINE					6
#define APDU_MAX_LENTH				512
#define APDU_USER_MAX_LENTH		512

struct _698_BLE_ADDR        //P   地址域a
{
	union 
	{
		unsigned char SA;//服务器地址
		struct{
		unsigned char uclenth:4;//bit 0~2 +1 = 地址域长度
		unsigned char uclogic:2;
		unsigned char uctype:2;
		}B;
	}S_ADDR;
	unsigned char addr[ADDR_MAX_LINE];//长度由sa决定
	unsigned char CA;//客户机地址
};

struct _698_APDU
{
	unsigned char apdu_cmd;//APDU命令
	unsigned char apdu_data[APDU_MAX_LENTH];
};

//struct _698_BLE_GETREQUEST
//{
//	unsigned char apdu_attitude;//APUD数据属性
//	unsigned char apdu_piid;
//	
//	union 
//	{
//		unsigned char ucoad[4];//长度
//		unsigned long uload;
//	}apdu_oad;
//	
//	unsigned char apdu_data[APDU_USER_MAX_LENTH];
//};

struct _698_BLE_GETREQUEST_NORMAL
{
	unsigned char apdu_attitude;//APUD数据属性
	unsigned char apdu_piid;
	
	union 
	{
		unsigned char ucoad[4];//长度
		unsigned long uload;
	}apdu_oad;
	
	unsigned char time;
};

struct _698_BLE_METER_ADDR
{
	unsigned char	data;
	unsigned char data_type;
	unsigned char addr_len;
	unsigned char addr[6];
	unsigned char optional;
	unsigned char time;
};
 

struct _698_BLE_FRAME       //698协议结构
{
	unsigned char head;  //起始帧头 = 0x68
	union 
	{
		unsigned char uclenth[2];//长度
		unsigned int ullenth;
	}datalenth;//帧长度
	
	union 
	{
		unsigned char ucControl;//控制字
		struct{
		unsigned char ucFuncode:3;//bit 0~2
		unsigned char ucSC:1;
		unsigned char ucTemp:1;
		unsigned char ucFrame:1;
		unsigned char ucPRM:1;//数据方向
		unsigned char ucDIR:1;//数据方向
		}B;
	}control;//控制字

	struct _698_BLE_ADDR _698_ADDR;//地址域a
	
	union 
	{
		unsigned char ucHcs[2];//长度
		unsigned int ulHcs;
	}HCS;
	
	struct _698_APDU apdu;
	
	union 
	{
		unsigned char ucFcs[2];//长度
		unsigned int ulFcs;
	}FCS;
	unsigned char end;//结束字符 = 0x16	
};
//#pragma pack ()

//enum{
// BLUETOOTH_CALLNUM=0xA0,        	//0xA0						//读取桩编号
// BLUETOOTH_SETNUM,     			//0xA1           //设置桩编号
// BLUETOOTH_SETPASSWORD,    	//0xA2           //设置充电密码
// BLUETOOTH_CHARGECHECK,    	//0xA3           //启动充电验证
// BLUETOOTH_STARTCHARGE,	    //0xA4           //启动充电
// BLUETOOTH_STOPCHARGE,	    //0xA5           //停止充电
// BLUETOOTH_REALDATA,		    //0xA6           //实时数据
// BLUETOOTH_CHARGERECOARD,   //0xA7            //充电记录
// BLUETOOTH_SETPOWER,		    //0xA8            //设置功率
// BLUETOOTH_SETLOCK,			    //0xA9            //电子锁控制
// BLUETOOTH_SETCLOCK,		    //0xAA            //对时
// BLUETOOTH_SETNAME,			    //0xAB            //设置蓝牙名称
// BLUETOOTH_RESERCHARGE,	    //0xAC            //预约充电
// BLUETOOTH_READVERSION,	    //0xAD            //读取程序版本号
// BLUETOOTH_BEAT,				    //0xAE            //连接心跳
// BLUETOOTH_READPASSWORD,    //0xAF            //读取充电密码
// BLUETOOTH_TIMESTARTCHARGE,	//0x81            //定时充电
// BLUETOOTH_TIMESTOPCHARGE,  //0x82            //定时停止充电
// BLUETOOTH_UPDATESOFT,     //0xC1            //更新程序
// BLUETOOTH_RESETWIFI,		    //0xC5            //wifi模块恢复出厂默认
// BLUETOOTH_SETKC,    				//0xE0            //开出量控制
// BLUETOOTH_READRAJDATA,    	//0xE1            //读取校准系数
// BLUETOOTH_SETRAJDATA,	    //0xE2            //设置校准系数
// BLUETOOTH_CLEARNUM,       //0xE3            //清除桩号
// BLUETOOTH_SETMDINFO,    		//0xE4            //设置出厂信息
// BLUETOOTH_SETGPRSIP,		    //0xE5						//设置无线模块IP
// BLUETOOTH_READGPRSIP,      //0xE6            //读取无线模块IP
// BLUETOOTH_SETJFMOD,		    //0xE7            //设置计费模型
// BLUETOOTH_RESETBLU,		    //0xE8            //蓝牙模块恢复出厂默认
// BLUETOOTH_SETPARKLOCK,     //0xE9            //车位锁控制
// BLUETOOTH_SETWHITEID,    	//0xEB            //设置白名单
// BLUETOOTH_DEBUG,				    //0xEF            //进入DEBUG模块
//}BLUETOOTH_CMD;


/////**********内部函数****************************************/
//unsigned char BLEStateUpdate(rt_device_t dev);

//void BLE_ProtocalRec(rt_device_t dev);

//rt_uint8_t SendBLUETOOTH_Frame(rt_device_t dev,rt_uint32_t cmd,rt_uint8_t reason);

//extern u8 SendMR_Frame(STR_UART_MSG_CTRL *pUart,u32 cmd);
//extern u8 MeterModbusStateUpdate(u8 *pArray,u8 ArrayLen,u32 cmd);
//extern void MeterModbus_RecProtocal(void);


#endif

