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
#include "drv_spi_esam.h"
#include "stm32f4xx_hal.h"
#include "drv_spi.h"

static rt_err_t esam_init(rt_device_t dev)
{
	return RT_EOK;
}
static rt_err_t esam_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t esam_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t esam_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK;
}

static rt_size_t esam_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
	
	return 0;
}

static rt_size_t esam_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
	return 0;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device esam_ops =
{
    esam_init,
    esam_open,
    esam_close,
    esam_read,
    esam_write,
    esam_control
};
#endif


#ifdef RT_USING_SPI_ESAM
static int spi_esam_init(void)
{
	rt_err_t res;

	res = rt_hw_spi_device_attach("spi1","spi10",GPIOA,GPIO_PIN_4);
	if(res != RT_EOK)
	{
		rt_kprintf("spi10 device register failed!\n");
		return res;
	}
}
INIT_COMPONENT_EXPORT(spi_esam_init);
#endif






