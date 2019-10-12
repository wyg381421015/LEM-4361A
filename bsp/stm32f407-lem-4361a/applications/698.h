
//#include <rtthread.h>
#include <rtdevice.h>
#include <strategy.h>

#define HPLC_UART_NAME       "uart5"  /* 串口设备名称 */

#define BLUE_TOOTH_UART_NAME       "uart3"  /* 串口设备名称 */
extern struct rt_thread hplc;
extern struct _698_STATE hplc_698_state;
extern rt_uint32_t hplc_event;
extern int hplc_lock1,hplc_lock2;
extern rt_uint32_t strategy_event;



/****************宏定义**********************************/
//定义事件类型
typedef enum {
	CTRL_NO_EVENT             =0x00000000,
	ChgPlanIssue_EVENT       	=0x00000001,        	//充电计划下发事件
	ChgPlanIssueGet_EVENT     =0x00000002,      		//充电计划召测事件	
	ChgPlanAdjust_EVENT    		=0x00000004,        	//充电计划调整事件
	StartChg_EVENT						=0x00000008,          //启动充电事件
	StopChg_EVENT							=0x00000010,          //停止充电事件		          			
} CTRL_EVENT_TYPE;//控制器→路由器的命令事件



typedef struct
{
	int array_size;
	PLAN_FAIL_EVENT *plan_fail_event;

}_698_PLAN_FAIL_EVENT;



typedef struct
{
	int array_size;
	CHARGE_STRATEGY *charge_strategy;

}_698_CHARGE_STRATEGY;
/**********/

typedef enum {
	Cmd_ChgPlanIssue=0, 					//充电计划下发
	Cmd_ChgPlanAdjust,                 		//充电计划调整
	Cmd_ChgPlanIssueAck,                 	//充电计划下发应答
	Cmd_ChgPlanAdjustAck,                 //充电计划调整应答
	Cmd_StartChg,								//启动充电参数下发
	Cmd_StartChgAck,						//启动充电应答
	Cmd_StopChg,								//停止充电参数下发
	Cmd_StopChgAck,							//停止充电应答
	Cmd_ChgRecord,													//上送充电订单
	Cmd_ChgPlanExeState,                    //上送充电计划执行状态
	Cmd_DeviceFault,                      	//上送路由器异常状态
	Cmd_PileFault,                 					//上送充电桩异常状态
	Cmd_ChgPlanIssueGetAck,
}COMM_CMD_C;//路由器→控制器的命令号
#define COMM_CMD_C rt_uint32_t

//事件

#define  link_request	1
#define  link_request_load	0
#define  link_request_heart_beat	1
#define  link_request_unload	1


#define  link_response 0x81  //要是有response也应该在list中
#define  connect_request 2
#define  release_request 3

#define  get_request 5
#define  set_request 6	
#define  action_request 7
#define  report_response 8
#define  proxy_request 9
#define  connect_response 0x82
#define  release_response 0x83
#define  release_notification 0x84  //这个怎么处理
#define  get_response 0x85
#define  set_response 0x86
#define  action_response 0x87
#define  report_notification 0x88
#define  proxy_response 0x89
#define  security_request 16
#define  security_response 144




//GET-Request∷=CHOICE
//读取一个对象属性请求 [1] ;读取若干个对象属性请求 [2] ;读取一个记录型对象属性请求 [3] 
//;读取若干个记录型对象属性请求 [4] 读取分帧响应的下一个数据块请求 [5];读取一个对象属性的 MD5 值 [6]
#define  GetRequestNormal	1
#define  GetRequestNormalList	2
#define  GetRequestRecord	3
#define  GetRequestRecordList 4
#define  GetRequestNext 5
#define  GetRequestMD5 6

//操作请求的数据类型
#define  ActionRequest 1//操作一个对象方法请求 [1] ，
#define  ActionRequestList 2//操作若干个对象方法请求 [2] ，
#define  ActionThenGetRequestNormalList 3//操作若干个对象方法后读取若干个对象属性请求 [3] 

//上报类型
#define  ReportNotificationList 1
#define  ReportNotificationRecordList 2



#define OI1_MASK 0XF0
#define OI2_MASK 0X0F

enum Data_T{
Data_NULL =0,
Data_array=1,
Data_structure=2,
Data_bool=3,
Data_bit_string=4,           //一个bit占一个byte
Data_double_long=5,         //32 比特位整数（ Integer32）
Data_double_long_unsigned=6,//32 比特位正整数（ double-long-unsigned）
Data_octet_string=9,
Data_visible_string=10,
Data_UTF8_string=12,
Data_integer=15,						//8 比特位整数（ integer）
Data_long=16,
Data_unsigned=17,
Data_long_unsigned=18,			//（ Unsigned16）
Data_long64=20,
Data_long64_unsigned=21,
Data_enum=22,
Data_float32=23,
Data_float64=24,
Data_date_time=25,
Data_date=26,
Data_time=27,
Data_date_time_s=28,
Data_OI=80,
Data_OAD=81,
Data_ROAD=82,
Data_OMD=83,
Data_TI=84,
Data_TSA=85,
Data_MAC=86,
Data_RN=87,
Data_Region=88,
Data_Scaler_Unit=89,
Data_RSD=90,
Data_CSD=91,
Data_MS=92,
Data_SID=93,
Data_SID_MAC=94,
Data_COMDCB=95,
Data_RCSD=96
};
enum Object_I{
oi_electrical_energy=0,//电能量类对象
oi_max_demand,
oi_variable,//变量类对象
oi_event,
oi_parameter,//参变量类对象
oi_freeze,
oi_Acquisition_monitor,//采集监控
oi_gather,//集合
oi_contral,//控制类对象的标识	
oi_file_transfer=0xf//文件传输
//oi_esam,//ESAM接口后面都是f
//oi_io_dev,//输入输出设备
//oi_display,//显示	
//oi_define_by_user//
};
/****************************************/

