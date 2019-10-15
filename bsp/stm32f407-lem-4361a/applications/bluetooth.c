#include <rtthread.h>
#include <bluetooth.h>
#include <string.h>
#include <stdio.h>
#include <global.h>
//#include <meter.h>
//#include <analog.h>
#include <board.h>
#include <698.h>
#include <storage.h>
#include <esam.h>


#define FUNC_PRINT_RX	"[bluetooth]:RX:"
#define FUNC_PRINT_TX	"[bluetooth]:TX:"

#define BLE_PIN    GET_PIN(E, 5)

#define BLE_PWR_ON()	{rt_pin_write(BLE_PIN, PIN_LOW);}//模块上电
#define BLE_PWR_OFF()	{rt_pin_write(BLE_PIN, PIN_HIGH);}//模块掉电

/* UART3接收事件标志*/
#define UART3_RX_EVENT (1 << 3)



#define THREAD_BLUETOOTH_PRIORITY     23
#define THREAD_BLUETOOTH_STACK_SIZE   1024*2
#define THREAD_BLUETOOTH_TIMESLICE    5

static struct rt_thread bluetooth;
static rt_device_t bluetooth_serial;
static rt_uint8_t bluetooth_stack[THREAD_BLUETOOTH_STACK_SIZE];//线程堆栈


char* AT_CmdDef[]={

	"ATE0\r\n",		//关闭回显功能
	"AT+BLEINIT=2\r\n",			//BLE 初始化，设置为Server模式
//	"AT+BLENAME=\"LN000000000001\"\r\n",	//设置 BLE 设备名称
	"AT+BLENAME=\"[NR000000000001]\"\r\n",	//设置 BLE 设备名称
	"AT+BLEADDR=1,\"f1:f2:f3:f4:f5:f6\"\r\n",
	
	"AT+BLEGATTSSRVCRE\r\n",	//创建GATTS 服务
	"AT+BLEGATTSSRVSTART\r\n",	//开启GATTS 服务
	
	"AT+BLEADVPARAM=32,64,0,1,7\r\n",							//配置广播参数
//	"AT+BLEADVDATA=\"0201060F094C4E3030303030303030303030310303E0FF\"\r\n",//配置扫描响应数据
	"AT+BLEADVDATA=\"02010611095B4E523030303030303030303030315D0303E0FF\"\r\n",//配置扫描响应数据		
	"AT+BLEADVSTART\r\n",		//开始广播
	
	"AT+BLESPPCFG=1,1,1,1,1\r\n",	//配置BLE透传模式
	"AT+BLESPP\r\n",				//开启透传模式
	
	"+++",						//退出透传模式
	"AT+BLEDISCONN\r\n",	//断开连接
	
	"AT+RST\r\n",	//重启模块

};
typedef enum 		 //AT指令
{
	BLE_ATE = 0,
	BLE_INIT,
	BLE_NAME,
	BLE_ADDR_SET,
	
	BLE_GATTS_SRV,
	BLE_GATTS_START,
	
	BLE_ADV_PARAM,
	BLE_ADV_DATA,
	BLE_ADV_START,
	
	BLE_SPP_CFG,
	BLE_SPP,
	
	BLE_QUIT_TRANS,
	BLE_DISCONN,

	BLE_RESET,
	
	BLE_NULL,
}BLE_AT_CMD;

typedef enum 		 //后台连接数据类型
{
	AT_MODE = 1,
	TRANS_MODE,
}PROTOCOL_MODE;

static BLE_AT_CMD BLE_ATCmd;
static BLE_AT_CMD BLE_ATCmd_Old;
static rt_uint8_t BLE_ATCmd_Count;
static PROTOCOL_MODE g_ucProtocol;

static struct rt_event bluetooth_event;//用于接收数据的信号量

CCMRAM static ScmUart_Comm stBLE_Comm;
CCMRAM static ScmEsam_Comm stBLE_Esam_Comm;
CCMRAM static rt_uint8_t BLE_698_data_buf[4][255];


CCMRAM CHARGE_APPLY stBLE_Charge_Apply;


static rt_uint8_t	Esam_KEY_R1[16];//R1数据
static rt_uint8_t	Esam_KEY_R2[16];//R2数据
static rt_uint8_t	Esam_KEY_R3[16];//R3数据
static rt_uint8_t	Esam_KEY_DATA[32];//DATA2数据


rt_uint8_t BLE_698_Get_Addr[]={0x68,0x17,0x00,0x43,0x45,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x10,0xDA,0x5F,0x05,0x01,0x00,0x40,0x01,0x02,0x00,0x00,0xED,0x03,0x16};


static rt_uint32_t g_ulBLE_Rx_Count;
static rt_uint32_t g_ulBLE_Tx_Count;
static rt_uint32_t g_ulBLE_Rx_Beg;
static rt_uint32_t g_ulBLE_Rx_Ptr;
static rt_uint32_t g_ulBLE_Rx_Pre;

static rt_uint32_t g_ulBLE_RX_Write;
static rt_uint32_t g_ulBLE_RX_Read;


//static rt_uint32_t g_ulRx_Size;
static rt_uint8_t g_ucRecv698_In_AT;//在AT指令模式下  收到了698协议数据
	
static rt_uint32_t g_BLE_Strategy_event;

struct _698_BLE_FRAME _698_ble_frame;

static rt_err_t BLE_Check_Data_to_Buf(ScmUart_Comm* stData);

extern int _698_HCS(unsigned char *data, int start_size,int size,unsigned short HCS);
extern int _698_FCS(unsigned char *data, int start_size,int size,unsigned short FCS);
extern int tryfcs16(unsigned char *cp, int len);

static void BLE_Trans_Send(rt_device_t dev,rt_uint32_t cmd,rt_uint8_t reason,ScmUart_Comm* stData);

rt_err_t BLE_698_Data_Analysis_and_Response(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData);

rt_uint8_t BLE_strategy_event_send(COMM_CMD_C cmd);//发送事件到策略
rt_uint32_t BLE_strategy_event_get(void);
rt_uint8_t BLE_strategy_event_send(COMM_CMD_C cmd);//发送事件到策略

/********************************************************************  
*	函 数 名: Uart_Recv_Data
*	功能说明: 串口数据接收
*	形    参: 无
*	返 回 值: 无
********************************************************************/
void BLE_Commit_TimeOut(void)
{
	g_ulBLE_Rx_Count++;
	g_ulBLE_Tx_Count++;
	
	if((g_ulBLE_Rx_Count>60)||(g_ulBLE_Tx_Count>60))
	{
		g_ulBLE_Rx_Count = 100;
		g_ulBLE_Tx_Count = 100;
		BLE_ATCmd = BLE_QUIT_TRANS;
		g_ucProtocol = AT_MODE;
	}
}

