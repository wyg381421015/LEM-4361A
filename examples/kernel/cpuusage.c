#include <rtthread.h>
#include <rthw.h>

#define CPU_USAGE_CALC_TICK    1000

static rt_uint8_t  cpu_usage_current = 0, cpu_usage_max= 0;
static rt_uint32_t total_count = 0;

static void cpu_usage_idle_hook()
{
    rt_tick_t tick = 0;
    rt_uint32_t count = 0;

    if (total_count == 0)
    {
        /* get total count */
        rt_enter_critical();/* lock scheduler */
        tick = rt_tick_get();
        while(rt_tick_get() - tick < CPU_USAGE_CALC_TICK)
        {
            total_count++;
        }
		rt_kprintf("total_count最大参考值=%u\n",total_count);
        rt_exit_critical();/* unlock scheduler */
    }

    count = 0;
    /* get CPU usage */
    tick = rt_tick_get();
    while (rt_tick_get() - tick < CPU_USAGE_CALC_TICK)
    {
        count++;
    }

    /* calculate usage_current and usage_max */
    if (count < total_count)
    {
        count = total_count - count;
        cpu_usage_current = (count * 100) / total_count;
		if(cpu_usage_current > cpu_usage_max)
		{
			cpu_usage_max = cpu_usage_current;
		}
    }
    else
    {
        total_count = count;
        /* no CPU usage */
        cpu_usage_current = 0;
        cpu_usage_max = 0;
    }
}

void cpu_usage_get(rt_uint8_t *usage_current, rt_uint8_t *usage_max)
{
    RT_ASSERT(usage_current != RT_NULL);
    RT_ASSERT(usage_max != RT_NULL);

    *usage_current = cpu_usage_current;
    *usage_max = cpu_usage_max;
}

void cpu_usage_init(void)
{
    /* set idle thread hook */
    rt_thread_idle_sethook(cpu_usage_idle_hook);
}
