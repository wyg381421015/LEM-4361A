#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_gpio.h"
#include "string.h"
#include "chargepile.h"
#include "stdlib.h"
#include "global.h"

#ifdef RT_USING_CAN

#ifndef FALSE
#define FALSE         0
#endif
#ifndef TRUE
#define TRUE          1
#endif

#ifndef SUCCESSFUL
#define SUCCESSFUL    0
#endif
#ifndef FAILED
#define FAILED        1
#endif

#ifndef TURN_ON
#define TURN_ON       0
#endif
#ifndef TURN_OFF
#define TURN_OFF      2
#endif

#define u8    uint8_t
#define u16   uint16_t
#define u32   uint32_t

#define delay_ms    rt_thread_mdelay

static u16 WaitStop_time = 0;
const u8 DestAdress = 0xF6;  	  //充电控制器---目的地址
const u8 SrcAdress = 0x8A;        //能源路由器---源地址
u16 Pro_Version = 0x0110;
static char CrjPileVersion[8] = {"V1.0.05"}; // 版本号
u8 software_date[8]={0};//软件生成日期 初始化为空格
u8 software_time[6]={0};//软件生成时间
u32 can_heart_count = 0;
static char  USARTx_TX_BUF[256] = {0};
static char Printf_buff[8];
#define FlashBufLenMax             1024
__align(4) u8 STMFLASH_BUFF[FlashBufLenMax+2];
u32 STMFLASH_LENTH = 0;
static u32 RunTime = 0;
//////////////////////////////////////////////////////////////////////////////////
void CAN_V110_RecProtocal(void);
void Inform_Communicate_Can(uint8_t SendCmd,uint8_t Resend);
u8 CAN1_Send_Msg(struct rt_can_msg CanSendMsg,u8 len);
//////////////////////////////////////////////////////////////////////////////////	 
#define PriorityLeve0           0x00
#define PriorityLeve1           0x01
#define PriorityLeve2           0x02
#define PriorityLeve3           0x03
#define PriorityLeve4           0x04
#define PriorityLeve5           0x05
#define PriorityLeve6           0x06
#define PriorityLeve7           0x07
/////////////////////////////////////////////////////////////////////////////////////
/////////CAN----PGN命令格式//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
#define ChargeStartFrame           0x01
#define ChargeStartFrameAck        0x02
#define ChargeStopFrame            0x03
#define ChargeStopFrameAck         0x04
#define TimingFrame                0x05
#define TimingFrameAck             0x06
#define VertionCheckFrame          0x07
#define VertionCheckFrameAck       0x08
#define ChargeParaInfoFrame        0x09
#define ChargeParaInfoFrameAck     0x0A
#define ChargeServeOnOffFrame      0x0B
#define ChargeServeOnOffFrameAck   0x0C
#define ElecLockFrame              0x0D
#define ElecLockFrameAck           0x0E
#define PowerAdjustFrame           0x0F
#define PowerAdjustFrameAck        0x10
#define PileParaInfoFrame          0x60
#define PileParaInfoFrameAck       0x61

#define ChargeStartStateFrame      0x11
#define ChargeStartStateFrameAck   0x12
#define ChargeStopStateFrame       0x13
#define ChargeStopStateFrameAck    0x14

#define YcRecDataFrame             0x30
#define YcSendDataFrame            0x31
#define YxRecDataFrame             0x32
#define YxBackupSendDataFrame      0x33

#define HeartSendFrame             0x40
#define HeartRecFrame              0x41

#define SendErrorStateFrame        0x51
#define RecErrorStateFrame         0x52

#define FunPwmFrame                0xFE
////////////////////////////////////////////////////////////////////////////////////
#define UpdateBeginFrame           0x70
#define UpdateBeginFrameAck        0x71

#define DataSendReqFrame           0x72
#define DataSendReqFrameAck        0x73

#define DataSendFrame              0x74
#define DataSendFrameAck           0x75

#define UpdateCheckFrame           0x76
#define UpdateCheckFrameAck        0x77

#define ResetFrame                 0x78
#define ResetFrameAck              0x79
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
/****************宏定义**********************************/
//定义故障类型
typedef enum {
//	NO_FAULT            =0x00000000,
//	Connect_FAULT       =0x00000001,        //输出接触器状态故障		      
//	StopEct_FAULT       =0x00000002,        //急停动作告警		      
//	Arrester_FAULT      =0x00000004,        //避雷器故障		
//	ACCir_FAULT         =0x00000008,        //交流进线开关
//	Dooropen_FAULT      =0x00000010,        //柜门打开故障
//	CardOffLine_FAULT   =0x00000020,        //读卡器通讯中断
//	MeterOffLine_FAULT  =0x00000040,        //电度表通讯中断
//	BgStop_FAULT        =0x00000080,        //后台通讯中断
//	ViHigh_FAULT        =0x00000100,        //输入电压过压
//	ViLow_FAULT         =0x00000200,        //输入电压欠压
//	Check_FAULT         =0x00000400,        //Check状态故障
//	IoHigh_FAULT        =0x00000800,        //输出过流告警
//	GunTempHigh_FAULT   =0x00001000,        //充电枪过温故障
	CanOffLine_FAULT    =0x00002000,        //CAN通讯中断
				
} CHARGE_PILE_FAULT_TYPE;
//定义故障类型汉化解析
char *err_string[] = 
{
           "                       ",               /* ERR_OK          0  */
           "输出接触器状态故障！   ",                 /* ERR             1  */
           "急停动作告警！         ",                /* ERR             2  */
           "避雷器故障！	       ",                   /* ERR             3  */
           "交流进线开关！         ",                /* ERR             4  */
           "柜门打开故障！         ",                /* ERR             5  */
           "读卡器通讯中断！       ",                /* ERR             6  */
           "电度表通讯中断！       ",                /* ERR             7  */
           "后台通讯中断！         ",                /* ERR             8  */
           "输入电压过压！         ",                /* ERR             9  */
           "输入电压欠压！         ",                /* ERR             10 */
           "Check状态故障！        ",                /* ERR             11 */
           "输出过流告警！         ",                /* ERR             12 */
	       "充电枪过温故障！       ",                /* ERR             13 */
           "GPRS通讯中断！         ",                /* ERR             14 */	
};
///////////////////////////////////////////////////////////////////
typedef struct
{
	uint8_t  ChargIdent;                       // 充电接口标识 单枪 双枪
	uint8_t  DeviceType;                       // 设备类型 0x01直流充电控制器 0x02交流充电控制器 0x03功率控制模块 0x04充电模块 0x05开关模块
    uint8_t  ChgSeviceState;                   // 系统充电服务状态  1：停用  2：可用
    uint8_t  ReplyState;                       // 成功标识 0成功 1失败
    uint8_t  LdSwitch;		                   // 负荷控制开关  1启用 2 关闭 	
    uint8_t  RecStatus;                        // 心跳报文接收状态
	uint8_t  RecCount;                         // 心跳报文接收计数
    uint8_t  SendCount;                        // 心跳报文发送计数	
    uint8_t  reaonIdent;	                   // 失败原因
    uint8_t  ConfIdent;                        // 确认标识 0成功 1失败
	uint8_t  StartReson;                       // 启原因
	uint8_t  StopReson;                        // 停原因
    uint8_t  ComUnitState;                     // 系统上电匹配状态  0 空闲  1匹配中  2 匹配完成
	uint8_t  ResetAct;                         // 系统上电匹配状态  0xAA 立即重启  其他无效
}STR_STATE_SYSTEM;                             // 系统状态帧标志位
STR_STATE_SYSTEM StrStateSystem;   // 系统状态帧标志
///////////////////////////////////////////////////////////////////
typedef struct
{	
	u8 ChargeStartFrameReSendFlag;
	u8 ChargeStartFrameReSendCnt;	
	
	u8 ChargeStartFrameAckReSendFlag;
	u8 ChargeStartFrameAckReSendCnt;

	u8 ChargeStopFrameReSendFlag;
	u8 ChargeStopFrameReSendCnt;
	
	u8 ChargeStopFrameAckReSendFlag;
	u8 ChargeStopFrameAckReSendCnt; 	

	u8 TimingFrameReSendFlag;
	u8 TimingFrameReSendCnt;
	
	u8 TimingFrameAckReSendFlag;
	u8 TimingFrameAckReSendCnt;	

	u8 VertionCheckFrameReSendFlag; 
	u8 VertionCheckFrameReSendCnt;
	
	u8 VertionCheckFrameAckReSendFlag; 
	u8 VertionCheckFrameAckReSendCnt;	

	u8 ChargeParaInfoFrameReSendFlag;
	u8 ChargeParaInfoFrameReSendCnt;	
	
	u8 ChargeParaInfoFrameAckReSendFlag;
	u8 ChargeParaInfoFrameAckReSendCnt;	

	u8 ChargeServeOnOffFrameReSendFlag;
	u8 ChargeServeOnOffFrameReSendCnt;
	
	u8 ChargeServeOnOffFrameAckReSendFlag;
	u8 ChargeServeOnOffFrameAckReSendCnt;	

	u8 ElecLockFrameReSendFlag;
	u8 ElecLockFrameReSendCnt;
	
	u8 ElecLockFrameAckReSendFlag;
	u8 ElecLockFrameAckReSendCnt;	

	u8 PowerAdjustFrameReSendFlag; 
	u8 PowerAdjustFrameReSendCnt;
	
	u8 PowerAdjustFrameAckReSendFlag; 
	u8 PowerAdjustFrameAckReSendCnt;	


	u8 PileParaInfoFrameAckReSendFlag;
	u8 PileParaInfoFrameAckReSendCnt;	
    
	u8 ChargeStartStateFrameReSendFlag;
	u8 ChargeStartStateFrameReSendCnt;
	
	u8 ChargeStartStateFrameAckReSendFlag;
	u8 ChargeStartStateFrameAckReSendCnt;
	
	u8 ChargeStopStateFrameReSendFlag; 
    u8 ChargeStopStateFrameReSendCnt;
	
	u8 ChargeStopStateFrameAckReSendFlag; 
    u8 ChargeStopStateFrameAckReSendCnt;	

	u8 YcSendDataFrameReSendFlag; 
	u8 YcSendDataFrameReSendCnt;

	u8 YxBackupSendDataFrameReSendFlag;
	u8 YxBackupSendDataFrameReSendCnt;

	u8 SendErrorStateFrameReSendFlag;
	u8 SendErrorStateFrameReSendCnt;

	u8 UpdateBeginFrameAckReSendFlag;
	u8 UpdateBeginFrameAckReSendCnt;
	
	u8 DataSendReqFrameAckReSendFlag;
	u8 DataSendReqFrameAckReSendCnt;
	
	u8 DataSendFrameAckReSendFlag;
	u8 DataSendFrameAckReSendCnt;

	u8 UpdateCheckFrameAckReSendFlag;
	u8 UpdateCheckFrameAckReSendCnt;
	
	u8 ResetFrameAckReSendFlag;
	u8 ResetFrameAckReSendCnt;
}STR_STATE_FRAME;                       
STR_STATE_FRAME StrStateFrame;   //帧重发状态
///////////////////////////////////////////////////////////////////
typedef struct 				
{
	uint8_t   Num;				//序号  16 01 10  liangbing
	uint8_t	  Elelock;          //电子锁控制 1-上锁 2-解锁

}STR_ELELOCK_CONTROL;
STR_ELELOCK_CONTROL strelelock;
///////////////////////////////////////////////////////////////////
typedef struct 				
{
	uint8_t   FraNum;           //报文总帧数
	uint16_t  ValData;          //报文有效数据长度
	uint16_t  ChargVa;		    //充电电压  1位小数
	uint16_t  ChargVb;		    //充电电压  1位小数
	uint16_t  ChargVc;		    //充电电压  1位小数
	uint16_t  ChargIa;          //充电电流  2位小数
	uint16_t  ChargIb;          //充电电流  2位小数
	uint16_t  ChargIc;          //充电电流  2位小数
	uint16_t  V_cp1;            //导引电压1 2位小数
	uint16_t  V_cp2;            //导引电压2 2位小数
	uint16_t  Addition;			//累加和
	u8  Tempt1;                 //温度1     1位小数
	u8  Tempt2;                 //温度2     1位小数	
	u8  Tempt3;                 //温度2     1位小数
	u8  Tempt4;                 //温度2     1位小数
}STR_YC; 
STR_YC strYC; // 遥测变量
///////////////////////////////////////////////////////////////////
//能源路由器下发的遥测数据
typedef struct 				
{
	uint8_t   MesNum;           //当前报文序号
	uint8_t   FraNum;           //报文总帧数
	uint16_t  ValData;          //报文有效数据长度
	uint16_t  Electricity;      //充电电量  单位 0.1KWH
	uint16_t  ChrgTime;         //充电时长  单位 1 min
	uint16_t  Addition;			//累加和
	uint16_t  TotalCheck;		//累加和校验码

}CHG_YC;
CHG_YC chgYC; //能源路由器下发的遥测变量
///////////////////////////////////////////////////////////////////
typedef struct 				
{
	u8  WorkState;		   //工作状态及开入状态  bit0-bit1：工作状态  0待机  1工作 2 完成  3暂停
	u8	TotalFau;          //总故障
	u8	TotalAlm;          //总告警
	u8  ConnState;         //车辆连接状态  	    0 连接 1未连接
	u8  StopEctFau;        //急停动作故障        0 正常 1异常
	u8	ArresterFau;       //避雷器故障	        0 正常 1异常
    u8  GunOut;            //充电枪未归位		0 正常 1异常
  	u8  ChgTemptOVFau;     //充电桩过温故障		
	u8  VOVWarn;           //输出电压过压
	u8  VLVWarn;           //输入电压欠压
	u8  OutConState;       //输出接触器状态      0正常，1异常
	u8  PilotFau;		   //充电中车辆控制导引故障        
	u8  ACConFau;		   //交流接触器故障	    0正常，1异常
	u8  OutCurrAla;		   //输出过流告警		            
	u8  OverCurrFau;	   //输出过流故障	    0正常，1保护
	u8  CurrentOutFlag;    //输出过流延时标识	TRUE开始计数   FALSE停止计数
	u8  CurrentOutCount;   //输出过流延时计时	
	u8  CCFau;	  		   //充电中枪头异常断开			 							
	u8  ACCirFau;		   //交流断路器故障
	u8  LockState;		   //充电接口电子锁状态
	u8  LockFauState;	   //充电接口电子锁故障状态
	u8  GunTemptOVFau;     //充电接口过温故障				
	u8  CC;				   //充电连接状态CC检测点4    1枪归位桩  0枪离开桩
	u8  CP;				   //充电控制状态CP检测点1
	u8  PEFau;			   //PE断线故障
	u8  DooropenFau;	   //柜门打开故障
	u8  ChgTemptOVAla;	   //充电桩过温告警				
	u8  GunTemptOVAla;	   //充电枪过温告警				
	u8  Contactoradjoin;   //接触器粘连
	u8  GenAlaFau;		   //通用告警和故障
	u8  OthWarnNum;        //其他报警编号
	u8  OthWarnVal;        //其他报警值

}STR_YX;
STR_YX strYX; // 遥信变量
///////////////////////////////////////////////////////////////////
typedef struct 				
{
	uint8_t  tCharParaAskTO          :1;   // 下发充电参数应答超时
	uint8_t  tCharStartAskTO         :1;   // 充电启动命令应答超时
	uint8_t	 tWaitCharStartConTO     :1;   // 等待充电启动完成状态超时
	uint8_t  tCharStopAskTO          :1;   // 充电停止命令应答超时
	uint8_t  tWaitCharStopConTO      :1;   // 等待充电停止完成状态超时
	uint8_t  tTimeAskTO              :1;   // 对时操作应答超时
	uint8_t  tChargeServeOnOffAskTO  :1;   // 充电服务启停应答超时
	uint8_t  tElecLockAskTO          :1;   // 电子锁控制应答超时
	uint8_t  tPowerAdjustAskTO       :1;   // 充电功率调节应答超时
	uint8_t  tPileParaInfoAskTO      :1;   // 充电桩配置信息查询应答超时
	uint8_t	 tCharYXRecTO            :1;   // 遥信报文接收超时
	uint8_t  tCharYCRecTO            :1;   // 遥测报文接收超时

}CHG_ERR_DATA;
CHG_ERR_DATA  ChgErrData; // 充电控制器错误帧数据
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
/* 定义充电状态 */
typedef enum {
	state_PowerON=0,                  //0
	state_WaitVertionCheck,           //1
	state_WaitPilePara,               //2
	state_Standby,                    //3
	state_RecChargStartCmd,           //4
	state_ChargStart,                 //5
	state_ChargStartErr,              //6
	state_ActStartOK,                 //7
	state_ActStartErr,                //8
	state_ChargStartOK,               //9
	state_WaitChargeStartFrameAsk,    //10
	state_Charging,	                  //11
	state_ToCheck,                    //12
	state_AutoCheck,                  //13
	state_ActStopOK,                  //14
	state_ActStopErr,                 //15
	state_WaitStop,                   //16
	state_ErrStop,                    //17
	state_ChargEnd,                   //18
	state_Update,                     //19
	state_UpdateData,                 //20
	state_WaitChargeStopFrameAsk,
} CHG_PILE_STATE_E;
/****************充电系统结构体***********************/
typedef struct //
{
	CHG_PILE_STATE_E ChgState;
	unsigned char  ChgPasState;
	unsigned long  PWM_Duty;             //占空比
	
	unsigned char  VOVWarn;              //输入电压过压
	unsigned char  VLVWarn;              //输入电压欠压
	unsigned char  OutCurrAla;			 //输出过流告警
	unsigned char  CheckState;           //连接确认开关状态  0 未连接 1连接 2可以充电 3状态转换中
	unsigned char  GunTemptOVFau;		 //充电枪过温告警
	unsigned char  Contactoradjoin;	     //接触器粘连

	unsigned long  TotalFau;             //总故障
	unsigned char  ConnectState;         //输出接触器状态    0闭合，1断开
	
}CHGPILE_TypeDef;
static CHGPILE_TypeDef STR_ChargePile_A;
//////////////////////////////////////////////////////////////////////////////////
//能源路由器下发的升级数据
typedef struct 				
{
	u8   UpdateConfIdent;          //确认标识 0x00：允许下载 0x01：禁止下载 其他：无效
	u8   NoDownloadReason;         //禁止下载原因	0x00：无 0x01：本身不支持此功能 0x02：数据合法性校验失败	
	u8   MessageNum;               //当前报文序号
	u8   FrameNum;                 //报文总帧数
	u16  ValDataLen;               //报文有效数据长度
	u16  AdditionUp;			   //累加和
	u16  RevCheckUp;		       //累加和校验码
	u8   ErrResendCount;           //重新请求数据包计数值

}CHG_UPDATE;
CHG_UPDATE chgUpdate; //能源路由器下发的升级数据
//////////////////////////////////////////////////////////////////////////////////
/****************程序升级结构体***********************/
typedef struct //update结构体
{
	u8   PileNum[8];               //桩号
	u32	 App_Mark;				   //升级标示 1 正在升级  0 未升级
	u32  App_Channel;			   //升级渠道 0：网口，1：2G，2：4G，3：can
	u32  App_Mark_addr;            
	u8	 file_Format;              //0：Bin，1：Hex
	u8   App_UpdateNo[8];	       //升级操作码
	u32  Update_Package_No;		   //升级文件段序列号
	u32  file_totalNo;             //文件总包数
	u32  file_ByteNo;              //文件总字节数
	u32	 Update_Confirm;           //升级标志  0成功   1失败
	u32  MajorVersion; 	           //升级主板本号
	u32  MinorVersion;	           //次版本号
	u8   Update_Message_No;        //发送超时次数
	u32  AdditionAll;			   //累加和
	
}Update_TypeDef;
Update_TypeDef STR_ProgramUpdate;//程序升级结构体
/********************************************************/
/**************模拟量校准结构体***************************/
typedef struct    //模拟量测量结果结构变量
{
	u8 uUpDate;		        //是否有更新
	u32 uiVolAdj;           //电压校准*1000
	u32 uiCurAdj;           //电流校准*1000
	u32 uiConVolAdj;        //连接确认电压校准*1000
	u32 TempatatureAdj;		//温度校准*1000
	
	u32 uiVolOverAlm;       //过压报警值 1位小数
	u32 uiVolUnderAlm;		//欠压报警值 1位小数
	u32 uiCurAlm;			//过流报警值 1位小数
	u32 TempatatureAlmPre;  //过温预警值 1位小数
	u32 TempatatureAlm;     //过温报警值 1位小数
	
	u32 pile_power;         //1位小数点   75=7.5Kw

}YC_AnalogPara_TypeDef;
YC_AnalogPara_TypeDef STR_YC_Para_A;
/********************************************************/
/********************************************************/
/********************************************************/
#define THREAD_CHARGEPILE_PRIORITY     21
#define THREAD_CHARGEPILE_STACK_SIZE   1024
#define THREAD_CHARGEPILE_TIMESLICE    5