/********************************************************************  
*	函 数 名: Uart_Recv_Data
*	功能说明: 串口数据接收
*	形    参: 无
*	返 回 值: 无
********************************************************************/
rt_err_t BLE_Uart_Data_Recv(rt_device_t dev,ScmUart_Comm* stData,rt_uint32_t size)
{
//	rt_uint8_t i,rx_len,rx_ptr;
	
	if(dev == RT_NULL)
		return RT_ERROR;
	
	if((g_ulBLE_Rx_Ptr+size) >= 1024)
		g_ulBLE_Rx_Ptr = 0;
	
	rt_device_read(dev, 0, stData->Rx_data+g_ulBLE_Rx_Ptr, size);
	
	g_ulBLE_Rx_Ptr += size;
	
	stData->DataRx_len = size;
	
	g_ulBLE_Rx_Beg = 1;
	
	return RT_EOK;
}

/********************************************************************  
*	函 数 名: BLE_Send_AT_TimeOut
*	功能说明: AT指令配置超时处理函数
*	形    参: 无
*	返 回 值: 无
********************************************************************/

void BLE_ATCmd_TimeOut(BLE_AT_CMD at_cmd)
{
	if(BLE_ATCmd_Old == at_cmd)
	{
		BLE_ATCmd_Count++;
		if(BLE_ATCmd_Count>10)
		{
			BLE_PWR_ON();
			BLE_ATCmd = BLE_ATE;
			BLE_ATCmd_Old = BLE_NULL;
			BLE_ATCmd_Count = 0;
		}
		else if(BLE_ATCmd_Count>6)//蓝牙模块响应超时 掉电重启
		{
			BLE_PWR_OFF();	
		}
		rt_kprintf("[bluetooth]:ble_send repeate times is %d\n",BLE_ATCmd_Count);
	}
	else
	{
		BLE_ATCmd_Old = at_cmd;
		BLE_ATCmd_Count = 0;
	}
}

/********************************************************************  
*	函 数 名: BLE_ATCmd_Send
*	功能说明: AT指令配置函数
*	形    参: 无
*	返 回 值: 无
********************************************************************/

void BLE_ATCmd_Send(rt_device_t dev,BLE_AT_CMD at_cmd)
{
	rt_size_t size;
	
	if((dev == RT_NULL)||(at_cmd == BLE_NULL))
		return;
	
	size = rt_device_write(dev, 0, AT_CmdDef[at_cmd], strlen(AT_CmdDef[at_cmd]));
	
	if(size == strlen(AT_CmdDef[at_cmd]))
	{
		rt_kprintf("[bluetooth]:ble_send: at_cmd is %s\n",AT_CmdDef[at_cmd]);
	}
	
	BLE_ATCmd_TimeOut(at_cmd);//AT指令发送超时处理
}
/********************************************************************  
*	函 数 名: BLE_ATCmd_Recv
*	功能说明: AT指令配置函数
*	形    参: 无
*	返 回 值: 无
********************************************************************/

void BLE_ATCmd_Recv(rt_device_t dev,BLE_AT_CMD at_cmd,ScmUart_Comm* stData)//AT指令接收处理
{
	rt_uint32_t i;
	
	if(dev == RT_NULL)
		return;
	
	if(stBLE_Comm.DataRx_len)
	{
		my_printf((char*)stBLE_Comm.Rx_data,stBLE_Comm.DataRx_len,MY_CHAR,1,FUNC_PRINT_RX);
	}
	
	if(at_cmd == BLE_NULL)//发送完 cfg 进入null 等待APP连接
	{
		if((strstr((char*)(stData->Rx_data),"+BLECONN"))||(strstr((char*)(stData->Rx_data),"+WRITE")))   //app已连接打开透传
		{				
			BLE_ATCmd = BLE_SPP;
			rt_thread_mdelay(1000);//发送  spp之前 等待2s
		}
		if(strstr((char*)(stData->Rx_data),"+BLEDISCONN"))//app断开连接
		{
			BLE_ATCmd = BLE_QUIT_TRANS;
			g_ucProtocol = AT_MODE;
		}
	}
	else
	{
		if(strstr((char*)(stData->Rx_data),"OK"))
		{
			if(BLE_SPP_CFG == BLE_ATCmd)   //设置cfg成功 进入等待
			{
				BLE_ATCmd =BLE_NULL;
			}
			else if(BLE_ATCmd == BLE_SPP)//打开透传 成功  进入透传模式
			{
				BLE_ATCmd = BLE_NULL;
				g_ucProtocol = TRANS_MODE;
				g_ucRecv698_In_AT = 1;//收到698协议数据
			}
			else
				BLE_ATCmd++;
		}
	}
	
	if(strstr((char*)(stData->Rx_data),"WRITE"))//进入透传模式之前 收到698 协议数据
	{
		for(i = 0; i < stData->DataRx_len; i++)
		{
			if((stData->Rx_data[i] == 0x68)&&(stData->Rx_data[i+1] == 0x17))
			{
				g_ucRecv698_In_AT = 1;//收到698协议数据
			}
		}
	}
	memset(stData->Rx_data,0,stData->DataRx_len);
	stData->DataRx_len = 0;
	g_ulBLE_Rx_Ptr = 0;
	g_ulBLE_Rx_Pre = 0;
	g_ulBLE_Rx_Beg = 0;
}


rt_uint32_t BLE_698_Data_Package(struct _698_BLE_FRAME *dev_recv,rt_uint8_t user_data_len,ScmUart_Comm* stData)
{
//		rt_uint8_t i,lenth,size;
		rt_uint16_t total_lenth;
	
//		stData->Tx_data[0] 							= dev_recv->head;
//		stData->Tx_data[1] 							= dev_recv->datalenth.uclenth[0];
//		stData->Tx_data[2] 							= dev_recv->datalenth.uclenth[1];
//		
//		stData->Tx_data[3] 							= dev_recv->control.ucControl|0x80;
//		stData->Tx_data[4] 							= dev_recv->_698_ADDR.S_ADDR.SA;
//	
//		lenth = dev_recv->_698_ADDR.S_ADDR.B.uclenth+1;
//		for(i=0;i<lenth;i++)
//			stData->Tx_data[5+i] 					= dev_recv->_698_ADDR.addr[i];
//		stData->Tx_data[5+lenth]				= dev_recv->_698_ADDR.CA;
//			
//		stData->Tx_data[8+lenth]				= dev_recv->user_data.apdu_cmd|0x80;
//		stData->Tx_data[9+lenth]				= dev_recv->user_data.apdu_cmd_type;
//		stData->Tx_data[10+lenth]				= dev_recv->user_data.apdu_piid;
//		stData->Tx_data[11+lenth]				= dev_recv->user_data.apdu_oad.ucoad[3];
//		stData->Tx_data[12+lenth]				= dev_recv->user_data.apdu_oad.ucoad[2];
//		stData->Tx_data[13+lenth]				= dev_recv->user_data.apdu_oad.ucoad[1];
//		stData->Tx_data[14+lenth]				= dev_recv->user_data.apdu_oad.ucoad[0];
//		
//		for(i=0;i<user_data_len;i++)
//		{
//			stData->Tx_data[15+i+lenth] 	= dev_recv->user_data.apdu_usrdata[i];
//		}
//		
//		total_lenth = 16+lenth+user_data_len;
//		
//		stData->Tx_data[1] = total_lenth&0xff;
//		stData->Tx_data[2] = (total_lenth>>8)&0xff;
//		
//		tryfcs16(stData->Tx_data,lenth+6);
//		tryfcs16(stData->Tx_data,total_lenth-1);

//		stData->Tx_data[total_lenth+1] = dev_recv->end;
		
//		total_lenth += 2;
		
		return total_lenth;
}

