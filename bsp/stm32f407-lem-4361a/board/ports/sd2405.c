/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-14     aubr.cool    1st version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "sd2405.h"

struct sd2405_device
{
    struct rt_device         parent;
    struct rt_i2c_bus_device *bus;
};

//static struct sd2405_device sd2405_drv;

/* RT-Thread device interface */

static rt_err_t sd2405_init(rt_device_t dev)
{
		struct sd2405_device *sd2405;
		struct rt_i2c_msg msg[5];
    rt_uint8_t mem_addr[5] = {0};
    rt_size_t ret = 0;
    RT_ASSERT(dev != 0);
		
		sd2405 = (struct sd2405_device *) dev;

    msg[0].addr     = ((rt_uint8_t) SD2405_ADDRESS>>1);
    msg[0].flags    = RT_I2C_WR;
		mem_addr[0]     = (rt_uint8_t) TXMODE|CTR1_ADDRESS;
		mem_addr[1]     = (rt_uint8_t) CTR1_VALUE;
		mem_addr[2]     = (rt_uint8_t) CTR2_VALUE;
		mem_addr[3]     = (rt_uint8_t) CTR3_VALUE;
		msg[0].buf      = (rt_uint8_t *) mem_addr;
    msg[0].len      =  4;
		
		ret = rt_i2c_transfer(sd2405->bus, msg, 1);
		return (ret == 1) ? RT_EOK : RT_ERROR;
}
static rt_err_t sd2405_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t sd2405_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t get_rtc_timestamp(rt_device_t dev,void *args)
{
    struct sd2405_device *sd2405;
		struct rt_i2c_msg msg[2];
    rt_size_t ret = 0;
    RT_ASSERT(dev != 0);
	
		sd2405 = (struct sd2405_device *) dev;
		
    msg[0].addr     = ((rt_uint8_t) SD2405_ADDRESS>>1);
    msg[0].flags    =  RT_I2C_RD;
		msg[0].buf      = (rt_uint8_t *) args;
    msg[0].len      =  7;// 秒 分 时 星期 日 月 年  共7个字节
	
    ret = rt_i2c_transfer(sd2405->bus, msg, 1);
    return (ret == 1) ? RT_EOK : RT_ERROR;
}

static rt_err_t set_rtc_time_stamp(rt_device_t dev,void *args)
{
    struct sd2405_device *sd2405;
		struct rt_i2c_msg msg[5];
		rt_uint8_t* ptr;
		rt_uint8_t mem_addr[5][8] = {0};
		rt_uint8_t i;
    rt_size_t ret = 0;
    RT_ASSERT(dev != 0);
		
		ptr = args;
	
		sd2405 = (struct sd2405_device *) dev;
	/*********************写使能****************************/
		msg[0].addr     = ((rt_uint8_t) SD2405_ADDRESS>>1);
    msg[0].flags    =  RT_I2C_WR;
		mem_addr[0][0]     = (rt_uint8_t) CTR2_ADDRESS;
		mem_addr[0][1]     = (rt_uint8_t) CTR2_VALUE;
		msg[0].buf      = (rt_uint8_t *) mem_addr[0];
    msg[0].len      =  2;
		
		msg[1].addr     = ((rt_uint8_t) SD2405_ADDRESS>>1);
    msg[1].flags    =  RT_I2C_WR;
		mem_addr[1][0]     = (rt_uint8_t) CTR1_ADDRESS;
		mem_addr[1][1]     = (rt_uint8_t) CTR1_VALUE;
		msg[1].buf      = (rt_uint8_t *) mem_addr[1];
    msg[1].len      =  2;
	/*******************************************************/
	
	/*********************写时钟****************************/	
		msg[2].addr     = ((rt_uint8_t) SD2405_ADDRESS>>1);
    msg[2].flags    =  RT_I2C_WR;
		mem_addr[2][0]     = (rt_uint8_t) TXMODE&SECOND_ADDRESS;
		for(i = 0;i < 7;i++)
		{
			mem_addr[2][1+i] = *(ptr+i);
		}
		msg[2].buf      = (rt_uint8_t *) mem_addr[2];
    msg[2].len      =  8;
		
		/*******************************************************/
		
		/*********************写禁止****************************/
		msg[3].addr     = ((rt_uint8_t) SD2405_ADDRESS>>1);
    msg[3].flags    =  RT_I2C_WR;
		mem_addr[3][0]     = (rt_uint8_t) CTR1_ADDRESS;
		mem_addr[3][1]     = (rt_uint8_t) CTR3_VALUE;
		msg[3].buf      = (rt_uint8_t *) mem_addr[3];
    msg[3].len      =  2;
		
		msg[4].addr     = ((rt_uint8_t) SD2405_ADDRESS>>1);
    msg[4].flags    =  RT_I2C_WR;
		mem_addr[4][0]     = (rt_uint8_t) CTR2_ADDRESS;
		mem_addr[4][1]     = (rt_uint8_t) CTR3_VALUE;
		msg[4].buf      = (rt_uint8_t *) mem_addr[4];
    msg[4].len      =  2;
		/*******************************************************/
		
    ret = rt_i2c_transfer(sd2405->bus, msg, 5);
    return (ret == 5) ? RT_EOK : RT_ERROR;
}