//static struct CharPointDataManage _698_data_rev,_698_data_send;//最好不用
//static struct _698_ADDR *_698_addr;//没啥用

static unsigned char test_hplc[200]={
0xFE , 0xFE , 0xFE , 0xFE , 0x68 , 0x34 , 
0x00 , 0xC3 , 0x05 , 0x02 , 0x00 , 0x04 , 
0x12 , 0x18 , 0x00 , 0x00 , 0xB7 , 0x02 , 
0x85 , 0x01 , 0x1D , 0x00 , 0x10 , 0x02 , 
0x00 , 0x01 , 0x01 , 0x05 , 0x06 , 0x00 , 
0x00 , 0x00 , 0x66 , 0x06 , 0x00 , 0x00 , 
0x00 , 0x24 , 0x06 , 0x00 , 0x00 , 0x00 , 
0x40 , 0x06 , 0x00 , 0x00 , 0x00 , 0x00 , 
0x06 , 0x00 , 0x00 , 0x00 , 0x01 , 0x00 , 
0x00 , 0xF7 , 0x08 , 0x16,	
0xfe, 0xfe, 0xfe, 0xfe, 0xfe,	
0xFE, 0xFE, 0xFE, 0xFE, 0x68, 
0x17 , 0x00 , 0x43 , 0x45 , 0xAA , 
0xAA , 0xAA , 0xAA , 0xAA , 0xAA ,
0x00 , 0x5B , 0x4F , 0x05 , 0x01 , 
0x00 , 0x40 , 0x01 , 0x02 , 0x00 , 
0x00 , 0xED , 0x03 , 0x16	,
0xfe, 0xfe, 0xfe, 0xfe, 0xfe,
0x68, 0x1e, 0x00, 0x81, 0x05,
0x07, 0x09, 0x19, 0x05, 0x16,
0x20, 0x00, 0x60, 0x30, 0x01, 
0x00, 0x00, 0x00, 0xb4, 0x07,
0xe0, 0x05, 0x13, 0x04, 0x08,	
0x05, 0x00, 0x00, 0xa4, 0xfc,
0x83, 0x16
};
/*, 0xfc, 0x49, 0x01, 
0x00, 0x00, 0x00, 0xb4, 0x07,
0xe0, 0x05, 0x13, 0x04, 0x08,	
0x05, 0x00, 0x00, 0xa4, 0xa8,
0xd3, 0x16	*/
/***********只留和698协议相关的结构体和函数*****************************/
/***********
开始定义的是基本数据类型,不定长度的才用指针，长度确定的用数组（指定长度的指针）
为方便书写，都用的小写，其实用开头是大写也不是太麻烦！
************/
struct _698_Scaler_Unit{
	int index;//换算——倍数因子的指数，基数为 10；如数值不是数字的，则换算应被置 0。
  int type;//枚举类型定义物理单位
};
struct _698_comdcb{
	unsigned char baud;//300bps（ 0）， 600bps（ 1）， 1200bps（ 2），2400bps（ 3）， 4800bps（ 4）， 7200bps（ 5），
										 //9600bps（ 6）， 19200bps（ 7）， 38400bps（ 8），57600bps（ 9）， 115200bps（ 10）， 自适应（ 255）	
	unsigned char parity;//无校验（ 0），奇校验（ 1），偶校验（ 2）
	unsigned char d_bits;//5（ 5）， 6（ 6）， 7（ 7）， 8（ 8）
	unsigned char s_bit;//1（ 1）， 2（ 2） 
	unsigned char f_control;//	无(0)， 硬件(1)， 软件(2)
};
struct _698_time{
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char data[3];//这样是不是更好	
};
struct _698_date{
	unsigned char year[2];//year、 milliseconds=FFFFH 表示无效。
	unsigned char month;//FFH 表示无效。下同
	unsigned char day;
	unsigned char data[4];//这样是不是更好	
};
struct _698_date_time_s{
	unsigned char year[2];//year、 milliseconds=FFFFH 表示无效。
	unsigned char month;//FFH 表示无效。下同
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char data[7];//这样是不是更好	
};
struct _698_date_time{
	unsigned char year[2];//year、 milliseconds=FFFFH 表示无效。
	unsigned char month;//FFH 表示无效。下同
	unsigned char day;
	unsigned char week;//day_of_week： 0 表示周日， 1~6 分别表示周一到周六。
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char millisconds[2];//
	unsigned char data[10];//这样是不是更好
};
/*
这个用来放
attribute_method_id
*/
struct _698_oad{//对象属性描述符OAD（ Object Attribute Descriptor）
	unsigned char oi[2];//对象标识（ OI）由两字节组成，采用分类编码的方式为系统的各个对象提供标识码.
											//参看P112,涵盖的对象：电能量类对象标识；最大需量类；变量类	；事件类（这个似乎也是需要上报的内容）；参变量类;
											//冻结类;采集监控类;集合类;控制类;文件传输类;ESAM 接口类;输入输出设备接口类;显示类;厂家自定义对象
	unsigned char  attribute_id;//属性标识及其特征,bit0…bit4 编码表示对象属性编号，取值 0…31，其中 0 表示整个对象属性，即对象的所有属性；
															//bit5…bit7 编码表示属性特征，属性特征是对象同一个属性在不同快照环境下取值模式，取值 0…7，特征含义在具体
															//类属性中描述。
	unsigned char  attribute_index;//属性内元素索引,00H 表示整个属性全部内容。如果属性是一个结构或数组， 01H 指向对象属性的第一个元素；如
																 //果属性是一个记录型的存储区，非零值 n 表示最近第 n 次的记录。
};
struct _698_road{ 				//记录型对象属性描述符ROAD,用于描述记录型对象中的一个或若干个关联对象属性。
	struct _698_oad priv_oad;
	struct _698_oad * oad_list;//随时用随时开辟空间
};
struct _698_ms{//可选下面参量
	unsigned char no_m;//相当于无效配置。
	unsigned char all_m;//全部可采集的电能表
	unsigned char * usr_type;
	struct _698_tsa * usr_tsa;
	unsigned short * config_no;
	struct _698_region * usr_type_region;
	struct _698_region * usr_tsa_region;
	struct _698_region * config_no_region;	
};
struct _698_ti{
	unsigned char type;//秒 （ 0），分 （ 1），时 （ 2），日 （ 3），月 （ 4），年 （ 5）
	unsigned short value;		
};