/********************************************************************  
*	函 数 名: BLE_698_Get_METER_ADDR_Package
*	功能说明: 698请求电表地址组帧
*	形    参: 无
*	返 回 值: 无
********************************************************************/

rt_err_t BLE_698_Get_METER_ADDR_Package(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	rt_uint8_t i,lenth;
	rt_uint16_t total_lenth;
	
	struct _698_BLE_METER_ADDR meter_addr;
	
	
	GetStorageData(Cmd_MeterNumRd,meter_addr.addr,6);
	////////////////////////////////测试用///////////////////////////////
	meter_addr.data = 0x01;
	meter_addr.data_type=0x09;
	meter_addr.addr_len = 0x06;
	meter_addr.addr[0] = 0x00;
	meter_addr.addr[1] = 0x00;
	meter_addr.addr[2] = 0x00;
	meter_addr.addr[3] = 0x00;
	meter_addr.addr[4] = 0x00;
	meter_addr.addr[5] = 0x01;
	meter_addr.optional = 0x00;
	meter_addr.time = 0x00;
	//////////////////////////////////////////////////////////////////////
	

	stData->Tx_data[0] 							= dev_recv->head;
	stData->Tx_data[1] 							= dev_recv->datalenth.uclenth[0];
	stData->Tx_data[2] 							= dev_recv->datalenth.uclenth[1];
	
	stData->Tx_data[3] 							= dev_recv->control.ucControl|0x80;
	stData->Tx_data[4] 							= dev_recv->_698_ADDR.S_ADDR.SA;

	lenth = dev_recv->_698_ADDR.S_ADDR.B.uclenth+1;
	for(i=0;i<lenth;i++)
		stData->Tx_data[5+i] 					= dev_recv->_698_ADDR.addr[i];
	stData->Tx_data[5+lenth]				= dev_recv->_698_ADDR.CA;
	stData->Tx_data[6+lenth]				= dev_recv->HCS.ucHcs[0];
	stData->Tx_data[7+lenth]				= dev_recv->HCS.ucHcs[1];
	
		
	stData->Tx_data[8+lenth]				= dev_recv->apdu.apdu_cmd|0x80;
	
	stData->Tx_data[9+lenth]				= dev_recv->apdu.apdu_data[0];
	stData->Tx_data[10+lenth]				= dev_recv->apdu.apdu_data[1];
	stData->Tx_data[11+lenth]				= dev_recv->apdu.apdu_data[2];
	stData->Tx_data[12+lenth]				= dev_recv->apdu.apdu_data[3];
	stData->Tx_data[13+lenth]				= dev_recv->apdu.apdu_data[4];
	stData->Tx_data[14+lenth]				= dev_recv->apdu.apdu_data[5];
	
	stData->Tx_data[15+lenth]				= meter_addr.data;
	stData->Tx_data[16+lenth]				= meter_addr.data_type;
	stData->Tx_data[17+lenth]				= meter_addr.addr_len;
	stData->Tx_data[18+lenth]				= meter_addr.addr[0];
	stData->Tx_data[19+lenth]				= meter_addr.addr[1];
	stData->Tx_data[20+lenth]				= meter_addr.addr[2];
	stData->Tx_data[21+lenth]				= meter_addr.addr[3];
	stData->Tx_data[22+lenth]				= meter_addr.addr[4];
	stData->Tx_data[23+lenth]				= meter_addr.addr[5];
	stData->Tx_data[24+lenth]				= meter_addr.optional;
	stData->Tx_data[25+lenth]				= meter_addr.time;
		

	total_lenth = 27+lenth;
		
	stData->Tx_data[1] = total_lenth&0xff;
	stData->Tx_data[2] = (total_lenth>>8)&0xff;
		
	tryfcs16(stData->Tx_data,lenth+6);
	tryfcs16(stData->Tx_data,total_lenth-1);

	stData->Tx_data[total_lenth+1] = dev_recv->end;
	
	stData->DataTx_len = total_lenth+2;
	
	return RT_EOK;
}

/********************************************************************  
*	函 数 名: BLE_698_Data_UnPackage
*	功能说明: 698数据 解包
*	形    参: 无
*	返 回 值: 无
********************************************************************/

rt_err_t BLE_698_Data_UnPackage(struct _698_BLE_FRAME *dev_recv,rt_uint8_t* buf)
{
	rt_uint32_t i,addr_lenth,apdu_lenth,total_lenth;
	
	if(buf[0] != 0x68)//数据头不正确  返回解析失败
		return RT_ERROR;
	
	dev_recv->head 												= buf[0];	//帧头
	dev_recv->datalenth.uclenth[0] 				= buf[1];
	dev_recv->datalenth.uclenth[1] 				= buf[2];	//数据长度
	dev_recv->control.ucControl 					= buf[3];	//控制字
	dev_recv->_698_ADDR.S_ADDR.SA 				= buf[4];	//服务器地址
	
	addr_lenth = dev_recv->_698_ADDR.S_ADDR.B.uclenth+1;			//地址域长度
	total_lenth = dev_recv->datalenth.ullenth+2;
	apdu_lenth = total_lenth-addr_lenth-12;

	for(i = 0;i<addr_lenth;i++)
		dev_recv->_698_ADDR.addr[i]					= buf[5+i];
	
	dev_recv->_698_ADDR.CA 								= buf[5+addr_lenth];
	dev_recv->HCS.ucHcs[0] 								= buf[6+addr_lenth];
	dev_recv->HCS.ucHcs[1] 								= buf[7+addr_lenth];
	
	dev_recv->apdu.apdu_cmd								= buf[8+addr_lenth];
	
	for(i=0;i<apdu_lenth;i++)
	{
		dev_recv->apdu.apdu_data[i] 				= buf[9+i+addr_lenth];
	}

	dev_recv->FCS.ucFcs[0] 								= buf[total_lenth-3];
	dev_recv->FCS.ucFcs[1] 								= buf[total_lenth-2];
	dev_recv->end 												= buf[total_lenth-1];
	
	
//	my_printf((char*)buf,total_lenth,MY_HEX,1,FUNC_PRINT_RX);
	
	return RT_EOK;//数据解析完毕
}
/********************************************************************  
*	函 数 名: BLE_698_Get_Request_Normal_Analysis
*	功能说明: 698 get request 解析
*	形    参: 无
*	返 回 值: 无
********************************************************************/
rt_err_t BLE_698_Get_Request_Normal_Analysis(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	rt_uint32_t apdu_oad;
	
	apdu_oad = dev_recv->apdu.apdu_data[2];
	apdu_oad = apdu_oad<<8|dev_recv->apdu.apdu_data[3];
	apdu_oad = apdu_oad<<8|dev_recv->apdu.apdu_data[4];
	apdu_oad = apdu_oad<<8|dev_recv->apdu.apdu_data[5];
	
	switch(apdu_oad)
	{
		case 0x40010200:				//会话协商
		{
			BLE_698_Get_METER_ADDR_Package(dev_recv,stData);
			break;
		}
	}
	return RT_EOK;		
}