#define THREAD_CHARGEPILE_REV_PRIORITY    20
#define THREAD_CHARGEPILE_REV_STACK_SIZE   1024
#define THREAD_CHARGEPILE_REV_TIMESLICE    5

/* can接收事件标志*/
#define CAN_RX_EVENT 0x00000001U

#define RT_CHARGEPILE_CAN "can1"

static rt_device_t chargepile_can = RT_NULL;
static struct rt_event can_rx_event;
/* 充电控制-事件控制块 */
struct rt_event ChargePileEvent;
static struct rt_thread chargepile;
static struct rt_thread chargepileRev;
static rt_uint8_t chargepile_stack[THREAD_CHARGEPILE_STACK_SIZE];//线程堆栈
static rt_uint8_t chargepileRevSend_stack[THREAD_CHARGEPILE_REV_STACK_SIZE];//线程堆栈
static struct rt_can_msg g_RxMessage;



///////////////////////////////////////////////////////////////
static void timer_create_init(void);
/* 定时器的控制块 */
static rt_timer_t CAN_Heart_Tx;
static rt_timer_t CAN_250ms_Tx;
static void CAN_Heart_Tx_callback(void* parameter);
static void CAN_250ms_Tx_callback(void* parameter);

ChargPilePara_TypeDef STR_ChargPilePara;