struct _698_selector1{//Selector1 为指定对象指定值。
	struct _698_oad priv_oad;
	unsigned char * data;	
};
struct _698_selector2{//Selector2 为指定对象区间内连续间隔值。
	struct _698_oad priv_oad;
	unsigned char * start_data;	
	unsigned char * end_data;	
	unsigned char * interval_data;	
};
struct _698_selector3{//Selector3 为组合筛选，即若干个指定对象连续值。
	struct _698_selector2	* selector2_list;	
};
struct _698_selector4{//Selector4 为指定电能表集合、指定采集启动时间。
	struct _698_date_time date_time;
	struct _698_ms selector4_ms;	
};
struct _698_selector5{//Selector5 为指定电能表集合、指定采集存储时间。
	struct _698_date_time date_time;
	struct _698_ms selector5_ms;		
};
struct _698_selector6{//Selector6 为指定电能表集合、指定采集启动时间区间内连续间隔值。
	struct _698_date_time s_date_time;
	struct _698_date_time e_date_time;	
	struct _698_ti ti;	
	struct _698_ms selector6_ms;		
};
struct _698_selector7{//Selector7 为指定电能表集合、指定采集存储时间区间内连续间隔值。	
	struct _698_date_time s_date_time;
	struct _698_date_time e_date_time;	
	struct _698_ti ti;	
	struct _698_ms selector7_ms;			
};
struct _698_selector8{//Selector8 为指定电能表集合、指定采集到时间区间内连续间隔值。	
	struct _698_date_time s_date_time;
	struct _698_date_time e_date_time;	
	struct _698_ti ti;	
	struct _698_ms selector8_ms;				
};
struct _698_selector9{//Selector9 为指定选取上第 n 次记录
	unsigned char time;	
};
struct _698_selector10{//Select10 为指定选取最新的 n 条记录。
	unsigned char time;
	struct _698_ms selector10_ms;				
};
/*
RSD 用于选择记录型对象属性的各条记录，即二维记录表的行选择，其通过对构成记录的某些对象属性数值进行指定来进行选择，
范围选择区间：前闭后开，即[起始值，结束值）。
例如：事件类对象的事件记录表属性、冻结类对象的冻结数据记录表属性、采集监控类的采集数据记录表。
应用提示：1) 对于事件记录，通常使用事件发生时间进行选择；2) 对于冻结数据记录，通常使用冻结时间进行选择。
*/
struct _698_rsd{//=CHOICE
	unsigned char no_choice; //P36 里面的成员结构体也定义在该页， 不选择 [0] NULL，是不是个表头填写序号值的，
	struct _698_selector1 selector1;
	struct _698_selector2 selector2;
	struct _698_selector3 selector3;
	struct _698_selector4 selector4;
	struct _698_selector5 selector5;
	struct _698_selector6 selector6;
	struct _698_selector7 selector7;
	struct _698_selector8 selector8;
	struct _698_selector9 selector9;
	struct _698_selector10 selector10;	
};
struct _698_csd{//CSD 用于描述记录型对象中记录的列关联对象属性。
	struct _698_oad oad;
	struct _698_road road;
};
struct _698_rcsd{//CSD 用于描述记录型对象中记录的列关联对象属性。
	struct _698_csd * csd;
};
struct _698_tsa{//目标服务器地址TSA,对应表示 1…16 个字节长度；
	unsigned char * tsa;
};
struct _698_region{//目标服务器地址TSA,对应表示 1…16 个字节长度；
	unsigned char type;//前闭后开 （ 0），前开后闭 （ 1），前闭后闭 （ 2），前开后开 （ 3）
	unsigned char * s_data;
	unsigned char *	e_data;
};
struct _698_oi{
	unsigned short oi;
};
struct _698_omd{//对象方法描述符OMD（ Object Method Descriptor）
	unsigned char oi[2];
	unsigned char method_id; //即对象方法编号。
	unsigned char op_mode;		
};
struct _698_scaler_unit{
	char factor; //换算——倍数因子的指数，基数为 10；如数值不是数字的，则换算应被置 0。
	int unit;	//单位——枚举类型定义物理单位，详见附 录 B。	
};
struct _698_mac{//电表的mac是电表地址，这个就不知道是什么了
	unsigned char * mac;
	int size_data;	
};

