/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2018-11-19     flybreak     add stm32f407-atk-explorer bsp
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <global.h>
#include <storage.h>

#ifdef RT_USING_DFS
/* dfs init */

/* dfs filesystem:ELM filesystem init */
//#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#include <dfs_posix.h>
#include <uffs/uffs_fd.h>

//#include <dfs_romfs.h>
#endif

//#define MAX_DIR_NUM 8

/* defined the LED0 pin: PF9 */
#define RUN_PIN    	GET_PIN(B, 11)
#define PWM_PIN     GET_PIN(A, 1)

//#define PWR_OUT				GET_PIN(F,2)
//#define PWR_OFF				GET_PIN(F,3)

 
//static void Power_out(void)
//{
//		rt_pin_write(PWR_OUT, PIN_HIGH);
//		rt_pin_write(PWR_OFF, PIN_LOW);
//		rt_thread_mdelay(200);
//		rt_pin_write(PWR_OUT, PIN_LOW);
//		rt_pin_write(PWR_OFF, PIN_LOW);
//}

//static void Power_off(void)
//{
//		rt_pin_write(PWR_OUT, PIN_LOW);
//		rt_pin_write(PWR_OFF, PIN_HIGH);
//		rt_thread_mdelay(200);
//		rt_pin_write(PWR_OUT, PIN_LOW);
//		rt_pin_write(PWR_OFF, PIN_LOW);
//}


int main(void)
{
    int result;
		rt_uint8_t i;
	
    /* set LED0 pin mode to output */
    rt_pin_mode(RUN_PIN, PIN_MODE_OUTPUT);
	
    while (1)
    {
        rt_pin_write(RUN_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(RUN_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
    return RT_EOK;
}

void start_debug(void)
{
	DEBUG_MSH = 1;
}
MSH_CMD_EXPORT_ALIAS(start_debug,	debug,	start_debug);
