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
#include "drv_pcf8563.h"

static rt_mutex_t pcf8563_mutex = RT_NULL;

struct pcf8563_device
{
    struct rt_device         parent;
    struct rt_i2c_bus_device *bus;
};

//static struct pcf8563_device pcf8563_drv;

/* RT-Thread device interface */

static rt_err_t pcf8563_init(rt_device_t dev)
{
		struct pcf8563_device *pcf8563;
		struct rt_i2c_msg msg[2];
    rt_uint8_t mem_addr[2] = {0};
    rt_size_t ret = 0;
    RT_ASSERT(dev != 0);
		
		pcf8563 = (struct pcf8563_device *) dev;

    msg[0].addr     = ((rt_uint8_t) PCF8563_ADDRESS>>1);
    msg[0].flags    = RT_I2C_WR;
		mem_addr[0]     = (rt_uint8_t) 0x00;
		mem_addr[1]     = (rt_uint8_t) 0x00;
		msg[0].buf      = (rt_uint8_t *) mem_addr;
    msg[0].len      =  2;
		
		msg[1].addr     = ((rt_uint8_t) PCF8563_ADDRESS>>1);
    msg[1].flags    = RT_I2C_WR;
		mem_addr[0]     = (rt_uint8_t) 0x01;
		mem_addr[1]     = (rt_uint8_t) 0x11;
		msg[1].buf      = (rt_uint8_t *) mem_addr;
    msg[1].len      =  2;
		
		ret = rt_i2c_transfer(pcf8563->bus, msg, 2);
		return (ret == 2) ? RT_EOK : RT_ERROR;
}
static rt_err_t pcf8563_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t pcf8563_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t get_rtc_timestamp(rt_device_t dev,void *args)
{
    struct pcf8563_device *pcf8563;
		struct rt_i2c_msg msg[14];
		rt_uint8_t i,mem_addr[2] = {0};
		rt_uint8_t* ptr;
    rt_size_t ret = 0;
    RT_ASSERT(dev != 0);
		
		rt_err_t result = rt_mutex_take(pcf8563_mutex, RT_WAITING_FOREVER);
		
		ptr = args;
	
		pcf8563 = (struct pcf8563_device *) dev;
		
		for(i =0; i <7; i++)
		{
			msg[2*i].addr     = ((rt_uint8_t) PCF8563_ADDRESS>>1);
			msg[2*i].flags    =  RT_I2C_WR;
			mem_addr[0]     = (rt_uint8_t) (0x02+i);
			msg[2*i].buf      = (rt_uint8_t *) mem_addr;
			msg[2*i].len      =  1;
			
			msg[2*i+1].addr     = ((rt_uint8_t) PCF8563_ADDRESS>>1);
			msg[2*i+1].flags    =  RT_I2C_RD;
			msg[2*i+1].buf      = (rt_uint8_t *)ptr+i;
			msg[2*i+1].len      =  1;// 秒 分 时 星期 日 月 年  共7个字节
			
			ret = rt_i2c_transfer(pcf8563->bus, &msg[2*i], 2);
			if(ret != 2)
			{
				if (result == RT_EOK)
				{
					/* 释放互斥锁 */
					rt_mutex_release(pcf8563_mutex);
				}
				return RT_ERROR;
			}
		}
		if (result == RT_EOK)
		{
			/* 释放互斥锁 */
			rt_mutex_release(pcf8563_mutex);
		}
		return RT_EOK;
}