struct _698_rn{
unsigned char * _698_rn;
};
struct _698_symmetrysecurity{//密文 1 为对客户机产生的随机数加密得到的密文。
	unsigned char * security_text;
	unsigned char * sign;	
	int size_text;
	int size_data_text;		
	int size_sign;
	int size_data_sign;	
};
struct _698_signaturesecurity{//密文 2 为客户机（主站）对服务器（终端）产生的主站证书等数据加密信息。客户机签名 2 为客户机对密文 2 的签名。
	unsigned char * security_text;
	unsigned char * sign;	
};
struct _698_sid{
	unsigned char id[4]; //标识 double-long-unsigned，
	unsigned char * data;	//附加数据 octet-string	
	int size;
	int size_data;	
};
struct _698_sid_mac{//SAM 所属安全标识以及消息鉴别码。
	struct _698_sid sid; //
	unsigned char * _698_mac;	
	int size;
	int size_data;	
};
#define NullS 0
#define PasswordS 1
#define symmetrys 2
#define signatures 3

struct _698_connectmechanisminfo{//SAM 所属安全标识以及消息鉴别码。
	unsigned char connect_type;//公共连接 [0] NullSecurity，公共连接 [0] NullSecurity，一般密码 [1] PasswordSecurity，对称加密 [2] SymmetrySecurity，数字签名 [3] SignatureSecurity
	unsigned char	NullSecurity;
	unsigned char	* PasswordSecurity;	//一般密码 [1] PasswordSecurity，
	struct _698_symmetrysecurity symmetrysecurity;//对称加密 [2] SymmetrySecurity，
	struct _698_signaturesecurity signaturesecurity;			//数字签名 [3] SignatureSecurity		
};
enum _698_connectresult{//SAM 所属安全标识以及消息鉴别码。
	permit_link
//允许建立应用连接 （ 0），密码错误 （ 1），对称解密错误 （ 2），非对称解密错误 （ 3），签名错误 （ 4），协议版本不匹配 （ 5），其他错误 （ 255）
};
struct _698_securitydata{
	unsigned char type;
	struct _698_rn securitydata_rn; //服务器随机数 RN，
	unsigned char	* s_sign_info;  //服务器签名信息 octet-string
	int size;
	int datesize;
};
struct _698_connectresponseinfo{
	unsigned char	connectresult;//认证结果
	struct _698_securitydata connectresponseinfo_sd;//认证附加信息 
};
//00 —— 连接响应对象 允许建立应用连接 （ 0）
//00 —— 认证附加信息 OPTIONAL=0 表示没有
//00 —— FollowReport OPTIONAL=0表示没有上报信息
//00 —— 没有时间标签
struct _698_PIID{//PIID 是用于客户机 APDU（ Client-APDU）的各服务数据类型中，基本定义如下，更具体应用约定应根据实际系统要求而定。
									//bit7（ 服务优先级） ——0：一般的， 1：高级的，在.response APDU中，其值与对应.request APDU 中的相等。
									//bit0~bit5（ 服务序号） ——二进制编码表示 0…63，在.response APDU中，其值与对应.request APDU 中的相等。
	unsigned char piid;
};
struct _698_PIID_ACD{//PIID-ACD 是用于服务器 APDU（ Server-APDU）的各服务数据类型中,bit6（ 请求访问 ACD） ——0：不请求， 1：请求。其他定义在piid中。
	unsigned char piid_acd;
};
enum _698_dar {//数据访问结果DAR（ Data Access Result）,DAR 采用枚举方式来描述数据访问的各种可能结果
	success=0, //成功 （ 0），成功 （ 0），暂时失效 （ 2），拒绝读写 （ 3），对象未定义 （ 4），对象接口类不符合 （ 5），
						 //对象不存在 （ 6），类型不匹配 （ 7），越界 （ 8），数据块不可用 （ 9），分帧传输已取消 （ 10），不处于分帧传输状态 （ 11），
						 //块写取消 （ 12），不存在块写状态 （ 13），数据块序号无效 （ 14），密码错/未授权 （ 15），通信速率不能更改 （ 16），						 
						 //年时区数超 （ 17），日时段数超 （ 18），费率数超 （ 19），安全认证不匹配 （ 20），
						 //重复充值 （ 21），ESAM 验证失败 （ 22），安全认证失败 （ 23），客户编号不匹配 （ 24），充值次数错误 （ 25），购电超囤积 （ 26），
						 //地址异常 （ 27），对称解密错误 （ 28），非对称解密错误 （ 29），签名错误 （ 30），电能表挂起 （ 31），时间标签无效 (32)，
					   //请求超时 （ 33），ESAM 的 P1P2 不正确 （ 34），ESAM 的 LC 错误 （ 35），其它 （ 255）
	hard_ineffect,
	temporary_ineffect	
} ;
/*****
上面是基本数据类型
******/
/*
服务对象的枚举：
包含三大类：
  1：预连接：
		a)：登录
				服务器发起，客户机响应，用关键字LINK
				link(.request,.confirm) link(.indication,.response)
				请求：1    响应  129
				函数名，link
		b):心跳
				保证预连接出于活动状态，只要服务器知道就可以了，我还有必要知道么，得知道，每次超时就
				查看一下，因为，其他口可能通过hplc传数据。
		c):	退出登录
				服务器重启或者变更时，我这里随时更改参数就可以了。我主动复位后，是谁发起的预登录呢，还
				是服务器么。
		
	2：应用连接：
		a)：建立应用连接（connect）
				由客户机发起，确认双方通讯的应用语境，第一次登录（link）时，这个自动连接。
				connect(.request,.confirm)   connect(.indication,.response)  
				一个客户机只有一个应用连接，当多次提交服务连接时，接收了新的请求，则前一个应用连接自动失效。
				函数名，connect
			
		b):断开服务连接
				断开应用连接（release）,通常情况下，服务器不得拒绝此请求。不允许服务器提出正常断开应用连接的请求。
				release(.request,.confirm)  release(.indication,.response)
				
				
		c):	超时处理
				应有连接建立过程中，可以协商应用连接的静态超时时间，当连续无通信时间达到静态超时时间后，服务
				器将使用release(.notification)通知客户机，应用连接失效将被断开，此服务不需要客户机做任何响应。
				再发送时怎么处理？重新建立连接？
		
	3：数据交换：
				通过逻辑名引用来访问接口对象的属性或方法。
				只有上报是通知/确认类，其他是请求/响应类型；
				请求/响应类型通信流程，客户机调用某个请求xx.request,服务器应用层接收到客户机的请求后向服务器应用进程
				发出服务指示xx.indication,然后应用进程通过调用服务xx.respnse以响应客户机请求，客户机应用层接收到
				服务器响应后向客户机应用进程返回服务确认xx.confirm。客户机跟服务器完全对等，上面的流程中位置可以互换。
		a):读取
				客户机发出，服务器响应
				get(.request,.confirm)    get(.indication,.response)
				可以通过这个，查询出服务器支持的可注册后上报的服务集（如事件或定时数据上报等）

				函数名，get		
			
		b):设置
				set(.request,.confirm)   set(.indication,.response)

				函数名，set		
				
				
		c):	操作
				action(.request,.confirm)    action(.indication,.response)

				函数名，action



		b):上报
				通知/确认类，其他是请求/响应类型
				report(.notification,.confirm) (服务器发出。是通知么？)  report(.indication,.response)
				通信流程：参看P18,服务器和客户机也是对等的。
				函数名，report				
				
				
		c):	代理
				
				proxy(.request,.confirm)   proxy(.indication,.response)

				函数名，proxy						

细分类实现：  
*/