void *RetStructADDR[End_cmdListNum];
/**************************************************************
* 函数名称: ChargepileDataGetSet
* 参    数: 
* 返 回 值: 
* 描    述: 
***************************************************************/
rt_uint8_t ChargepileDataGetSet(COMM_CMD_P cmd,void *STR_SetGetPara)
{
	rt_uint8_t result = FAILED;
	
	switch(cmd)
	{
		case Cmd_ChargeStart://
			if(STR_ChargePile_A.ChgState == state_Standby)
			{
				STR_ChargePile_A.ChgState = state_ChargStart;
				result = SUCCESSFUL;
			}
			else
			{
				result = FAILED;
			}	
			
			break;
		case Cmd_ChargeStartResp://
			((ChargPilePara_TypeDef*)STR_SetGetPara)->StartReson = STR_ChargPilePara.StartReson;
			result = SUCCESSFUL;	
			
			break;
		case Cmd_ChargeStop://
			if(STR_ChargePile_A.ChgState == state_Charging)
			{
				STR_ChargePile_A.ChgState = state_ToCheck;
				result = SUCCESSFUL;
			}
			else
			{
				result = FAILED;
			}
		
			break;
		case Cmd_ChargeStopResp://
			((ChargPilePara_TypeDef*)STR_SetGetPara)->StopReson = STR_ChargPilePara.StopReson;
			result = SUCCESSFUL;	
			
			break;			
		case Cmd_SetPower://
			STR_ChargPilePara.PWM_Duty = ((ChargPilePara_TypeDef*)STR_SetGetPara)->PWM_Duty;
			result = SUCCESSFUL;
		
			break;
		case Cmd_GetPower://
			((ChargPilePara_TypeDef*)STR_SetGetPara)->PWM_Duty = STR_ChargPilePara.PWM_Duty;
			result = SUCCESSFUL;
		
			break;		
		case Cmd_RdVertion://
			result = SUCCESSFUL;
		
			break;
		case Cmd_RdVoltCurrPara://
			result = SUCCESSFUL;
		
			break;		
		default:
			rt_kprintf("Waring：收到未定义指令%u\r\n",cmd);
			break;	
	}
	
	return  result;
}
/**************************************************************
* 函数名称: print_can_msg
* 参    数: 
* 返 回 值: 
* 描    述: 
***************************************************************/
void print_can_msg(struct rt_can_msg *data)
{
	sprintf((char*)USARTx_TX_BUF,"CAN1RecFrame:"); 
	sprintf((char*)Printf_buff,"%08X",data->id); 
	strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
	strcat((char*)USARTx_TX_BUF,(const char*)"--");

	for(int i=0;i<data->len;i++)   //循环发送数据
	{
		sprintf((char*)Printf_buff,"%02X",data->data[i]); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
	}
	rt_lprintf("%s\n",USARTx_TX_BUF);
}
/**************************************************************
* 函数名称: can_rx_ind
* 参    数: 
* 返 回 值: 
* 描    述: 
***************************************************************/
static rt_err_t can_rx_ind(rt_device_t device, rt_size_t len)
{
	rt_event_send(&can_rx_event, CAN_RX_EVENT);
	return RT_EOK;
}
/**************************************************************
* 函数名称: chargepile_thread_entry
* 参    数: 
* 返 回 值: 
* 描    述: 
***************************************************************/
static void chargepile_thread_entry(void *parameter)
{
	strYC.ChargVa = 2200;		    //充电电压  1位小数
	strYC.ChargVb = 2200;		    //充电电压  1位小数
	strYC.ChargVc = 2200;		    //充电电压  1位小数
	STR_ChargePile_A.ChgState = state_WaitVertionCheck;
	StrStateSystem.LdSwitch = 0x01;	      // 负荷控制开关
	rt_thread_mdelay(100);
	while (1)
	{

		if((STR_ChargePile_A.ChgState == state_Standby)&&(RunTime%5 == 0x00))
		{
			ChargPilePara_TypeDef *STR_CHG_test = NULL;
			ChargepileDataGetSet(Cmd_ChargeStart,STR_CHG_test);
			rt_lprintf("[chargepile]:下发启动充电命令\n");
		}
		else if((STR_ChargePile_A.ChgState == state_Charging)&&(RunTime%10 == 0x00))
		{
			ChargPilePara_TypeDef *STR_CHG_test = NULL;
			ChargepileDataGetSet(Cmd_ChargeStop,STR_CHG_test);
			rt_lprintf("[chargepile]:下发停止充电命令\n");
		}		
		
		switch(STR_ChargePile_A.ChgState)
		{
			case state_PowerON:// 上电状态
				rt_lprintf("chargepile:State_PowerON\n");
			
				break;			
			case state_WaitVertionCheck://等待版本校验应答
				Inform_Communicate_Can(VertionCheckFrame,FALSE);
				rt_lprintf("chargepile:State_WaitVertionCheck\n");
			
				break;
			case state_WaitPilePara://等待充电桩参数应答
				Inform_Communicate_Can(ChargeParaInfoFrame,FALSE);
				rt_lprintf("chargepile:State_WaitPilePara\n");
			
				break;
			case state_Standby://初始化完成->待机

		
//				if(rt_event_recv(&ChargePileEvent, ChargeStartOK_EVENT | ChargeStopOK_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,500, &ch) == RT_EOK)
//				{
//					rt_kprintf("chargepile:接收到ChargePileEvent 0x%02X\n", ch);	
//				}
//				else
//				{
//					rt_kprintf("chargepile:未接收ChargePileEvent 0x%02X\n", ch);
//				}				
			
				rt_lprintf("chargepile:State_Standby\n");
			
				break;
			case state_ChargStart://接收到启动充电命令 开始下发充电命令
				Inform_Communicate_Can(ChargeStartFrame,FALSE);
				rt_lprintf("chargepile:ChargeStartFrame\n");
				STR_ChargePile_A.ChgState = state_WaitChargeStartFrameAsk;
			
				break;
			case state_WaitChargeStartFrameAsk://接收到启动充电命令 开始下发充电命令
				rt_lprintf("chargepile:State_WaitCharging\n");
			
				break;
			case state_Charging://交互成功，充电中
				rt_lprintf("chargepile:state_Charging\n");
			
				break;
			case state_ToCheck://接收到停机命令 开始下发停机命令
				Inform_Communicate_Can(ChargeStopFrame,FALSE);
				rt_lprintf("chargepile:ChargeStopFrame\n");
				STR_ChargePile_A.ChgState = state_WaitChargeStartFrameAsk;
			
				break;
			case state_WaitChargeStopFrameAsk://等待停止充电应答帧
				rt_lprintf("chargepile:State_WaitChargeStopFrameAsk\n");
			
				break;
			case state_WaitStop://等待停止充电应答帧
				rt_lprintf("chargepile:State_WaitStop\n");
			
				break;
			case state_ChargEnd://等待停止充电应答帧
				memset(&StrStateFrame,0x00,sizeof(STR_STATE_FRAME));
				StrStateFrame.YcSendDataFrameReSendFlag = TRUE; 
				StrStateFrame.YxBackupSendDataFrameReSendFlag = TRUE; 

				STR_ChargePile_A.ChgState = state_Standby;
				rt_lprintf("chargepile:State_ChargEnd->State_Standby\n");
			
				break;
			default:
				rt_lprintf("chargepile:default\n");
				break;			
		}
//        extern rt_uint8_t cpu_usage_current,cpu_usage_max;
//		rt_lprintf("cpu_usage_current=%u%%,cpu_usage_max=%u%%\n",cpu_usage_current,cpu_usage_max);
//		rt_lprintf("RunTime=%u秒\n",RunTime);
//		extern long list_thread(void);
//		list_thread();
//		extern void list_mem(void);
//		list_mem();
		
		/* lock scheduler */
		rt_enter_critical();
		/* unlock scheduler */
		rt_exit_critical();
//		rt_event_send(&ChargePileEvent, ChargeStartOK_EVENT);
//		rt_event_send(&ChargePileEvent, ChargeStartER_EVENT);
//		rt_uint32_t ch = 0;
//		if(rt_event_recv(&ChargePileEvent, ChargeStartOK_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,500, &ch) == RT_EOK)
//		{
//			rt_kprintf("chargepile:接收到ChargePileEvent 0x%02X\n", ch);	
//		}
//		else
//		{
//			rt_kprintf("chargepile:未接收ChargePileEvent 0x%02X\n", ch);
//		}	
//        rt_base_t level = rt_hw_interrupt_disable();


//		rt_hw_interrupt_enable(level);


		rt_thread_mdelay(500);
	}
}
/**************************************************************
* 函数名称: chargepileRev_thread_entry
* 参    数: 
* 返 回 值: 
* 描    述: 
***************************************************************/
#define can1RevCycle    200
static void chargepileRev_thread_entry(void *parameter)
{
	rt_uint32_t err;
	rt_err_t res = RT_EOK;
    rt_uint32_t can1RevCycleCount = 0;
	while (1)
	{
		res = rt_event_recv(&can_rx_event, CAN_RX_EVENT, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 0, &err);
		if (res == RT_EOK)//没有数据需要接收 RT_ETIMEOUT
		{
			while(rt_device_read(chargepile_can, 0, &g_RxMessage, sizeof(struct rt_can_msg)))
			{
				CAN_V110_RecProtocal();
			}			
		}
		else
		{
//			rt_lprintf("[chargepileRev]:没有can数据res=%d\n",res);
		}

//        can1RevCycleCount++;
//		//定时上传
//		if(can1RevCycleCount >= (1000/can1RevCycle))
//		{		
//			Inform_Communicate_Can(HeartSendFrame,FALSE);
//			can1RevCycleCount = 0;
//		}
//		rt_lprintf("[chargepileRev]:没有can数据res=%d\n",res);
		rt_thread_mdelay(can1RevCycle);
	}
}
/**************************************************************
 * 函数名称: timer_create_init 
 * 参    数: 
 * 返 回 值: 
 * 描    述: CAN周期回复回调函数  250ms周期发送
 **************************************************************/
int chargepile_thread_init(void)
{
	rt_err_t res;

	rt_kprintf("HEAP_BEGIN=0x%08X,HEAP_END=0x%08X\n",HEAP_BEGIN,HEAP_END);
    /* 初始化充电控制-事件对象 */
    rt_event_init(&ChargePileEvent, "ChargePileEvent", RT_IPC_FLAG_FIFO);	
	/* 初始化定时器 */
	timer_create_init();
	
	
	struct can_configure config = CANDEFAULTCONFIG;
	chargepile_can = rt_device_find(RT_CHARGEPILE_CAN);
		
	if(chargepile_can != RT_NULL)
	{
		rt_device_control(chargepile_can, RT_CAN_CMD_SET_MODE, &config);
		if(rt_device_open(chargepile_can, (RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX)) == RT_EOK)
		{
				//初始化事件对象
			rt_event_init(&can_rx_event, "can_rx_event", RT_IPC_FLAG_FIFO);			
			rt_kprintf("[ChargePile]:Open %s sucess!\n",RT_CHARGEPILE_CAN);
		}
		else
		{
			rt_kprintf("[ChargePile]:%s Device open failed!\n",RT_CHARGEPILE_CAN);
		}
	}
	else
	{
		res = RT_ERROR;
		rt_kprintf("[ChargePile]:Open %s error\n",RT_CHARGEPILE_CAN);
		
		return res;
	}
	/* 接收回调函数*/
	rt_device_set_rx_indicate(chargepile_can, can_rx_ind);	
	
#ifdef BSP_USING_CAN1
	res=rt_thread_init(&chargepile,
						"chargepile",
						chargepile_thread_entry,
						RT_NULL,
						chargepile_stack,
						THREAD_CHARGEPILE_STACK_SIZE,
						THREAD_CHARGEPILE_PRIORITY,
						THREAD_CHARGEPILE_TIMESLICE);
	if (res == RT_EOK) 
	{
		rt_thread_startup(&chargepile);
		rt_kprintf("[ChargePile]:chargepile_thread_entry sucess\r\n");
	}
	else
	{
		res = RT_ERROR;
		rt_kprintf("[ChargePile]:chargepile_thread_entry fail\r\n");
	}
/*********************************************************************************/	
	res=rt_thread_init(&chargepileRev,
						"chargepileRev",
						chargepileRev_thread_entry,
						RT_NULL,
						chargepileRevSend_stack,
						THREAD_CHARGEPILE_REV_STACK_SIZE,
						THREAD_CHARGEPILE_REV_PRIORITY,
						THREAD_CHARGEPILE_REV_TIMESLICE);
	if (res == RT_EOK) 
	{
		rt_thread_startup(&chargepileRev);
		rt_kprintf("[ChargePile]:chargepileRev_thread_entry sucess\r\n");
	}
	else
	{
		res = RT_ERROR;
		rt_kprintf("[ChargePile]:chargepileRev_thread_entry fail\r\n");
	}
/*********************************************************************************/	
#endif /*BSP_USING_CAN1*/
	
	return res;
}


#if defined (RT_CHARGEOILE_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(chargepile_thread_init);
#endif
MSH_CMD_EXPORT(chargepile_thread_init, chargepile thread run);


/**************************************************************
 * 函数名称: timer_create_init 
 * 参    数: 
 * 返 回 值: 
 * 描    述: CAN周期回复回调函数  250ms周期发送
 **************************************************************/
void timer_create_init()
{
    /* 创建心跳包定时器 */
	CAN_Heart_Tx = rt_timer_create("CAN_Heart_Tx",  /* 定时器名字是 CAN_Heart_Tx */
									CAN_Heart_Tx_callback, /* 超时时回调的处理函数 */
									RT_NULL, /* 超时函数的入口参数 */
									1000, /* 定时长度，以OS Tick为单位，即1000个OS Tick */
									RT_TIMER_FLAG_PERIODIC); /* 周期性定时器 */
	
    /* 启动心跳包定时器 */
    if (CAN_Heart_Tx != RT_NULL)
        rt_timer_start(CAN_Heart_Tx);
    else
        rt_kprintf("CAN_Heart_Tx create error\n");

	
	/* 创建周期回复定时器 */
	CAN_250ms_Tx = rt_timer_create("CAN_250ms_Tx",  /* 定时器名字是 CAN_Heart_Tx */
									CAN_250ms_Tx_callback, /* 超时时回调的处理函数 */
									RT_NULL, /* 超时函数的入口参数 */
									250, /* 定时长度，以OS Tick为单位，即250个OS Tick */
									RT_TIMER_FLAG_PERIODIC); /* 周期性定时器 */

	/* 启动心跳包定时器 */
	if (CAN_250ms_Tx != RT_NULL)
		rt_timer_start(CAN_250ms_Tx);
	else
		rt_kprintf("CAN_250ms_Tx create error\n");
}
//**************************************************************
/**************************************************************
 * 函数名称: CAN_Heart_Tx_callback 
 * 参    数: 
 * 返 回 值: 
 * 描    述: CAN心跳包回调函数  1S周期发送 
 **************************************************************/
static void CAN_Heart_Tx_callback(void* parameter)
{
    RunTime++;
	WaitStop_time++;

//	Inform_Communicate_Can(HeartSendFrame,FALSE);
//	if(StrStateFrame.YcSendDataFrameReSendFlag == TRUE)//停止充电命令回复标志
//	{
//		Inform_Communicate_Can(YcSendDataFrame,FALSE);//停止充电命令回复
//	}	
}
/**************************************************************
 * 函数名称: CAN_250ms_Tx_callback 
 * 参    数: 
 * 返 回 值: 
 * 描    述: CAN周期回复回调函数  250ms周期发送
 **************************************************************/
