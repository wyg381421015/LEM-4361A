#ifndef __ESAM_H__
#define __ESAM_H__

#include <rtthread.h>

#define ESAM_HEAD 	0x55


typedef struct{
	rt_uint16_t DataRx_len;
	rt_uint8_t Rx_data[1024];
	rt_uint16_t DataTx_len;
	rt_uint8_t Tx_data[1024];
	
}ScmEsam_Comm;

typedef enum
{
	RD_INFO_01 = 1,//读取ESAM信息
	RD_INFO_02,//
	RD_INFO_03,
	RD_INFO_04,
	RD_INFO_05,
	RD_INFO_06,
	RD_INFO_07,
	RD_INFO_08,
	RD_INFO_09,
	RD_INFO_10,
	RD_INFO_11,
	RD_INFO_12,
	RD_INFO_FF,
	HOST_KEY_AGREE,//主站会话密钥协商
	HOST_KEY_UPDATE,//主站密钥更新
	HOST_CERT_UPDATE,//主站证书更新
	HOST_SESS_CALC_MAC_11,//主站会话加密计算 明文+MAC
	HOST_SESS_CALC_MAC_A2,//主站会话加密计算 密文
	HOST_SESS_CALC_MAC_A7,//主站会话加密计算 密文+MAC
	HOST_SESS_VERI_MAC,//主站会话解密验证MAC
	
	CON_KEY_AGREE,//控制器会话密钥协商
	CON_KEY_UPDATE,//控制器密钥更新
	CON_CERT_UPDATE,//控制器证书更新
	CON_SESS_CALC_MAC_11,//控制器会话加密计算 明文+MAC
	CON_SESS_CALC_MAC_12,//控制器会话加密计算 密文
	CON_SESS_CALC_MAC_13,//控制器会话加密计算 密文+MAC
	CON_SESS_VERI_MAC_11,//控制器会话解密验证 明文+MAC
	CON_SESS_VERI_MAC_12,//控制器会话解密验证 密文
	CON_SESS_VERI_MAC_13,//控制器会话解密验证 密文+MAC
	
	APP_KEY_AGREE_ONE,//主站会话密钥协商
	APP_KEY_AGREE_TWO,//主站会话密钥协商
	APP_KEY_AGREE_THREE,//主站会话密钥协商
	APP_KEY_AGREE_FOUR,//主站会话密钥协商
	
	APP_SESS_AGREE_ONE,//主站会话密钥协商
	APP_SESS_AGREE_TWO,//主站会话密钥协商
	
	APP_KEY_UPDATE,//主站密钥更新
	APP_CERT_UPDATE,//主站证书更新
	APP_SESS_CALC_MAC_11,//主站会话加密计算 明文+MAC
	APP_SESS_CALC_MAC_A2,//主站会话加密计算 密文
	APP_SESS_CALC_MAC_A7,//主站会话加密计算 密文+MAC
	APP_SESS_VERI_MAC,//主站会话解密验证MAC
	
	HOST_READ,//主站抄读
}ESAM_CMD;

typedef struct{
	rt_uint8_t	Version[5];//ESAM 版本号
	rt_uint8_t	Ser_Num[8];//ESAM 序列号
	rt_uint8_t	temp1;//保留
	rt_uint8_t	Sym_KeyVer[16];//对称秘钥版本
	rt_uint8_t	Sess_Limit[4];//会话时效门限
	rt_uint8_t	Sess_Last[4];//会话时效剩余时间
	rt_uint8_t	ASCTR[4];//单地址应用协商计数器
	rt_uint8_t	temp2[4];//保留
	rt_uint8_t	temp3[4];//保留
	rt_uint8_t	AMTCTR[4];//终端与路由器会话协商计数器
	rt_uint8_t	Esam_Info[40];//ESAM 发行信息
	rt_uint8_t	Con_Sess_Limit[4];//控制器会话时效门限
	rt_uint8_t	Con_Sess_Last[4];//控制器会话时效剩余时间
}SCMEsam_Info;

extern rt_err_t ESAM_Communicattion(ESAM_CMD cmd,ScmEsam_Comm* l_stEsam_Comm);


#endif