/*各个事件对应的结构体
开始的几个是在里面被下面其中一个调用的，顺序使写代码的自然顺序。


*/
struct _698_protocolconformance{
	unsigned char protocolconformance[8];
	//应用连接协商 Application Association （ 0），请求对象属性 Get Normal （ 1），批量请求基本对象属性 Get With List （ 2），请求记录型对象属性 Get Record （ 3），
	//代理请求对象属性 Get Proxy （ 4），代理请求记录型对象属性 Get Proxy Record （ 5），请求分帧后续帧 Get Subsequent Frame （ 6），
	//设置基本对象属性 Set Normal （ 7），批量设置基本对象属性 Set With List （ 8），设置后读取 Set With Get （ 9），代理设置对象属性 Set Proxy （ 10），
	//代理设置后读取对象属性 Set Proxy With Get （ 11），执行对象方法 Action Normal （ 12），批量执行对象方法 Action With List （ 13），执行方法后读取 Action With List （ 14），
	//代理执行对象方法 Action Proxy （ 15），代理执行后读取 Action Proxy With Get （ 16），事件主动上报 Active Event Report （ 17），事件尾随上报 （ 18)，
	//事件请求访问位 ACD 上报 （ 19)，分帧数据传输 Slicing Service （ 20），Get-request 分帧 （ 21），Get-response 分帧 （ 22），Set-request 分帧 （ 23），
	//Set-response 分帧 （ 24），Action-request 分帧 （ 25），Action-response 分帧 （ 26），Proxy-request 分帧 （ 27），Proxy-response 分帧 （ 28），
	//事件上报分帧 （ 29），DL/T645-2007 （ 30），安全方式传输 （ 31），对象属性 ID 为 0 的读取访问 （ 32），对象属性 ID 为 0 的设置访问 （ 33）
};

struct _698_functionconformance{
	unsigned char functionconformance[16];
	//电能量计量 （ 0），双向有功计量 （ 1），无功电能计量 （ 2），视在电能计量 （ 3），有功需量 （ 4），无功需量 （ 5），视在需量 （ 6），复费率 （ 7），阀控 （ 8），
	//本地费控 （ 9），远程费控 （ 10），基波电能 （ 11），谐波电能 （ 12），谐波含量 （ 13），波形失真度 （ 14），多功能端子输出 （ 15），事件记录 （ 16），
	//事件上报 （ 17），温度测量 （ 18），状态量监测（如：开表盖/开端钮盖） （ 19），以太网通信 （ 20），红外通信 （ 21），蓝牙通信 （ 22），多媒体采集 （ 23），
	//级联 （ 24），直流模拟量 （ 25），弱电端子 12V 输出 （ 26），搜表 （ 27），三相负载平衡 （ 28），升级 （ 29），比对 （ 30）
};