/********************************************************************  
*	函 数 名: BLE_698_Get_Request_Analysis_and_Response
*	功能说明: 698 get request 解析
*	形    参: 无
*	返 回 值: 无
********************************************************************/

rt_err_t BLE_698_Get_Request_Analysis_and_Response(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	rt_uint8_t apdu_attitude;
	
	apdu_attitude 	= dev_recv->apdu.apdu_data[0];
	switch(apdu_attitude)
	{
		case GET_REQUEST_NOMAL:
		{
			BLE_698_Get_Request_Normal_Analysis(dev_recv,stData);
			break;
		}
	}
	return RT_EOK;	
}
/********************************************************************  
*	函 数 名: BLE_698_Security_Request_PlainText_Analysis
*	功能说明: 698 Security Request 明文解析
*	形    参: 无
*	返 回 值: 无
********************************************************************/
rt_err_t BLE_698_Security_Request_PlainText_Analysis(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	return RT_EOK;
}

/********************************************************************  
*	函 数 名: BLE_698_Security_Request_PlainText_Analysis
*	功能说明: 698 Security Request 密文解析
*	形    参: 无
*	返 回 值: 无
********************************************************************/
rt_err_t BLE_698_Security_Request_CipherText_Analysis(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	rt_uint32_t cipherdata_lenth,appenddata_lenth,lenth;
	rt_uint8_t i,ptr;
//	rt_uint32_t apdu_oad;
	
	ptr = 0;
	cipherdata_lenth = dev_recv->apdu.apdu_data[1];

	memcpy(&stBLE_Esam_Comm.Tx_data[ptr],Esam_KEY_DATA,32);
	
	ptr+=32;
	
	for(i=0;i<cipherdata_lenth;i++)
	{
		stBLE_Esam_Comm.Tx_data[ptr++] = dev_recv->apdu.apdu_data[2+i];
	}
	stBLE_Esam_Comm.DataTx_len = ptr;
	

	if(ESAM_Communicattion(APP_SESS_VERI_MAC,&stBLE_Esam_Comm) == RT_EOK)//解密验证mac
	{
		if((stBLE_Esam_Comm.Rx_data[0] == 0x90)&&(stBLE_Esam_Comm.Rx_data[1] == 0x00))
		{			
				memcpy(&dev_recv->apdu.apdu_cmd,&stBLE_Esam_Comm.Rx_data[4],stBLE_Esam_Comm.DataRx_len-5);
				BLE_698_Data_Analysis_and_Response(dev_recv,stData);
		}
		else
		{
			rt_kprintf("[bluetooth]:esam return err,errcode is 0x%X%X!\n",stBLE_Esam_Comm.Rx_data[0],stBLE_Esam_Comm.Rx_data[1]);
		}
	}

	return RT_EOK;		
}
/********************************************************************  
*	函 数 名: BLE_698_Security_Request_Analysis_and_Response
*	功能说明: 698 Security Request 解析
*	形    参: 无
*	返 回 值: 无
********************************************************************/
rt_err_t BLE_698_Security_Request_Analysis_and_Response(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	rt_uint8_t apdu_attitude;
	
	apdu_attitude 	= dev_recv->apdu.apdu_data[0];
	switch(apdu_attitude)
	{
		case PLAINTEXT://明文
		{
			BLE_698_Get_Request_Normal_Analysis(dev_recv,stData);
			break;
		}
		case CIPHERTEXT://密文
		{
			BLE_698_Security_Request_CipherText_Analysis(dev_recv,stData);
			break;
		}
	}
	return RT_EOK;
}







/********************************************************************  
*	函 数 名: BLE_698_ESAM_SESS_INTI_Package
*	功能说明: 698请求电表地址组帧
*	形    参: 无
*	返 回 值: 无
********************************************************************/