static void CAN_250ms_Tx_callback(void* parameter)
{
// 	if(StrStateFrame.ChargeStartFrameReSendFlag == TRUE)//启动充电命令回复标志
//	{	
//		Inform_Communicate_Can(ChargeStartFrame,FALSE);//
//		StrStateFrame.ChargeStartFrameReSendCnt++;
//		if(StrStateFrame.ChargeStartFrameReSendCnt >= 8)//大于2s
//		{
//			StrStateFrame.ChargeStartFrameReSendFlag = FALSE;
//			StrStateFrame.ChargeStartFrameReSendCnt = 0;
//		}
//	}
//	
//	if(StrStateFrame.ChargeStopFrameReSendFlag == TRUE)//停止充电命令回复标志
//	{
//		Inform_Communicate_Can(ChargeStopFrame,FALSE);//停止充电命令回复
//		StrStateFrame.ChargeStopFrameReSendCnt++;
//		if(StrStateFrame.ChargeStopFrameReSendCnt >= 8)//大于2s
//		{
//			StrStateFrame.ChargeStopFrameReSendFlag = FALSE;
//			StrStateFrame.ChargeStopFrameReSendCnt = 0;
//		}
//	}
	
//	rt_kprintf("CAN_250ms_Tx_callback\n");
}
/***************************************************************
* 函数名称: Inform_Communicate_Can(uint8_t SendCmd,uint8_t Resend)
* 参    数:
* 返 回 值:
* 描    述: 充电控制器向计费单元发送数据
***************************************************************/
void Inform_Communicate_Can(uint8_t SendCmd,uint8_t Resend)
{
	uint8_t i;
	uint8_t PriorityLeve,DataLength;
	uint16_t temp1;
	struct rt_can_msg TxBuf; // CAN口发送报文结构体
	switch(SendCmd)
	{
		case ChargeStartFrame:
		{
			//能源路由器向充电控制器发送"启动充电"命令：优先级0X04，PF：0X01
			PriorityLeve = PriorityLeve4;

			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//

			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;      // 充电接口标识
			TxBuf.data[1] =  StrStateSystem.LdSwitch&0xFF;	      // 负荷控制开关	
			DataLength = 0x02;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeStartFrameReSendFlag = TRUE;
				StrStateFrame.ChargeStartFrameReSendCnt = 0;	
			}			
			
			break;
		}
		case ChargeStopFrame:
		{			
			//能源路由器向充电控制器发送"停止充电"：优先级0X04，PF：0X03。
			PriorityLeve = PriorityLeve4;	   //优先级4		
						
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress
			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;	  //充电接口标识
			TxBuf.data[1] =  StrStateSystem.StopReson;            //停止充电原因 01正常停止 02自身故障停止 03非自身故障停止
			DataLength = 0x02;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeStopFrameReSendFlag = TRUE;
				StrStateFrame.ChargeStopFrameReSendCnt = 0;
			}			

			break;
		}
		case TimingFrame:
		{			
			//能源路由器向充电控制器发送"校时"：优先级0X06，PF：0X05。
			PriorityLeve = PriorityLeve6;	   //优先级6
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress
            temp1 = System_Time_STR.Second*1000;
			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;                          // 充电接口标识
			TxBuf.data[1] =  temp1&0xFF;                                              // 毫秒  低8位
			TxBuf.data[2] =  (temp1>>8)&0xFF;                                         // 毫秒  高8位	  
			TxBuf.data[3] =  System_Time_STR.Minute&0x3F;                             // 分
			TxBuf.data[4] =  System_Time_STR.Hour&0x1F;                               // 小时	
			TxBuf.data[5] =  (System_Time_STR.Week<<5)+(System_Time_STR.Day&0x1F);    // 0-4日期 5-7星期
			TxBuf.data[6] =  System_Time_STR.Month&0x0F;                              // 月
			TxBuf.data[7] =  System_Time_STR.Year&0x7F;                               // 0-6位年	
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.TimingFrameReSendFlag = TRUE;
				StrStateFrame.TimingFrameReSendCnt = 0;
			}			

			break;
		}		
		case TimingFrameAck:
		{			
			//充电控制器向能源路由器发送"校时"命令确认：优先级0X06，PF：0X06
			PriorityLeve = PriorityLeve6;	   //优先级6
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress
            temp1 = System_Time_STR.Second*1000;
			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;                          // 充电接口标识
			TxBuf.data[1] =  temp1&0xFF;                                              // 毫秒  低8位
			TxBuf.data[2] =  (temp1>>8)&0xFF;                                         // 毫秒  高8位	  
			TxBuf.data[3] =  System_Time_STR.Minute&0x3F;                             // 分
			TxBuf.data[4] =  System_Time_STR.Hour&0x1F;                               // 小时	
			TxBuf.data[5] =  (System_Time_STR.Week<<5)+(System_Time_STR.Day&0x1F);    // 0-4日期 5-7星期
			TxBuf.data[6] =  System_Time_STR.Month&0x0F;                              // 月
			TxBuf.data[7] =  System_Time_STR.Year&0x7F;                               // 0-6位年	
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.TimingFrameAckReSendFlag = TRUE;
				StrStateFrame.TimingFrameAckReSendCnt = 0;
			}			

			break;
		}
		case VertionCheckFrame:
		{
			//能源路由器向充电控制器发送"版本校验"：优先级0X06，PF：0X07。
			PriorityLeve = PriorityLeve6;	   //优先级6
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress

			TxBuf.data[0] = StrStateSystem.ChargIdent;	  //充电接口标识
			TxBuf.data[1] =  Pro_Version&0xFF;     // 次板本号
			TxBuf.data[2] = (Pro_Version>>8)&0xFF; // 主版本号
			DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.VertionCheckFrameReSendFlag = TRUE;
				StrStateFrame.VertionCheckFrameReSendCnt = 0;
			}			
			
			break;
		}		
//		case VertionCheckFrameAck:
//		{
//			//充电控制器向能源路由器发送"版本校验"命令确认：优先级0X06，PF：0X08
//			PriorityLeve = PriorityLeve6;	   //优先级6
//			
//			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress

