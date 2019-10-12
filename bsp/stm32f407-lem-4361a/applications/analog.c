#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include <stdio.h>
#include <analog.h>
#include <board.h>
#include <global.h>

#define ADC_Channel_10 		0x0A
#define ADC_Channel_11 		0x0B
#define ADC_Channel_15 		0x0F

#define ADC_CHANNEL_NUM		3
#define BAT_RATE	0
#define POW5V_RATE	1
#define POW3V_RATE	2


#define BAT_CHARGE_PIN    	GET_PIN(G, 0)//电池充电使能引脚
#define BAT_START_CHARGE()    {rt_pin_write(BAT_CHARGE_PIN, PIN_HIGH);}//电池开始充电
#define BAT_STOP_CHARGE()    	{rt_pin_write(BAT_CHARGE_PIN, PIN_LOW);}//电池停止充电

#define BAT_STAT1_PIN    		GET_PIN(E, 2)//电池状态1 引脚
#define BAT_STAT2_PIN    		GET_PIN(E, 3)//电池状态2 引脚

//#define DEVICE_NAME			"adc1"//串口5设备名

#define THREAD_ANALOG_PRIORITY     22
#define THREAD_ANALOG_STACK_SIZE   1024
#define THREAD_ANALOG_TIMESLICE    7

//extern rt_thread_t meter_sig;					//meter thread
//extern rt_uint8_t meter_signal_flag;

static struct rt_thread analog;
static rt_adc_device_t analog_adc;
static rt_uint8_t analog_stack[THREAD_ANALOG_STACK_SIZE];//线程堆栈

static rt_uint32_t s_lPowratevalue[ADC_CHANNEL_NUM] = {5000,6250,5000};

Power_Analog_TypeDef Str_Power_Analog;

static void analog_adc_read(rt_adc_device_t device);
static void battery_manage(void);

static void analog_thread_entry(void *parameter)
{
	rt_err_t res = RT_ERROR;
	
	analog_adc = (rt_adc_device_t)rt_device_find(RT_ANALOG_ADC);
	
	if(analog_adc != RT_NULL)
	{
		rt_lprintf("[Analog]:%s device find sucess!\r\n",RT_ANALOG_ADC);
		
		/* 使能ADC设备 */
    res = rt_adc_enable(analog_adc, ADC_Channel_10);
		if(res == RT_EOK)
		{
			rt_lprintf("[Analog]:ADC_Channel_10 enable ok!\r\n");
		}
		res = rt_adc_enable(analog_adc, ADC_Channel_11);
		if(res == RT_EOK)
		{
			rt_lprintf("[Analog]:ADC_Channel_11 enable ok!\r\n");
		}
		res = rt_adc_enable(analog_adc, ADC_Channel_15);
		
		if(res == RT_EOK)
		{
			rt_lprintf("[Analog]:ADC_Channel_15 enable ok!\r\n");
		}
	}
	else
	{
		res = RT_ERROR;
		rt_lprintf("[Analog]:Not find device %s!\r\n",RT_ANALOG_ADC);
		return;
	}
	
	rt_pin_mode(BAT_CHARGE_PIN, PIN_MODE_OUTPUT);//电池充电使能引脚
	rt_pin_mode(BAT_STAT1_PIN, PIN_MODE_INPUT);//电池状态
	rt_pin_mode(BAT_STAT2_PIN, PIN_MODE_INPUT);//电池状态
	
	rt_thread_mdelay(100);
	analog_adc_read(analog_adc);
	
	while (1)
	{
		analog_adc_read(analog_adc);
		battery_manage();
		
		
		rt_thread_mdelay(2000);
	}
}

static void analog_adc_read(rt_adc_device_t device)
{
	rt_uint8_t	i;
	rt_uint32_t adc_value;
	
	adc_value = 0;
	for(i=0;i<8;i++)
	{
		adc_value += rt_adc_read(device, ADC_Channel_10);			
	}
	adc_value =adc_value/8;
	
	Str_Power_Analog.Bat_vol = (rt_uint32_t)((s_lPowratevalue[BAT_RATE]*adc_value)>>12);
	
	adc_value = 0;
	for(i=0;i<8;i++)
	{
		adc_value += rt_adc_read(device, ADC_Channel_11);			
	}
	adc_value =adc_value/8;
	
	Str_Power_Analog.Pow_5V = (rt_uint32_t)((s_lPowratevalue[POW5V_RATE]*adc_value)>>12);
	
	adc_value = 0;
	for(i=0;i<8;i++)
	{
		adc_value += rt_adc_read(device, ADC_Channel_15);			
	}
	adc_value =adc_value/8;
	
	Str_Power_Analog.Pow_3V = (rt_uint32_t)((s_lPowratevalue[POW3V_RATE]*adc_value)>>12);
	
	rt_lprintf("[Analog]:Bat_vol = %d.%03dV----", Str_Power_Analog.Bat_vol/1000,Str_Power_Analog.Bat_vol%1000);
	rt_lprintf("Pow_5V = %d.%03dV----", Str_Power_Analog.Pow_5V/1000,Str_Power_Analog.Pow_5V%1000);
	rt_lprintf("Pow_3V = %d.%03dV----\r\n", Str_Power_Analog.Pow_3V/1000,Str_Power_Analog.Pow_3V%1000);
}

static void battery_manage(void)
{
	rt_uint8_t stat1,stat2;
	
	
	stat1 = rt_pin_read(BAT_STAT1_PIN);
	stat2 = rt_pin_read(BAT_STAT2_PIN);
	
	if((Str_Power_Analog.Bat_vol>500)&&(Str_Power_Analog.Bat_vol<3700)&&(Str_Power_Analog.Pow_5V>4000))
	{
		BAT_START_CHARGE();
	}
	else if((Str_Power_Analog.Bat_vol>4200)||((stat1 == 1)&&(stat2 == 0)))
	{
		BAT_STOP_CHARGE();
		
		rt_lprintf("[Analog]:battery stat1 is %d stat2 is %d\r\n", stat1,stat2);
	}
}

int get_analog_data(Power_Analog_TypeDef *analog)
{
	analog->Bat_vol = Str_Power_Analog.Bat_vol;
	analog->Pow_5V	= Str_Power_Analog.Pow_5V;
	analog->Pow_3V	= Str_Power_Analog.Pow_3V;
	
	return 0;
}




int analog_thread_init(void)
{
	rt_err_t res;
	
	res=rt_thread_init(&analog,
											"analog",
											analog_thread_entry,
											RT_NULL,
											analog_stack,
											THREAD_ANALOG_STACK_SIZE,
											THREAD_ANALOG_PRIORITY,
											THREAD_ANALOG_TIMESLICE);
	if (res == RT_EOK) 
	{
			rt_thread_startup(&analog);
	}
	return res;
}


#if defined (RT_ANALOG_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(analog_thread_init);
#endif
MSH_CMD_EXPORT(analog_thread_init, analog thread run);


