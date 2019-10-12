#ifndef __ANALOG_H__
#define __ANALOG_H__

#include <rtdevice.h>


/***********模拟量YC结构体******************************/
typedef struct        //模拟量测量结果结构变量
{	
	unsigned long  Bat_vol;		    //电池电压  3位小数
	unsigned long  Pow_5V;       //充电电流  3位小数
	unsigned long  Pow_3V;          //导引电压  1位小数
}Power_Analog_TypeDef;


extern int get_analog_data(Power_Analog_TypeDef *analog);

#endif

