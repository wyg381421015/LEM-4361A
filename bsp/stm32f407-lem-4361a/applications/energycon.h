#ifndef __ENERGYCON_H__
#define __ENERGYCON_H__

#include <string.h>
#include <stdio.h>
#include "global.h"
#include "chargepile.h"

extern ChargPilePara_TypeDef ChargePilePara_Set;
extern ChargPilePara_TypeDef ChargePilePara_Get;


extern struct rt_thread energycon;
extern struct rt_semaphore rx_sem_energycon;

/******************************* 充电控制 *************************************/
typedef struct
{
	char OrderSn[17];			//订单号  octet-string（SIZE(16)）
	char cAssetNO[23];			//路由器资产编号  visible-string（SIZE(22)）
	unsigned char GunNum;		//枪序号	{A枪（1）、B枪（2）}
	unsigned long SetPower;		//设定充电功率（单位：W，换算：-1）
	unsigned char cSucIdle;		//成功或失败原因:{0：成功 1：失败 255：其他}
}CTL_CHARGE;/*控制器充电控制*/

#endif