struct _698_FactoryVersion{
unsigned char manufacturer_code[4];//厂商代码 visible-string(SIZE (4))，
unsigned char soft_version[4];//软件版本号 visible-string(SIZE (4))，
unsigned char soft_date[6];//软件版本日期 visible-string(SIZE (6))，
unsigned char hard_version[4];//硬件版本号 visible-string(SIZE (4))，
unsigned char hard_date[6];//硬件版本日期 visible-string(SIZE (6))，
unsigned char manufacturer_ex_info[8];//厂家扩展信息 visible-string(SIZE (8))

};
/*
*/
struct _698_RELEASE_Notification{//他从服务器而来
	struct _698_PIID_ACD rel_noti_piid_acd;
	struct _698_date_time date_time_establish;
	struct _698_date_time date_time_current;		
	int size;
};
struct _698_RELEASE_Response{
	struct _698_PIID_ACD rel_req_piid_acd;
	unsigned char rusult;//结果 ENUMERATED{成功 （ 0）}
};
struct _698_RELEASE_Request{
	struct _698_PIID rel_req_piid;
};
/*
result
允许建立应用连接 （ 0），
密码错误 （ 1），
对称解密错误 （ 2），
非对称解密错误 （ 3），
签名错误 （ 4），
协议版本不匹配 （ 5），
其他错误 （ 255）
*/
struct _698_connect_response{
	unsigned char type;
	unsigned char piid_acd;
	//unsigned char result;
	struct _698_FactoryVersion connect_res_fv;
	unsigned char apply_version[2];
	struct _698_protocolconformance connect_res_pro;
	struct _698_functionconformance connect_res_func;	
	unsigned char max_size_send[2];
	unsigned char max_size_rev[2];
	unsigned char max_size_rev_windown;//单位是个，看来是需要实现多个的情况？
	unsigned char max_size_handle[2]; //客户机最大可处理APDU尺寸
	unsigned char connect_overtime[4];//期望的应用连接超时时间
	struct _698_connectresponseinfo connect_res_info;//里面有不定长度的数，要用sizeof比较困难，里面加一个计算长度的么
	unsigned char FollowReport;
	unsigned char time_tag;
	int size;
};
struct _698_connect_request{//引用
	unsigned char type;
	unsigned char piid;
	unsigned char version[2];
	struct _698_protocolconformance connect_req_pro;
	struct _698_functionconformance connect_req_func;
	unsigned char max_size_send[2];
	unsigned char max_size_rev[2];
	unsigned char max_size_rev_windown;//单位是个，看来是需要实现多个的情况？ 就写了一个
	unsigned char max_size_handle[2];//客户机最大可处理APDU尺寸
	unsigned char connect_overtime[4];//期望的应用连接超时时间
	struct _698_connectmechanisminfo connect_req_cmi;//里面有不定长度的数，要用sizeof比较困难，里面加一个计算长度的么
	unsigned char time_tag;
	int size;
};
/*


*/
struct _698_link_response{
	unsigned char type;//LINK-Response
	unsigned char piid; ////参看	P35.服务优先级 bit7 0,一般的 1:高级的,与piid_acd相等; 与piid_acd相等，bit0-bit5 是服务序号，与piid_acd相等
											//二进制编码表示0.....63 与piid_acd相等	
	unsigned char result;//bit7 0 不可信，1 可信。bit0-bit2 0成功  1地址重复   2非法设备   3 容量不足
	struct _698_date_time date_time_ask;
	struct _698_date_time date_time_rev;	
	struct _698_date_time date_time_response;	
	int position;//私有位置变量，最末端数下一位的位置，也是长度
};

static struct _698_link_response hplc_698_link_response;

struct _698_link_request
{
	unsigned char type;
	unsigned char piid_acd;//参看	P35，bit6 （ 请求访问 ACD） ——0：不请求， 1：请求。
	unsigned char work_type;//登录 （ 0），心跳 （ 1），退出登录 （ 2）
	unsigned char heartbeat_time[2];
	struct _698_date_time date_time;//年(两位char)月日，星期几(0是周日，其他是)，时分秒，多少毫秒(两位char)
	int position;
	
};
/*地址域sa*/
#define ADDR_SA_TYPE_MASK 0X3F /*0表示单地址,1表示通配地址,2表示组地址,3表示广播地址*/
#define ADDR_SA_LOGIC_ADDR_MASK ~(0X30)
#define ADDR_SA_ADDR_LENGTH_MASK 0X0F  //地址长度，没用到

struct _698_ADDR        //P   地址域a
{
	unsigned char sa;   //由bit0-bit3决定,对应1到16,就是需要加一     
	unsigned char s_addr[8];//长度由sa决定
	unsigned char ca;//客户机地址CA用1字节无符号整数表示，取值范围0…255，值为0表示不关注客户机地址。
	int s_addr_len;
};
struct _698_strategy{
	unsigned char lock1;//，标志着使用串口中，最好用lock,锁住设备，但会不会影响其他设备使用
	unsigned char lock2;//使用软件的双锁来控制临界资源
	unsigned char heart_beat_time0;//特例低字节在后，高字节在前，是个正序，单位是秒
	unsigned char heart_beat_time1;
	unsigned char dev_type;//任务类型是从哪里到哪里，如果是其他接口的，就调用
	unsigned char cmd_type;//任务类型是从哪里到哪里，如果是其他接口的，就调用
	unsigned char heart_beat_flag;	
};	
/*
结构体名：
成员：链路用户数据
希望结构固定
*/
//分帧使用的宏定义。
#define FRAME_APART_MASK ~(0XC0) //掩码
#define FRAME_APART_START 0X00  //起始帧
#define FRAME_APART_RESPONSE 0X80 
//确认帧，若接收不正确就不发确认帧，超时的时间设置的长着点，毕竟分帧，收全不容易。
//确认帧格式是后面没有链路层数据。
#define FRAME_APART_END 0X40 //最后帧
#define FRAME_APART_MIDDLE 0XC0 //中间帧
/*
分帧格式域：
		需要分帧时，前面加两个字节，用于分帧的控制。高位在后，低位在前
			含义：bit0-bit11 是帧序号,取值0-4095，循环使用
            bit12-bit13  保留
				    bit14-bit15  帧类型，使用宏定义。	
			后面接的就是链路层的数据。
			每一帧都需要应答，接收完后处理，各种协议都可能有分帧
*/
struct usrData {
	unsigned char head;
};

