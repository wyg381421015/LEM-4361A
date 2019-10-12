/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-14     aubr.cool    1st version
 */


#ifndef __pcf8563_H__
#define __pcf8563_H__

#include <rtthread.h>

#define PCF8563_ADDRESS     0xA2 
#define PCF8563_READ        0x01
#define TXMODE             0x1f 

#define CTR1_ADDRESS       0x0f
#define CTR2_ADDRESS       0x10
#define CTR3_ADDRESS       0x11

#define SECOND_ADDRESS     0x00
#define CTR1_VALUE         0x84  //1000 0100
#define CTR2_VALUE         0x80  //1000 0000
#define CTR3_VALUE         0x00  //0000 0000

/******************** Alarm register ********************/
#define		Alarm_SC	0x07
#define		Alarm_MN	0x08
#define		Alarm_HR	0x09
#define		Alarm_WK	0x0A
#define		Alarm_DY	0x0B
#define		Alarm_MO	0x0C
#define		Alarm_YR	0x0D
#define		Alarm_EN	0x0E
/***************** Time Trimming Register ***********************/
#define		TTF			        0x12
/***************** Timer Counter Register ***********************/
#define		Timer_Counter		0x13
/******************** User Ram Register ***********************/
#define		User_ram14H			0x14
#define		User_ram15H			0x15
#define		User_ram16H			0x16
#define		User_ram17H			0x17
#define		User_ram18H			0x18
#define		User_ram19H			0x19
#define		User_ram1AH			0x1A
#define		User_ram1BH			0x1B
#define		User_ram1CH			0x1C
#define		User_ram1DH			0x1D
#define		User_ram1EH			0x1E
#define		User_ram1FH			0x1F



struct pcf8563_config
{
    rt_uint32_t              size;
    rt_uint16_t              addr;
    rt_uint16_t              flags;
};

extern rt_err_t pcf8563_register(const char *e2m_device_name,const char *i2c_bus,void *user_data);

#endif /*__pcf8563_H__*/
