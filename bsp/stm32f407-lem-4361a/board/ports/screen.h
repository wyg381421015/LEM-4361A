/**************************************************************
 * Copyright (c) 2017 鲁能智能公司 All rights reserved.
 * 项    目:380V交流充电监控模块
 * 文件名称: screen.h
 * 当前版本: v1.00 
 * 作    者：
 * 开发日期: 2017-05-01
 * 固 件 库: v3.50
 * 描    述: 头文件
 **************************************************************	
 */	
#include "sys.h"
#include "delay.h"
#include "usb_app.h"
#include "ff.h"
#include "exfuns.h"	
#include "sdio_sdcard.h"
#include "malloc.h"	
#include "global.h"
#include "usart3.h"
#if SYSTEM_SUPPORT_OS    //如果使用ucos,则包括下面的头文件即可.
#include "includes.h"	 //ucos 使用
#endif


#define SHOW_SHADE  1   //反白显示
#define SHOW_NORMAL 0   //正常显示
#define MEM_FLASH   1   //字符常量显示
#define MEM_RAM     0   //字符变量显示

#define  LCM_DOT_WIDTH   192
#define  LCM_DOT_HEIGHT  32
#define  LCM_CHAR_WIDTH  30
#define  LCM_CHAR_HEIGHT 16

// ASCII 字模宽度及高度定义
#define ASC_CHR_WIDTH		 8
#define ASC_CHR_HEIGHT	16

#define SCR_000		0
#define SCR_001		1
#define SCR_002		2
#define SCR_003		3
#define SCR_004		4
#define SCR_005		5
#define SCR_006		6
#define SCR_007		7
#define SCR_008		8
#define SCR_009		9
#define SCR_010		10
#define SCR_011		11
#define SCR_012		12
#define SCR_013		13
#define SCR_014		14
#define SCR_015		15
#define SCR_016		16
#define SCR_017		17
#define SCR_018		18
#define SCR_019		19
#define SCR_020		20
#define SCR_021		21
#define SCR_022		22
#define SCR_023		23
#define SCR_024 	24
#define SCR_025		25
#define SCR_026		26
#define SCR_027		27
#define SCR_028		28
#define SCR_029		29
#define SCR_030		30
#define SCR_040		40
#define SCR_050		50
#define SCR_060		60
#define SCR_070		70
#define SCR_080		80
#define SCR_090		90
#define SCR_100		100
#define SCR_110		110
#define SCR_120		120
#define SCR_130		130
#define SCR_200		200


#define CS_ON()	  GPIO_SetBits(GPIOD, GPIO_Pin_4)
#define RST_ON()	GPIO_ResetBits(GPIOD, GPIO_Pin_5)
#define A0_ON()   GPIO_SetBits(GPIOD, GPIO_Pin_3)		//充电指示灯
#define SCL_ON()  GPIO_SetBits(GPIOD, GPIO_Pin_0)		//故障指示灯
#define SDA_ON()  GPIO_SetBits(GPIOD, GPIO_Pin_1)
#define BLK_ON()  GPIO_ResetBits(GPIOA, GPIO_Pin_8)

#define CS_OFF()   GPIO_ResetBits(GPIOD, GPIO_Pin_4)
#define RST_OFF()  GPIO_SetBits(GPIOD, GPIO_Pin_5)
#define A0_OFF()   GPIO_ResetBits(GPIOD, GPIO_Pin_3)
#define SCL_OFF()  GPIO_ResetBits(GPIOD, GPIO_Pin_0)
#define SDA_OFF()  GPIO_ResetBits(GPIOD, GPIO_Pin_1)
#define BLK_OFF()  GPIO_SetBits(GPIOA, GPIO_Pin_8)

extern u32 ucLCD_ScrNum;//当前显示界面号
extern u32 ucLCD__PasPageNum;//



struct DisPlay_CFG
{
	void (*DisPlay_PageNum)(void);
};


struct typFNT_GB16		//汉字字模数据结构
{
	char Index[2];
	char Msk[32];
};
#define STR_TYPFNT_GB16	struct typFNT_GB16 

extern void Display(void);
extern void Lcmcls(void);
extern void initLCDM(void);
void Scr_Refresh(void);

extern void Dis_Screen_000(void);
extern void Dis_Screen_001(void);
extern void Dis_Screen_002(void);
extern void Dis_Screen_003(void);
extern void Dis_Screen_004(void);
extern void Dis_Screen_005(void);
extern void Dis_Screen_006(void);
extern void Dis_Screen_007(void);
extern void Dis_Screen_008(void);
extern void Dis_Screen_009(void);
extern void Dis_Screen_010(void);
extern void Dis_Screen_011(void);
extern void Dis_Screen_012(void);
extern void Dis_Screen_013(void);
extern void Dis_Screen_014(void);
extern void Dis_Screen_015(void);
extern void Dis_Screen_016(void);
extern void Dis_Screen_017(void);
extern void Dis_Screen_018(void);
extern void Dis_Screen_019(void);
extern void Dis_Screen_020(void);
extern void Dis_Screen_021(void);
extern void Dis_Screen_022(void);
extern void Dis_Screen_023(void);
extern void Dis_Screen_024(void);
extern void Dis_Screen_025(void);
extern void Dis_Screen_026(void);
extern void Dis_Screen_027(void);
extern void Dis_Screen_028(void);
extern void Dis_Screen_029(void);
extern void Dis_Screen_030(void);
extern void Dis_Screen_040(void);
extern void Dis_Screen_050(void);
extern void Dis_Screen_060(void);
extern void Dis_Screen_070(void);
extern void Dis_Screen_080(void);
extern void Dis_Screen_090(void);
extern void Dis_Screen_100(void);
extern void Dis_Screen_110(void);
extern void Dis_Screen_120(void);
extern void Dis_Screen_130(void);
extern void Dis_Screen_200(void);

void Update_Screen_001(void);
void Update_Screen_004(void);


extern void DisplayScreen1(void);
extern void DisplayScreen8(void);
extern void DisplayScreen81(void);
extern void DisplayScreen9(void);
extern void DisplayScreen10(void);
extern void DisplayScreen11(void);
extern void DisplayScreen12(void);
extern void DisplayScreen13(void);
extern void DisplayScreen14(void);
extern void LCD_16Font(u8 x,u8 y,void *ptr,u8 l,u8 mem);
extern void Display1(void);
extern void DisplayScreen2(void);
extern void DisplayScreen20(void);
extern void DisplayScreen21(void);
extern void DisplayScreen22(void);
extern void DisplayScreen23(void);
extern void DisplayScreen24(void);
extern void DisplayScreen25(void);
extern void DisplayScreen26(void);
extern void DisplayScreen27(void);
extern void DisplayScreen28(void);
extern void DisplayScreen3(void);
extern void DisplayScreen40(void);
extern void DisplayScreen41(void);
extern void DisplayScreen42(void);
extern void DisplayScreen5(void);
extern void DisplayScreen60(void);
extern void DisplayScreen61(void);
extern void DisplayScreen62(void);
extern void DisplayScreen63(void);
extern void DisplayScreen64(void);
extern void DisplayScreen65(void);
extern void DisplayScreen7(void);
extern void DisplayScreen71(void);
extern void PollDisplayScreen(void);
extern void SendDisplayIndex(unsigned char index);

extern void DisplayScreen100(void);
extern void DisplayScreen101(void);
extern void DisplayScreen102(void);
extern void DisplayScreen103(void);
extern void DisplayScreen104(void);
extern void DisplayScreen105(void);
extern void DisplayScreen106(void);
extern void DisplayScreen107(void);
extern void Alarm_Display(void);