/*长度  ,最长不应该超过这个,没啥用*/
#define MAX_LENGTH 0X3FFF
/*控制域c 或上这个功能*/
#define CON_DIR_START_MASK ~(0X3F)
#define CON_START_MASK (0X40)  //u   是1   s是    0
#define CON_DIR_MASK (0X80)    //UTS 是0   STU 是 1 


#define CON_UTS_S ((0<<7)|(0<<6))   /*方向UTS 发起者是服务器S ,含义客户机对服务器上报的响应*/
#define CON_UTS_U (0<<7)|(1<<6)     /*电表端是服务器，控制器是客户机*/
#define CON_STU_U (1<<7)|(1<<6)
#define CON_STU_S (1<<7)|(0<<6)

#define CON_MORE_FRAME (1<<5)   /*分帧标志位*/

#define CON_FUNCTION_MASK 0X02

#define CON_LINK_MANAGE 0X1 /*链路连接管理（登录,心跳,退出登录）*/
#define CON_U_DATA 			 0X3 /*应用连接管理及数据交换服务*/


//最大长度
#define  HPLC_DATA_MAX_SIZE 1024

struct _698_FRAME       //698协议结构,可能用不到,以防用的到
{
	unsigned char head;  //起始帧头 = 0x68
	unsigned char length0;   //长度,两个字节,由bit0-bit13有效,是传输帧中除起始字符和结束字符之外的帧字节数。
	unsigned char length1;
	unsigned int length;
	unsigned char control;   //控制域c,bit7,传输方向位
	struct _698_ADDR addr;//地址域a
	unsigned char HCS0;//帧头校验,是对帧头部分除起始字符和HCS本身之外的所有字节的校验
	unsigned char HCS1;
	unsigned char *usrData;//链路用户数据,调试用，后期去掉[HPLC_DATA_MAX_SIZE]
	//struct usrData;//链路用户数据
	int usrData_len;//数组的结束位
	int usrData_size;//用户数组的总长度，接收时是相等的usrData指向要指的地方
	unsigned short FCS0;//帧校验
	unsigned short FCS1;
	unsigned char end;//结束字符 = 0x16	
	struct _698_date_time rev_tx_frame_date_time;
//	unsigned char link_ok;	//1是完成了登录，其他的可以不用这个
//	unsigned char connect_ok;//完成了用户层连接
	unsigned char frame_apart;//默认是0，就是部分帧，如果被置0,下一阵就会被加上
	unsigned char frame_no;
	unsigned int list_no;//只有第一个的有用
	struct _698_strategy strategy; 
	int need_package;//这个为1是发送后应答的帧
	int time_flag_positon;	
};



/*

字符型指针的管理结构体

*/
struct CharPointDataManage        //
{
	struct _698_FRAME _698_frame;
	unsigned char priveData[HPLC_DATA_MAX_SIZE];
	unsigned int size;
	unsigned int dataSize;
	struct CharPointDataManage *next;
	int list_no;//一共是个位子，用了就
	
};

//static struct CharPointDataManage _698_usr_data_apdu;//用来放置用户的数据

//发送中不接收数据，但是发送中可能会收到非预期的数据，所以有预期收到的数据时放到

/*结构体名：

结构体用途：某个使用698协议的端口的状态。

*/
struct _698_STATE        //地址域a
{
	unsigned char link_flag;   // 登录后置1 
	unsigned char connect_flag;//连接后置1
	int try_link_type;//返回的没有标志是那种登录类型
	unsigned char heart_beat_time0;//特例低字节在后，高字节在前，是个正序，单位是秒
	unsigned char heart_beat_time1;
	struct _698_date_time last_link_requset_time;
	struct _698_ADDR addr;
	unsigned char piid;//这个后来去掉
	unsigned char meter_addr_send_ok;
	unsigned char version[2];
	unsigned char protocolconformance[8];
	unsigned char functionconformance[16];
	unsigned char connect_overtime[4];
	struct _698_FactoryVersion FactoryVersion;	
	struct _698_oad oad_omd;
	unsigned char lock1;//，标志着使用串口中，最好用lock,锁住设备，但会不会影响其他设备使用
	unsigned char lock2;//使用软件的双锁来控制临界资源
	int HCS_position;
	int FCS_position;
	int len_position;
	int USART_RX_STA;
	int len_left;
	int len_all;	
	int len_sa;
	int FE_no;	
};