static rt_err_t sd2405_control(rt_device_t dev, int cmd, void *args)
{
		rt_uint8_t* ptr;
		rt_err_t result = RT_EOK;		
    RT_ASSERT(dev != RT_NULL);
    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
        result = get_rtc_timestamp(dev,(rt_uint8_t *)args);
				if(result == RT_EOK)
				{
					ptr = args;
					if(ptr[2] & 0x80)//24小时制
					{
						ptr[2] = ptr[2]&0x3F;
					}
					else if(ptr[2]&0x20)//PM
					{
						if(ptr[2] == 0x32)
							ptr[2] = 0x12;
						else
							ptr[2] = (ptr[2]&0x1F)+0x12;
					}
					else//AM
					{
						if(ptr[2] == 0x12)
									ptr[2] = 0x00;
							else		
									ptr[2] = ptr[2]&0x1F;
					}
				}
//        rt_kprintf("SD2405: get rtc_time %x\n", *(rt_uint8_t *)args);
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:
        result = set_rtc_time_stamp(dev,(rt_uint8_t *)args);
				if(result == RT_EOK)
					rt_kprintf("SD2405: set rtc_time sucess\r\n");
				else
					rt_kprintf("SD2405: set rtc_time fail\r\n");
        break;
    }

    return result;
}

static rt_size_t sd2405_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
	return 0;
}

static rt_size_t sd2405_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
	return 0;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device sd2405_ops =
{
    sd2405_init,
    sd2405_open,
    sd2405_close,
    sd2405_read,
    sd2405_write,
    sd2405_control
};
#endif


rt_err_t sd2405_register(const char *fm_device_name, const char *i2c_bus,void *user_data)
{
    static struct sd2405_device sd2405_drv;
    struct rt_i2c_bus_device *bus;

    bus = rt_i2c_bus_device_find(i2c_bus);
    if (bus == RT_NULL)
    {
        return RT_ENOSYS;
    }

    sd2405_drv.bus = bus;
    sd2405_drv.parent.type      = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
    sd2405_drv.parent.ops       = &sd2405_ops;
#else
    sd2405_drv.parent.init      = sd2405_init;
    sd2405_drv.parent.open      = sd2405_open;
    sd2405_drv.parent.close     = sd2405_close;
    sd2405_drv.parent.read      = sd2405_read;
    sd2405_drv.parent.write     = sd2405_write;
    sd2405_drv.parent.control   = sd2405_control;
#endif
		sd2405_drv.parent.user_data = user_data;

    return rt_device_register(&sd2405_drv.parent, fm_device_name, RT_DEVICE_FLAG_RDWR);
}



#ifdef RT_USING_SD2405
static int rtc_sd2405_init(void)
{
	rt_err_t result = RT_EOK;
	
	result = sd2405_register("sd2405","i2c1",RT_NULL);
	
	if(result == RT_EOK)
		rt_kprintf("SD2405: sd2405 register sucess!!!\r\n");
	else
		rt_kprintf("SD2405: sd2405 register fail!!!\r\n");
	
	return result;
	
}
INIT_COMPONENT_EXPORT(rtc_sd2405_init);
#endif