rt_err_t BLE_698_ESAM_SESS_INTI_Package(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	rt_uint8_t	Esam_Version[5];//ESAM 版本号
//	rt_uint8_t	Esam_KEY_R2[16];//R2数据
	rt_uint8_t	Esam_KEY_DATA2[16];//DATA2数据
	rt_uint8_t 	i,ptr,lenth,total_lenth;
	rt_uint16_t Esam_endata_len;
	
	
	stBLE_Esam_Comm.DataTx_len = 0;
	ESAM_Communicattion(RD_INFO_01,&stBLE_Esam_Comm);
	if((stBLE_Esam_Comm.Rx_data[0] == 0x90)&&(stBLE_Esam_Comm.Rx_data[1] == 0x00))
	{
		memcpy(Esam_Version,&stBLE_Esam_Comm.Rx_data[4],5);   //DATA1
	}
	else
	{
		rt_kprintf("[bluetooth]:(%s) read esam info fail,errcode =0x%X%X!\n",__func__,stBLE_Esam_Comm.Rx_data[0],stBLE_Esam_Comm.Rx_data[1]);
		return RT_ERROR;
	}
	
	ESAM_Communicattion(APP_KEY_AGREE_ONE,&stBLE_Esam_Comm);
	if((stBLE_Esam_Comm.Rx_data[0] == 0x90)&&(stBLE_Esam_Comm.Rx_data[1] == 0x00))
	{
		memcpy(Esam_KEY_R2,&stBLE_Esam_Comm.Rx_data[4],16);
	}
	else
	{
		rt_kprintf("[bluetooth]:(%s) read esam r2 fail,errcode =0x%X%X!",__func__,stBLE_Esam_Comm.Rx_data[0],stBLE_Esam_Comm.Rx_data[1]);
		return RT_ERROR;
	}
	
	
	memcpy(stBLE_Esam_Comm.Tx_data,&dev_recv->apdu.apdu_data[11],16);//R1
	memcpy(Esam_KEY_R1,&dev_recv->apdu.apdu_data[11],16);
	memcpy(stBLE_Esam_Comm.Tx_data+16,Esam_Version,5);
	stBLE_Esam_Comm.DataTx_len = 21;
	
	ESAM_Communicattion(APP_KEY_AGREE_TWO,&stBLE_Esam_Comm);//R1+DATA1 
	if((stBLE_Esam_Comm.Rx_data[0] == 0x90)&&(stBLE_Esam_Comm.Rx_data[1] == 0x00))
	{
		memcpy(Esam_KEY_DATA2,&stBLE_Esam_Comm.Rx_data[4],16);
	}
	else
	{
		rt_kprintf("[bluetooth]:(%s) read esam data2 fail,errcode =0x%X%X!\n",__func__,stBLE_Esam_Comm.Rx_data[0],stBLE_Esam_Comm.Rx_data[1]);
		return RT_ERROR;
	}
	
	memcpy(stBLE_Esam_Comm.Tx_data,Esam_KEY_DATA2,16);
	memcpy(stBLE_Esam_Comm.Tx_data+16,&dev_recv->apdu.apdu_data[11],16);//R1
	memcpy(stBLE_Esam_Comm.Tx_data+32,Esam_KEY_R2,16);
	stBLE_Esam_Comm.DataTx_len = 48;
	
	ESAM_Communicattion(APP_KEY_AGREE_THREE,&stBLE_Esam_Comm); //DATA2+R1+R2
	if((stBLE_Esam_Comm.Rx_data[0] == 0x90)&&(stBLE_Esam_Comm.Rx_data[1] == 0x00))
	{
			Esam_endata_len = stBLE_Esam_Comm.Rx_data[2];
			Esam_endata_len = (rt_uint16_t)(((Esam_endata_len<<8)&0xff00)|(stBLE_Esam_Comm.Rx_data[3]));
		
			ptr = 0;
		
			stData->Tx_data[ptr++] 							= dev_recv->head;
			stData->Tx_data[ptr++] 							= dev_recv->datalenth.uclenth[0];
			stData->Tx_data[ptr++] 							= dev_recv->datalenth.uclenth[1];
			
			stData->Tx_data[ptr++] 							= dev_recv->control.ucControl|0x80;
			stData->Tx_data[ptr++] 							= dev_recv->_698_ADDR.S_ADDR.SA;

			lenth = dev_recv->_698_ADDR.S_ADDR.B.uclenth+1;
			for(i=0;i<lenth;i++)
				stData->Tx_data[ptr++] 					= dev_recv->_698_ADDR.addr[i];
			stData->Tx_data[ptr++]				= dev_recv->_698_ADDR.CA;
			stData->Tx_data[ptr++]				= dev_recv->HCS.ucHcs[0];
			stData->Tx_data[ptr++]				= dev_recv->HCS.ucHcs[1];
			
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_cmd|0x80;
			
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[0];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[1];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[2];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[3];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[4];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[5];
			
			stData->Tx_data[ptr++]				= 0;//成功
			stData->Tx_data[ptr++]				= 1;//optional

			stData->Tx_data[ptr++]				= Data_octet_string;//数据类型
			stData->Tx_data[ptr++]				= Esam_endata_len+2;
			
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[8];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[9];//版本号
			
			for(i = 0;i < Esam_endata_len;i++)
			{
				stData->Tx_data[ptr++]			= stBLE_Esam_Comm.Rx_data[4+i];
			}	
			stData->Tx_data[ptr++]				= 0;//无时间标签
			
			total_lenth = ptr+2;
				
			stData->Tx_data[1] = total_lenth&0xff;
			stData->Tx_data[2] = (total_lenth>>8)&0xff;
				
			tryfcs16(stData->Tx_data,lenth+6);
			tryfcs16(stData->Tx_data,total_lenth-1);

			stData->Tx_data[total_lenth+1] = dev_recv->end;
			
			stData->DataTx_len = total_lenth+2;
	}
	else
	{
		rt_kprintf("[bluetooth]:(%s) read endata1 fail,errcode =0x%X%X!\n",__func__,stBLE_Esam_Comm.Rx_data[0],stBLE_Esam_Comm.Rx_data[1]);
		return RT_ERROR;
	}
}

/********************************************************************  
*	函 数 名: BLE_698_ESAM_SESS_INTI_Package
*	功能说明: 698请求电表地址组帧
*	形    参: 无
*	返 回 值: 无
********************************************************************/