static unsigned short fcstab[256]={
0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/*
变量
*/

unsigned short pppfcs16(unsigned short fcs, unsigned char *cp, int len);
int tryfcs16(unsigned char *cp, int len);
int _698_FCS(unsigned char *data, int start_size,int size,unsigned short FCS);
int _698_analysis(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev,struct CharPointDataManage * hplc_data_wait_list);
int rev_698_del_affairs(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev);
int _address_unPackage(unsigned char * data,struct _698_ADDR *_698_addr,int size);
int _698_unPackage(unsigned char * data,struct  _698_FRAME  *_698_frame_rev,int size);
int _698_package(unsigned char * data,int size);
int unPackage_698_link_request(struct  _698_FRAME  *_698_frame,struct _698_link_request * request,int * size);
int unPackage_698_connect_request(struct _698_STATE  * priv_698_state,struct  _698_FRAME  * _698_frame,struct _698_connect_response * prive_struct);
int _698_HCS(unsigned char *data, int start_size,int size,unsigned short HCS);
int priveData_analysis(struct CharPointDataManage * data_rev,struct CharPointDataManage * data_tx);
int my_strcpy(unsigned char *dst,unsigned char *src,int startSize,int size);
int save_char_point_data(struct CharPointDataManage *hplc_data,int position,unsigned char *Res,int size);
int array_inflate(unsigned char *data, int size,int more_size);
int array_deflate(unsigned char *data, int size,int de_size);
int hplc_data_inflate(struct CharPointDataManage *data_rev);
int save_char_point_usrdata(unsigned char *data,int *length,unsigned char *Res,int position,int size);
int get_current_time(unsigned char * data );
int get_date_time_s(struct _698_date_time_s *date_time_s);
int hplc_tx_frame(struct _698_STATE  * priv_698_state,rt_device_t serial,struct CharPointDataManage * data_tx);
int link_response_package(struct  _698_FRAME  *_698_frame_rev,struct _698_FRAME  *_698_frame_send,struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx);
int get_response_package(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int connect_response_package(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx);
int copy_to_work_wait_list(struct CharPointDataManage *hplc_data,struct CharPointDataManage * hplc_data_wait_list);
int connect_request_package(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);
int link_request_package(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);
int copy_char_point_data(struct CharPointDataManage * des,struct CharPointDataManage * source);
int copy_698_frame(struct  _698_FRAME * des,struct  _698_FRAME  * source);
int init_CharPointDataManage(struct CharPointDataManage *des);
int free_CharPointDataManage(struct CharPointDataManage *des);
int init_698_FRAME(struct  _698_FRAME  * des);
int free_698_FRAME(struct  _698_FRAME  * des);
int iterate_wait_response_list(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev,struct CharPointDataManage * hplc_data_list);
int iterate_wait_request_list(struct _698_STATE  * priv_698_state,struct CharPointDataManage * data_tx,struct CharPointDataManage * data_rev,struct CharPointDataManage * hplc_data_list);
int init_698_state(struct _698_STATE  * priv_698_state);
int hplc_priveData_analysis(struct CharPointDataManage * data_rev,struct CharPointDataManage * data_tx);
int hplc_package(unsigned char * data,int size);
int clear_data(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev,struct  _698_FRAME  *_698_frame);
int save_hplc_data(struct CharPointDataManage *data_rev,int position,unsigned char Res);
int printmy(struct  _698_FRAME  *_698_frame);
int get_single_frame_frome_hplc(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev,struct CharPointDataManage *data_tx);
int hplc_tx(struct CharPointDataManage *hplc_data);
int hplc_inition(struct CharPointDataManage *  data_wait_list,struct CharPointDataManage * data_rev,struct CharPointDataManage * data_tx);
void hplc_thread_entry(void * parameter);
int hplc_645_addr_receive(struct CharPointDataManage *data_rev);
int hplc_645_addr_response(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev,struct CharPointDataManage *data_tx);
int init_698_state(struct _698_STATE  * priv_698_state);
int my_free(unsigned char  *des);
int oad_package(struct _698_oad *priv_struct,struct  _698_FRAME  *_698_frame_rev,int position);
int get_response_package_normal(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int get_response_parameter_oia(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int get_response_normal_oad(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int oi_parameter_get_addr(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int get_data_class(struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data,enum Data_T data_type);
int send_event(void);
extern rt_uint8_t CtrlUnit_RecResp(rt_uint32_t cmd,void *STR_SetPara,int count);
int STR_SYSTEM_TIME_to__date_time_s(STR_SYSTEM_TIME * SYSTEM_TIME,struct _698_date_time_s *date_time_s);
int my_strcpy_char(char *dst,char *src,int startSize,int size);
int action_response_package(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int Report_Cmd_ChgRecord(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);
int Report_Cmd_ChgPlanExeState(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);
int check_afair_from_botom(struct _698_STATE  * priv_698_state,struct CharPointDataManage *data_rev,struct CharPointDataManage *data_tx);
extern rt_uint8_t strategy_event_get(COMM_CMD_C cmd);
extern rt_uint8_t strategy_event_send(COMM_CMD_C cmd);
int package_for_test(struct  _698_FRAME  *_698_frame_rev,struct _698_STATE  * priv_698_state,struct CharPointDataManage * hplc_data);
int hplc_tx(struct CharPointDataManage *hplc_data);
int charge_strategy_package(CHARGE_STRATEGY *priv_struct_STRATEGY,struct CharPointDataManage * hplc_data);
int charge_exe_state_package(CHARGE_EXE_STATE *priv_struct,struct CharPointDataManage * hplc_data);	
int plan_fail_event_package(PLAN_FAIL_EVENT *priv_struct,struct CharPointDataManage * hplc_data);
int Report_Cmd_DeviceFault(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);
int Report_Cmd_PileFault(struct CharPointDataManage *hplc_data,struct _698_STATE  * priv_698_state);