static rt_err_t set_rtc_time_stamp(rt_device_t dev,void *args)
{
    struct pcf8563_device *pcf8563;
		struct rt_i2c_msg msg[7];
		rt_uint8_t* ptr;
		rt_uint8_t mem_addr[2] = {0};
		rt_uint8_t i;
    rt_size_t ret = 0;
    RT_ASSERT(dev != 0);
		
		rt_err_t result = rt_mutex_take(pcf8563_mutex, RT_WAITING_FOREVER);
		
		ptr = args;
	
		pcf8563 = (struct pcf8563_device *) dev;
	/*********************写使能****************************/
		rt_kprintf("pcf8563: set rtc_time is ");	
		
		for(i = 0; i<7;i++)
		{
			msg[i].addr     = ((rt_uint8_t) PCF8563_ADDRESS>>1);
			msg[i].flags    =  RT_I2C_WR;
			mem_addr[0]     = (rt_uint8_t) (0x02+i);
			mem_addr[1]     = *(ptr+i);
			msg[i].buf      = (rt_uint8_t *) mem_addr;
			msg[i].len      =  2;
			
			ret = rt_i2c_transfer(pcf8563->bus, &msg[i], 1);
			if(ret != 1)
			{
				if (result == RT_EOK)
				{
					/* 释放互斥锁 */
					rt_mutex_release(pcf8563_mutex);
				}
				return RT_ERROR;
			}
			rt_kprintf("%02X ",mem_addr[1]);			
		}
		rt_kprintf("\r\n");
		
		if (result == RT_EOK)
		{
			/* 释放互斥锁 */
			rt_mutex_release(pcf8563_mutex);
		}
		return RT_EOK;
}

static rt_err_t pcf8563_control(rt_device_t dev, int cmd, void *args)
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
					ptr[0] = ptr[0]&0x7f;
					ptr[1] = ptr[1]&0x7f;
					ptr[2] = ptr[2]&0x3f;
					ptr[3] = ptr[3]&0x3f;
					ptr[4] = ptr[4]&0x07;
					ptr[5] = ptr[5]&0x1f;
				}
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:
        result = set_rtc_time_stamp(dev,(rt_uint8_t *)args);
				if(result == RT_EOK)
					rt_kprintf("pcf8563: set rtc_time sucess\r\n");
				else
					rt_kprintf("pcf8563: set rtc_time fail\r\n");
        break;
    }
    return result;
}

static rt_size_t pcf8563_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
	return 0;
}

static rt_size_t pcf8563_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
	return 0;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device pcf8563_ops =
{
    pcf8563_init,
    pcf8563_open,
    pcf8563_close,
    pcf8563_read,
    pcf8563_write,
    pcf8563_control
};
#endif


rt_err_t pcf8563_register(const char *fm_device_name, const char *i2c_bus,void *user_data)
{
    static struct pcf8563_device pcf8563_drv;
    struct rt_i2c_bus_device *bus;

    bus = rt_i2c_bus_device_find(i2c_bus);
    if (bus == RT_NULL)
    {
        return RT_ENOSYS;
    }

    pcf8563_drv.bus = bus;
    pcf8563_drv.parent.type      = RT_Device_Class_Block;
#ifdef RT_USING_DEVICE_OPS
    pcf8563_drv.parent.ops       = &pcf8563_ops;
#else
    pcf8563_drv.parent.init      = pcf8563_init;
    pcf8563_drv.parent.open      = pcf8563_open;
    pcf8563_drv.parent.close     = pcf8563_close;
    pcf8563_drv.parent.read      = pcf8563_read;
    pcf8563_drv.parent.write     = pcf8563_write;
    pcf8563_drv.parent.control   = pcf8563_control;
#endif
		pcf8563_drv.parent.user_data = user_data;

    return rt_device_register(&pcf8563_drv.parent, fm_device_name, RT_DEVICE_FLAG_RDWR);
}



#ifdef RT_USING_PCF8563
static int rtc_pcf8563_init(void)
{
	rt_err_t result = RT_EOK;
	
		/* 创建互斥锁 */
	pcf8563_mutex = rt_mutex_create("pcf8563_mutex", RT_IPC_FLAG_FIFO);
	if (pcf8563_mutex == RT_NULL)
	{
		rt_kprintf("rt_mutex_create make fail\r\n");
	}	
	
	result = pcf8563_register("pcf8563","i2c1",RT_NULL);
//	if(result == RT_EOK)
//		rt_kprintf("pcf8563: pcf8563 register sucess!!!\r\n");
//	else
//		rt_kprintf("pcf8563: pcf8563 register fail!!!\r\n");
//	
	return result;
	
}
INIT_COMPONENT_EXPORT(rtc_pcf8563_init);
#endif