rt_err_t BLE_698_ESAM_SESS_KEY_Package(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	rt_uint8_t 	i,ptr,lenth,total_lenth;
	rt_uint16_t Esam_Keydata_len;
	
	
	Esam_Keydata_len = dev_recv->apdu.apdu_data[7]-2;
	
	memcpy(stBLE_Esam_Comm.Tx_data,Esam_KEY_R1,16);
	memcpy(stBLE_Esam_Comm.Tx_data+16,Esam_KEY_R2,16);
	stBLE_Esam_Comm.DataTx_len = 32;
	
	ESAM_Communicattion(APP_SESS_AGREE_ONE,&stBLE_Esam_Comm);
	if((stBLE_Esam_Comm.Rx_data[0] == 0x90)&&(stBLE_Esam_Comm.Rx_data[1] == 0x00))
	{
		memcpy(Esam_KEY_DATA,&stBLE_Esam_Comm.Rx_data[4],32);   //KEY DATA
	}
	else
	{
		rt_kprintf("[bluetooth]:(%s) sess agree state one fail,errcode =0x%X%X!\n",__func__,stBLE_Esam_Comm.Rx_data[0],stBLE_Esam_Comm.Rx_data[1]);
		return RT_ERROR;
	}
	
	memcpy(stBLE_Esam_Comm.Tx_data,Esam_KEY_DATA,32);
	memcpy(stBLE_Esam_Comm.Tx_data+32,&dev_recv->apdu.apdu_data[10],Esam_Keydata_len);
	stBLE_Esam_Comm.DataTx_len = 32+Esam_Keydata_len;
	
	ESAM_Communicattion(APP_SESS_AGREE_TWO,&stBLE_Esam_Comm);
	if((stBLE_Esam_Comm.Rx_data[0] == 0x90)&&(stBLE_Esam_Comm.Rx_data[1] == 0x00))
	{
		memcpy(Esam_KEY_R3,&stBLE_Esam_Comm.Rx_data[4],16);   //KEY DATA
		
		if(memcmp(Esam_KEY_R3,Esam_KEY_R1,16) == 0)
		{
			ptr = 0;
			stData->Tx_data[ptr++] 				= dev_recv->head;
			stData->Tx_data[ptr++] 				= dev_recv->datalenth.uclenth[0];
			stData->Tx_data[ptr++] 				= dev_recv->datalenth.uclenth[1];
			
			stData->Tx_data[ptr++] 				= dev_recv->control.ucControl|0x80;
			stData->Tx_data[ptr++] 				= dev_recv->_698_ADDR.S_ADDR.SA;

			lenth = dev_recv->_698_ADDR.S_ADDR.B.uclenth+1;
			for(i=0;i<lenth;i++)
				stData->Tx_data[ptr++] 			= dev_recv->_698_ADDR.addr[i];
			stData->Tx_data[ptr++]				= dev_recv->_698_ADDR.CA;
			stData->Tx_data[ptr++]				= dev_recv->HCS.ucHcs[0];
			stData->Tx_data[ptr++]				= dev_recv->HCS.ucHcs[1];
			
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_cmd|0x80;
			
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[0];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[1];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[2];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[3];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[4];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[5];
			
			stData->Tx_data[ptr++]				= 0;//成功
			stData->Tx_data[ptr++]				= 1;//optional

			stData->Tx_data[ptr++]				= Data_octet_string;//数据类型
			stData->Tx_data[ptr++]				= 4;
			
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[4];
			stData->Tx_data[ptr++]				= dev_recv->apdu.apdu_data[5];//版本号
			
			stData->Tx_data[ptr++]				= 0;
			stData->Tx_data[ptr++]				= 0;//错误代码 0000 正确
	
			stData->Tx_data[ptr++]				= 0;//无时间标签
			
			total_lenth = ptr+2;
				
			stData->Tx_data[1] = total_lenth&0xff;
			stData->Tx_data[2] = (total_lenth>>8)&0xff;
				
			tryfcs16(stData->Tx_data,lenth+6);
			tryfcs16(stData->Tx_data,total_lenth-1);

			stData->Tx_data[total_lenth+1] = dev_recv->end;
			
			stData->DataTx_len = total_lenth+2;
		}
	}
	else
	{
		rt_kprintf("[bluetooth]:%s sess agree state two fail,errcode =0x%X%X!\n",__func__,stBLE_Esam_Comm.Rx_data[0],stBLE_Esam_Comm.Rx_data[1]);
		return RT_ERROR;
	}
}

/********************************************************************  
*	函 数 名: BLE_698_Action_Request_Normal_Analysis
*	功能说明: 698 get request 解析
*	形    参: 无
*	返 回 值: 无
********************************************************************/
rt_err_t BLE_698_Action_Request_Normal_Charge_Appply(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	rt_uint8_t ptr,data_len;
	rt_uint32_t power;
	rt_uint16_t time;
	
	ptr = 0;
	data_len = dev_recv->apdu.apdu_data[10];//充电申请单号
	stBLE_Charge_Apply.cRequestNO[0] = data_len;
	memcpy(&stBLE_Charge_Apply.cRequestNO[1],&dev_recv->apdu.apdu_data[11],data_len);//充电申请单号
	
	rt_kprintf("[bluetooth]: (%s) RequestNO:");
	my_printf((char*)&stBLE_Charge_Apply.cRequestNO[1],data_len,MY_HEX,1," ");
	
	ptr += data_len+2;
	data_len = dev_recv->apdu.apdu_data[10+ptr];
	stBLE_Charge_Apply.cUserID[0] = data_len;
	memcpy(&stBLE_Charge_Apply.cUserID[1],&dev_recv->apdu.apdu_data[11+ptr],data_len);//用户ID
	
	ptr += data_len+2;
	data_len = dev_recv->apdu.apdu_data[10+ptr];
	stBLE_Charge_Apply.cAssetNO[0] = data_len;
	memcpy(&stBLE_Charge_Apply.cAssetNO[1],&dev_recv->apdu.apdu_data[11+ptr],data_len);//路由器资产编号
	
	ptr += data_len+1;
	data_len = 4;
	power = dev_recv->apdu.apdu_data[11+ptr];
	power = (power<<8) | dev_recv->apdu.apdu_data[12+ptr];
	power = (power<<8) | dev_recv->apdu.apdu_data[13+ptr];
	power = (power<<8) | dev_recv->apdu.apdu_data[14+ptr];//充电功率
	stBLE_Charge_Apply.ulChargeReqEle = power;

	ptr += data_len+1;
	data_len = 7;
	time = dev_recv->apdu.apdu_data[11+ptr];
	time = (time<<8)|dev_recv->apdu.apdu_data[12+ptr];
	time = time%100;
	Int_toBCD(&stBLE_Charge_Apply.PlanUnChg_TimeStamp.Year,(rt_uint8_t*)&time,1);
	Int_toBCD(&stBLE_Charge_Apply.PlanUnChg_TimeStamp.Month,&dev_recv->apdu.apdu_data[13+ptr],1);
	Int_toBCD(&stBLE_Charge_Apply.PlanUnChg_TimeStamp.Day,&dev_recv->apdu.apdu_data[14+ptr],1);
	Int_toBCD(&stBLE_Charge_Apply.PlanUnChg_TimeStamp.Hour,&dev_recv->apdu.apdu_data[15+ptr],1);
	Int_toBCD(&stBLE_Charge_Apply.PlanUnChg_TimeStamp.Minute,&dev_recv->apdu.apdu_data[16+ptr],1);
	Int_toBCD(&stBLE_Charge_Apply.PlanUnChg_TimeStamp.Second,&dev_recv->apdu.apdu_data[17+ptr],1);
		
	ptr += data_len+1;
	data_len = 1;
	stBLE_Charge_Apply.cUserID[0] = data_len;
	stBLE_Charge_Apply.ChargeMode = dev_recv->apdu.apdu_data[11+ptr];

	ptr += data_len+2;
	data_len = dev_recv->apdu.apdu_data[10+ptr];
	stBLE_Charge_Apply.Token[0] = data_len;
	memcpy(&stBLE_Charge_Apply.Token[1],&dev_recv->apdu.apdu_data[11+ptr],data_len);//登陆令牌
	
	
	
	BLE_strategy_event_send(Cmd_StartChg);
	

}


/********************************************************************  
*	函 数 名: BLE_698_Action_Request_Normal_Analysis
*	功能说明: 698 get request 解析
*	形    参: 无
*	返 回 值: 无
********************************************************************/
rt_err_t BLE_698_Action_Request_Normal_Analysis(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	rt_uint32_t apdu_oad;
	
	apdu_oad = dev_recv->apdu.apdu_data[2];
	apdu_oad = apdu_oad<<8|dev_recv->apdu.apdu_data[3];
	apdu_oad = apdu_oad<<8|dev_recv->apdu.apdu_data[4];
	apdu_oad = apdu_oad<<8|dev_recv->apdu.apdu_data[5];
	
	switch(apdu_oad)
	{
		case 0xF1000B00:				//
		{
			BLE_698_ESAM_SESS_INTI_Package(dev_recv,stData);
			break;
		}
		case 0xF1000C00:
		{
			BLE_698_ESAM_SESS_KEY_Package(dev_recv,stData);
			break;
		}
		case 0x90027F00:
		{
			BLE_698_Action_Request_Normal_Charge_Appply(dev_recv,stData);
			break;
		}
	}
	return RT_EOK;		
}

