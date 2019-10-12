/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-14     aubr.cool    1st version
 */


#ifndef __SPI_ESAM_H__
#define __SPI_ESAM_H__

#include <rtthread.h>


extern rt_err_t pcf8563_register(const char *e2m_device_name,const char *i2c_bus,void *user_data);

#endif /*__pcf8563_H__*/