//			TxBuf.data[0] = StrStateSystem.ChargIdent;	  //充电接口标识
//			TxBuf.data[1] =  Pro_Version&0xFF;     // 次板本号
//			TxBuf.data[2] = (Pro_Version>>8)&0xFF; // 主版本号
//			TxBuf.data[3] = 0x00; // 所支持的bms通信协议号 	  
//			TxBuf.data[4] = (((CrjPileVersion[5]-0x30)<<4)|(CrjPileVersion[6]-0x30))&0xFF;//发行版本号
//			TxBuf.data[5] = (CrjPileVersion[3]-0x30)&0xFF;//次版本号	
//			TxBuf.data[6] = (CrjPileVersion[1]-0x30)&0xFF;//主版本号
//			DataLength = 0x07;
//			CAN1_Send_Msg(TxBuf,DataLength);
//			if(Resend == TRUE)
//			{
//				StrStateFrame.VertionCheckFrameAckReSendFlag = TRUE;
//				StrStateFrame.VertionCheckFrameAckReSendCnt = 0;
//			}			
//			
//			break;
//		}
		case ChargeParaInfoFrame:
		{
			//能源路由器向充电控制器发送"充电参数"：优先级0X06，PF：0X09。
			PriorityLeve = PriorityLeve6;	   //优先级6
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;

			TxBuf.data[0] = StrStateSystem.ChargIdent; //参数确认成功标识 默认为0
			
			for(i=1;i<8;i++)
			{
				TxBuf.data[i] = STR_ProgramUpdate.PileNum[i];	
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeParaInfoFrameReSendFlag = TRUE;
				StrStateFrame.ChargeParaInfoFrameReSendCnt = 0;
			}				
			
			break;
		}
		case ChargeServeOnOffFrame: 
		{		
			//能源路由器向充电控制器发送"充电服务控制"：优先级0X04，PF：0X0B。
            PriorityLeve = PriorityLeve4;  

            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] = StrStateSystem.ChargIdent&0xFF;//充电接口标识
			TxBuf.data[1] = 0x02&0xFF;//充电服务启动停止操作指令 strChgSevCtrl.ChargSeveiceCmd 01停止 02启动 其他 无效
            DataLength = 0x02;
            CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeServeOnOffFrameReSendFlag = TRUE;
				StrStateFrame.ChargeServeOnOffFrameReSendCnt = 0;
			}			

            break;
		}		
		case ChargeServeOnOffFrameAck: 
		{		
			//充电控制器向能源路由器发送"充电服务控制"命令确认：优先级0X04，PF：0X0C
            PriorityLeve = PriorityLeve4;  

            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;//充电接口标识
//			TxBuf.data[1] =  strChgSevCtrl.ChargSeveiceCmd&0xFF;//操作指令
//			TxBuf.data[2] =  strChgSevCtrl.CtrlState&0xFF;//执行结果
//			TxBuf.data[3] =  strChgSevCtrl.Ctrreaon&0xFF;
            DataLength = 0x04;
            CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeServeOnOffFrameAckReSendFlag = TRUE;
				StrStateFrame.ChargeServeOnOffFrameAckReSendCnt = 0;
			}			

            break;
		}
		case ElecLockFrame:
		{			
			//能源路由器向充电控制器发送"电子锁控制"：优先级0X04，PF：0X0D
            PriorityLeve = PriorityLeve4;  
			
            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;// 充电接口标识
            TxBuf.data[1] =  strelelock.Num&0xFF;       // 电子锁序号
            TxBuf.data[2] =  strelelock.Elelock&0xFF;   // 操作指令
            DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ElecLockFrameReSendFlag = TRUE;
				StrStateFrame.ElecLockFrameReSendCnt = 0;
			}			

            break;
		}		
		case ElecLockFrameAck:
		{			
			//充电控制器向能源路由器发送"电子锁控制"命令确认：优先级0X04，PF：0X0E
            PriorityLeve = PriorityLeve4;  
			
            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;// 充电接口标识
            TxBuf.data[1] =  strelelock.Num&0xFF;       // 电子锁序号
            TxBuf.data[2] =  strelelock.Elelock&0xFF;   // 操作指令
            TxBuf.data[3] =  StrStateSystem.ReplyState;  // 执行结果;
            TxBuf.data[4] =  StrStateSystem.reaonIdent&0xFF; // 失败原因
            DataLength = 0x05;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ElecLockFrameAckReSendFlag = TRUE;
				StrStateFrame.ElecLockFrameAckReSendCnt = 0;
			}			

            break;
		}
		case PowerAdjustFrame:
		{
			//能源路由器向充电控制器发送"功率调节"：优先级0X04，PF：0X0F
            PriorityLeve = PriorityLeve4;  
			
            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;           // 充电接口标识
//			TxBuf.data[1] =  strcapacity.C_capacity_style;             // 功率调节类型
//			TxBuf.data[2] =  strcapacity.C_capacity_value&0xFF;	       // 功率调节值
//			TxBuf.data[3] =  (strcapacity.C_capacity_value>>8)&0xFF;
            DataLength = 0x04;
            CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.PowerAdjustFrameReSendFlag = TRUE;
				StrStateFrame.PowerAdjustFrameReSendCnt = 0;
			}				

            break;
		}		
		case PowerAdjustFrameAck:
		{
			//充电控制器向能源路由器发送"功率调节"命令确认：优先级0X04，PF：0X10
            PriorityLeve = PriorityLeve4;  
			
            TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

            TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;           // 充电接口标识
//			TxBuf.data[1] =  strcapacity.C_capacity_style;             // 功率调节类型
//			TxBuf.data[2] =  strcapacity.C_capacity_value&0xFF;	       // 功率调节值
//			TxBuf.data[3] =  (strcapacity.C_capacity_value>>8)&0xFF;
            TxBuf.data[4] =  StrStateSystem.ReplyState;                // 成功标识 1成功 2失败
			TxBuf.data[5] =  StrStateSystem.reaonIdent;                // 失败原因
            DataLength = 0x06;
            CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.PowerAdjustFrameAckReSendFlag = TRUE;
				StrStateFrame.PowerAdjustFrameAckReSendCnt = 0;
			}				

            break;
		}
		case ChargeStartStateFrameAck:
		{
			//能源路由器向充电控制器发送"充电启动状态"命令确认：优先级0X04，PF：0X12
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF; // 充电接口标识
			TxBuf.data[1] =  StrStateSystem.LdSwitch&0xFF;   // 负荷控制开关
			TxBuf.data[2] =  StrStateSystem.ReplyState;      // 充电启动完成状态帧成功标识
			DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ChargeStartStateFrameAckReSendFlag = TRUE;
				StrStateFrame.ChargeStartStateFrameAckReSendCnt = 0;
			}				
			
			break;
		}
		case ChargeStopStateFrameAck:
		{
			//能源路由器向充电控制器发送"充电停止状态"命令确认：优先级0X04，PF：0X14
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;	//充电接口标识
			TxBuf.data[1] =  StrStateSystem.StopReson&0xFF;     //停止原因0x01-能源路由器控制停止充电   0x02-充电桩故障，充电控制器自行主动终止充电
			TxBuf.data[2] =  StrStateSystem.ReplyState;         //停止充电完成状态帧成功标识
			DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
			StrStateFrame.ChargeStopStateFrameAckReSendFlag = TRUE;
			StrStateFrame.ChargeStopStateFrameAckReSendCnt = 0;
			}			

			break;
		}
		case YcSendDataFrame://能源路由器向充电控制器周期性发送遥测数据：优先级0X06，PF0X31。
		{
			PriorityLeve = PriorityLeve6;  
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 
			strYC.ValData = 0x15;  // 
			TxBuf.data[0] = 0x01; // 当前报文序号
			TxBuf.data[1] = 0x04; // 报文总帧数
			TxBuf.data[2] = strYC.ValData&0xFF;      // 报文有效数据长度低字节
			TxBuf.data[3] = (strYC.ValData>>8)&0xFF; // 报文有效数据长度高字节 
			TxBuf.data[4] = StrStateSystem.ChargIdent&0xFF; // 充电接口标识	  
			TxBuf.data[5] = strYC.ChargVa&0xFF;  // A相电压
			TxBuf.data[6] = (strYC.ChargVa>>8)&0xFF;	  
			TxBuf.data[7] = strYC.ChargVb&0xFF;  // B相电压
			strYC.Addition = 0x0000;
			for(i=1;i<8;i++)
			{
				strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			
			delay_ms(10);

			TxBuf.data[0] = 0x02; // 当前报文序号
			TxBuf.data[1] = (strYC.ChargVb>>8)&0xFF;
			TxBuf.data[2] = strYC.ChargVc&0xFF; // C相电压
			TxBuf.data[3] = (strYC.ChargVc>>8)&0xFF;
			TxBuf.data[4] = strYC.ChargIa&0xFF; // A相电流
			TxBuf.data[5] = (strYC.ChargIa>>8)&0xFF;
			TxBuf.data[6] = strYC.ChargIb&0xFF; // B相电流
			TxBuf.data[7] = (strYC.ChargIb>>8)&0xFF;
			for(i=1;i<8;i++)
			{
				strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(10);
			 
			TxBuf.data[0] =  0x03;//当前报文序号
			TxBuf.data[1] =  strYC.ChargIc&0xFF;                    //C相电流
			TxBuf.data[2] =  (strYC.ChargIc>>8)&0xFF;	            //
			TxBuf.data[3] =  chgYC.Electricity&0xFF;                //充电电量  
			TxBuf.data[4] =  (chgYC.Electricity>>8)&0xFF;
			TxBuf.data[5] =  chgYC.ChrgTime&0xFF;                   //充电时长
			TxBuf.data[6] =  (chgYC.ChrgTime>>8)&0xFF;	            //

			for(i=1;i<7;i++)
			{
				strYC.Addition += TxBuf.data[i];
			}
			TxBuf.data[7] =  strYC.Addition&0xff;      // 报文有效数据长度低字节
			DataLength = 0x08;	
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(10);		
			 
			TxBuf.data[0] = 0x04;//当前报文序号
			TxBuf.data[1] =  (strYC.Addition>>8)&0xFF; // 报文有效数据长度高字节

			DataLength = 0x02;
			CAN1_Send_Msg(TxBuf,DataLength);				

			break;
		}													   
		case HeartSendFrame://充电控制器向能源路由器周期性发送心跳数据：优先级0X06，PF:0X40。
		{			 
			PriorityLeve = PriorityLeve6;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;   // 充电接口标识
			TxBuf.data[1] =  StrStateSystem.SendCount&0xFF;    // 心跳报文发送计数
			if((STR_ChargePile_A.TotalFau&CanOffLine_FAULT) == CanOffLine_FAULT) // 通信超时
			{
                TxBuf.data[2] = 0x01; // 通讯超时
			}
			else
			{											  
                TxBuf.data[2] = 0x00; // 通讯成功
			}			
			
			TxBuf.data[3] = 0x02;// 1:充电服务状态 停用  2：可用
			DataLength = 0x04;
			CAN1_Send_Msg(TxBuf,DataLength);
			StrStateSystem.SendCount++;   //加到255 自动溢出回零

			break;
		}
		case SendErrorStateFrame://充电控制器错误数据帧：优先级0X04，PF:0X52。
		{
		 	PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 
			TxBuf.data[0] =  StrStateSystem.ChargIdent&0xFF;//充电接口标识
							/* 下发充电参数应答超时 */
							/* 充电启动命令应答超时*/
							/* 等待充电启动完成状态超时*/
							/* 充电停止命令应答超时*/
							/* 等待充电停止完成状态超时*/
							/* 对时操作应答超时*/
							/* 充电服务启停应答超时*/
			TxBuf.data[1] = (ChgErrData.tCharParaAskTO<<1|\
							 ChgErrData.tCharStartAskTO<<2|\
							 ChgErrData.tWaitCharStartConTO<<3|\
							 ChgErrData.tCharStopAskTO<<4|\
							 ChgErrData.tWaitCharStopConTO<<5|\
							 ChgErrData.tTimeAskTO<<6|\
							 ChgErrData.tChargeServeOnOffAskTO<<7)&0xFF;
							/* 电子锁控制应答超时*/
							/* 充电功率调节应答超时*/		
							/* 充电桩配置信息查询应答超时*/
							/* 遥信报文接收超时*/
							/* 遥测报文接收超时*/	
			TxBuf.data[2] = (ChgErrData.tElecLockAskTO|\
							 ChgErrData.tPowerAdjustAskTO<<1|\
							 ChgErrData.tPileParaInfoAskTO<<2|\
							 ChgErrData.tCharYXRecTO<<3|\
							 ChgErrData.tCharYCRecTO<<4)&0xFF;			 
			DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.SendErrorStateFrameReSendFlag = TRUE;
				StrStateFrame.SendErrorStateFrameReSendCnt = 0;
			}
			break;
		}
		case PileParaInfoFrameAck:
		{
			PriorityLeve = PriorityLeve6;	   //优先级6  wyg170510
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;//SrcAdress

			TxBuf.data[0] = 0x01;//当前报文序号
			TxBuf.data[1] = 0x06;//报文总帧数
			TxBuf.data[2] = 0x25;//报文有效数据长度低字节
			TxBuf.data[3] = 0x00;//报文有效数据长度高字节
			TxBuf.data[4] = StrStateSystem.ChargIdent;	  //充电接口标识
			TxBuf.data[5] = 0xEA;	 //厂家编码
			TxBuf.data[6] = 0x03;
			TxBuf.data[7] = 0x00;
			strYC.Addition = 0;
			for(i=1;i<8;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
				  
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);

			TxBuf.data[0] = 0x02;   //当前报文序号
			TxBuf.data[1] = 0x00;
			TxBuf.data[2] = 0x80;	//设备型号A
			TxBuf.data[3] = 0x52;	//
			TxBuf.data[4] = 0x02;   //BMS协议版本号
			TxBuf.data[5] = 0xEA;	//充电控制器序列号（1~4：厂商编码）
			TxBuf.data[6] = 0x03;	
			TxBuf.data[7] = 0x00;

			for(i=1;i<8;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);

			TxBuf.data[0] = 0x03; //当前报文序号
			TxBuf.data[1] = 0x00; 	
			TxBuf.data[2] = 0x17;//充电控制器序列号（5~6：生产年份）
			TxBuf.data[3] = 0x20;
			TxBuf.data[4] = 0x34;//充电控制器序列号（7~8：生产批次）
			TxBuf.data[5] = 0x12;	
			TxBuf.data[6] = 0x78;//充电控制器序列号（9~12：生产序号）
			TxBuf.data[7] = 0x56; 			

			for(i=1;i<8;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);
			
			
			TxBuf.data[0] = 0x04; //当前报文序号
			TxBuf.data[1] = 0x34;
			TxBuf.data[2] = 0x12;
			TxBuf.data[3] = 0x00; //硬件次版本号
			TxBuf.data[4] = 0x01; //硬件主版本号
			TxBuf.data[5] = (((CrjPileVersion[5]-0x30)<<4)|(CrjPileVersion[6]-0x30))&0xFF;//发行版本号  {"V1.0.00"}; // 版本号	 		
			TxBuf.data[6] = (CrjPileVersion[3]-0x30)&0xFF;//次版本号
			TxBuf.data[7] = (CrjPileVersion[1]-0x30)&0xFF;//主版本号
			for(i=1;i<8;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);
			
			TxBuf.data[0] = 0x05;//当前报文序号
			TxBuf.data[1] = ((software_date[6]-'0')<<4)|(software_date[7]-'0');//年  LCF190307
			TxBuf.data[2] = ((software_date[4]-'0')<<4)|(software_date[5]-'0');//0x20	
			TxBuf.data[3] = ((software_date[0]-'0')<<4)|(software_date[1]-'0');//月	  
			TxBuf.data[4] = ((software_date[2]-'0')<<4)|(software_date[3]-'0');//日
			TxBuf.data[5] = STR_YC_Para_A.uiVolOverAlm&0xFF;			//最高输出电压
			TxBuf.data[6] = (STR_YC_Para_A.uiVolOverAlm>>8)&0xFF;
			TxBuf.data[7] = STR_YC_Para_A.uiVolUnderAlm&0xFF;			//最低输出电压		

			for(i=1;i<8;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);			
			
			TxBuf.data[0] = 0x06;//当前报文序号
			TxBuf.data[1] = (STR_YC_Para_A.uiVolUnderAlm>>8)&0xFF; 
			TxBuf.data[2] = (STR_YC_Para_A.uiCurAlm/10)&0xFF;		//最大输出电流 	
			TxBuf.data[3] = ((STR_YC_Para_A.uiCurAlm/10)>>8)&0xFF; 	 
			TxBuf.data[4] = 0x00;					  				//最小输出电流         
			TxBuf.data[5] = 0x00;	
			for(i=1;i<6;i++)
			{
			 	strYC.Addition += TxBuf.data[i];
			}
			
			TxBuf.data[6] =  strYC.Addition&0xff;      // 报文总帧数
			TxBuf.data[7] = (strYC.Addition>>8)&0xFF;  // 报文有效数据长度低字节			

			DataLength = 0x08;
			CAN1_Send_Msg(TxBuf,DataLength);
			delay_ms(2);
			
			if(Resend == TRUE)
			{
				StrStateFrame.PileParaInfoFrameAckReSendFlag = TRUE;
				StrStateFrame.PileParaInfoFrameAckReSendCnt = 0;
			}
			
			break;
		}
		case UpdateBeginFrameAck:
		{
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;
//	        TxBuf.data[0] = CrjPileVersion[3] - '0';//程序版本号  两字节
//	        TxBuf.data[1] = CrjPileVersion[6] - '0';
	        TxBuf.data[0] = StrStateSystem.ChargIdent;//充电接口标识
	        TxBuf.data[1] = StrStateSystem.DeviceType;//设备类型
	        TxBuf.data[2] = chgUpdate.UpdateConfIdent;//确认标识 0x00：允许下载 0x01：禁止下载 其他：无效
	        TxBuf.data[3] = chgUpdate.NoDownloadReason;//禁止下载原因	0x00：无 0x01：本身不支持此功能 0x02：数据合法性校验失败		
			DataLength = 0x04;
			CAN1_Send_Msg(TxBuf,DataLength);
			
			if(Resend == TRUE)
			{
				StrStateFrame.UpdateBeginFrameAckReSendFlag = TRUE;
				StrStateFrame.UpdateBeginFrameAckReSendCnt = 0;
			}
			
			break;
		}
		case DataSendReqFrameAck:
		{
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

	        TxBuf.data[0] = StrStateSystem.ChargIdent;//充电接口标识
	        TxBuf.data[1] = StrStateSystem.DeviceType;//设备类型

	        TxBuf.data[2] = 0x00;//确认标识 0x00：允许发送 0x01：禁止发送 其他：无效	
	        TxBuf.data[3] = STR_ProgramUpdate.file_totalNo&0xFF;//总包数
	        TxBuf.data[4] = (STR_ProgramUpdate.file_totalNo>>8)&0xFF;	
			
	        TxBuf.data[5] = STR_ProgramUpdate.Update_Package_No&0xFF;//获取包数
	        TxBuf.data[6] = (STR_ProgramUpdate.Update_Package_No>>8)&0xFF;			 	
			DataLength = 0x07;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.DataSendReqFrameAckReSendFlag = TRUE;
				StrStateFrame.DataSendReqFrameAckReSendCnt = 0;
			}
			
			break;
		}
		case DataSendFrameAck:
		{
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

	        TxBuf.data[0] = StrStateSystem.ChargIdent;//充电接口标识
	        TxBuf.data[1] = StrStateSystem.DeviceType;//设备类型
			
	        TxBuf.data[2] = chgUpdate.UpdateConfIdent; //确认标识 0x00：允许下载 0x01：禁止下载 其他：无效
	        TxBuf.data[3] = chgUpdate.NoDownloadReason;//禁止下载原因	0x00：无 0x01：本身不支持此功能 0x02：数据合法性校验失败			
			
	        TxBuf.data[4] = STR_ProgramUpdate.Update_Package_No&0xFF;//获取包数
	        TxBuf.data[5] = (STR_ProgramUpdate.Update_Package_No>>8)&0xFF;			 	
			DataLength = 0x06;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.DataSendFrameAckReSendFlag = TRUE;
				StrStateFrame.DataSendFrameAckReSendCnt = 0;
			}
			
			break;
		}		
		case UpdateCheckFrameAck:
		{
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

	        TxBuf.data[0] = StrStateSystem.ChargIdent;//充电接口标识
	        TxBuf.data[1] = StrStateSystem.DeviceType;//设备类型
	        TxBuf.data[2] = chgUpdate.UpdateConfIdent;//确认标识 0x00：成功 0x01：失败 其他：无效	
	        TxBuf.data[3] = chgUpdate.NoDownloadReason;//失败原因	0x00：无 0x01：检验不成功
			DataLength = 0x04;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.UpdateCheckFrameAckReSendFlag = TRUE;
				StrStateFrame.UpdateCheckFrameAckReSendCnt = 0;
			}
			
			break;
		}
		case ResetFrameAck:
		{
			PriorityLeve = PriorityLeve4;  
			
			TxBuf.id = (PriorityLeve<<26)+(SendCmd<<16)+(DestAdress<<8)+SrcAdress;	 

	        TxBuf.data[0] = StrStateSystem.ChargIdent;//充电接口标识
	        TxBuf.data[1] = StrStateSystem.DeviceType;//设备类型

	        TxBuf.data[2] = 0xAA;//确认标识 0xAA：成功 其他：无效	
			 	
			DataLength = 0x03;
			CAN1_Send_Msg(TxBuf,DataLength);
			if(Resend == TRUE)
			{
				StrStateFrame.ResetFrameAckReSendFlag = TRUE;
				StrStateFrame.ResetFrameAckReSendCnt = 0;
			}
			
			break;
		}
		default:
		{
			break;
		}
	}	
}
/********************************************************************  
接收一帧通讯函数  
原型：CAN_V110_RecProtocal(void)
功能：接收一帧通讯  
入口参数：  
出口参数：接收正确标志，0x00为接收处理中, 0x01成功 0x02失败
********************************************************************/ 
void CAN_V110_RecProtocal(void)
{
	static rt_uint32_t temp = 0,i = 0;
	static rt_uint8_t tempanalog = 0;
    struct rt_can_msg* pCan = &g_RxMessage;
	u8 pRxcmdstate;
	
	//接收数据处理
	pRxcmdstate = (pCan->id>>16)&0xFF;
	if(pRxcmdstate == ChargeStopFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeStopFrameAck:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == TimingFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecTimingFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == VertionCheckFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecVertionCheckFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == ChargeParaInfoFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecChargeParaInfoFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == ChargeServeOnOffFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecChargeServeOnOffFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == ElecLockFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecElecLockFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == PowerAdjustFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecPowerAdjustFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == PileParaInfoFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecPileParaInfoFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == ChargeStartStateFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecChargeStartStateFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == ChargeStopStateFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"RecChargeStopStateFrameAck:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == YcRecDataFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecYcRecDataFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == HeartRecFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecHeartFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == RecErrorStateFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecErrorStateFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}	
	else if(pRxcmdstate == FunPwmFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecFunPwmFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else if(pRxcmdstate == UpdateBeginFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"RecUpdateBeginFrame:"); 
		sprintf((char*)Printf_buff,"%08X",pCan->id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	else
	{
		sprintf((char*)USARTx_TX_BUF,"CAN1_RecFrame:");
		sprintf((char*)Printf_buff,"%08X",pCan->id);
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<pCan->len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",pCan->data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);			
	}
	
	switch(pRxcmdstate)   //命令号
	{
		case ChargeStopFrameAck:	//充电停止帧：优先级0x04，PF：0x03。
		{
			if(StrStateFrame.ChargeStopFrameReSendFlag == TRUE)  // 收到重复命令
			{
				rt_lprintf("Waring:Receive Double ChargeStopFrame!\n");
				
				break;
			}

            StrStateFrame.ChargeStopFrameReSendFlag = TRUE;			
			rt_lprintf("CAN_V110_Rec:Rec ChargeStopFrameAck\n");
			StrStateSystem.ChargIdent = pCan->data[0];       //充电接口标识
//			strCharStop.StopChargReson = pCan->data[1];      //停止充电原因0x01-正常停止0x02-故障终止0x03计费单元判断充电控制器故障停止
			STR_ChargePile_A.ChgState = state_WaitStop;
			WaitStop_time = 0;
			rt_lprintf("Waring:State_WaitStop,WaitStop_time\n");
			
			break;
		}
		case TimingFrame:	//能源路由器向充电控制器发送对时命令：优先级0x06；PF：0x05。
		{
			if(StrStateFrame.TimingFrameAckReSendFlag == TRUE)  // 收到重复命令
			{
				rt_lprintf("Waring:Receive Double TimingFrame!\n");
				
				break;
			}				
			StrStateSystem.ChargIdent = pCan->data[0]; //充电接口标识
			temp = ((pCan->data[1])+(pCan->data[2]<<8))/1000;
			if((temp<=60)&&((pCan->data[3]&0x3F)<60)&&((pCan->data[4]&0x1F)<24)\
				&&((pCan->data[5]&0x1F)<=31)&&((pCan->data[5]&0x1F)>=1)\
				&&((pCan->data[6]&0x0F)<=12) &&((pCan->data[6]&0x0F)>=1)\
				&&((pCan->data[7]&0x7F)<100)&&((pCan->data[3]&0x3F)!=System_Time_STR.Minute))//误差超过1分钟
			{								
				System_Time_STR.Second = temp;//秒
				System_Time_STR.Minute = pCan->data[3]&0x3F;	// 0-5位 -- 分钟
				System_Time_STR.Hour = pCan->data[4]&0x1F;   // 0-4位 -- 小时

				System_Time_STR.Day = pCan->data[5]&0x1F;    // 0-4位 -- 日期
				System_Time_STR.Week = pCan->data[5]&0xE0;   // 5-7位 -- 星期
				System_Time_STR.Month = pCan->data[6]&0x0F;  // 0-3位 -- 月
				System_Time_STR.Year = pCan->data[7]&0x7F;   // 0-6位 -- 年 后两位0-99
				
//					In_RTC_Init(&System_Time_STR);      //初始化片内RTC
//					SystemTimeToRTC(&System_Time_STR);  //初始化片外RTC
				
				rt_lprintf("Date:%02X-%02X-%02X Week:%x Time:%02X:%02X:%02X\n",System_Time_STR.Year,\
																				System_Time_STR.Month,\
																				System_Time_STR.Day,\
																				System_Time_STR.Week,\
																				System_Time_STR.Hour,\
																				System_Time_STR.Minute,\
																				System_Time_STR.Second); 		
				rt_lprintf("CAN_V110_Rec:timing ok!\n");//									
			}
			else
			{
				rt_lprintf("CAN_V110_Rec:rec time data error||gap<60s noneed timeing!\n");//
				rt_lprintf("Date:%02X-%02X-%02X Week:%x",pCan->data[7]&0x7F,\
														pCan->data[6]&0x0F,\
														pCan->data[5]&0x1F,\
														pCan->data[5]&0xE0); 		
				rt_lprintf("Time:%02X:%02X:%02X\n",pCan->data[4]&0x1F,\
													 pCan->data[3]&0x3F,\
													 temp);
			}
			Inform_Communicate_Can(TimingFrameAck,TRUE);//对时命令应答		

			break;
		}
		case VertionCheckFrameAck:	//能源路由器向充电控制器发送版本校验命令：优先级0x06，PF:0x07。
		{
			if(StrStateFrame.VertionCheckFrameReSendFlag == TRUE)  // 收到重复命令
			{
				rt_lprintf("Waring:Receive Double VertionCheckFrame!\n");
				break;
			}				
			
			StrStateSystem.ChargIdent = pCan->data[0]; //充电接口标识
			temp =(pCan->data[2]<<8)+pCan->data[1]; 
			if((STR_ChargePile_A.ChgState == state_WaitVertionCheck)&&(Pro_Version == temp))
			{			
				STR_ChargePile_A.ChgState = state_WaitPilePara;	
				rt_lprintf("CAN_V110_Rec:ChgState = State_WaitPilePara!\n");
				StrStateFrame.VertionCheckFrameReSendFlag = TRUE;
			}
			else
			{
				rt_lprintf("cuowu:CAN_V110_Rec:ChgState=%u!\n",STR_ChargePile_A.ChgState);
			}
			
			break;
		}			
		case ChargeParaInfoFrameAck:	//能源路由器下发充电桩(或一体充电机)参数信息。
		{
			if(StrStateFrame.ChargeParaInfoFrameReSendFlag == TRUE)  // 收到重复命令
			{
				rt_lprintf("Waring:Receive Double ChargeParaInfoFrameAck\n");
				break;
			}
			
			StrStateSystem.ChargIdent  = pCan->data[0]; //充电接口标识
			if(pCan->data[1] == 0x00)
			{
				strYX.WorkState = 0; // 0待机 1充电中工作 2充电完成 3充电暂停
				STR_ChargePile_A.ChgState = state_Standby;
				StrStateFrame.VertionCheckFrameAckReSendFlag = FALSE;
				StrStateFrame.VertionCheckFrameAckReSendCnt = 0;
				rt_lprintf("CAN_V110_Rec:State_Standby\n");
				
				StrStateFrame.YcSendDataFrameReSendFlag = TRUE;
				StrStateFrame.YcSendDataFrameReSendCnt = 0;				
				rt_lprintf("CAN_V110_Rec:Begin YcSendDataFrameReSendFlag\n");					
			}
			else
			{
				rt_lprintf("Waring:Receive ChargeParaInfoFrameAck\n");
				
			}

			break;
		}		
		case ChargeServeOnOffFrame:		 //充电服务启停控制
		{
			if(StrStateFrame.ChargeServeOnOffFrameAckReSendFlag == TRUE)  // 收到重复命令
			{
				rt_lprintf("Waring:Receive Double ChargeServeOnOffFrame!\n");
				break;
			}				
			
//				if(pCan->data[1] == SEVICEOK)
//				{
//					strChgSevCtrl.ChargSeveiceCmd = SEVICEOK;
//					strChgSevCtrl.CtrlState  = CTRLOK;
//					strChgSevCtrl.Ctrreaon= 0; 
//				}
//				else if((STR_ChargePile_A.TotalFau&CanOffLine_FAULT) == CanOffLine_FAULT)	//通信超时
//				{
//					strChgSevCtrl.ChargSeveiceCmd = pCan->data[1];
//					strChgSevCtrl.CtrlState       = CTRLERR;
//					strChgSevCtrl.Ctrreaon       = 2;		
//				}
//				else if((pCan->data[1]<1)||(pCan->data[1]>2))
//				{
//					strChgSevCtrl.ChargSeveiceCmd = pCan->data[1];
//					strChgSevCtrl.CtrlState       = CTRLERR;
//					strChgSevCtrl.Ctrreaon       = 1;
//				}
//				else
//				{
//					StrStateSystem.ChgSeviceState =      SEVICESTOP;
//					strChgSevCtrl.ChargSeveiceCmd = SEVICESTOP;
//					strChgSevCtrl.CtrlState       = CTRLOK;
//					strChgSevCtrl.Ctrreaon       = 0;
//				}

			Inform_Communicate_Can(ChargeServeOnOffFrameAck,TRUE);//充电服务启停应答

			break;
		}
		case ElecLockFrame:	   //电子锁控制	  16 01 10  liangbing
		{	
			if(StrStateFrame.ElecLockFrameAckReSendFlag == TRUE)  // 收到重复命令
			{
				rt_lprintf("Waring:Receive Double ElecLockFrame!\n");
				break;
			}				
			
			StrStateSystem.ChargIdent = pCan->data[0];  //充电接口标识
			strelelock.Num = pCan->data[1];  //电磁锁序号
			strelelock.Elelock = pCan->data[2];	 //操作指令 1-上锁，2-解锁

			if((pCan->data[1]<1)||(pCan->data[1]>2))
			{
				StrStateSystem.ReplyState = FAILED;
				StrStateSystem.reaonIdent = 0x01; //故障
			}
			else
			{
//					if(strelelock.Elelock == 1)
//					{
//						for(i=0;i<3;i++)
//						{
//							POWER_LED_ON();	
//						}
//					}
//					else 
//					{
//						for(i=0;i<3;i++)
//						{
//							POWER_LED_OFF();	
//						}
//					}
			}	
			Inform_Communicate_Can(ElecLockFrameAck,TRUE);//电子锁控制应答
			
			break;
		}
		case PowerAdjustFrame:	   //功率调节
		{
			if(StrStateFrame.PowerAdjustFrameAckReSendFlag == TRUE)  // 收到重复命令
			{
				rt_lprintf("Waring:Receive Double PowerAdjustFrame!\n");
				break;
			}					
			
			StrStateSystem.ChargIdent = pCan->data[0];    // 充电接口标识
//				strcapacity.C_capacity_style = pCan->data[1]; // 功率调节类型
//				strcapacity.C_capacity_value = (pCan->data[3]<<8)|pCan->data[2];			
//				if(strcapacity.C_capacity_style == 1) // 功率绝对值
//				{
//					if(strcapacity.C_capacity_value >=10000)
//					{
//						temp = strcapacity.C_capacity_value-10000;
//						rt_lprintf("CAN_V110_Rec:SetPower=%u.%ukW!\n",temp/10,temp%10);
//						if(temp<=200)
//						{
//							temp = temp*100*5/22/9;   //20 000/220/3 *5/3
//							if((temp < PWMDUTYRATIOMAX)&&(temp > PWMDUTYRATIOMIN))
//							{
//								STR_ChargePile_A.PWM_Duty = temp;
//								StrStateSystem.ReplyState = SUCCESSFUL;
//								StrStateSystem.reaonIdent = 0;
//							}
//							else
//							{						 	
//								STR_ChargePile_A.PWM_Duty = PWMDUTYRATIOMIN;
//								StrStateSystem.ReplyState = FAILED;
//								StrStateSystem.reaonIdent = 1;
//							}
//							rt_lprintf("CAN_V110_Rec:PWM_Duty1=%d!\n",STR_ChargePile_A.PWM_Duty);	
//						}
//						else
//						{
//							StrStateSystem.ReplyState = FAILED;
//							StrStateSystem.reaonIdent = 1;
//						}
//					}
//					else
//					{
//						StrStateSystem.ReplyState = FAILED;
//						StrStateSystem.reaonIdent = 1;
//					}
//			    }
//			    else if(strcapacity.C_capacity_style == 2) // 占空比
//			    {
//				  if(strcapacity.C_capacity_value<=100)
//				  {
//					  temp =(long) POWERMAX*strcapacity.C_capacity_value*10/132;
//				 
//					  if(temp < PWMDUTYRATIOMAX)
//					  {
//						 STR_ChargePile_A.PWM_Duty = temp;
//						 StrStateSystem.ReplyState = SUCCESSFUL;
//						 StrStateSystem.reaonIdent = 0;
//					  }
//					  else
//					  { 
//						 STR_ChargePile_A.PWM_Duty =  PWMDUTYRATIOMAX;
//						 StrStateSystem.ReplyState = FAILED;
//						 StrStateSystem.reaonIdent = 1;

//					  }
//					  rt_lprintf("CAN_V110_Rec:PWM_Duty2=%d!\n",STR_ChargePile_A.PWM_Duty);						  
//				  }
//				  else
//				  {
//					  StrStateSystem.ReplyState = FAILED;
//					  StrStateSystem.reaonIdent = 1;
//				  }				  
//			  }
//			  else
//			  {
//				  StrStateSystem.ReplyState = FAILED;
//				  StrStateSystem.reaonIdent = 1;
//			  }
		  Inform_Communicate_Can(PowerAdjustFrameAck,FALSE); // 功率调节应答

		  break;
		}
		case ChargeStartStateFrame:
		{
			//充电控制器向能源路由器发送"充电启动状态"：优先级0X04，PF：0X11
			if(StrStateFrame.ChargeStartStateFrameAckReSendFlag == TRUE)  // 收到重复命令
			{
				rt_lprintf("Waring:Receive Double ChargeStartStateFrame\n");
				break;
			}
			StrStateSystem.ChargIdent = pCan->data[0];         // 充电接口标识
			StrStateSystem.LdSwitch = pCan->data[1];           // 负荷控制开关 
			StrStateSystem.ConfIdent = pCan->data[2];          // 充电启动完成确认标识
			StrStateSystem.StartReson = pCan->data[3];     // 启动失败原因
			if(StrStateSystem.ConfIdent == SUCCESSFUL)
			{
				Inform_Communicate_Can(ChargeStartStateFrameAck,FALSE); // 启动状态应答
				STR_ChargePile_A.ChgState = state_Charging;
				rt_event_send(&ChargePileEvent, ChargeStartOK_EVENT);
				rt_lprintf("monitor_task:state_Charging&ChargeStartOK_EVENT\n");
			}

			StrStateFrame.ChargeStartStateFrameAckReSendFlag = TRUE;
			rt_lprintf("CAN_V110_Rec:Receive ChargeStartStateFrame\n");
//				rt_timer_stop(DownCount_Time);	//关闭定时器	
//				rt_lprintf("CAN_V110_Rec:rt_timer_stop DownCount_Time!\n");				

			break;
		}		
		case ChargeStopStateFrame:
		{
			//充电控制器向能源路由器发送"充电停止状态"：优先级0X04，PF：0X13
			if(StrStateFrame.ChargeStopStateFrameAckReSendFlag == TRUE)  // 收到重复命令
			{
				rt_lprintf("Waring:Receive Double ChargeStopStateFrame\n");
				break;
			}
			StrStateSystem.ChargIdent = pCan->data[0];         // 充电接口标识
			StrStateSystem.StopReson = pCan->data[1];          // 停止原因 
			StrStateSystem.ConfIdent = pCan->data[2];          // 充电启动完成确认标识
			if(StrStateSystem.ConfIdent == SUCCESSFUL)
			{
				Inform_Communicate_Can(ChargeStopStateFrameAck,FALSE); // 启动状态应答
				STR_ChargePile_A.ChgState = state_ChargEnd;
				rt_event_send(&ChargePileEvent, ChargeStopOK_EVENT);
				rt_lprintf("monitor_task:State_ChargEnd&ChargeStopOK_EVENT\n");
			}

			StrStateFrame.ChargeStopStateFrameAckReSendFlag = TRUE;
			rt_lprintf("CAN_V110_Rec:Receive ChargeStopStateFrameAckReSendFlag\n");
//				rt_timer_stop(DownCount_Time);	//关闭定时器	
//				rt_lprintf("CAN_V110_Rec:rt_timer_stop DownCount_Time!\n");				

			break;
		}
		case YcRecDataFrame://能源路由器向充电控制器周期性发送遥测数据：优先级0X06，PF0X31。
		{
//				rt_lprintf("CAN_V110_Rec:Rec YcRecDataFrame!\n");
			chgYC.MesNum = pCan->data[0]; //当前报文序号
			if(chgYC.MesNum == 1)
			{
//					chgYC.FraNum = pCan->data[1]; //当前报文总帧数
//					chgYC.ValData = (pCan->data[3]<<8)+pCan->data[2];
				StrStateSystem.ChargIdent = pCan->data[4];//充电接口标识
				strYC.ChargVa = (pCan->data[6]<<8)+pCan->data[5];
				tempanalog = pCan->data[7];	//B相电压低字节
				chgYC.Addition = 0x00000000;
				for(i=1;i<8;i++)
				{
					chgYC.Addition += pCan->data[i];
				}
			}
			else if(chgYC.MesNum == 2)
			{
				strYC.ChargVb = (pCan->data[1]<<8) + tempanalog;
				strYC.ChargVc = (pCan->data[3]<<8) + pCan->data[2];
				strYC.ChargIa = (pCan->data[5]<<8) + pCan->data[4];
				strYC.ChargIb = (pCan->data[7]<<7) + pCan->data[6];
				rt_lprintf("ChargIa=%u(.xx安)\n",strYC.ChargIa);
				rt_lprintf("ChargIb=%u(.xx安)\n",strYC.ChargIb);				
				for(i=1;i<8;i++)
				{
					chgYC.Addition += pCan->data[i];
				}
			}
			else if(chgYC.MesNum == 3)
			{
				strYC.ChargIc = (pCan->data[2]<<8) + pCan->data[1];
				rt_lprintf("ChargIc=%u(.xx安)\n",strYC.ChargIc);
				strYC.V_cp1 = (pCan->data[4]<<8) + pCan->data[3];
				STR_ChargePile_A.PWM_Duty = ((pCan->data[6]<<8) + pCan->data[5])/10;
				strYC.Tempt1 = pCan->data[7];			   
				for(i=1;i<8;i++)
				{
					chgYC.Addition += pCan->data[i];
				}
			}
			else if(chgYC.MesNum == 4)
			{
				strYC.Tempt2 = pCan->data[1];
				strYC.Tempt3 = pCan->data[2];
				strYC.Tempt4 = pCan->data[3];
				chgYC.TotalCheck = (pCan->data[5]<<8) + pCan->data[4];//累加校验
				for(i=1;i<4;i++)
				{
					chgYC.Addition += pCan->data[i];
				}				
				
			   if(chgYC.Addition == chgYC.TotalCheck)
			   {
					rt_lprintf("CAN_V110_Rec:Rec YC,Send YcSendDataFrame!\n");
//						Inform_Communicate_Can(YcSendDataFrame,FALSE);							
			   }
			   else
			   {
					rt_lprintf("CAN_V110_Rec:YcRecDataFrame TotalCheck error!\n");// 接收校验失败
			   }
			}
			break;
		}
		case YxRecDataFrame://充电控制器向计费单元周期性发送遥信数据：优先级0X06，PF：0X32。
		{
			StrStateSystem.ChargIdent = pCan->data[0]; // 充电接口标识

			strYX.WorkState = pCan->data[1]&0x03;
			strYX.TotalFau = (pCan->data[1]>>2)&0x01;
			strYX.TotalAlm = (pCan->data[1]>>3)&0x01;
			strYX.ConnState = (pCan->data[1]>>4)&0x01;
			strYX.StopEctFau = (pCan->data[1]>>5)&0x01;
			strYX.ArresterFau = (pCan->data[1]>>6)&0x01;
			strYX.GunOut = (pCan->data[1]>>7)&0x01;
			
			strYX.ChgTemptOVFau = pCan->data[2]&0x01;
			strYX.VOVWarn = (pCan->data[2]>>1)&0x01;
			strYX.VLVWarn = (pCan->data[2]>>2)&0x01;
			strYX.OutConState = (pCan->data[2]>>3)&0x01;
			strYX.PilotFau = (pCan->data[2]>>4)&0x01;
			strYX.ACConFau = (pCan->data[2]>>5)&0x01;
			strYX.OutCurrAla = (pCan->data[2]>>6)&0x01;
			strYX.OverCurrFau = (pCan->data[2]>>7)&0x01;
				   
			strYX.ACCirFau = pCan->data[3]&0x01;
			strYX.LockState = (pCan->data[3]>>1)&0x01;
			strYX.LockFauState = (pCan->data[3]>>2)&0x01;
			strYX.GunTemptOVFau = (pCan->data[3]>>3)&0x01;
			strYX.CC = (pCan->data[3]>>4)&0x01;
			strYX.CP = (pCan->data[3]>>5)&0x03;
			strYX.PEFau = (pCan->data[3]>>7)&0x01;
			
			strYX.DooropenFau = pCan->data[4]&0x01;
			strYX.ChgTemptOVAla = (pCan->data[4]>>1)&0x01;
			strYX.GunTemptOVAla = (pCan->data[4]>>2)&0x01;
			strYX.Contactoradjoin = (pCan->data[4]>>3)&0x01;

			strYX.GenAlaFau = pCan->data[5];
			strYX.OthWarnNum = pCan->data[6]; // 其他报警编号
			strYX.OthWarnVal = pCan->data[7]; // 其他报警值

			break;			             
		}			
		case HeartRecFrame://能源路由器向充电控制器周期性发送心跳数据：优先级0X06，PF：0X31。
		{
			StrStateSystem.ChargIdent = pCan->data[0]; // 充电接口标识
			StrStateSystem.RecCount = pCan->data[1];   // 心跳报文发送计数
			StrStateSystem.RecStatus = pCan->data[2];  // 心跳报文接收状态
			if(StrStateSystem.RecStatus == FAILED)
			{
				rt_lprintf("Error:接收到异常心跳帧\n");
			}
			
			break;
		}
		case RecErrorStateFrame://能源路由器错误数据帧：优先级0X04，PF：0X51。
		{
			StrStateSystem.ChargIdent = pCan->data[0];           // 充电接口标识
//				StrErrData.VerChTimeOut = pCan->data[1]&0x01;        // 版本校验超时 
//				StrErrData.ParSetTimeOut = pCan->data[1]&0x02;       // 参数设置超时 	
//				StrErrData.StartCharConTimeOut = pCan->data[1]&0x04; // 启动充电命令确认超时
//				StrErrData.WStartCharTimeOut = pCan->data[1]&0x08;   // 等待启动充电完成状态超时
//				StrErrData.StopCharConTimeOut = pCan->data[1]&0x10;  // 停止充电命令确认超时  
//				StrErrData.WStopCharTimeOut = pCan->data[1]&0x20;    // 等待停止充电完成状态超时 
			rt_lprintf("Error:Receive RecErrorStateFrame\n");
			
			break;
		}
		case PileParaInfoFrame:				//充电桩配置信息查询
		{
			if(StrStateFrame.PileParaInfoFrameAckReSendFlag == TRUE)  // 收到重复命令
			{
				rt_lprintf("Waring:Receive Double PileParaInfoFrame!\n");
				break;
			}
			Inform_Communicate_Can(PileParaInfoFrameAck,FALSE);
			
			break;
		}
		case FunPwmFrame:				   //充电桩配置信息查询
		{
//				StrStateSystem.ChargIdent = pCan->data[0];    // 充电接口标识
//				STR_Charge_B.PWM_Duty = pCan->data[1]*10;     // 设置B枪PWM占空比	
//				PWM_SET_B(STR_Charge_B.PWM_Duty); 			

//				rt_lprintf("CAN_V110_Rec:STR_Charge_B.PWM_Duty=%u!\n",STR_Charge_B.PWM_Duty);
			
			break;
		}
		case UpdateBeginFrame:			   //升级命令
		{
			if(StrStateFrame.UpdateBeginFrameAckReSendFlag == TRUE)  // 收到重复命令
			{
				rt_lprintf("Waring:Receive Double UpdateBeginFrame!\n");
				
				break;
			}				
			if((STR_ChargePile_A.ChgState == state_Standby)&&(STR_ProgramUpdate.App_Mark == 0x00)) //1 正在升级  0 未升级
			{
				StrStateSystem.ChargIdent = pCan->data[0];
				StrStateSystem.DeviceType = pCan->data[1];
				
//					if(g_flash_txt == NULL)  
//					{
//						g_flash_txt=(FIL *)mymalloc(SRAMCCM,sizeof(FIL));//开辟FIL字节的内存区域
//						rt_lprintf("Can_Update:g_flash_txt=%u\n",g_flash_txt);
//					}	
//					//目标文件夹存在该文件,先删除
//					rt_lprintf("%s file delete,res=%u!\n",FLASH_FILE_PROGRAM_BIN_FILE,f_unlink((const TCHAR *)FLASH_FILE_PROGRAM_BIN_FILE));
//					
//					STR_ProgramUpdate.App_Mark = 0x01;          //1：正在升级  0 未升级  2准备中
//					STR_ProgramUpdate.App_Channel = 0x03;       //0：网口，1：2G，2：4G，3：can
//					STR_ProgramUpdate.Update_Package_No = 0x01;	//升级文件段包数清零  从1开始计数
//					STR_ProgramUpdate.Update_Message_No = 0x01; //升级每包序列号      从1开始计数
//					STMFLASH_LENTH = 0;
//					chgUpdate.ErrResendCount = 0;
//					STR_ProgramUpdate.AdditionAll = 0;
//					STR_ProgramUpdate.Update_Confirm = 1;       //默认升级失败
//	                chgUpdate.UpdateConfIdent = 0x00;           //确认标识 0x00：允许下载 0x01：禁止下载 其他：无效
//	                chgUpdate.NoDownloadReason = 0x00;          //禁止下载原因	0x00：无 0x01：本身不支持此功能 0x02：数据合法性校验失败
//					ProgramUpdatePara(WRITE);
//					STR_ChargePile_A.ChgState = State_Update;
//					rt_lprintf("CAN_V110_Rec:ChgState:State_Update\n");

				rt_timer_stop(CAN_Heart_Tx);// 关闭定时器
				rt_timer_stop(CAN_250ms_Tx);// 关闭定时器
			}
			else
			{
//					StrStateSystem.ChargIdent = pCan->data[0];
//					StrStateSystem.DeviceType = pCan->data[1];
//	                chgUpdate.UpdateConfIdent = 0x01;//确认标识 0x00：允许下载 0x01：禁止下载 其他：无效
//	                chgUpdate.NoDownloadReason = 0x01;//禁止下载原因	0x00：无 0x01：本身不支持此功能 0x02：数据合法性校验失败					
//					Inform_Communicate_Can(UpdateBeginFrameAck,FALSE);										
				rt_lprintf("CAN_V110_Rec:ChgState=%u,App_Mark=%u\n",STR_ChargePile_A.ChgState,STR_ProgramUpdate.App_Mark);
			}
			rt_lprintf("CAN_V110_Rec:接收到升级命令!!!!!!!!!!!!\n");
			
			break;
		}
		case DataSendReqFrame://收到请求发送数据帧命令
		{
			if(STR_ChargePile_A.ChgState == state_Update) //1 正在升级  0 未升级
			{
				StrStateSystem.ChargIdent = pCan->data[0];
				StrStateSystem.DeviceType = pCan->data[1];
				STR_ProgramUpdate.file_totalNo = (pCan->data[3]<<8) + pCan->data[2];
				STR_ProgramUpdate.file_ByteNo = (pCan->data[7]<<24) + (pCan->data[6]<<16) + (pCan->data[5]<<8) + pCan->data[4];									
				rt_lprintf("CAN_V110_Rec:file_totalNo=%u,file_ByteNo=%u\n",STR_ProgramUpdate.file_totalNo,STR_ProgramUpdate.file_ByteNo);
//					OSSemPost(MY_CAN_A);//发送信号量 收到请求发送数据帧
			}
			else
			{										
				rt_lprintf("CAN_V110_Rec:ChgState=%u\n",STR_ChargePile_A.ChgState);
			}
			
			break;
		}			
		case DataSendFrame:	//接收升级数据中
		{
			chgUpdate.MessageNum = pCan->data[0]; //当前报文序号
			if(chgUpdate.MessageNum == 1)
			{
				chgUpdate.FrameNum = pCan->data[1]; //当前报文总帧数
				chgUpdate.ValDataLen = (pCan->data[3]<<8) + pCan->data[2];//报文有效数据长度
				StrStateSystem.ChargIdent = pCan->data[4];//充电接口标识
				StrStateSystem.DeviceType = pCan->data[5];//设备类型
				rt_lprintf("CAN_V110_Rec:FrameNum=%u,ValDataLen=%u\n",chgUpdate.FrameNum,chgUpdate.ValDataLen);
				memcpy(STMFLASH_BUFF+STMFLASH_LENTH,&(pCan->data[6]),2);	        //将当前数据包放入缓存中
				STMFLASH_LENTH += 2;															
				STR_ProgramUpdate.Update_Message_No++;					
				rt_lprintf("CAN_V110_Rec:STMFLASH_LENTH=%u,Update_Message_No=%u\n",STMFLASH_LENTH,STR_ProgramUpdate.Update_Message_No);
				chgUpdate.AdditionUp = 0x0000;
				for(i=1;i<8;i++)
				{
					chgUpdate.AdditionUp += pCan->data[i];
				}
				for(i=6;i<8;i++)
				{
					STR_ProgramUpdate.AdditionAll += pCan->data[i];
				}
			}				
			else if(chgUpdate.MessageNum == STR_ProgramUpdate.Update_Message_No)
			{
				//判断最后一帧数据
				if(chgUpdate.FrameNum == STR_ProgramUpdate.Update_Message_No)
				{
					rt_lprintf("CAN_V110_Rec:ValDataLen=%u,STMFLASH_LENTH=%u\n",chgUpdate.ValDataLen,STMFLASH_LENTH);
					memcpy(STMFLASH_BUFF+STMFLASH_LENTH,&(pCan->data[1]),chgUpdate.ValDataLen - STMFLASH_LENTH);//将当前数据包放入缓存中
					STMFLASH_LENTH = chgUpdate.ValDataLen - 2;
					for(i=2;i<STMFLASH_LENTH;i++)//前两个数据已经在第一帧中校验
					{
						chgUpdate.AdditionUp += STMFLASH_BUFF[i];
						STR_ProgramUpdate.AdditionAll += STMFLASH_BUFF[i];
					}
					
					chgUpdate.RevCheckUp = (STMFLASH_BUFF[STMFLASH_LENTH+1]<<8) + STMFLASH_BUFF[STMFLASH_LENTH];//累加校验
					if(chgUpdate.AdditionUp == chgUpdate.RevCheckUp)
					{
//							Bin_Write_File((char *)FLASH_FILE_PROGRAM_BIN_FILE,STMFLASH_BUFF,STMFLASH_LENTH);
//							rt_lprintf("CAN_V110_Rec:接收第%u包-序号%u帧正确\n",STR_ProgramUpdate.Update_Package_No,STR_ProgramUpdate.Update_Message_No);
//							//判断不是最后一包数据
//							if(STR_ProgramUpdate.file_totalNo == STR_ProgramUpdate.Update_Package_No)
//							{
//								STR_ProgramUpdate.Update_Package_No = 0x00;
//							}
//							else
//							{
//								STR_ProgramUpdate.Update_Package_No++;
//								chgUpdate.ErrResendCount = 0;
//							}
//							STMFLASH_LENTH = 0;
//							chgUpdate.UpdateConfIdent = 0x00;           //确认标识 0x00：成功 0x01：失败 其他：无效
//							chgUpdate.NoDownloadReason = 0x00;          //禁止下载原因	0x00：无 0x01：校验不成功 0x02：檫除失败 0x03：接收不完整					
//							Inform_Communicate_Can(DataSendFrameAck,FALSE);
//							STR_ProgramUpdate.Update_Message_No = 0x01;	
					}
					else
					{
						chgUpdate.ErrResendCount++;
						rt_lprintf("CAN_V110_Rec:CRC错误-AdditionUp=%u,RevCheckUp=%u,ErrResendCount=%u\n",chgUpdate.AdditionUp,chgUpdate.RevCheckUp,chgUpdate.ErrResendCount);

						if(chgUpdate.ErrResendCount >= 3)
						{
							STMFLASH_LENTH = 0;
							STR_ProgramUpdate.Update_Message_No = 0x00;								
							chgUpdate.UpdateConfIdent = 0x01;           //确认标识 0x00：成功 0x01：失败 其他：无效
							chgUpdate.NoDownloadReason = 0x01;          //禁止下载原因	0x00：无 0x01：校验不成功 0x02：檫除失败 0x03：接收不完整
							STR_ProgramUpdate.Update_Package_No = 0x01;								
							Inform_Communicate_Can(DataSendFrameAck,FALSE);

							STR_ProgramUpdate.App_Mark = 0x00;        //1 正在升级  0 未升级
							Inform_Communicate_Can(SendErrorStateFrame,FALSE);
							chgUpdate.ErrResendCount = 0;
							STR_ChargePile_A.ChgState = state_Standby;
							rt_lprintf("CAN_V110_Rec:升级失败,回到待机\n");
							rt_timer_start(CAN_Heart_Tx);	// 开启定时器
							rt_timer_start(CAN_250ms_Tx);	// 开启定时器
						}
						else
						{
							STMFLASH_LENTH = 0;					
							chgUpdate.UpdateConfIdent = 0x01;           //确认标识 0x00：成功 0x01：失败 其他：无效
							chgUpdate.NoDownloadReason = 0x01;          //禁止下载原因	0x00：无 0x01：校验不成功 0x02：檫除失败 0x03：接收不完整							
							Inform_Communicate_Can(DataSendFrameAck,FALSE);
							STR_ProgramUpdate.Update_Message_No = 0x01;								
						}
					}	
				}					
				else
				{
					memcpy(STMFLASH_BUFF+STMFLASH_LENTH,&(pCan->data[1]),7);//将当前数据包放入缓存中
					STMFLASH_LENTH += 7;
					STR_ProgramUpdate.Update_Message_No++;
				}	
			}
			else
			{
				rt_lprintf("CAN_V110_Rec:错误-MessageNum=%u\n",chgUpdate.MessageNum);
			}
//				OSSemPost(MY_CAN_A);//发送信号量 离线上传成功
			rt_lprintf("CAN_V110_Rec:接收到数据帧序号=%u\n",chgUpdate.MessageNum);
			break; 
		}
		case UpdateCheckFrame:	//接收程序校验数据帧
		{
			StrStateSystem.ChargIdent = pCan->data[0];
			StrStateSystem.DeviceType = pCan->data[1];
			if(STR_ProgramUpdate.AdditionAll == ((pCan->data[5]<<24) +(pCan->data[4]<<16) +(pCan->data[3]<<8) + pCan->data[2]))
			{
				STR_ProgramUpdate.Update_Confirm = 0;  //升级成功
				chgUpdate.UpdateConfIdent = 0x00;      //确认标识 0x00：成功 0x01：失败 其他：无效
				chgUpdate.NoDownloadReason = 0x00;     //禁止下载原因	0x00：无 0x01：校验不成功 0x02：檫除失败 0x03：接收不完整						
			}
			else
			{
				STR_ProgramUpdate.Update_Confirm = 0x01;    //升级标志  0成功   1失败
				chgUpdate.UpdateConfIdent = 0x01;           //确认标识 0x00：成功 0x01：失败 其他：无效
				chgUpdate.NoDownloadReason = 0x01;          //禁止下载原因	0x00：无 0x01：校验不成功 0x02：檫除失败 0x03：接收不完整
				rt_lprintf("CAN_V110_Rec:Error AdditionAll=0x%08X\n",STR_ProgramUpdate.AdditionAll);
			}
			Inform_Communicate_Can(UpdateCheckFrameAck,FALSE);
//				OSSemPost(MY_CAN_A);//发送信号量 离线上传成功	
			
			break;
		}
		case ResetFrame:	//接收程序重启帧
		{
			StrStateSystem.ChargIdent = pCan->data[0];
			StrStateSystem.DeviceType = pCan->data[1];
			StrStateSystem.ResetAct = pCan->data[2];				
			if(StrStateSystem.ResetAct ==0xAA)
			{
				Inform_Communicate_Can(ResetFrameAck,FALSE);
//					if(g_flash_txt != NULL)  
//					{
//						myfree(SRAMCCM,g_flash_txt);
//						rt_lprintf("CAN_V110_Rec:g_flash_txt=%u\n",g_flash_txt);
//						g_flash_txt = NULL;
//						if(ProgramUpdatePara(WRITE))
//						{
//							//目标文件夹存在该文件,先删除
//							rt_lprintf("%s file delete,res=%u!\n",FLASH_FILE_PROGRAM_BIN_FILE,f_unlink((const TCHAR *)FLASH_FILE_PROGRAM_BIN_FILE));//																	
//						}
//						else
//						{
//							rt_lprintf("CAN_V110_Rec:ProgramUpdatePara WRITE_OK\n");									
//						}						
//					}
//					if(usbx.hdevclass==1)//优先存储到U盘
//					{
//						NAND_LogWrite(LogBuffer,&LogBufferLen);
//						usbapp_mode_stop();//先停止当前USB工作模式																
//					}
				
				__set_FAULTMASK(1); //关闭所有中断
				NVIC_SystemReset(); //复位
			}
			else
			{
				rt_lprintf("CAN_V110_Rec:Error ResetAct=%u!\n",StrStateSystem.ResetAct);
			}
			
			break;
		}			
		default:
		{
			break;
		}
	}
	can_heart_count	= 0; //收到数据，心跳包计数清零
	STR_ChargePile_A.TotalFau &= ~CanOffLine_FAULT;
}
/***************************************************************
* 函数名称: CAN1_Send_Msg(struct rt_can_msg CanSendMsg,u8 len)
* 参    数:
* 返 回 值:
* 描    述: 充电控制器向计费单元发送数据
***************************************************************/
u8 CAN1_Send_Msg(struct rt_can_msg CanSendMsg,u8 length)
{
//	uint32_t can1tx_mailbox = HAL_OK;
	u8 SendCmd = 0;
	struct rt_can_msg Tx_Message;	
	u32 i;
	memset(Tx_Message.data,0x00,sizeof(Tx_Message.data)); // 初始化发送缓存
	Tx_Message.id = CanSendMsg.id;                        // 扩展标示ID（29 位）
	Tx_Message.ide = RT_CAN_EXTID;                        // 扩展帧
	Tx_Message.rtr = RT_CAN_DTR;                          // 数据帧
	Tx_Message.len = 0x08;
	
	for(i=0;i<length;i++)
	{
	    Tx_Message.data[i]=CanSendMsg.data[i];
	}

	SendCmd = (Tx_Message.id>>16)&0xFF;
	if(SendCmd == ChargeStartFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeStartFrameAck:");
	    sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}	
	else if(SendCmd == ChargeStopFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeStopFrameAck:"); 
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);	
	}
	else if(SendCmd == YcSendDataFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"YcSendDataFrame:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}	
	else if(SendCmd == TimingFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"TimingFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == VertionCheckFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"VertionCheckFrame:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}	
	else if(SendCmd == ChargeParaInfoFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeParaInfoFrame:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == ChargeServeOnOffFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeServeOnOffFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == ElecLockFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ElecLockFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == PowerAdjustFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"PowerAdjustFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == PileParaInfoFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"PileParaInfoFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == ChargeStartStateFrameAck)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeStartStateFrameAck:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == ChargeStopStateFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"ChargeStopStateFrame:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == HeartSendFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"HeartSendFrame:");
	    sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}
	else if(SendCmd == SendErrorStateFrame)
	{
		sprintf((char*)USARTx_TX_BUF,"ErrorStateFrame:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);
	}	
	else
	{
		sprintf((char*)USARTx_TX_BUF,"CAN1_Send_Msg:");
		sprintf((char*)Printf_buff,"%08X",Tx_Message.id); 
		strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);
		strcat((char*)USARTx_TX_BUF,(const char*)"--");
			
		for(i=0;i<Tx_Message.len;i++)   //循环发送数据
		{
			sprintf((char*)Printf_buff,"%02X",Tx_Message.data[i]); 
			strcat((char*)USARTx_TX_BUF,(const char*)Printf_buff);			
		}
		rt_lprintf("%s\n",USARTx_TX_BUF);		
	}

	rt_device_write(chargepile_can, 0, &Tx_Message, sizeof(struct rt_can_msg));

	return 0;		   	
}

#endif /*RT_USING_CAN*/