/********************************************************************  
*	函 数 名: BLE_698_Security_Request_Analysis_and_Response
*	功能说明: 698 Security Request 解析
*	形    参: 无
*	返 回 值: 无
********************************************************************/
rt_err_t BLE_698_Action_Request_Analysis_and_Response(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	rt_uint8_t apdu_attitude;
	
	apdu_attitude 	= dev_recv->apdu.apdu_data[0];
	switch(apdu_attitude)
	{	
		case ACTION_REQUEST_NOMAL:
		{
			BLE_698_Action_Request_Normal_Analysis(dev_recv,stData);
			break;
		}
	}
	return RT_EOK;
}

/********************************************************************  
*	函 数 名: BLE_698_Data_Analysis_and_Response
*	功能说明: 698 解析 返回
*	形    参: 无
*	返 回 值: 无
********************************************************************/

rt_err_t BLE_698_Data_Analysis_and_Response(struct _698_BLE_FRAME *dev_recv,ScmUart_Comm* stData)
{
	switch(dev_recv->apdu.apdu_cmd)
	{
		case GET_REQUEST:
		{
			BLE_698_Get_Request_Analysis_and_Response(dev_recv,stData);
			break;
		}
		case SECURITY_REQUEST:
		{
			BLE_698_Security_Request_Analysis_and_Response(dev_recv,stData);
			break;
		}
		case ACTION_REQUEST:
		{
			BLE_698_Action_Request_Analysis_and_Response(dev_recv,stData);
			break;
		}
	}
	
	if(stData->DataTx_len)
	{
		BLE_Trans_Send(bluetooth_serial,0,0,stData);
		memset(stData->Tx_data,0,stData->DataTx_len);
		memset(dev_recv,0,sizeof(struct _698_BLE_FRAME));
		stData->DataTx_len = 0;
	}
	return RT_EOK;
}

/********************************************************************  
*	函 数 名: BLE_Trans_Send
*	功能说明: 串口发送
*	形    参: 无
*	返 回 值: 无
********************************************************************/
void BLE_Trans_Send(rt_device_t dev,rt_uint32_t cmd,rt_uint8_t reason,ScmUart_Comm* stData)
{
	rt_uint8_t size;

	if(stData->DataTx_len)
	{
		my_printf((char*)stData->Tx_data,stData->DataTx_len,MY_HEX,1,FUNC_PRINT_TX);
		
		size = rt_device_write(dev, 0, stData->Tx_data, stData->DataTx_len);
							
//		if(size == stData->DataTx_len)
//		{
//			rt_kprintf("[bluetooth]:ble_send sucess!!!\n");
//		}
	}
}
/********************************************************************  
*	函 数 名: BLE_Trans_Recv
*	功能说明: 串口数据接收
*	形    参: 无
*	返 回 值: 无
********************************************************************/

void BLE_Trans_Recv(rt_device_t dev,BLE_AT_CMD at_cmd,ScmUart_Comm* stData)//AT指令接收处理
{

	my_printf((char*)stBLE_Comm.Rx_data+g_ulBLE_Rx_Pre,g_ulBLE_Rx_Ptr,MY_HEX,1,FUNC_PRINT_RX);

	if(BLE_Check_Data_to_Buf(stData) == RT_EOK)
	{
		while(g_ulBLE_RX_Write != g_ulBLE_RX_Read)
		{
			if(BLE_698_Data_UnPackage(&_698_ble_frame,BLE_698_data_buf[g_ulBLE_RX_Read]) == RT_EOK)
			{		
				BLE_698_Data_Analysis_and_Response(&_698_ble_frame,stData);
			}
			
			memset(BLE_698_data_buf[g_ulBLE_RX_Read],0,255);
			g_ulBLE_RX_Read++;
			if(g_ulBLE_RX_Read >= 4)
				g_ulBLE_RX_Read = 0;
		}		
	}
	
	
	if(strstr((char*)(stData->Rx_data),"+BLEDISCONN"))
	{
		BLE_ATCmd = BLE_QUIT_TRANS;
		g_ucProtocol = AT_MODE;
	}
	
	g_ulBLE_Rx_Beg = 0;
	g_ulBLE_Rx_Ptr = 0;
	g_ulBLE_Rx_Pre = 0;
}



//void BLE_SenData_Frame(rt_device_t dev,PROTOCOL_MODE protocol,BLE_AT_CMD at_cmd,ScmUart_Comm* stData)
//{
//	if(dev == RT_NULL)
//		return;
//	switch(protocol)
//	{
//		case AT_MODE:
//			BLE_ATCmd_Send(dev,at_cmd);
//			break;
//		case TRANS_MODE:
////			BLE_Trans_Send(dev,0,0,stData);
//			break;
//		default:
//			break;
//	}
//	g_ulBLE_Tx_Count = 0;
//}


/********************************************************************  
*	函 数 名: BLE_ATCmd_Recv
*	功能说明: AT指令配置函数
*	形    参: 无
*	返 回 值: 无
********************************************************************/

void BLE_RecvData_Process(rt_device_t dev,PROTOCOL_MODE protocol,BLE_AT_CMD at_cmd,ScmUart_Comm* stData)
{
	if(dev == RT_NULL)
		return;
	switch(protocol)
	{
		case AT_MODE:
			BLE_ATCmd_Recv(dev,at_cmd,stData);
			break;
		case TRANS_MODE:
			BLE_Trans_Recv(dev,at_cmd,stData);
			break;
		default:
			break;
	}
//	memset(stData->Rx_data,0,stData->DataRx_len);
//	stData->DataRx_len = 0;
}



static rt_err_t BLE_Check_Data_to_Buf(ScmUart_Comm* stData)
//static rt_err_t check_698_data_to_buf(ScmUart_Comm* stData,struct CharPointDataManage* data_rev)
{
	rt_uint32_t i,lenth,hcs_size;
	
	if(g_ucRecv698_In_AT == 1)
	{
		g_ucRecv698_In_AT = 0;		
		memcpy(&stData->Rx_data[g_ulBLE_Rx_Pre],BLE_698_Get_Addr,sizeof(BLE_698_Get_Addr));
		
		g_ulBLE_Rx_Ptr = sizeof(BLE_698_Get_Addr);
	}
	
	for(i = g_ulBLE_Rx_Pre; i < g_ulBLE_Rx_Ptr; i++)
	{
		if(i > 1023)
		{
			rt_kprintf("[bluetooth]:Recv buf too much,%s !\n",__func__);
			return RT_ERROR;
		}
		
		if(stData->Rx_data[i] == 0x68)
		{
			lenth = stData->Rx_data[i+2];
			lenth = (lenth<<8)|(stData->Rx_data[i+1]);
			if(stData->Rx_data[i+lenth+1] == 0x16)//698一帧数据  接受完整
			{
				hcs_size = (rt_uint32_t)((*(stData->Rx_data+i+4)&0x0f)+8);
				if((_698_HCS(stData->Rx_data,i+1,hcs_size,0) == 0)&&(_698_FCS(stData->Rx_data,i+1,lenth,0) == 0))
				{
					g_ulBLE_Rx_Pre = i+lenth+2;
					memcpy(BLE_698_data_buf[g_ulBLE_RX_Write],&stData->Rx_data[i],lenth+2);
					g_ulBLE_RX_Write++;
					if(g_ulBLE_RX_Write >= 4)
						g_ulBLE_RX_Write = 0;
//					return RT_EOK;
				}
			}
		}
	}
	
	if(g_ulBLE_RX_Write != g_ulBLE_RX_Read)
	{
		return RT_EOK;
	}
	else
		return RT_ERROR;
}

/******************************************与策略数据传递接口******************************************/

rt_uint8_t BLE_strategy_event_send(COMM_CMD_C cmd)//发送事件到策略
{
	rt_kprintf("[bluetooth]: (%s)   cmd=%d  \n",__func__,cmd);	
	g_BLE_Strategy_event=g_BLE_Strategy_event|cmd;
		
	return 0;	
}

rt_uint32_t BLE_strategy_event_get(void)
{
	rt_uint32_t result=CTRL_NO_EVENT;

	result=g_BLE_Strategy_event;
	g_BLE_Strategy_event=CTRL_NO_EVENT;//清除事件
		
	return result;
}

rt_uint8_t BLE_CtrlUnit_RecResp(COMM_CMD_C cmd,void *STR_SetPara,int count){
	rt_uint8_t result=1;
	rt_uint32_t event;

	event = cmd;

	switch(event){					
		case(Cmd_StartChg):	//启动充电 申请单  CHARGE_APPLY stBLE_Charge_Apply;
			rt_kprintf("[hplc] (%s) Cmd_StartChg \n",__func__);	
			STR_SetPara=(CHARGE_APPLY *)&stBLE_Charge_Apply;			
			result=0;										
		break;
		
		default:
			rt_kprintf("[hplc]  (%s)   not  \n",__func__);
			return 1;
			break;	
	}

	return result;			
}

/*************************************************************************************************************/



















static rt_err_t bluetooth_rx_ind(rt_device_t dev, rt_size_t size)
{
	BLE_Uart_Data_Recv(dev,&stBLE_Comm,size);
  return RT_EOK;
}

static void bluetooth_thread_entry(void *parabluetooth)
{
	rt_err_t res,result;
//	rt_uint32_t e;
	
	bluetooth_serial = rt_device_find(RT_BLUETOOTH_USART);
	
	if(bluetooth_serial != RT_NULL)
	{
		if (rt_device_open(bluetooth_serial, RT_DEVICE_FLAG_DMA_RX) == RT_EOK)
		{
			rt_lprintf("[bluetooth]:Open serial uart3 sucess!\r\n");
		}
	}
	else
	{
		res = RT_ERROR;
		rt_lprintf("[bluetooth]:Open serial uart3 err!\r\n");
		return;
	}
	//初始化事件对象
	rt_event_init(&bluetooth_event, "uart_rx_event", RT_IPC_FLAG_FIFO);

	/* 接收回调函数*/
	rt_device_set_rx_indicate(bluetooth_serial, bluetooth_rx_ind);
	
	rt_pin_mode(BLE_PIN, PIN_MODE_OUTPUT);
	BLE_PWR_ON()	//模块上电
	
	
	rt_thread_mdelay(3000);
	
	BLE_ATCmd = BLE_ATE;
	BLE_ATCmd_Old = BLE_NULL; 
	g_ucProtocol = AT_MODE;
	
//	g_ulRx_Ptr = 0;
	
//	rt_uint8_t buf[]={0x68,0x17,0x00,0x43,0x45,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x10,0xDA,0x5F,0x05,0x01,0x00,0x40,0x01,0x02,0x00,0x00,0xED,0x03,0x16};
//	memcpy(stBLE_Comm.Rx_data,buf,sizeof(buf));
//	stBLE_Comm.DataRx_len = 25;
//	check_698_data_to_buf(&stBLE_Comm,_698_data_buf);	
		
	while (1)
	{
		if((g_ulBLE_Rx_Beg == 1)||(g_ucRecv698_In_AT == 1))
		{
			g_ulBLE_Rx_Beg = 0;
			BLE_RecvData_Process(bluetooth_serial,g_ucProtocol,BLE_ATCmd,&stBLE_Comm);
		}
		
		if(g_ucProtocol == AT_MODE)
		{
			BLE_ATCmd_Send(bluetooth_serial,BLE_ATCmd);
			rt_thread_mdelay(1000);
		}
		else
			rt_thread_mdelay(500);
		
//		BLE_Commit_TimeOut();
//		rt_thread_mdelay(1000);
	}
}



int bluetooth_thread_init(void)
{
	rt_err_t res;
	
	res=rt_thread_init(&bluetooth,
											"bluetooth",
											bluetooth_thread_entry,
											RT_NULL,
											bluetooth_stack,
											THREAD_BLUETOOTH_STACK_SIZE,
											THREAD_BLUETOOTH_PRIORITY,
											THREAD_BLUETOOTH_TIMESLICE);
	if (res == RT_EOK) 
	{
		rt_thread_startup(&bluetooth);
	}
	
	return res;
}


#if defined (RT_BLUETOOTH_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(bluetooth_thread_init);
#endif
MSH_CMD_EXPORT(bluetooth_thread_init, bluetooth thread run);

void AT_CMD(int argc, char**argv)
{
	rt_size_t size;
	char* buf;
	
	buf = (char*)rt_malloc(50);
	
	memset(buf,0,50);
	
	strcpy(buf,argv[1]);
	if(strstr(buf,"+++"))
	{}
	else
		strcat(buf+strlen(argv[1]),"\r\n");
	
	
	size = rt_device_write(bluetooth_serial, 0, buf, strlen(buf));
	
	if(size == sizeof(buf))
	{
		rt_lprintf("size = %d,send cmd is %s",size,buf);
	}
	rt_free(buf);
}
MSH_CMD_EXPORT(AT_CMD, Send AT CMD);



