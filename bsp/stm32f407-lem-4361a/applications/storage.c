#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>   //中断函数头文件 rt_hw_interrupt_disable rt_hw_interrupt_enable
#include <string.h>
#include <stdio.h>
#include <global.h>
#include <storage.h>
#include <meter.h>
#include "strategy.h"
#include "global.h"

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_posix.h>
#include <uffs/uffs_fd.h>
#endif

#define THREAD_STORAGE_PRIORITY     19
#define THREAD_STORAGE_STACK_SIZE   1024*4
#define THREAD_STORAGE_TIMESLICE    5

#define WRITE    1
#define READ     2

static char* get_name(char* strtemp,rt_uint8_t*fpname,rt_uint8_t *fpcnt);
static char* get_pvalue(char* strtemp,rt_uint32_t*fpvalue,rt_uint8_t cnt);
static rt_uint8_t str2num(rt_uint8_t*src,rt_uint32_t *dest);
static rt_uint8_t str2nnum(rt_uint8_t*src,rt_uint8_t *dest);
static rt_uint32_t pow_df(rt_uint8_t m,rt_uint8_t n);

static struct rt_thread storage;
static rt_uint8_t storage_stack[THREAD_STORAGE_STACK_SIZE];//线程堆栈
static rt_mutex_t storage_ReWr_mutex = RT_NULL;
#define MAX_DIR_NUM    14

#define MAX_MALLOC_NUM    2048

///* 定义邮箱控制块 */
//rt_mailbox_t m_save_mail = RT_NULL;

const char* _dir_name[MAX_DIR_NUM]={
	"/SysPara",
	"/ChargePilePara",
	"/Strategy",
	"/Strategy/OrderCharge",
	"/Strategy/PlanOffer",
	"/Strategy/PlanFail",
	"/Strategy/OnlineState",
	"/Meter",
	"/HistoryRecord",
	"/ChargeRecord",
	"/PERecord",
	"/AlarmRecord",
	"/EventRecord",
	"/LOG",
};

//ROUTER_IFO_UNIT RouterIfo;

__align(4) char *Para_Buff = NULL; //固化参数buff
const char* METER_POWER_PATH=(const char*)"/Meter";
const char* LOG_PATH=(const char*)"/LOG";
const char* CHARGE_RECORD_PATH=(const char*)"/ChargeRecord";
const char* HISTORY_RECORD_PATH=(const char*)"/HistoryRecord";

const char* ORDER_CHARGE_PATH=(const char*)"/Strategy/OrderCharge";
const char* PLAN_OFFER_PATH=(const char*)"/Strategy/PlanOffer";
const char* PLAN_FAIL_PATH=(const char*)"/Strategy/PlanFail";
const char* ONLINE_STATE_PATH=(const char*)"/Strategy/OnlineState";

const char* NAND_LOG_PATH_FILE=(const char*)"/LOG/log.txt";
const char* METER_ANALOG_PATH_FILE=(const char*)"/Meter/analog.txt";
const char* ROUTER_PARA_PATH_FILE=(const char*)"/SysPara/RouterPata.txt";
const char* METER_GJFMode_PATH_FILE=(const char*)"/Meter/GJFMode.txt";
const char* METER_HALF_POWER_PATH_FILE=(const char*)"/Meter/MeterHalfPower.txt";

static int Meter_Anolag_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd);
static int Router_Para_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd);
static int Meter_Power_Storage(const char *file,void *Storage_Para,rt_uint32_t YMD,rt_uint32_t cmd);
static int Meter_GJFMode_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd);
static int Meter_HalfPower_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd);
static int Charge_Record_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd);

static int Order_Charge_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd);
static int Plan_Offer_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd);	
static int Plan_Fail_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd);
static int Online_State_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd);

static  int Log_Process(void);
static  int NAND_LogWrite(char* Pnand_Log,rt_uint32_t *nand_LogLen);
static  int LOG_BackUP(void);
static  int Catalogue_Del_Oldest(const char *PATH);
static  int Find_path_file(const char *PATH,int number,char *pathfile,rt_uint32_t *filenum);

static void storage_thread_entry(void *parameter)
{
//	extern int df(const char *path);
//	df("/");
//	extern void ls(const char *pathname);
//	ls("/Meter");
//	ls("/LOG");	
	rt_thread_delay(100);	
	while (1)
	{
//		/* lock scheduler */
//		rt_enter_critical();
///***************************************************************************************************/
////		Meter_AnalogW.ulVol--;// 交流电压
////		Meter_AnalogW.ulCur--;// 交流电流
////		Meter_AnalogW.ulAcPwr--;// 瞬时有功功率
////		Meter_AnalogW.ulMeterTotal--;//有功总电量
////		Meter_AnalogW.ulPwrFactor--;//功率因数
////		Meter_AnalogW.ulFrequency--;//频率

////		Meter_Anolag_Storage(METER_ANALOG_PATH_FILE,WRITE);

////		Meter_Anolag_Storage(METER_ANALOG_PATH_FILE,READ);
///***************************************************************************************************/		
////		ScmMeter_HisData pMeter_HisData;
////		pMeter_HisData.ulMeter_Day.ulPowerT = 100;
////		pMeter_HisData.ulMeter_Day.ulPowerJ = 100;
////		pMeter_HisData.ulMeter_Day.ulPowerF = 100;
////		pMeter_HisData.ulMeter_Day.ulPowerP = 100;
////		pMeter_HisData.ulMeter_Day.ulPowerG = 100;
////		
////		pMeter_HisData.ulMeter_Month.ulPowerT = 200;
////		pMeter_HisData.ulMeter_Month.ulPowerJ = 200;
////		pMeter_HisData.ulMeter_Month.ulPowerF = 200;
////		pMeter_HisData.ulMeter_Month.ulPowerP = 200;
////		pMeter_HisData.ulMeter_Month.ulPowerG = 200;
////		
////		SetStorageData(Cmd_MeterPowerWr,&pMeter_HisData,0x20190801);
////		SetStorageData(Cmd_MeterPowerWr,&pMeter_HisData,0x20190802);
////		SetStorageData(Cmd_MeterPowerWr,&pMeter_HisData,0x20190803);
////		GetStorageData(Cmd_MeterPowerRd,&pMeter_HisData,0x20190802);
////		memset(&pMeter_HisData,0x00,sizeof(pMeter_HisData));		
///***************************************************************************************************/		
//		ScmMeter_PriceModle pPriceModleData;

//	    int i = 0;
//		for(i=0;i<8;i++)
//		{
//			pPriceModleData.uiJFmodID[i] = i+1;
//			
//		}

//		pPriceModleData.EffectiveTime.Second = System_Time_STR.Second;        // 秒
//		pPriceModleData.EffectiveTime.Minute = System_Time_STR.Minute;        // 分
//		pPriceModleData.EffectiveTime.Hour = System_Time_STR.Hour;          // 时
//		pPriceModleData.EffectiveTime.Day = System_Time_STR.Day;           // 日 
//		pPriceModleData.EffectiveTime.Month = System_Time_STR.Month;         // 月
//		pPriceModleData.EffectiveTime.Year = System_Time_STR.Year;          // 年 后两位		

//		pPriceModleData.unEffectiveTime.Second = System_Time_STR.Second;        // 秒
//		pPriceModleData.unEffectiveTime.Minute = System_Time_STR.Minute;        // 分
//		pPriceModleData.unEffectiveTime.Hour = System_Time_STR.Hour;          // 时
//		pPriceModleData.unEffectiveTime.Day = System_Time_STR.Day;           // 日 
//		pPriceModleData.unEffectiveTime.Month = System_Time_STR.Month;         // 月
//		pPriceModleData.unEffectiveTime.Year = System_Time_STR.Year;          // 年 后两位			
//		
//		pPriceModleData.state = 200;//执行状态
//		pPriceModleData.style =30;//计量类型
//		
//		for(i=0;i<48;i++)
//		{
//			pPriceModleData.ulTimeNo[i] = i+1;
//		}		

//		pPriceModleData.count = 48;//有效费率数
//		
//		for(i=0;i<48;i++)
//		{
//			pPriceModleData.ulPriceNo[i] = i+1;
//			
//		}
//		
//		SetStorageData(Cmd_MeterGJFModeWr,&pPriceModleData,0x20190803);	
//	
//		rt_lprintf("存储完成\n");
//		
//		ScmMeter_PriceModle pPriceModleDataRR;

//		GetStorageData(Cmd_MeterGJFModeRd,&pPriceModleDataRR,0x20190802);
//		
//		
//		for(i=0;i<8;i++)
//		{
//			rt_lprintf("pPriceModleData.uiJFmodID[%d]=%d\n",i,pPriceModleData.uiJFmodID[i]);
//		}

//		rt_lprintf("pPriceModleData.EffectiveTime.Year=%02X\n",pPriceModleData.EffectiveTime.Year);	
//		rt_lprintf("pPriceModleData.EffectiveTime.Month=%02X\n",pPriceModleData.EffectiveTime.Month);
//		rt_lprintf("pPriceModleData.EffectiveTime.Day=%02X\n",pPriceModleData.EffectiveTime.Day);	
//		rt_lprintf("pPriceModleData.EffectiveTime.Hour=%02X\n",pPriceModleData.EffectiveTime.Hour);
//		rt_lprintf("pPriceModleData.EffectiveTime.Minute=%02X\n",pPriceModleData.EffectiveTime.Minute);	
//		rt_lprintf("pPriceModleData.EffectiveTime.Second=%02X\n",pPriceModleData.EffectiveTime.Second);

//		rt_lprintf("pPriceModleData.unEffectiveTime.Year=%02X\n",pPriceModleData.unEffectiveTime.Year);	
//		rt_lprintf("pPriceModleData.unEffectiveTime.Month=%02X\n",pPriceModleData.unEffectiveTime.Month);
//		rt_lprintf("pPriceModleData.unEffectiveTime.Day=%02X\n",pPriceModleData.unEffectiveTime.Day);	
//		rt_lprintf("pPriceModleData.unEffectiveTime.Hour=%02X\n",pPriceModleData.unEffectiveTime.Hour);
//		rt_lprintf("pPriceModleData.unEffectiveTime.Minute=%02X\n",pPriceModleData.unEffectiveTime.Minute);	
//		rt_lprintf("pPriceModleData.unEffectiveTime.Second=%02X\n",pPriceModleData.unEffectiveTime.Second);

//		rt_lprintf("pPriceModleData.state=%d\n",pPriceModleData.state);	
//		rt_lprintf("pPriceModleData.style=%d\n",pPriceModleData.style);	

//		for(i=0;i<48;i++)
//		{
//			rt_lprintf("pPriceModleData.ulTimeNo[%d]=%d\n",i,pPriceModleData.ulTimeNo[i]);
//		}	
//		
//		rt_lprintf("pPriceModleData.count=%d\n",pPriceModleData.count);
//		
//		for(i=0;i<48;i++)
//		{
//			rt_lprintf("pPriceModleData.ulPriceNo[%d]=%d\n",i,pPriceModleData.ulPriceNo[i]);
//		}	

//		memset(&pPriceModleData,0x00,sizeof(pPriceModleData));
///***************************************************************************************************/
///***************************************************************************************************/		
////		static rt_uint32_t pulMeter_Half[48]; 
////		int i = 0;
////		for(i=0;i<48;i++)
////		{
////			pulMeter_Half[i] = i+1;		
////			
////		}
////		
////		SetStorageData(Cmd_MeterHalfPowerWr,&pulMeter_Half,0x20190803);	
////	
////		rt_lprintf("存储完成\n");
////		
////		static rt_uint32_t pulMeter_HalfRd[48]; 
////		GetStorageData(Cmd_MeterHalfPowerRd,&pulMeter_HalfRd,0x20190802);
////		for(i=0;i<48;i++)
////		{
////			rt_lprintf("pulMeter_HalfRd[%d]=%d\n",i,pulMeter_HalfRd[i]);
////		}	

////		memset(pulMeter_HalfRd,0x00,sizeof(pulMeter_HalfRd));
///***************************************************************************************************/
//		
//		/* unlock scheduler */
//		rt_exit_critical();	

//		Log_Process();// 根据log大小存储LOG本地
		
//		df("/");
//		extern void list_mem(void);
//		list_mem();		
//		static struct statfs freespace;
//		int rval = statfs((const char*)("/"),&freespace);
//		if(RT_EOK == rval)
//		{
//			rt_lprintf("Storage:freespace.f_bfree=%u\n",freespace.f_bfree); /* free blocks in file system */
//			if(freespace.f_bfree < 1024/16)//剩余空间小于1/16=8MB时，删除LOG文件夹最老的日志  1024/16
//			{
//				Catalogue_Del_Oldest(LOG_PATH);
//			}
//		}
//		else
//		{
//			rt_lprintf("Storage:获取NANDFLASH剩余空间失败=%d\n",rval);	
//		}

		rt_thread_mdelay(1000);
	}
}
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG函数
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/


extern rt_uint32_t LogBufferLen;
extern char LogBuffer[4096];

static int Log_Process(void)
{
	if(LogBufferLen > sizeof(LogBuffer)/2)	
	{	
		//存储到FLASH
		NAND_LogWrite(LogBuffer,&LogBufferLen);	
	}
	else
	{
//		rt_lprintf("[storage]:log未满\n");
	}
	
    return 0;	
}
/*********************************************************************************************************
** Function name:		NAND_LogWrite
** Descriptions:		LOG函数
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int NAND_LogWrite(char* Pnand_Log,rt_uint32_t *nand_LogLen)   //函数添加锁 为了在写文件时候防止丢log
{	   	   
	rt_uint8_t rval=0;			//返回值	
	
	/* 只写并在末尾添加打开 */
	int fd= open(NAND_LOG_PATH_FILE,O_WRONLY | O_CREAT | O_APPEND);
	if(fd >= 0)
	{
		rt_lprintf("[Storage]:%s文件打开成功\n",NAND_LOG_PATH_FILE);
	}
	else
	{
		strcpy((char*)Pnand_Log,"");//清空LogBuffer之后，strlen(LogBuffer) = 0;
		rt_lprintf("[Storage]:%s文件打开失败 fd=%d\n",NAND_LOG_PATH_FILE,fd);
		rval = 1;
		return rval;
	}
	
	static struct stat logstat;
	rval = fstat(fd,&logstat);
	if(rval == RT_EOK)
	{
		rt_lprintf("[Storage]:log字节数=%u\n",logstat.st_size);
	}
	else
	{
		rt_lprintf("[Storage]:log文件状态错误=%d\n",rval);	
	}
	
	int writelen = write(fd,Pnand_Log,*nand_LogLen);//写入首部   返回值0：成功	
	strcpy((char*)Pnand_Log,"");//清空USBLogBuffer之后，strlen(USBLogBuffer) = 0;
	if(writelen > 0)
	{
		rt_lprintf("[Storage]:log文件写入成功 writelen=%d\n",writelen);
	}
	else
	{
		extern rt_thread_t rt_current_thread;
		rt_lprintf("[Storage]:log文件写入失败 writelen=%d,error=%d\n",writelen,rt_current_thread->error);
		rval = 2;
	}
	
	if(close(fd) != UENOERR)
	{
		rt_lprintf("[Storage]:log文件关闭失败\n");
		rval = 3;
		return rval; 			
	}
	else
	{
		rt_lprintf("[Storage]:log文件关闭成功\n");
	}
		
	if(logstat.st_size > 3*1024*1024)//3MB
	{
		rt_lprintf("[Storage]:LOG文件>3MB\n");
		LOG_BackUP();
	}
	
	return rval;
}
/*
*********************************************************************************************************
*	函 数 名: LOG_BakeUP
*	功能说明: 将LOG文件移动到同目录下时间命令文件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static int LOG_BackUP(void)
{
	rt_uint8_t rval=0;			//返回值	  
	char timestamp[32];
	char logBack_file[64];

	sprintf((char*)logBack_file,"%s","/LOG");	
	sprintf((char*)timestamp,"/%02X%02X%02X%02X%02X%02X.TXT",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
															 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
	strcat((char*)logBack_file,(const char*)timestamp);
	rval = unlink((const char*)logBack_file);//目标文件夹存在该文件,先删除
	if(rval == UENOERR)
	{
		rt_lprintf("%s 文件删除成功 fd=%d\n",logBack_file,rval);
	}
	else
	{
		rt_lprintf("%s 删除文件不存在 fd=%d\n",logBack_file,rval);
	}

	rval = rename((const char*)NAND_LOG_PATH_FILE,(const char*)logBack_file);//从源文件夹移动到目的文件夹  名字保持不变
	if(rval == UENOERR)
	{
		rt_lprintf("LOG_BackUP:%s to %s成功\n",NAND_LOG_PATH_FILE,logBack_file);
	}
	else
	{
		rt_lprintf("LOG_BackUP:%s to %s失败,rval=%u\n",NAND_LOG_PATH_FILE,logBack_file,rval);
	}			
	
	return rval;
}
/*********************************************************************************************************
** Function name:		Catalogue_Del_Oldest
** Descriptions:		
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Catalogue_Del_Oldest(const char *PATH)
{
	int result;
	DIR *dirp;
	struct dirent *fd;
	rt_uint32_t filenum = 0;
	char oldest_file[] = "9999-991231080808.TXT";//默认最老的文件
	char oldest_path_file[] = "/AAAAAAAAAAAA/BBBBBBBBBBBA/9999-991231080808.TXT";	
	
	/* 打开PATH 目录 */
	dirp = opendir(PATH);
	if(dirp == RT_NULL)
	{
		rt_lprintf("opendir %s error\n",PATH);
		result = -1;
	}
	else
	{
		rt_lprintf("opendir %s ok\n",PATH);
	}

	/* 读取目录 */
	while ((fd = readdir(dirp)) != RT_NULL)
	{
		rt_lprintf("found %s\n", fd->d_name);
		if(strcmp((const char*)oldest_file,(const char*)fd->d_name) > 0)
		{
			strcpy((char*)oldest_file,(const char*)fd->d_name);
		}
		rt_lprintf("oldest_file:%s\n",(const char*)oldest_file);
		filenum++;	
	}
	rt_lprintf("%s has %d files\n",PATH,filenum);

	sprintf((char*)oldest_path_file,"%s/",PATH);
	strcat((char*)oldest_path_file,(const char*)oldest_file);	
	result = unlink((const char*)oldest_path_file); //删除oldest_file 必须完整路径
	if(UENOERR == result)
	{
		rt_lprintf("%s 文件删除成功\n",oldest_path_file);
	}
	else
	{
		rt_lprintf("%s 删除文件不存在 result=%d\n",oldest_path_file,result);
	}
//	/* 获取目录流的读取位置 */
//	off_t offset = telldir(dirp);	
//	seekdir(dirp,offset);	
//	rt_lprintf("current offset=%d\n",offset);
//	/* 重设读取目录为开头位置 */
//	rewinddir(dirp);		

	/* 关闭目录 */
	result = closedir(dirp);	
	if(result == RT_NULL)
	{
		rt_lprintf("closedir %s ok\n",PATH);
	}
	else
	{
		rt_lprintf("closedir %s error\n",PATH);
		result = -2;
	}	
	return result;
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//	int result;
//	struct dirent *fd;
//	int filenum = 0;
//	char oldest_path_file[64];
//	char history_log[40][32]={0};

//	/* 打开目录 */
//	DIR *dirp = opendir(PATH);
//	if(dirp == RT_NULL)
//	{
//		rt_lprintf("opendir %s error\n",PATH);
//		result = -1;
//		return result;
//	}
//	else
//	{
//		rt_lprintf("opendir %s ok\n",PATH);
//	}

//	/* 读取目录 */
//	while ((fd = readdir(dirp)) != RT_NULL)
//	{
//		rt_lprintf("found %s\n", fd->d_name);
//		strcpy((char*)history_log[filenum],(const char*)fd->d_name);
//		rt_lprintf("history_log[%d]=%s\n",filenum,history_log[filenum]);
//		filenum++;
//	}
//	
//	rt_lprintf("filenum=%d\n",filenum);
//	char temp_file[] = "999999999999.TXT";//临时文件
//	for(int i=0;i<filenum-1;i++)
//	{
//		for(int j=0;j<filenum-i-1;j++)
//		{
//			if(strcmp((char*)history_log[j],(char*)history_log[j+1]) > 0) //寻找最老文件名在最前面
//			{
//				strcpy((char*)temp_file,(char*)history_log[j]);
//				strcpy((char*)(char*)history_log[j],(char*)history_log[j+1]);
//				strcpy((char*)(char*)history_log[j+1],(char*)temp_file);
//			}
//		}
//	}

//	for(int i=0;i<filenum;i++)
//	{
//		rt_lprintf("history_log[%d]=%s\n",i,history_log[i]);
//	}
//	
//	/* 更新需要的目标文件绝对路径 */
//	sprintf((char*)oldest_path_file,"%s/",PATH);
//	strcat((char*)oldest_path_file,(const char*)history_log[0]);
//	result = unlink((const char*)oldest_path_file); //删除oldest_file 必须完整路径
//	if(UENOERR == result)
//	{
//		rt_lprintf("%s 文件删除成功\n",oldest_path_file);
//	}
//	else
//	{
//		rt_lprintf("%s 删除文件不存在 result=%d\n",oldest_path_file,result);
//	}
//	
//	/* 关闭目录 */
//	result = closedir(dirp);	
//	if(result == RT_NULL)
//	{
//		rt_lprintf("closedir %s ok\n",PATH);
//	}
//	else
//	{
//		rt_lprintf("closedir %s error\n",PATH);
//		result = -2;
//	}	
//	return result;
}
/*********************************************************************************************************
** Function name:		Find_path_file
** Descriptions:		
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#define MAX_History_num_10    10
static int Find_path_file(const char *PATH,int number,char *pathfile,rt_uint32_t *filenum)
{
	int result;
	struct dirent *fd;
	*filenum = 0;
	
	char history_record[MAX_History_num_10][24]={0};	
	
	/* 打开目录 */
	DIR *dirp = opendir(PATH);
	if(dirp == RT_NULL)
	{
		rt_lprintf("opendir %s error\n",PATH);
		result = -1;
		return result;
	}
	else
	{
		rt_lprintf("opendir %s ok\n",PATH);
	}
	/* 读取目录文件个数 */
	while ((fd = readdir(dirp)) != RT_NULL)
	{
		(*filenum)++;
	}
	rt_lprintf("current filenum=%d\n",*filenum);
	/* 删除过多文件 */
	if(*filenum > MAX_History_num_10)
	{
		/* 关闭目录 */
		result = closedir(dirp);	
		if(result == RT_NULL)
		{
			rt_lprintf("closedir %s ok\n",PATH);
		}
		else
		{
			rt_lprintf("closedir %s error\n",PATH);
			result = -2;
			return result;
		}
		
		rt_lprintf("错误: too many filenum=%d\n",*filenum);
		int i;
		for(int i=0;i<*filenum - MAX_History_num_10;i++)
		{
			Catalogue_Del_Oldest(PATH);
		}
		/* 二次打开目录 */
		dirp = opendir(PATH);
		if(dirp == RT_NULL)
		{
			rt_lprintf("opendir %s error\n",PATH);
			result = -1;
			return result;
		}
		else
		{
			rt_lprintf("opendir %s ok\n",PATH);
		}		
	}
	else
	{
		/* 重设读取目录为开头位置 */
		rewinddir(dirp);
	}
	
	/* 清除filenum，读取目录 */
	(*filenum) = 0;
	while ((fd = readdir(dirp)) != RT_NULL)
	{
		rt_lprintf("found %s\n", fd->d_name);
		if(*filenum < MAX_History_num_10)
		{
			strcpy((char*)history_record[*filenum],(const char*)fd->d_name);
			rt_lprintf("history_record[%d]=%s\n",*filenum,history_record[*filenum]);
		}
		(*filenum)++;
	}
	/* 关闭目录 */
	result = closedir(dirp);	
	if(result == RT_NULL)
	{
		rt_lprintf("closedir %s ok\n",PATH);
	}
	else
	{
		rt_lprintf("closedir %s error\n",PATH);
		result = -2;
		return result;
	}	
	
	/* 冒泡排序法 */
	RT_ASSERT(*filenum <= MAX_History_num_10);
	char temp_file[] = "9999-999999999999.TXT";//临时文件
	for(int i=0;i<(*filenum)-1;i++)
	{
		for(int j=0;j<(*filenum)-i-1;j++)
		{
			if(strcmp((char*)history_record[j],(char*)history_record[j+1]) < 0) //寻找最新文件名在最前面
			{
				strcpy((char*)temp_file,(char*)history_record[j+1]);
				strcpy((char*)(char*)history_record[j+1],(char*)history_record[j]);
				strcpy((char*)(char*)history_record[j],(char*)temp_file);
			}
		}
	}
	/* 列出历史记录 */
	for(int i=0;i<(*filenum);i++)
	{
		rt_lprintf("history_record[%d]=%s\n",i,history_record[i]);
	}
	
	/* 更新返回的目标文件绝对路径 */
	sprintf((char*)pathfile,"%s/",PATH);
	strcat((char*)pathfile,(const char*)history_record[number]);
		
	return result;
}
/*********************************************************************************************************
** Function name:		Meter_Anolag_Storage
** Descriptions:		
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Meter_Anolag_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd)
{
	int fd,writelen = 0,readlen = 0;
	char buffer[64];
	ScmMeter_Analog* pMeter_Anolag = (ScmMeter_Analog*)Storage_Para;
	
	if(cmd == WRITE)//保存到本地write
	{
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"ulVol=%u\n",(rt_uint32_t)pMeter_Anolag->ulVol);// 交流电压
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulCur=%u\n",(rt_uint32_t)pMeter_Anolag->ulCur);// 交流电流
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulAcPwr=%u\n",(rt_uint32_t)pMeter_Anolag->ulAcPwr);// 瞬时有功功率
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulMeterTotal=%u\n",(rt_uint32_t)pMeter_Anolag->ulMeterTotal);//有功总电量
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPwrFactor=%u\n",(rt_uint32_t)pMeter_Anolag->ulPwrFactor);//功率因数
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulFrequency=%u\n",(rt_uint32_t)pMeter_Anolag->ulFrequency);//频率
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		if(strlen((const char*)Para_Buff)>MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			return -1;
		}
		else
		{
//			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}
		/* Opens the file, if it is existing. If not, a new file is created. */
		fd= open(file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("%s文件打开成功 fd=%d\n",file,fd);
		}
		else
		{
			rt_lprintf("%s文件打开失败 fd=%d\n",file,fd);
		}
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//写入首部   返回值0：成功	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:文件写入成功 writelen=%d\n",writelen);
		}
		else
		{
			rt_lprintf("[storage]:文件写入失败 writelen=%d\n",writelen);
		}				
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:文件关闭失败\n");
		}			
	}
	else if(cmd == READ) //从本地read
	{
		fd= open(file,O_RDONLY);//打开文件。如果文件不存在，则打开失败。
		if(fd >= 0)
		{
			rt_lprintf("%s文件打开成功\n",file);
		}
		else
		{
			rt_lprintf("%s文件打开失败 fd=%d\n",file,fd);
		}
		readlen=read(fd,Para_Buff,strlen(Para_Buff));	//读出txt里面的内容	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:文件读取成功 readlen=%d\n",readlen);
		}
		else
		{
			rt_lprintf("[storage]:文件读取失败 readlen=%d\n",readlen);
		}
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:文件关闭失败\n");
		}
		*(Para_Buff+readlen) = '\0';//需要追加结束符
		if(readlen < RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:文件读取内容\n%s\n",Para_Buff);
		}
	
		char *fpoint = Para_Buff;//参数
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0};        //最多记录32个字节	

////////////////////交流电压//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulVol")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulVol,1);//返回当前文件读指针
				rt_lprintf("ulVol=%u;\n",pMeter_Anolag->ulVol);
			}			
			else
			{
				rt_lprintf("文件名不符合 ulVol=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		} 			
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////交流电流////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulCur")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulCur,1);//返回当前文件读指针
				rt_lprintf("ulCur=%u;\n",pMeter_Anolag->ulCur);
			}			
			else
			{
				rt_lprintf("文件名不符合 ulCur=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////瞬时有功功率////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulAcPwr")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulAcPwr,1);//返回当前文件读指针
				rt_lprintf("ulAcPwr=%u;\n",pMeter_Anolag->ulAcPwr);
			}			
			else
			{
				rt_lprintf("文件名不符合 ulAcPwr=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////有功总电量////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulMeterTotal")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulMeterTotal,1);//返回当前文件读指针
				rt_lprintf("ulMeterTotal=%u;\n",pMeter_Anolag->ulMeterTotal);
			}			
			else
			{
				rt_lprintf("文件名不符合 ulMeterTotal=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}	
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////功率因数////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulPwrFactor")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulPwrFactor,1);//返回当前文件读指针
				rt_lprintf("ulPwrFactor=%u;\n",pMeter_Anolag->ulPwrFactor);
			}			
			else
			{
				rt_lprintf("文件名不符合 ulPwrFactor=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////频率///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulFrequency")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)pMeter_Anolag->ulFrequency,1);//返回当前文件读指针
				rt_lprintf("ulFrequency=%u;\n",pMeter_Anolag->ulFrequency);
			}			
			else
			{
				rt_lprintf("[storage]:文件名不符合 ulFrequency=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("[storage]:namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:文件无效命令\n");
	}
	
	return 0;
}
/*********************************************************************************************************
** Function name:		Router_Para_Storage
** Descriptions:		
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Router_Para_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd)	
{
	int fd,writelen = 0,readlen = 0;
	char buffer[64];
	
	if(cmd == WRITE)//保存到本地write
	{
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		strcat((char*)Para_Buff,(const char*)"cAssetNum=");
		for(int i=0;i<sizeof(RouterIfo.AssetNum);i++)
		{
			sprintf((char*)buffer,"%02X",RouterIfo.AssetNum[i]);// 表号
			strcat((char*)Para_Buff,(const char*)buffer);
		}
		strcat((char*)Para_Buff,(const char*)"\n");		
		
		if(strlen((const char*)Para_Buff)>MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow\n");
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}
		
/************************************************************************************************/			
		/* Opens the file, if it is existing. If not, a new file is created. */
		int fd= open(file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",file,fd);

			return -2; 			
		}		
/************************************************************************************************/			
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//写入首部   返回值0：成功	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:文件写入成功 writelen=%d\n",writelen);
		}
		else
		{
			rt_lprintf("[storage]:文件写入失败 writelen=%d\n",writelen);
		}				
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:文件关闭失败\n");
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //从本地read
	{
		fd= open(file,O_RDONLY);//打开文件。如果文件不存在，则打开失败。
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",file,fd);
		}
        /***************************************************************************************/
		readlen=read(fd,Para_Buff,strlen(Para_Buff));	//读出txt里面的内容	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:文件读取成功 readlen=%d\n",readlen);
		}
		else
		{
			rt_lprintf("[storage]:文件读取失败 readlen=%d\n",readlen);
		}
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:文件关闭失败\n");
		}
		*(Para_Buff+readlen) = '\0';//需要追加结束符
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:文件读取内容\n%s\n",Para_Buff);
		}
	
		char *fpoint = Para_Buff;//参数
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0};        //最多记录32个字节	

////////////////////资产编号//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"cAssetNum")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&RouterIfo.AssetNum,sizeof(RouterIfo.AssetNum));//返回当前文件读指针
				rt_lprintf("[storage]:cAssetNum：%s\n",RouterIfo.AssetNum);

			}			
			else
			{
				rt_lprintf("[storage]:文件名不符合 ulVol=%s\n",fpname); 
			}
			namelen=0;		
		}
		else
		{
            rt_lprintf("[storage]:namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:文件无效命令\n");
	}

	return 0;	
	
}
/*********************************************************************************************************
** Function name:		Meter_GJFMode_Storage
** Descriptions:		LOG函数
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Meter_GJFMode_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd)	
{
	int fd,writelen = 0,readlen = 0;
	char buffadd[32];
	int i = 0;
	ScmMeter_PriceModle* pMeter_GJFMode = (ScmMeter_PriceModle*)Storage_Para;

	rt_lprintf("Meter_GJFMode_Storage:file = %s\n",(char*)file);	
	
	if(cmd == WRITE)//保存到本地write
	{
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffadd,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",\
								System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
								System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffadd);

		/****************************************************************************************/	
			//计费模型ID
			sprintf((char*)buffadd,"uiJFmodID=%02X%02X%02X%02X%02X%02X%02X%02X\n",\
									pMeter_GJFMode->uiJFmodID[0],pMeter_GJFMode->uiJFmodID[1],\
									pMeter_GJFMode->uiJFmodID[2],pMeter_GJFMode->uiJFmodID[3],
									pMeter_GJFMode->uiJFmodID[4],pMeter_GJFMode->uiJFmodID[5],\
									pMeter_GJFMode->uiJFmodID[6],pMeter_GJFMode->uiJFmodID[7]); 
			strcat((char*)Para_Buff,(const char*)buffadd);
		/****************************************************************************************/
			//生效时间	
			sprintf((char*)buffadd,"EffectiveTime=%02X%02X%02X%02X%02X%02X\n",\
									pMeter_GJFMode->EffectiveTime.Second,\
									pMeter_GJFMode->EffectiveTime.Minute,\
									pMeter_GJFMode->EffectiveTime.Hour,\
									pMeter_GJFMode->EffectiveTime.Day,\
									pMeter_GJFMode->EffectiveTime.Month,\
									pMeter_GJFMode->EffectiveTime.Year); 
			strcat((char*)Para_Buff,(const char*)buffadd);				
		/****************************************************************************************/	
		/****************************************************************************************/
			//失效时间	
			sprintf((char*)buffadd,"unEffectiveTime=%02X%02X%02X%02X%02X%02X\n",\
									pMeter_GJFMode->unEffectiveTime.Second,\
									pMeter_GJFMode->unEffectiveTime.Minute,\
									pMeter_GJFMode->unEffectiveTime.Hour,\
									pMeter_GJFMode->unEffectiveTime.Day,\
									pMeter_GJFMode->unEffectiveTime.Month,\
									pMeter_GJFMode->unEffectiveTime.Year); 
			strcat((char*)Para_Buff,(const char*)buffadd);		
		/****************************************************************************************/
		/****************************************************************************************/	
			//执行状态
			sprintf((char*)buffadd,"state=%u\n",pMeter_GJFMode->state);
			strcat((char*)Para_Buff,(const char*)buffadd);
		/****************************************************************************************/
			//计量类型
			sprintf((char*)buffadd,"style=%u\n",pMeter_GJFMode->style);
			strcat((char*)Para_Buff,(const char*)buffadd);		
		/****************************************************************************************/			
			//48个费率号 0:00~24:00
			for(i=0;i<48;i++)
			{
				sprintf((char*)buffadd,"ulTimeNo[%u]=%u\n",i,pMeter_GJFMode->ulTimeNo[i]); 
				strcat((char*)Para_Buff,(const char*)buffadd);			
			}
		/****************************************************************************************/
			//有效费率数
			sprintf((char*)buffadd,"count=%u\n",pMeter_GJFMode->count); //存储计费模型费率个数
			strcat((char*)Para_Buff,(const char*)buffadd);
		/****************************************************************************************/	
			// count个电价
			for(i=0;i<pMeter_GJFMode->count;i++)
			{
				sprintf((char*)buffadd,"ulPriceNo[%u]=%u\n",i,(rt_uint32_t)pMeter_GJFMode->ulPriceNo[i]); 
				strcat((char*)Para_Buff,(const char*)buffadd);		
				
			}
		/****************************************************************************************/
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)>MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",file,fd);
			return -2;		
		}
/************************************************************************************************/		
		struct dfs_fd *fp= fd_get(fd);//与fd_put配套使用否则打开文件过多
		fd_put(fp);
		rt_lprintf("Storage:fp->pos=%d,fp->size=%d\n",fp->pos,fp->size);
/************************************************************************************************/			
/************************************************************************************************/				
		if(fp->size <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:文件写内容\n%s\n",Para_Buff);
		}
		
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//写入首部   返回值0：成功	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s文件写入成功 writelen=%d\n",(char*)file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s文件写入失败 writelen=%d\n",(char*)file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:文件关闭失败\n");
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //从本地read
	{
		fd= open(file,O_RDONLY);//打开文件。如果文件不存在，则打开失败。
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",file,fd);
			return -2;
		}
        /***************************************************************************************/	
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//读出txt里面的内容	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:计费模型文件读取成功 readlen=%d\n",readlen);
		}
		else
		{
			rt_lprintf("[storage]:计费模型文件读取失败 readlen=%d\n",readlen);
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:文件关闭失败\n");
		}
		
		*(Para_Buff+readlen) = '\0';//需要追加结束符
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:文件读取内容\n%s\n",Para_Buff);
		}

		/************************************************************************************/		
		/************************************************************************************/		
		char *fpoint = Para_Buff;//参数
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0};        //最多记录32个字节
		rt_uint8_t fpnameRd[32] = {0};      //最多记录32个字节	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////需要升级uiJFmodID//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);
		if(namelen)
		{
			sprintf((char*)fpnameRd,"uiJFmodID"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->uiJFmodID,8);
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pMeter_GJFMode->uiJFmodID[0],pMeter_GJFMode->uiJFmodID[1],pMeter_GJFMode->uiJFmodID[2],pMeter_GJFMode->uiJFmodID[3],\
							pMeter_GJFMode->uiJFmodID[4],pMeter_GJFMode->uiJFmodID[5],pMeter_GJFMode->uiJFmodID[6],pMeter_GJFMode->uiJFmodID[7]);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");	
		}
//////////////////生效时间//////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"EffectiveTime"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->EffectiveTime.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pMeter_GJFMode->EffectiveTime.Year,\
							pMeter_GJFMode->EffectiveTime.Month,\
							pMeter_GJFMode->EffectiveTime.Day,\
							pMeter_GJFMode->EffectiveTime.Hour,\
							pMeter_GJFMode->EffectiveTime.Minute,\
							pMeter_GJFMode->EffectiveTime.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
//////////////////失效时间//////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"unEffectiveTime"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->unEffectiveTime.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pMeter_GJFMode->unEffectiveTime.Year,\
							pMeter_GJFMode->unEffectiveTime.Month,\
							pMeter_GJFMode->unEffectiveTime.Day,\
							pMeter_GJFMode->unEffectiveTime.Hour,\
							pMeter_GJFMode->unEffectiveTime.Minute,\
							pMeter_GJFMode->unEffectiveTime.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
////////////////////////////////////////////////////////////////		
///////////////////执行状态/////////////////////////////////////////////////			
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"state"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)			
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->state,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pMeter_GJFMode->state);
			}			
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
///////////////////计量类型/////////////////////////////////////////////////			
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"style"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->style,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pMeter_GJFMode->style);
			}			
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}		
////////////////////计费模型/////////////////////////////////////////////////////////////
		for(i=0;i<48;i++)
		{
			sprintf((char*)fpnameRd,"ulTimeNo[%u]",i); 
		    fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
			if(namelen)//接收完了
			{
				if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
				{
					fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->ulTimeNo[i],1);//返回当前文件读指针
					rt_lprintf("%s=%u;\n",fpnameRd,pMeter_GJFMode->ulTimeNo[i]);
				}
				else
				{
					rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
				}
				namelen=0;
			}
			else
			{
				rt_lprintf("namelen=0\n");
			}
		}
///////////////////计费模型费率个数/////////////////////////////////////////////////			
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"count"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->count,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pMeter_GJFMode->count);
			}			
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		} 		
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////计费模型费率价格//////////////////////////////////////////////////////////
		for(i=0;i<pMeter_GJFMode->count;i++)//
		{
			sprintf((char*)fpnameRd,"ulPriceNo[%u]",i); 

		    fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
			if(namelen)//接收完了
			{
				if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
				{
					fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_GJFMode->ulPriceNo[i],1);//返回当前文件读指针
					rt_lprintf("%s=%u;\n",fpnameRd,pMeter_GJFMode->ulPriceNo[i]);
				}
				else
				{
					rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
				}
				namelen=0;
			}
			else
			{
				rt_lprintf("namelen=0\n");
			}
		}
        rt_lprintf("pMeter_GJFMode file read OK\n");		
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:文件无效命令\n");
	}

	return 0;	
	
}
/*********************************************************************************************************
** Function name:		Meter_HalfPower_Storage
** Descriptions:		Meter_HalfPower_Storage函数
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Meter_HalfPower_Storage(const char *file,void *Storage_Para,rt_uint32_t datalen,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffadd[32];
	int i = 0;
	rt_uint32_t* ulMeter_Half = (rt_uint32_t*)Storage_Para;

	rt_lprintf("Meter_HalfPower_Storage:file = %s\n",(char*)file);	
	
	if(cmd == WRITE)//保存到本地write
	{
		int writelen = 0;
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffadd,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",\
								System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
								System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffadd);

		/****************************************************************************************/			
		//48个HalfPower 0:00~24:00
		for(i=0;i<48;i++)
		{
			sprintf((char*)buffadd,"ulMeter_Half[%u]=%u\n",i,ulMeter_Half[i]); 
			strcat((char*)Para_Buff,(const char*)buffadd);			
		}
		/****************************************************************************************/
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)>MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",file,fd);

			return -2; 			
		}
/************************************************************************************************/		
		struct dfs_fd *fp= fd_get(fd);//与fd_put配套使用否则打开文件过多
		fd_put(fp);
		rt_lprintf("Storage:fp->pos=%d,fp->size=%d\n",fp->pos,fp->size);
/************************************************************************************************/			
/************************************************************************************************/
		if(fp->size <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:文件写内容\n%s\n",Para_Buff);
		}
		
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//写入首部   返回值0：成功	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s文件写入成功 writelen=%d\n",(char*)file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s文件写入失败 writelen=%d\n",(char*)file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:文件关闭失败\n");
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //从本地read
	{
		int readlen = 0;
		fd= open(file,O_RDONLY);//打开文件。如果文件不存在，则打开失败。
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",file,fd);
			return -2; 
		}
        /***************************************************************************************/	
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//读出txt里面的内容	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:半小时电量文件读取成功 readlen=%d\n",readlen);
		}
		else
		{
			rt_lprintf("[storage]:半小时电量文件读取失败 readlen=%d\n",readlen);
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:文件关闭失败\n");
		}
		
		*(Para_Buff+readlen) = '\0';//需要追加结束符
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:半小时电量文件读取内容\n%s\n",Para_Buff);
		}

		/************************************************************************************/		
		/************************************************************************************/		
		char *fpoint = Para_Buff;//参数
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0};        //最多记录32个字节
		rt_uint8_t fpnameRd[32] = {0};      //最多记录32个字节	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////半小时电量////////////////////////////////////////////////////////////
		for(i=0;i<48;i++)
		{
			sprintf((char*)fpnameRd,"ulMeter_Half[%u]",i); 
		    fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
			if(namelen)//接收完了
			{
				if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
				{
					fpoint = get_pvalue(fpoint,(rt_uint32_t*)&ulMeter_Half[i],1);//返回当前文件读指针
					rt_lprintf("%s=%u;\n",fpnameRd,ulMeter_Half[i]);
				}
				else
				{
					rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
				}
				namelen=0;
			}
			else
			{
				rt_lprintf("namelen=0\n");
			}	

		}
        rt_lprintf("ulMeter_Half file read OK\n");		
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:文件无效命令\n");
	}

	return 0;	
	
}
/*********************************************************************************************************
** Function name:		Meter_Power_Storage
** Descriptions:		LOG函数
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Meter_Power_Storage(const char *file,void *Storage_Para,rt_uint32_t YMD,rt_uint32_t cmd)	
{
	int fd,writelen = 0,readlen = 0;
	char buffer[32]; 
	char path[14];
	char path_file[64];
	ScmMeter_HisData* pMeter_HisData = (ScmMeter_HisData*)Storage_Para;

	sprintf((char*)path_file,"%s",METER_POWER_PATH);	
	sprintf((char*)path,"/%06X.txt",YMD>>8);
	strcat((char*)path_file,(const char*)path);
	rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);	
	
	if(cmd == WRITE)//保存到本地write
	{
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"%08X\n",YMD);// 日-20190801
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"ulPowerTday=%06u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Day.ulPowerT);// 日-总电量
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerJday=%06u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Day.ulPowerJ);// 日-尖
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerFday=%06u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Day.ulPowerF);// 日-峰
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerPday=%06u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Day.ulPowerP);// 日-平
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerGday=%06u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Day.ulPowerG);// 日-谷
		strcat((char*)Para_Buff,(const char*)buffer);
        /****************************************************************************************/
		sprintf((char*)buffer,"ulPowerTmon=%08u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Month.ulPowerT);// 月-总电量
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerJmon=%08u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Month.ulPowerJ);// 月-总电量
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerFmon=%08u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Month.ulPowerF);// 月-总电量
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerPmon=%08u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Month.ulPowerP);// 月-总电量
		strcat((char*)Para_Buff,(const char*)buffer);

		sprintf((char*)buffer,"ulPowerGmon=%08u\n",(rt_uint32_t)pMeter_HisData->ulMeter_Month.ulPowerG);// 月-总电量
		strcat((char*)Para_Buff,(const char*)buffer);	
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_RDWR | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}
/************************************************************************************************/		
		struct dfs_fd *fp= fd_get(fd);//与fd_put配套使用否则打开文件过多
		fd_put(fp);
		rt_lprintf("Storage:fp->pos=%d,fp->size=%d\n",fp->pos,fp->size);
/************************************************************************************************/
		if(fp->size > 0)
		{
			char* Para_Bufftemp = rt_malloc(fp->size);//申请内存
			if(Para_Bufftemp == NULL)
			{
				rt_lprintf("[storage]:Para_Bufftemp 分配内存失败\n");
				if(close(fd) != UENOERR)
				{
					rt_lprintf("[storage]:%s文件关闭失败\n",file);
				}			
				return -3;
			}		
			readlen=read(fd,Para_Bufftemp,fp->size);	//读出txt里面的内容	
			if(readlen > 0) //0
			{
				rt_lprintf("[storage]:电表电量文件读取成功 readlen=%d\n",readlen);
			}
			else
			{
				rt_lprintf("[storage]:电表电量文件读取为空 readlen=%d\n",readlen);
			}
			/******文件不为空才进行查找哪天电量，否则直接写当天电量****************************/
			if(readlen > 0) //0
			{
				char ymd[] ="20180808";
				sprintf((char*)ymd,"%08X",YMD);// 日-20190801	
				char *addr = strstr(Para_Bufftemp,ymd);
				if(addr == RT_NULL)
				{
					rt_lprintf("[storage]:%s未找到\n",ymd);		
				}
				else
				{
					rt_lprintf("[storage]:%s找到\n",ymd);
					rt_uint32_t addr_offset = 0x00;
					addr_offset = addr - Para_Bufftemp;
					rt_lprintf("[storage]:addr_offset=%d\n",addr_offset);			
					lseek(fd,addr_offset-23,SEEK_SET);//偏移到指定位置 23：标识更新最开始的时间戳		
				}
				if(readlen <= RT_CONSOLEBUF_SIZE)
				{
					rt_lprintf("[storage]:文件读内容\n%s\n",Para_Bufftemp);
				}		
				rt_free(Para_Bufftemp);Para_Bufftemp = NULL;
			}
		}		
/************************************************************************************/				
/**********更新写当天电量**************************************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//写入首部   返回值0：成功	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s文件写入成功 writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s文件写入失败 writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //从本地read
	{
		fd= open(path_file,O_RDONLY);//打开文件。如果文件不存在，则打开失败。
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/
/************************************************************************************************/		
		struct dfs_fd *fp= fd_get(fd);//与fd_put配套使用否则打开文件过多
		fd_put(fp);
		rt_lprintf("Storage:fp->pos=%d,fp->size=%d\n",fp->pos,fp->size);
		if(fp->size == NULL)
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
			}
			rt_lprintf("[storage]:%s文件为空,直接返回\n",path_file);
			return -1;
		}		
/************************************************************************************************/		
		char* Para_Bufftemp = rt_malloc(fp->size);//申请内存
		if(Para_Bufftemp == NULL)
		{
			rt_lprintf("[storage]:Para_Bufftemp 分配内存失败\n");
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s文件关闭失败\n",file);
			}	
			return -2;
		}		
		readlen=read(fd,Para_Bufftemp,fp->size);	//读出txt里面的内容	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s文件读取成功 readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s文件关闭失败\n",file);
			}
			rt_free(Para_Bufftemp);Para_Bufftemp = NULL;
			rt_lprintf("[storage]:%s文件读取为空 readlen=%d\n",path_file,readlen);
			return -3;
		}
		/******文件不为空才进行查找哪天电量,否则直接写当天电量****************************/
		char ymd[] ="20180808";
		sprintf((char*)ymd,"%08X",YMD);// 日-20190801	
		char *addr = strstr(Para_Bufftemp,ymd);
		if(addr == RT_NULL)
		{
			rt_lprintf("[storage]:%s未找到\n",ymd);	
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s文件关闭失败\n",file);
			}
			rt_free(Para_Bufftemp);Para_Bufftemp = NULL;
			return -4;				
		}
		else
		{
			rt_lprintf("[storage]:%s找到\n",ymd);
			rt_uint32_t addr_offset = 0x00;
			addr_offset = addr - Para_Bufftemp;
			rt_lprintf("[storage]:addr_offset=%d\n",addr_offset);			
			lseek(fd,addr_offset-23,SEEK_SET);//偏移到指定位置 23：标识更新最开始的时间戳		
		}
		*(Para_Bufftemp+readlen) = '\0';//需要追加结束符
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:文件写内容\n%s\n",Para_Bufftemp);
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = addr;//参数
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0};        //最多记录32个字节	
////////////////////日--总电量//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerTday")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Day.ulPowerT,1);//返回当前文件读指针
				rt_lprintf("ulPowerTday=%u;\n",pMeter_HisData->ulMeter_Day.ulPowerT);
			}			
			else        
			{
				rt_lprintf("文件名不符合 ulPowerTday=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
			rt_lprintf("namelen=0\n");
		} 			
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////日--尖电量////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerJday")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Day.ulPowerJ,1);//返回当前文件读指针
				rt_lprintf("ulPowerJday=%u;\n",pMeter_HisData->ulMeter_Day.ulPowerJ);
			}			
			else        
			{
				rt_lprintf("文件名不符合 ulPowerJday=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
			rt_lprintf("namelen=0\n");
		} 
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////日--峰电量////////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerFday")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Day.ulPowerF,1);//返回当前文件读指针
				rt_lprintf("ulPowerFday=%u;\n",pMeter_HisData->ulMeter_Day.ulPowerF);
			}			
			else        
			{
				rt_lprintf("文件名不符合 ulPowerFday=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
			rt_lprintf("namelen=0\n");
		} 
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////日--平电量///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerPday")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Day.ulPowerP,1);//返回当前文件读指针
				rt_lprintf("ulPowerPday=%u;\n",pMeter_HisData->ulMeter_Day.ulPowerP);
			}			
			else        
			{
				rt_lprintf("文件名不符合 ulPowerPday=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
			rt_lprintf("namelen=0\n");
		} 
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////日--谷电量///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerGday")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Day.ulPowerG,1);//返回当前文件读指针
				rt_lprintf("ulPowerGday=%u;\n",pMeter_HisData->ulMeter_Day.ulPowerG);
			}			
			else        
			{
				rt_lprintf("文件名不符合 ulPowerGday=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
			rt_lprintf("namelen=0\n");
		} 
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////月--总电量///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerTmon")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Month.ulPowerT,1);//返回当前文件读指针
				rt_lprintf("ulPowerTmon=%u;\n",pMeter_HisData->ulMeter_Month.ulPowerT);
			}			
			else        
			{
				rt_lprintf("文件名不符合 ulPowerTmon=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////月--尖电量///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerJmon")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Month.ulPowerJ,1);//返回当前文件读指针
				rt_lprintf("ulPowerJmon=%u;\n",pMeter_HisData->ulMeter_Month.ulPowerJ);
			}			
			else        
			{
				rt_lprintf("文件名不符合 ulPowerJmon=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////月--峰电量///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerFmon")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Month.ulPowerF,1);//返回当前文件读指针
				rt_lprintf("ulPowerFmon=%u;\n",pMeter_HisData->ulMeter_Month.ulPowerF);
			}			
			else        
			{
				rt_lprintf("文件名不符合 ulPowerFmon=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////月--平电量///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerPmon")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Month.ulPowerP,1);//返回当前文件读指针
				rt_lprintf("ulPowerPmon=%u;\n",pMeter_HisData->ulMeter_Month.ulPowerP);
			}			
			else        
			{
				rt_lprintf("文件名不符合 ulPowerPmon=%s\n",fpname); 
			}
			namelen=0;			
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////月--谷电量///////////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)"ulPowerGmon")==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pMeter_HisData->ulMeter_Month.ulPowerG,1);//返回当前文件读指针
				rt_lprintf("ulPowerGmon=%u;\n",pMeter_HisData->ulMeter_Month.ulPowerG);
			}			
			else        
			{
				rt_lprintf("文件名不符合 ulPowerGmon=%s\n",fpname); 
			}
			namelen=0;
		}
		else
		{
            rt_lprintf("namelen=0\n");
		}
		rt_free(Para_Bufftemp);Para_Bufftemp = NULL;		
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:文件无效命令\n");
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Charge_Record_Storage
** Descriptions:		LOG函数
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Charge_Record_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffer[48]; 
	char path_file[64];
	CHG_ORDER_EVENT* pCharge_Record = (CHG_ORDER_EVENT*)Storage_Para;
		
	if(cmd == WRITE)//保存到本地write
	{
		// 创建文件名
		int writelen = 0;
		char path[18];
		sprintf((char*)path_file,"%s",PATH);
		sprintf((char*)path,"/%04u-%02X%02X%02X%02X%02X%02X.txt",(rt_uint32_t)pCharge_Record->OrderNum,\
																			  pCharge_Record->StartTimestamp.Second,\
																			  pCharge_Record->StartTimestamp.Minute,\
																			  pCharge_Record->StartTimestamp.Hour,\
																			  pCharge_Record->StartTimestamp.Day,\
																			  pCharge_Record->StartTimestamp.Month,\
																			  pCharge_Record->StartTimestamp.Year);
		strcat((char*)path_file,(const char*)path);
		rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);		
		
		// 准备写的内容
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"OrderNum=%04u\n",(rt_uint32_t)pCharge_Record->OrderNum);//	事件记录序号
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"StartTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->StartTimestamp.Second,\
																		  (rt_uint32_t)pCharge_Record->StartTimestamp.Minute,\
																		  (rt_uint32_t)pCharge_Record->StartTimestamp.Hour,\
																		  (rt_uint32_t)pCharge_Record->StartTimestamp.Day,\
																		  (rt_uint32_t)pCharge_Record->StartTimestamp.Month,\
																		  (rt_uint32_t)pCharge_Record->StartTimestamp.Year);//  事件发生时间
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		sprintf((char*)buffer,"FinishTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->FinishTimestamp.Second,\
																		   (rt_uint32_t)pCharge_Record->FinishTimestamp.Minute,\
																		   (rt_uint32_t)pCharge_Record->FinishTimestamp.Hour,\
																		   (rt_uint32_t)pCharge_Record->FinishTimestamp.Day,\
																		   (rt_uint32_t)pCharge_Record->FinishTimestamp.Month,\
																		   (rt_uint32_t)pCharge_Record->FinishTimestamp.Year);//  事件结束时间
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// 事件发生原因
		sprintf((char*)buffer,"Reason=%03u\n",(rt_uint32_t)pCharge_Record->Reason); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// 事件上报状态 = 通道上报状态
		sprintf((char*)buffer,"ChannelState=%03u\n",(rt_uint32_t)pCharge_Record->ChannelState);
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// 充电申请单号（SIZE(16)）
		sprintf((char*)buffer,"RequestNO=");
		char bytebuf[] = "FF";
		for(int i=0;i<sizeof(pCharge_Record->RequestNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pCharge_Record->RequestNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
		
		// 用户id  visible-string（SIZE(64)）
		sprintf((char*)buffer,"cUserID=");
		for(int i=0;i<sizeof(pCharge_Record->cUserID);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pCharge_Record->cUserID[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
	
		//	路由器资产编号 visible-string（SIZE(22)）
		sprintf((char*)buffer,"AssetNO=");
		for(int i=0;i<sizeof(pCharge_Record->AssetNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pCharge_Record->AssetNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");	
	
		//	充电需求电量（单位：kWh，换算：-2）
		sprintf((char*)buffer,"ChargeReqEle=%08u\n",(rt_uint32_t)pCharge_Record->ChargeReqEle);
		strcat((char*)Para_Buff,(const char*)buffer);	
		
		sprintf((char*)buffer,"RequestTimeStamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->RequestTimeStamp.Second,\
																		    (rt_uint32_t)pCharge_Record->RequestTimeStamp.Minute,\
																		    (rt_uint32_t)pCharge_Record->RequestTimeStamp.Hour,\
																		    (rt_uint32_t)pCharge_Record->RequestTimeStamp.Day,\
																		    (rt_uint32_t)pCharge_Record->RequestTimeStamp.Month,\
																		    (rt_uint32_t)pCharge_Record->RequestTimeStamp.Year);//	充电申请时间
		strcat((char*)Para_Buff,(const char*)buffer);	
	
		sprintf((char*)buffer,"PlanUnChg_TimeStamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Second,\
																		       (rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Minute,\
																		       (rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Hour,\
																		       (rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Day,\
																		       (rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Month,\
																		       (rt_uint32_t)pCharge_Record->PlanUnChg_TimeStamp.Year);//	计划用车时间
		strcat((char*)Para_Buff,(const char*)buffer);		

		//	充电模式 {正常（0），有序（1）}
		sprintf((char*)buffer,"ChargeMode=%08u\n",(rt_uint32_t)pCharge_Record->ChargeMode);
		strcat((char*)Para_Buff,(const char*)buffer);
		
		//	启动时电表表底值
		sprintf((char*)buffer,"StartMeterValue=%08u\n",(rt_uint32_t)pCharge_Record->StartMeterValue);
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		//	停止时电表表底值
		sprintf((char*)buffer,"StopMeterValue=%08u\n",(rt_uint32_t)pCharge_Record->StopMeterValue);
		strcat((char*)Para_Buff,(const char*)buffer);
		
	
		sprintf((char*)buffer,"ChgStartTime=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->ChgStartTime.Second,\
																		(rt_uint32_t)pCharge_Record->ChgStartTime.Minute,\
																		(rt_uint32_t)pCharge_Record->ChgStartTime.Hour,\
																		(rt_uint32_t)pCharge_Record->ChgStartTime.Day,\
																		(rt_uint32_t)pCharge_Record->ChgStartTime.Month,\
																		(rt_uint32_t)pCharge_Record->ChgStartTime.Year);//	充电启动时间	
	
		sprintf((char*)buffer,"ChgStopTime=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pCharge_Record->ChgStopTime.Second,\
																	   (rt_uint32_t)pCharge_Record->ChgStopTime.Minute,\
																	   (rt_uint32_t)pCharge_Record->ChgStopTime.Hour,\
																	   (rt_uint32_t)pCharge_Record->ChgStopTime.Day,\
																	   (rt_uint32_t)pCharge_Record->ChgStopTime.Month,\
																	   (rt_uint32_t)pCharge_Record->ChgStopTime.Year);//	充电停止时间	

		//	已充电量（单位：kWh，换算：-2）
		sprintf((char*)buffer,"ucChargeEle=%08u\n",(rt_uint32_t)pCharge_Record->ucChargeEle);
		strcat((char*)Para_Buff,(const char*)buffer);
		
		//	已充时间（单位：s）	
		sprintf((char*)buffer,"ucChargeTime=%08u\n",(rt_uint32_t)pCharge_Record->ucChargeTime);
		strcat((char*)Para_Buff,(const char*)buffer);		
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
		/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}		
		/************************************************************************************/				
		/**********更新写当天电量************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//写入首部   返回值0：成功	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s文件写入成功 writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s文件写入失败 writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //从本地read
	{
		int readlen = 0;
		rt_uint32_t file_num;	
		Find_path_file(PATH,ordernum,path_file,&file_num);

		fd= open(path_file,O_RDONLY);//打开文件。如果文件不存在，则打开失败。
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/			
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//读出txt里面的内容	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s文件读取成功 readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
			}
			rt_lprintf("[storage]:%s文件读取为空 readlen=%d\n",path_file,readlen);
			return -3;
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}
		/***************************************************************************************/
		*(Para_Buff+readlen) = '\0';//需要追加结束符
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:%s文件读内容\n%s\n",path_file,Para_Buff);
		}		
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = Para_Buff;//参数
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0,};        //最多记录32个字节
		rt_uint8_t fpnameRd[32] = {0,};      //最多记录32个字节	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////事件记录序号/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OrderNum"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->OrderNum,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->OrderNum);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////事件发生时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"StartTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->StartTimestamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->StartTimestamp.Year,\
							pCharge_Record->StartTimestamp.Month,\
							pCharge_Record->StartTimestamp.Day,\
							pCharge_Record->StartTimestamp.Hour,\
							pCharge_Record->StartTimestamp.Minute,\
							pCharge_Record->StartTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////事件结束时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"FinishTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->FinishTimestamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->FinishTimestamp.Year,\
							pCharge_Record->FinishTimestamp.Month,\
							pCharge_Record->FinishTimestamp.Day,\
							pCharge_Record->FinishTimestamp.Hour,\
							pCharge_Record->FinishTimestamp.Minute,\
							pCharge_Record->FinishTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////事件发生原因/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Reason"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->Reason,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->Reason);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////事件上报状态 = 通道上报状态/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChannelState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ChannelState,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->ChannelState);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////充电申请单号 SIZE(16)/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"RequestNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->RequestNO,sizeof(pCharge_Record->RequestNO));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pCharge_Record->RequestNO);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////用户id  visible-string（SIZE(64)）/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"cUserID"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->cUserID,sizeof(pCharge_Record->cUserID));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pCharge_Record->cUserID);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////路由器资产编号 visible-string（SIZE(22)）////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"AssetNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->AssetNO,sizeof(pCharge_Record->AssetNO));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pCharge_Record->AssetNO);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}	
/////////充电需求电量（单位：kWh，换算：-2）/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChargeReqEle"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ChargeReqEle,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->ChargeReqEle);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////充电申请时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"RequestTimeStamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->RequestTimeStamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->RequestTimeStamp.Year,\
							pCharge_Record->RequestTimeStamp.Month,\
							pCharge_Record->RequestTimeStamp.Day,\
							pCharge_Record->RequestTimeStamp.Hour,\
							pCharge_Record->RequestTimeStamp.Minute,\
							pCharge_Record->RequestTimeStamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////计划用车时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"PlanUnChg_TimeStamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->PlanUnChg_TimeStamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->PlanUnChg_TimeStamp.Year,\
							pCharge_Record->PlanUnChg_TimeStamp.Month,\
							pCharge_Record->PlanUnChg_TimeStamp.Day,\
							pCharge_Record->PlanUnChg_TimeStamp.Hour,\
							pCharge_Record->PlanUnChg_TimeStamp.Minute,\
							pCharge_Record->PlanUnChg_TimeStamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
/////////充电模式 {正常（0），有序（1）}/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChargeMode"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ChargeMode,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->ChargeMode);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}		
/////////启动时电表表底值////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"StartMeterValue"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->StartMeterValue,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->StartMeterValue);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}		
/////////停止时电表表底值////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"StopMeterValue"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->StopMeterValue,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->StopMeterValue);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}	
////////////////////充电启动时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"ChgStartTime"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ChgStartTime.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->ChgStartTime.Year,\
							pCharge_Record->ChgStartTime.Month,\
							pCharge_Record->ChgStartTime.Day,\
							pCharge_Record->ChgStartTime.Hour,\
							pCharge_Record->ChgStartTime.Minute,\
							pCharge_Record->ChgStartTime.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////充电启动时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"ChgStopTime"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ChgStopTime.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pCharge_Record->ChgStopTime.Year,\
							pCharge_Record->ChgStopTime.Month,\
							pCharge_Record->ChgStopTime.Day,\
							pCharge_Record->ChgStopTime.Hour,\
							pCharge_Record->ChgStopTime.Minute,\
							pCharge_Record->ChgStopTime.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
/////////已充电量（单位：kWh，换算：-2）////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ucChargeEle"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ucChargeEle,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->ucChargeEle);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////已充时间（单位：s）	///////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ucChargeTime"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pCharge_Record->ucChargeTime,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pCharge_Record->ucChargeTime);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
		rt_lprintf("[storage]:%s文件写成功\n",path_file);
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:%s文件无效命令\n",path_file);
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Order_Charge_Storage
** Descriptions:		
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Order_Charge_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffer[48]; 
	char path_file[64];
	ORDER_CHG_EVENT* pOrder_Charge = (ORDER_CHG_EVENT*)Storage_Para;
	
	if(cmd == WRITE)//保存到本地write
	{
		// 创建文件名
		int writelen = 0;
		char path[18];
		sprintf((char*)path_file,"%s",PATH);
		sprintf((char*)path,"/%04u-%02X%02X%02X%02X%02X%02X.txt",(rt_uint32_t)pOrder_Charge->OrderNum,\
																			  pOrder_Charge->StartTimestamp.Second,\
																			  pOrder_Charge->StartTimestamp.Minute,\
																			  pOrder_Charge->StartTimestamp.Hour,\
																			  pOrder_Charge->StartTimestamp.Day,\
																			  pOrder_Charge->StartTimestamp.Month,\
																			  pOrder_Charge->StartTimestamp.Year);
		strcat((char*)path_file,(const char*)path);
		rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);		
		
		// 准备写的内容
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"OrderNum=%04u\n",(rt_uint32_t)pOrder_Charge->OrderNum);//	事件记录序号
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"StartTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pOrder_Charge->StartTimestamp.Second,\
																		  (rt_uint32_t)pOrder_Charge->StartTimestamp.Minute,\
																		  (rt_uint32_t)pOrder_Charge->StartTimestamp.Hour,\
																		  (rt_uint32_t)pOrder_Charge->StartTimestamp.Day,\
																		  (rt_uint32_t)pOrder_Charge->StartTimestamp.Month,\
																		  (rt_uint32_t)pOrder_Charge->StartTimestamp.Year);//  事件发生时间
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		sprintf((char*)buffer,"FinishTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pOrder_Charge->FinishTimestamp.Second,\
																		   (rt_uint32_t)pOrder_Charge->FinishTimestamp.Minute,\
																		   (rt_uint32_t)pOrder_Charge->FinishTimestamp.Hour,\
																		   (rt_uint32_t)pOrder_Charge->FinishTimestamp.Day,\
																		   (rt_uint32_t)pOrder_Charge->FinishTimestamp.Month,\
																		   (rt_uint32_t)pOrder_Charge->FinishTimestamp.Year);//  事件结束时间
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// 事件发生原因
		sprintf((char*)buffer,"Reason=%03u\n",(rt_uint32_t)pOrder_Charge->Reason); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// 事件上报状态 = 通道上报状态
		sprintf((char*)buffer,"ChannelState=%03u\n",(rt_uint32_t)pOrder_Charge->ChannelState);
		strcat((char*)Para_Buff,(const char*)buffer);

		// 总故障
		sprintf((char*)buffer,"TotalFault=%08u\n",(rt_uint32_t)pOrder_Charge->TotalFault);
		strcat((char*)Para_Buff,(const char*)buffer);	
		
		// 各项故障状态
		sprintf((char*)buffer,"Fau=");
		char bytebuf[] = "FF";
		for(int i=0;i<sizeof(pOrder_Charge->Fau);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pOrder_Charge->Fau[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");	
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
		/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}		
		/************************************************************************************/				
		/**********更新写当天电量************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//写入首部   返回值0：成功	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s文件写入成功 writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s文件写入失败 writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //从本地read
	{
		int readlen = 0;
		rt_uint32_t file_num;	
		Find_path_file(PATH,ordernum,path_file,&file_num);

		fd= open(path_file,O_RDONLY);//打开文件。如果文件不存在，则打开失败。
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/			
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//读出txt里面的内容	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s文件读取成功 readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
			}
			rt_lprintf("[storage]:%s文件读取为空 readlen=%d\n",path_file,readlen);
			return -3;
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}
		/***************************************************************************************/
		*(Para_Buff+readlen) = '\0';//需要追加结束符
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:%s文件读内容\n%s\n",path_file,Para_Buff);
		}		
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = Para_Buff;//参数
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0,};        //最多记录32个字节
		rt_uint8_t fpnameRd[32] = {0,};      //最多记录32个字节	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////事件记录序号/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OrderNum"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->OrderNum,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pOrder_Charge->OrderNum);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////事件发生时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"StartTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->StartTimestamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pOrder_Charge->StartTimestamp.Year,\
							pOrder_Charge->StartTimestamp.Month,\
							pOrder_Charge->StartTimestamp.Day,\
							pOrder_Charge->StartTimestamp.Hour,\
							pOrder_Charge->StartTimestamp.Minute,\
							pOrder_Charge->StartTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////事件结束时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"FinishTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->FinishTimestamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pOrder_Charge->FinishTimestamp.Year,\
							pOrder_Charge->FinishTimestamp.Month,\
							pOrder_Charge->FinishTimestamp.Day,\
							pOrder_Charge->FinishTimestamp.Hour,\
							pOrder_Charge->FinishTimestamp.Minute,\
							pOrder_Charge->FinishTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////事件发生原因/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Reason"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->Reason,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pOrder_Charge->Reason);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////事件上报状态 = 通道上报状态/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChannelState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->ChannelState,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pOrder_Charge->ChannelState);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////总故障///////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"TotalFault"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->TotalFault,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pOrder_Charge->TotalFault);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
		rt_lprintf("[storage]:%s文件写成功\n",path_file);		
/////////各项故障状态/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Fau"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOrder_Charge->Fau,sizeof(pOrder_Charge->Fau));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pOrder_Charge->Fau);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////	
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:%s文件无效命令\n",path_file);
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Plan_Offer_Storage
** Descriptions:		
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Plan_Offer_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffer[48]; 
	char path_file[64];
	PLAN_OFFER_EVENT* pPlan_Offer = (PLAN_OFFER_EVENT*)Storage_Para;
	
	if(cmd == WRITE)//保存到本地write
	{
		// 创建文件名
		int writelen = 0;
		char path[18];
		sprintf((char*)path_file,"%s",PATH);
		sprintf((char*)path,"/%04u-%02X%02X%02X%02X%02X%02X.txt",(rt_uint32_t)pPlan_Offer->OrderNum,\
																			  pPlan_Offer->StartTimestamp.Second,\
																			  pPlan_Offer->StartTimestamp.Minute,\
																			  pPlan_Offer->StartTimestamp.Hour,\
																			  pPlan_Offer->StartTimestamp.Day,\
																			  pPlan_Offer->StartTimestamp.Month,\
																			  pPlan_Offer->StartTimestamp.Year);
		strcat((char*)path_file,(const char*)path);
		rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);		
		
		// 准备写的内容
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"OrderNum=%04u\n",(rt_uint32_t)pPlan_Offer->OrderNum);//	事件记录序号
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"StartTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Offer->StartTimestamp.Second,\
																		  (rt_uint32_t)pPlan_Offer->StartTimestamp.Minute,\
																		  (rt_uint32_t)pPlan_Offer->StartTimestamp.Hour,\
																		  (rt_uint32_t)pPlan_Offer->StartTimestamp.Day,\
																		  (rt_uint32_t)pPlan_Offer->StartTimestamp.Month,\
																		  (rt_uint32_t)pPlan_Offer->StartTimestamp.Year);//  事件发生时间
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		sprintf((char*)buffer,"FinishTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Offer->FinishTimestamp.Second,\
																		   (rt_uint32_t)pPlan_Offer->FinishTimestamp.Minute,\
																		   (rt_uint32_t)pPlan_Offer->FinishTimestamp.Hour,\
																		   (rt_uint32_t)pPlan_Offer->FinishTimestamp.Day,\
																		   (rt_uint32_t)pPlan_Offer->FinishTimestamp.Month,\
																		   (rt_uint32_t)pPlan_Offer->FinishTimestamp.Year);//  事件结束时间
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// 事件发生原因
		sprintf((char*)buffer,"Reason=%03u\n",(rt_uint32_t)pPlan_Offer->Reason); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// 事件上报状态 = 通道上报状态
		sprintf((char*)buffer,"ChannelState=%03u\n",(rt_uint32_t)pPlan_Offer->ChannelState);
		strcat((char*)Para_Buff,(const char*)buffer);

		// 充电申请单号（SIZE(16)）
		sprintf((char*)buffer,"RequestNO=");
		char bytebuf[] = "FF";
		for(int i=0;i<sizeof(pPlan_Offer->RequestNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->RequestNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
		
		// 用户id  visible-string（SIZE(64)）
		sprintf((char*)buffer,"cUserID=");
		for(int i=0;i<sizeof(pPlan_Offer->cUserID);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->cUserID[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
	
		//	路由器资产编号 visible-string（SIZE(22)）
		sprintf((char*)buffer,"AssetNO=");
		for(int i=0;i<sizeof(pPlan_Offer->AssetNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->AssetNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");	
	
		//	充电需求电量（单位：kWh，换算：-2）
		sprintf((char*)buffer,"ChargeReqEle=%08u\n",(rt_uint32_t)pPlan_Offer->ChargeReqEle);
		strcat((char*)Para_Buff,(const char*)buffer);	
		
		sprintf((char*)buffer,"RequestTimeStamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Offer->RequestTimeStamp.Second,\
							   										        (rt_uint32_t)pPlan_Offer->RequestTimeStamp.Minute,\
																		    (rt_uint32_t)pPlan_Offer->RequestTimeStamp.Hour,\
																		    (rt_uint32_t)pPlan_Offer->RequestTimeStamp.Day,\
																		    (rt_uint32_t)pPlan_Offer->RequestTimeStamp.Month,\
																		    (rt_uint32_t)pPlan_Offer->RequestTimeStamp.Year);//	充电申请时间
		strcat((char*)Para_Buff,(const char*)buffer);	
	
		sprintf((char*)buffer,"PlanUnChg_TimeStamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Second,\
																		       (rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Minute,\
																		       (rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Hour,\
																		       (rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Day,\
																		       (rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Month,\
																		       (rt_uint32_t)pPlan_Offer->PlanUnChg_TimeStamp.Year);//	计划用车时间
		strcat((char*)Para_Buff,(const char*)buffer);		

		//	充电模式 {正常（0），有序（1）}
		sprintf((char*)buffer,"ChargeMode=%08u\n",(rt_uint32_t)pPlan_Offer->ChargeMode);
		strcat((char*)Para_Buff,(const char*)buffer);
	
		//	用户登录令牌  visible-string（SIZE(38)）
		sprintf((char*)buffer,"Token=");
		for(int i=0;i<sizeof(pPlan_Offer->Token);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->Token[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");		
		//	充电用户账号  visible-string（SIZE(9)）
		sprintf((char*)buffer,"UserAccount=");
		for(int i=0;i<sizeof(pPlan_Offer->UserAccount);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->UserAccount[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
		//	第n个关联对象属性的数据	
		sprintf((char*)buffer,"Data=");
		for(int i=0;i<sizeof(pPlan_Offer->Data);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Offer->Data[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
		/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}		
		/************************************************************************************/				
		/**********更新写当天电量************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//写入首部   返回值0：成功	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s文件写入成功 writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s文件写入失败 writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //从本地read
	{
		int readlen = 0;
		rt_uint32_t file_num;	
		Find_path_file(PATH,ordernum,path_file,&file_num);

		fd= open(path_file,O_RDONLY);//打开文件。如果文件不存在，则打开失败。
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/			
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//读出txt里面的内容	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s文件读取成功 readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
			}
			rt_lprintf("[storage]:%s文件读取为空 readlen=%d\n",path_file,readlen);
			return -3;
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}
		/***************************************************************************************/
		*(Para_Buff+readlen) = '\0';//需要追加结束符
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:%s文件读内容\n%s\n",path_file,Para_Buff);
		}		
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = Para_Buff;//参数
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0,};        //最多记录32个字节
		rt_uint8_t fpnameRd[32] = {0,};      //最多记录32个字节	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////事件记录序号/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OrderNum"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->OrderNum,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Offer->OrderNum);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////事件发生时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"StartTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->StartTimestamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Offer->StartTimestamp.Year,\
							pPlan_Offer->StartTimestamp.Month,\
							pPlan_Offer->StartTimestamp.Day,\
							pPlan_Offer->StartTimestamp.Hour,\
							pPlan_Offer->StartTimestamp.Minute,\
							pPlan_Offer->StartTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////事件结束时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"FinishTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->FinishTimestamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Offer->FinishTimestamp.Year,\
							pPlan_Offer->FinishTimestamp.Month,\
							pPlan_Offer->FinishTimestamp.Day,\
							pPlan_Offer->FinishTimestamp.Hour,\
							pPlan_Offer->FinishTimestamp.Minute,\
							pPlan_Offer->FinishTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////事件发生原因/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Reason"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->Reason,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Offer->Reason);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////事件上报状态 = 通道上报状态/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChannelState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->ChannelState,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Offer->ChannelState);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////充电申请单号 SIZE(16)/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"RequestNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->RequestNO,sizeof(pPlan_Offer->RequestNO));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->RequestNO);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////用户id  visible-string（SIZE(64)）/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"cUserID"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->cUserID,sizeof(pPlan_Offer->cUserID));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->cUserID);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////路由器资产编号 visible-string（SIZE(22)）////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"AssetNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->AssetNO,sizeof(pPlan_Offer->AssetNO));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->AssetNO);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}	
/////////充电需求电量（单位：kWh，换算：-2）/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChargeReqEle"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->ChargeReqEle,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Offer->ChargeReqEle);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////充电申请时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"RequestTimeStamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->RequestTimeStamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Offer->RequestTimeStamp.Year,\
							pPlan_Offer->RequestTimeStamp.Month,\
							pPlan_Offer->RequestTimeStamp.Day,\
							pPlan_Offer->RequestTimeStamp.Hour,\
							pPlan_Offer->RequestTimeStamp.Minute,\
							pPlan_Offer->RequestTimeStamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////计划用车时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"PlanUnChg_TimeStamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->PlanUnChg_TimeStamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Offer->PlanUnChg_TimeStamp.Year,\
							pPlan_Offer->PlanUnChg_TimeStamp.Month,\
							pPlan_Offer->PlanUnChg_TimeStamp.Day,\
							pPlan_Offer->PlanUnChg_TimeStamp.Hour,\
							pPlan_Offer->PlanUnChg_TimeStamp.Minute,\
							pPlan_Offer->PlanUnChg_TimeStamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
/////////充电模式 {正常（0），有序（1）}/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChargeMode"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->ChargeMode,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Offer->ChargeMode);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}		
/////////用户登录令牌  visible-string（SIZE(38)）////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Token"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->Token,sizeof(pPlan_Offer->Token));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->Token);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}		
/////////充电用户账号  visible-string（SIZE(9)）/////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"UserAccount"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->UserAccount,sizeof(pPlan_Offer->UserAccount));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->UserAccount);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}		
/////////第n个关联对象属性的数据	///////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Data"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Offer->Data,sizeof(pPlan_Offer->Data));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Offer->Data);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////	
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:%s文件无效命令\n",path_file);
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Plan_Fail_Storage
** Descriptions:		
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Plan_Fail_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffer[48]; 
	char path_file[64];
	PLAN_FAIL_EVENT* pPlan_Fail = (PLAN_FAIL_EVENT*)Storage_Para;
	
	if(cmd == WRITE)//保存到本地write
	{
		// 创建文件名
		int writelen = 0;
		char path[18];
		sprintf((char*)path_file,"%s",PATH);
		sprintf((char*)path,"/%04u-%02X%02X%02X%02X%02X%02X.txt",(rt_uint32_t)pPlan_Fail->OrderNum,\
																			  pPlan_Fail->StartTimestamp.Second,\
																			  pPlan_Fail->StartTimestamp.Minute,\
																			  pPlan_Fail->StartTimestamp.Hour,\
																			  pPlan_Fail->StartTimestamp.Day,\
																			  pPlan_Fail->StartTimestamp.Month,\
																			  pPlan_Fail->StartTimestamp.Year);
		strcat((char*)path_file,(const char*)path);
		rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);		
		
		// 准备写的内容
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"OrderNum=%04u\n",(rt_uint32_t)pPlan_Fail->OrderNum);//	事件记录序号
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"StartTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Fail->StartTimestamp.Second,\
																		  (rt_uint32_t)pPlan_Fail->StartTimestamp.Minute,\
																		  (rt_uint32_t)pPlan_Fail->StartTimestamp.Hour,\
																		  (rt_uint32_t)pPlan_Fail->StartTimestamp.Day,\
																		  (rt_uint32_t)pPlan_Fail->StartTimestamp.Month,\
																		  (rt_uint32_t)pPlan_Fail->StartTimestamp.Year);//  事件发生时间
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		sprintf((char*)buffer,"FinishTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pPlan_Fail->FinishTimestamp.Second,\
																		   (rt_uint32_t)pPlan_Fail->FinishTimestamp.Minute,\
																		   (rt_uint32_t)pPlan_Fail->FinishTimestamp.Hour,\
																		   (rt_uint32_t)pPlan_Fail->FinishTimestamp.Day,\
																		   (rt_uint32_t)pPlan_Fail->FinishTimestamp.Month,\
																		   (rt_uint32_t)pPlan_Fail->FinishTimestamp.Year);//  事件结束时间
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// 事件发生原因
		sprintf((char*)buffer,"Reason=%03u\n",(rt_uint32_t)pPlan_Fail->Reason); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// 事件上报状态 = 通道上报状态
		sprintf((char*)buffer,"ChannelState=%03u\n",(rt_uint32_t)pPlan_Fail->ChannelState);
		strcat((char*)Para_Buff,(const char*)buffer);

		// 充电申请单号（SIZE(16)）
		sprintf((char*)buffer,"RequestNO=");
		char bytebuf[] = "FF";
		for(int i=0;i<sizeof(pPlan_Fail->RequestNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Fail->RequestNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
	
		//	路由器资产编号 visible-string（SIZE(22)）
		sprintf((char*)buffer,"AssetNO=");
		for(int i=0;i<sizeof(pPlan_Fail->AssetNO);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Fail->AssetNO[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");	

		//	第n个关联对象属性的数据	
		sprintf((char*)buffer,"Data=");
		for(int i=0;i<sizeof(pPlan_Fail->Data);i++)
		{
			sprintf((char*)bytebuf,"%02X",(rt_uint32_t)pPlan_Fail->Data[i]);
			strcat((char*)buffer,(const char*)bytebuf);
		}
		strcat((char*)buffer,(const char*)"\n");
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
		/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}		
		/************************************************************************************/				
		/**********更新写当天电量************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//写入首部   返回值0：成功	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s文件写入成功 writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s文件写入失败 writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //从本地read
	{
		int readlen = 0;
		rt_uint32_t file_num;	
		Find_path_file(PATH,ordernum,path_file,&file_num);

		fd= open(path_file,O_RDONLY);//打开文件。如果文件不存在，则打开失败。
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/			
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//读出txt里面的内容	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s文件读取成功 readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
			}
			rt_lprintf("[storage]:%s文件读取为空 readlen=%d\n",path_file,readlen);
			return -3;
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}
		/***************************************************************************************/
		*(Para_Buff+readlen) = '\0';//需要追加结束符
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:%s文件读内容\n%s\n",path_file,Para_Buff);
		}		
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = Para_Buff;//参数
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0,};        //最多记录32个字节
		rt_uint8_t fpnameRd[32] = {0,};      //最多记录32个字节	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////事件记录序号/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OrderNum"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->OrderNum,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Fail->OrderNum);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////事件发生时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"StartTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->StartTimestamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Fail->StartTimestamp.Year,\
							pPlan_Fail->StartTimestamp.Month,\
							pPlan_Fail->StartTimestamp.Day,\
							pPlan_Fail->StartTimestamp.Hour,\
							pPlan_Fail->StartTimestamp.Minute,\
							pPlan_Fail->StartTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////事件结束时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"FinishTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->FinishTimestamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pPlan_Fail->FinishTimestamp.Year,\
							pPlan_Fail->FinishTimestamp.Month,\
							pPlan_Fail->FinishTimestamp.Day,\
							pPlan_Fail->FinishTimestamp.Hour,\
							pPlan_Fail->FinishTimestamp.Minute,\
							pPlan_Fail->FinishTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
////////////////////事件发生原因/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Reason"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->Reason,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Fail->Reason);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////事件上报状态 = 通道上报状态/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChannelState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->ChannelState,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pPlan_Fail->ChannelState);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////充电申请单号 SIZE(16)/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"RequestNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->RequestNO,sizeof(pPlan_Fail->RequestNO));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Fail->RequestNO);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////路由器资产编号 visible-string（SIZE(22)）////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"AssetNO"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->AssetNO,sizeof(pPlan_Fail->AssetNO));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Fail->AssetNO);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////第n个关联对象属性的数据	///////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"Data"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pPlan_Fail->Data,sizeof(pPlan_Fail->Data));//返回当前文件读指针
				rt_lprintf("%s=%s;\n",fpnameRd,pPlan_Fail->Data);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////	
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:%s文件无效命令\n",path_file);
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Online_State_Storage
** Descriptions:		
** input parameters:	YMD:20190731 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
static int Online_State_Storage(const char *PATH,void *Storage_Para,rt_uint32_t ordernum,rt_uint32_t cmd)	
{
	int fd = 0;
	char buffer[48]; 
	char path_file[64];
	ONLINE_STATE* pOnline_State = (ONLINE_STATE*)Storage_Para;	
	
	if(cmd == WRITE)//保存到本地write
	{
		// 创建文件名
		int writelen = 0;
		char path[18];
		sprintf((char*)path_file,"%s",PATH);
		sprintf((char*)path,"/%04u-%02X%02X%02X%02X%02X%02X.txt",(rt_uint32_t)pOnline_State->OrderNum,\
																			  pOnline_State->OnlineTimestamp.Second,\
																			  pOnline_State->OnlineTimestamp.Minute,\
																			  pOnline_State->OnlineTimestamp.Hour,\
																			  pOnline_State->OnlineTimestamp.Day,\
																			  pOnline_State->OnlineTimestamp.Month,\
																			  pOnline_State->OnlineTimestamp.Year);
		strcat((char*)path_file,(const char*)path);
		rt_lprintf("%s:path_file = %s\n",__FUNCTION__,(char*)path_file);		
		
		// 准备写的内容
		strcpy((char*)Para_Buff,"");
		sprintf((char*)buffer,"Time=%02X-%02X-%02X-%02X-%02X-%02X\n",System_Time_STR.Year,System_Time_STR.Month,System_Time_STR.Day,\
																	 System_Time_STR.Hour,System_Time_STR.Minute,System_Time_STR.Second); 
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"OrderNum=%04u\n",(rt_uint32_t)pOnline_State->OrderNum);//	事件记录序号
		strcat((char*)Para_Buff,(const char*)buffer);
		
		sprintf((char*)buffer,"StartTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pOnline_State->OfflineTimestamp.Second,\
																		  (rt_uint32_t)pOnline_State->OfflineTimestamp.Minute,\
																		  (rt_uint32_t)pOnline_State->OfflineTimestamp.Hour,\
																		  (rt_uint32_t)pOnline_State->OfflineTimestamp.Day,\
																		  (rt_uint32_t)pOnline_State->OfflineTimestamp.Month,\
																		  (rt_uint32_t)pOnline_State->OfflineTimestamp.Year);//  上线时间
		strcat((char*)Para_Buff,(const char*)buffer);		
		
		sprintf((char*)buffer,"FinishTimestamp=%02X%02X%02X%02X%02X%02X\n",(rt_uint32_t)pOnline_State->OfflineTimestamp.Second,\
																		   (rt_uint32_t)pOnline_State->OfflineTimestamp.Minute,\
																		   (rt_uint32_t)pOnline_State->OfflineTimestamp.Hour,\
																		   (rt_uint32_t)pOnline_State->OfflineTimestamp.Day,\
																		   (rt_uint32_t)pOnline_State->OfflineTimestamp.Month,\
																		   (rt_uint32_t)pOnline_State->OfflineTimestamp.Year);//  离线时间
		strcat((char*)Para_Buff,(const char*)buffer);
		
		// 事件上报状态 = 通道上报状态
		sprintf((char*)buffer,"ChannelState=%03u\n",(rt_uint32_t)pOnline_State->ChannelState);
		strcat((char*)Para_Buff,(const char*)buffer);
		
		//状态变化 {上线（0）， 离线（1）}
		sprintf((char*)buffer,"AutualState=%03u\n",(rt_uint32_t)pOnline_State->AutualState);
		strcat((char*)Para_Buff,(const char*)buffer);
		
		//离线原因 {未知（0），停电（1），信道变化（2）}
		sprintf((char*)buffer,"OfflineReason=%03u\n",(rt_uint32_t)pOnline_State->OfflineReason);
		strcat((char*)Para_Buff,(const char*)buffer);
		/****************************************************************************************/
		if(strlen((const char*)Para_Buff)> MAX_MALLOC_NUM)
		{
			rt_lprintf("[storage]: Para_Buff overflow=%d\n",strlen((const char*)Para_Buff));
			
			return -1;
		}
		else
		{
			rt_lprintf("[storage]:strlen(Para_Buff)=%d\n",strlen(Para_Buff));
		}	
		/************************************************************************************************/			
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */
		int fd = open(path_file,O_WRONLY | O_CREAT);
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}		
		/************************************************************************************/				
		/**********更新写当天电量************************************************************/				
		writelen = write(fd,Para_Buff,strlen((const char*)Para_Buff));//写入首部   返回值0：成功	
		if(writelen > 0)
		{
			rt_lprintf("[storage]:%s文件写入成功 writelen=%d\n",(char*)path_file,writelen);
		}
		else
		{
			rt_lprintf("[storage]:%s文件写入失败 writelen=%d\n",(char*)path_file,writelen);
		}
		/*****************************************************************************************/
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}			
	}
/************************************************************************************************/
/************************************************************************************************/
/************************************************************************************************/
	else if(cmd == READ) //从本地read
	{
		int readlen = 0;
		rt_uint32_t file_num;	
		Find_path_file(PATH,ordernum,path_file,&file_num);

		fd= open(path_file,O_RDONLY);//打开文件。如果文件不存在，则打开失败。
		if(fd >= 0)
		{
			rt_lprintf("[storage]:%s文件打开成功\n",path_file);
		}
		else
		{
			rt_lprintf("[storage]:%s文件打开失败 fd=%d\n",path_file,fd);
			return -2;
		}
        /***************************************************************************************/			
		readlen=read(fd,Para_Buff,MAX_MALLOC_NUM);	//读出txt里面的内容	
		if(readlen > 0) //0
		{
			rt_lprintf("[storage]:%s文件读取成功 readlen=%d\n",path_file,readlen);
		}
		else
		{
			if(close(fd) != UENOERR)
			{
				rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
			}
			rt_lprintf("[storage]:%s文件读取为空 readlen=%d\n",path_file,readlen);
			return -3;
		}
		/***************************************************************************************/	
		if(close(fd) != UENOERR)
		{
			rt_lprintf("[storage]:%s文件关闭失败\n",path_file);
		}
		/***************************************************************************************/
		*(Para_Buff+readlen) = '\0';//需要追加结束符
		if(readlen <= RT_CONSOLEBUF_SIZE)
		{
			rt_lprintf("[storage]:%s文件读内容\n%s\n",path_file,Para_Buff);
		}		
		/************************************************************************************/			
		/************************************************************************************/		
		char *fpoint = Para_Buff;//参数
		rt_uint8_t namelen = 0;
		rt_uint8_t fpname[32] = {0,};        //最多记录32个字节
		rt_uint8_t fpnameRd[32] = {0,};      //最多记录32个字节	
		strcpy((char*)fpnameRd,"");
		strcpy((char*)fpname,"");
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////事件记录序号/////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OrderNum"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->OrderNum,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pOnline_State->OrderNum);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////////////////事件发生时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"OnlineTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->OnlineTimestamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pOnline_State->OnlineTimestamp.Year,\
							pOnline_State->OnlineTimestamp.Month,\
							pOnline_State->OnlineTimestamp.Day,\
							pOnline_State->OnlineTimestamp.Hour,\
							pOnline_State->OnlineTimestamp.Minute,\
							pOnline_State->OnlineTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}		
/////////////////////////////////////////////////////////////////////////////////////////		
////////////////////事件结束时间//////////////////////////////////////////////////////////
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			sprintf((char*)fpnameRd,"OfflineTimestamp"); 
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->OfflineTimestamp.Second,6);//返回当前文件读指针
				rt_lprintf("%s=%02X%02X%02X%02X%02X%02X;\n",fpname,\
							pOnline_State->OfflineTimestamp.Year,\
							pOnline_State->OfflineTimestamp.Month,\
							pOnline_State->OfflineTimestamp.Day,\
							pOnline_State->OfflineTimestamp.Hour,\
							pOnline_State->OfflineTimestamp.Minute,\
							pOnline_State->OfflineTimestamp.Second);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname);
			}
			namelen=0;							
		}
		else
		{
			rt_lprintf("namelen=0\n");			
		}
/////////事件上报状态 = 通道上报状态/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"ChannelState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->ChannelState,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pOnline_State->ChannelState);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////状态变化 {上线（0）， 离线（1）}/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"AutualState"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->AutualState,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pOnline_State->AutualState);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
////////离线原因 {未知（0），停电（1），信道变化（2）}/////////////////////////////////////////////////////////////////////////////////
		sprintf((char*)fpnameRd,"OfflineReason"); 
		fpoint = get_name(fpoint,fpname,&namelen);//返回当前文件读指针
		if(namelen)//接收完了
		{
			if(strcmp((const char*)fpname,(const char*)fpnameRd)==0)
			{
				fpoint = get_pvalue(fpoint,(rt_uint32_t*)&pOnline_State->OfflineReason,1);//返回当前文件读指针
				rt_lprintf("%s=%u;\n",fpnameRd,pOnline_State->OfflineReason);
			}
			else
			{
				rt_lprintf("%s变量名不符合fpname=%s\n",fpnameRd,fpname); 
			}
			namelen=0;
		}
		else
		{
			rt_lprintf("namelen=0\n");
		}
/////////////////////////////////////////////////////////////////////////////////////////	
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////			
	}
	else
	{
		rt_lprintf("[storage]:%s文件无效命令\n",path_file);
	}

	return 0;	
}
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG函数
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
//从str中得到变量名
static char* get_name(char* strtemp,rt_uint8_t*fpname,rt_uint8_t *fpcnt)
{
	while(*strtemp!='\0')//没有结束 文件第一行为标题 第二行为变量开始
	{
		if((*strtemp=='\r')||(*strtemp=='\n'))//最多记录50个字符
		{	
			break;
		}
		strtemp++; 
		
	}
	while(*strtemp!='\0')//没有结束
	{
		if((*strtemp!=' ')&&(*strtemp!='=')&&(*strtemp!='\r')&&(*strtemp!='\n')&&\
			(*strtemp!='/')&&(*fpcnt)<50)//最多记录50个字符
		{	
			fpname[(*fpcnt)++]=*strtemp;//fpcnt必须加括号，++优先级高于*
			strtemp++; 
		}
		else if(*fpcnt) break;//存储之后第一次打断退出
		else strtemp++; 
		
	}
	if(*fpcnt)//接收完了
	{
		fpname[*fpcnt]='\0';//加入结束符
	}
	
  return strtemp; 	
	
}
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG函数
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
//从str中得到变量值   最大位数16位
static char* get_pvalue(char* strtemp,rt_uint32_t*fpvalue,rt_uint8_t cnt)
{
	rt_uint16_t i,j = 0;
	rt_uint8_t valueArry[48] = {0};
	rt_uint8_t arryadd[72] = {0};
	rt_uint8_t Arrycnt = 0;
	
	while(*strtemp!='\0')//没有结束
	{	
		if(((*strtemp!=' ')&&(*strtemp!='=')&&(*strtemp!=';')&& \
			(*strtemp<='9')&&(*strtemp>='0')&&(Arrycnt<48))||((*strtemp>='A')&&(*strtemp<='F')))//最多记录48个字符
		{			
			valueArry[Arrycnt++]=*strtemp;//
			strtemp++; 
		}
		else if(Arrycnt) break;//存储之后第一次打断退出
		else strtemp++; 
	}
	if(Arrycnt)//接收完了
	{
		valueArry[Arrycnt++]='\0';//加入结束符
	} 
    if(cnt == 1) //一个变量值
	{		
	    str2num(valueArry,fpvalue);
	}
	else  //多个变量值
	{
		for(i=0,j=0;i<cnt*2; )  //在cnt个变量之间添加分解符 '\0'
		{
		    arryadd[j++] = valueArry[i++];
			arryadd[j++] = valueArry[i++];
			arryadd[j++] = '\0';
		}		

		for(i=0;i<cnt;i++)
		{
		    str2nnum(arryadd+3*i,((rt_uint8_t*)fpvalue)+i);//16进制+空格=3字符
		}
		
	}
	
  return strtemp; 	
}
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG函数
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
int storage_thread_init(void)
{
//	extern void nerase_all(void);
//	nerase_all();//格式化nand文件系统
	
	if(dfs_mount("nand0", "/", "uffs", 0, 0) == RT_EOK)
	{	
		rt_lprintf("[storage]:nand flash mount to / success\n");
	}
	else
	{
		rt_lprintf("[storage]:nand flash mount to / fail\n");
	}
	
	for(rt_uint8_t i = 0; i< MAX_DIR_NUM; i++)
	{
		int result = mkdir(_dir_name[i],0x777);//创建_dir_name包含的文件目录
		if(result == UENOERR)
		{
			rt_lprintf("[storage]:make %s dir ok\n",_dir_name[i]);
		}
		else if(result == (-EEXIST))
		{
			rt_lprintf("[storage]:%s dir is already exist\n",_dir_name[i]);
		}
		else
		{
			rt_lprintf("[storage]:make %s dir fail,error num is %d\n",_dir_name[i],result);
		}
	}
/**********************************************************************************************/	
/**********************************************************************************************/
/**********************************************************************************************/
		/*O_CREAT: Opens the file, if it is existing. If not, a new file is created. */
		/*O_TRUNC: Creates a new file. If the file is existing, it is truncated and overwritten. */
		/*O_EXCL: Creates a new file. The function fails if the file is already existing. */

//		int fd = unlink(METER_GJFMode_PATH_FILE);
//		if(fd >= UENOERR)
//		{
//			rt_lprintf("%s文件删除成功 fd=%d\n",METER_GJFMode_PATH_FILE,fd);
//		}
//		else
//		{
//			rt_lprintf("%s删除文件不存在 fd=%d\n",METER_GJFMode_PATH_FILE,fd);
//		}

//		int fd = rmdir("/LOG");//必须为空目录
//		if(fd >= UENOERR)
//		{
//			rt_lprintf("%s目录删除成功 fd=%d\n","/LOG",fd);
//		}
//		else
//		{
//			rt_lprintf("%s删除目录不存在 fd=%d\n","/LOG",fd);
//		}	
	
//		int fd = open(NAND_LOG_PATH_FILE,O_CREAT);//此处创建文件必须用 O_CREAT
//		if(fd >= 0)
//		{
//			rt_lprintf("%s文件创建成功 fd=%d\n",NAND_LOG_PATH_FILE,fd);
//		}
//		else
//		{
//			rt_lprintf("%s文件创建失败 fd=%d\n",NAND_LOG_PATH_FILE,fd);
//		}

//		
//		if(close(fd) != UENOERR)
//		{
//			rt_lprintf("[storage]:%s文件关闭失败\n",NAND_LOG_PATH_FILE);
//		}
//		else
//		{
//			rt_lprintf("[storage]:%s文件关闭成功\n",NAND_LOG_PATH_FILE);
//		}	
/**********************************************************************************************/	
/**********************************************************************************************/
/**********************************************************************************************/	
	memcpy(RouterIfo.AssetNum, "0011223344000000000011", sizeof(RouterIfo.AssetNum));// 表号
    rt_lprintf("[storage]:电表资产号：%s\n",RouterIfo.AssetNum);	
/**********************************************************************************************/	
/**********************************************************************************************/
/**********************************************************************************************/	
	/* 创建互斥锁 */
	storage_ReWr_mutex = rt_mutex_create("storage_ReWr_mutex", RT_IPC_FLAG_FIFO);
	if (storage_ReWr_mutex == RT_NULL)
	{
		rt_lprintf("[storage]:storage_ReWr_mutex创建互斥锁失败\n");
		return 0;
	}	
/**********************************************************************************************/	
/**********************************************************************************************/
/**********************************************************************************************/	
	rt_err_t res = rt_thread_init(&storage,
											"storage",
											storage_thread_entry,
											RT_NULL,
											storage_stack,
											THREAD_STORAGE_STACK_SIZE,
											THREAD_STORAGE_PRIORITY,
											THREAD_STORAGE_TIMESLICE);
	if (res == RT_EOK) 
	{
			rt_thread_startup(&storage);
	}
	return res;
}


#if defined (RT_STORAGE_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(storage_thread_init);
#endif
MSH_CMD_EXPORT(storage_thread_init, storage thread run);
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG函数
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
int GetStorageData(STORAGE_CMD_ENUM cmd,void *Storage_Para,rt_uint32_t datalen)
{
	if(Para_Buff == NULL)
	{
	    Para_Buff = rt_malloc(MAX_MALLOC_NUM);//申请内存
		if(Para_Buff == NULL)
		{
			rt_lprintf("[storage]:Para_Buff 分配内存失败\n");
			return 1;
		}
		rt_lprintf("[storage]:Para_Buff rt_mallocOK\n");
	}
	rt_err_t ret = 0;	
	rt_err_t result = rt_mutex_take(storage_ReWr_mutex, RT_WAITING_FOREVER);
	
	switch(cmd)
	{
		case Cmd_MeterNumRd://
			ret = Router_Para_Storage(ROUTER_PARA_PATH_FILE,Storage_Para,datalen,READ);
			// 表号		
			*(char*)Storage_Para = ((*((char*)(RouterIfo.AssetNum)+10)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 11)-'0';//
			*(((char*)Storage_Para)+1) = ((*((char*)(RouterIfo.AssetNum)+12)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 13)-'0';//
			*(((char*)Storage_Para)+2) = ((*((char*)(RouterIfo.AssetNum)+14)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 15)-'0';//
			*(((char*)Storage_Para)+3) = ((*((char*)(RouterIfo.AssetNum)+16)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 17)-'0';//
			*(((char*)Storage_Para)+4) = ((*((char*)(RouterIfo.AssetNum)+18)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 19)-'0';//
			*(((char*)Storage_Para)+5) = ((*((char*)(RouterIfo.AssetNum)+20)-'0')<<4) + *((char*)(RouterIfo.AssetNum) + 21)-'0';//

			break;
		case Cmd_MeterPowerRd://
			ret = Meter_Power_Storage(ROUTER_PARA_PATH_FILE,Storage_Para,datalen,READ);

			break;
		case Cmd_MeterGJFModeRd://
			ret = Meter_GJFMode_Storage(METER_GJFMode_PATH_FILE,Storage_Para,datalen,READ);	
			
			break;			
		case Cmd_MeterHalfPowerRd://
			ret = Meter_HalfPower_Storage(METER_HALF_POWER_PATH_FILE,Storage_Para,datalen,READ);	
			
			break;
		case Cmd_MeterAnalogRd://
			ret = Meter_Anolag_Storage(METER_ANALOG_PATH_FILE,Storage_Para,datalen,READ);	
			
			break;
		case Cmd_HistoryRecordRd://
			ret = Charge_Record_Storage(HISTORY_RECORD_PATH,Storage_Para,datalen,READ);	
			
			break;
		case Cmd_OrderChargeRd:/*有序充电事件记录单元*/
			ret = Order_Charge_Storage(ORDER_CHARGE_PATH,Storage_Para,datalen,READ);	
			
			break;	
		case Cmd_PlanOfferRd:/*充电计划上报记录单元*/
			ret = Plan_Offer_Storage(PLAN_OFFER_PATH,Storage_Para,datalen,READ);	
			
			break;
		case Cmd_PlanFailRd:/*充电计划生成失败记录单元*/
			ret = Plan_Fail_Storage(PLAN_FAIL_PATH,Storage_Para,datalen,READ);	
			
			break;
		case Cmd_OnlineStateRd:/*表计在线状态*/
			ret = Online_State_Storage(ONLINE_STATE_PATH,Storage_Para,datalen,READ);	
			
			break;
		default:
			rt_lprintf("[storage]:Waring：%s收到未定义指令%u\r\n",__FUNCTION__,cmd);
		    ret = 1;
			break;
	}	
	
	rt_free(Para_Buff);Para_Buff = NULL;
	
	if (result == RT_EOK)
	{
		/* 释放互斥锁 */
		rt_mutex_release(storage_ReWr_mutex);
	}
	else
	{
		rt_lprintf("[storage]:storage_ReWr_mutex 释放互斥锁失败\n");
	}
	
	return ret;
}
int SetStorageData(STORAGE_CMD_ENUM cmd,void *Storage_Para,rt_uint32_t datalen)
{
	if(Para_Buff == NULL)
	{
	  Para_Buff = rt_malloc(MAX_MALLOC_NUM);//申请内存
		if(Para_Buff == NULL)
		{
			rt_lprintf("[storage]:Para_Buff 分配内存失败\n");
			return 1;
		}
		rt_lprintf("[storage]:Para_Buff rt_mallocOK\n");
	}
	rt_err_t ret = 0;	
	rt_err_t result = rt_mutex_take(storage_ReWr_mutex, RT_WAITING_FOREVER);

	
	switch(cmd)
	{
		case Cmd_MeterNumWr://
			ret = Router_Para_Storage(ROUTER_PARA_PATH_FILE,Storage_Para,datalen,WRITE);	
			
			break;	

		case Cmd_MeterPowerWr://
			ret = Meter_Power_Storage(ROUTER_PARA_PATH_FILE,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_MeterGJFModeWr://
			ret = Meter_GJFMode_Storage(METER_GJFMode_PATH_FILE,Storage_Para,datalen,WRITE);	
			
			break;		
		case Cmd_MeterHalfPowerWr://
			ret = Meter_HalfPower_Storage(METER_HALF_POWER_PATH_FILE,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_MeterAnalogWr://
			ret = Meter_Anolag_Storage(METER_ANALOG_PATH_FILE,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_HistoryRecordWr://
			ret = Charge_Record_Storage(HISTORY_RECORD_PATH,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_OrderChargeWr: /*有序充电事件记录单元*/
			ret = Order_Charge_Storage(ORDER_CHARGE_PATH,Storage_Para,datalen,WRITE);	
			
			break;	
		case Cmd_PlanOfferWr:/*充电计划上报记录单元*/
			ret = Plan_Offer_Storage(PLAN_OFFER_PATH,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_PlanFailWr:/*充电计划生成失败记录单元*/
			ret = Plan_Fail_Storage(PLAN_FAIL_PATH,Storage_Para,datalen,WRITE);	
			
			break;
		case Cmd_OnlineStateWr:/*表计在线状态*/
			ret = Online_State_Storage(ONLINE_STATE_PATH,Storage_Para,datalen,WRITE);	
			
			break;
		default:
			rt_lprintf("[storage]:Waring：%s收到未定义指令%u\r\n",__FUNCTION__,cmd);
		    ret = 1;
			break;	
	}
	
	rt_free(Para_Buff);Para_Buff = NULL;
	
	if (result == RT_EOK)
	{
		/* 释放互斥锁 */
		rt_mutex_release(storage_ReWr_mutex);
	}
	else
	{
		rt_lprintf("[storage]:storage_ReWr_mutex 释放互斥锁失败\n");
	}	
	
	return ret;
}
/**************************************************************
 * 函数名称: rt_uint8_t str2num(rt_uint8_t*src,rt_uint32_t *dest)
 * 参    数: 无
 * 返 回 值: 无
 * 描    述: 把字符串转为数字
 **************************************************************/	
//支持16进制转换,格式为以0X或者0x开头的.
//不支持负数 
//*src:数字字符串指针
//*dest:转换完的结果存放地址.
//返回值:0，成功转换完成.其他,错误代码.
//1,数据格式错误.2,16进制位数为0.3,起始格式错误.4,十进制位数为0.
static rt_uint8_t str2num(rt_uint8_t*src,rt_uint32_t *dest)
{
	rt_uint32_t t;
	rt_uint8_t bnum=0;	//数字的位数
	rt_uint8_t *p;		  
	rt_uint8_t hexdec=10;//默认为十进制数据
	p=src;
	*dest=0;//清零.
	while(1)
	{
		if((*p<='9'&&*p>='0')||(*p<='F'&&*p>='A')||(*p=='X'&&bnum==1))//参数合法
		{
			if(*p>='A') hexdec=16;	//字符串中存在字母,为16进制格式.
			bnum++;					//位数增加.
		}
		else if(*p=='\0')
		    break;	                //碰到结束符,退出.
		else 
			return 1;				//不全是十进制或者16进制数据.
		p++; 
	} 
	p=src;			    //重新定位到字符串开始的地址.
	if(hexdec==16)		//16进制数据
	{
		if(bnum<3) return 2;			//位数小于3，直接退出.因为0X就占了2个,如果0X后面不跟数据,则该数据非法.
		if(*p=='0'&&((*(p+1)=='X')||(*(p+1)=='x')))//必须以'0X'或'0x'开头.
		{
			p+=2;	//偏移到数据起始地址.
			bnum-=2;//减去偏移量	 
		}
		else 
			return 3;//起始头的格式不对
	}
	else if(bnum==0)
		return 4;//位数为0，直接退出.	  
	while(1)
	{
		if(bnum) bnum--;
		if(*p<='9'&&*p>='0') t=*p-'0';	//得到数字的值
		else t=*p-'A'+10;				//得到A~F对应的值	    
		*dest += t*pow_df(hexdec,bnum);		   
		p++;
		if(*p=='\0')break;//数据都查完了.	
	}
	return 0;//成功转换
}
/**************************************************************
 * 函数名称: rt_uint8_t str2nnum(rt_uint8_t*src,rt_uint8_t *dest)
 * 参    数: 无
 * 返 回 值: 无
 * 描    述: 把字符串转为数字
 **************************************************************/	
//支持16进制转换,但是16进制字母必须是大写的,且格式为以0X开头的.
//不支持负数 
//*src:数字字符串指针
//*dest:转换完的结果存放地址.
//返回值:0，成功转换完成.其他,错误代码.
//1,数据格式错误.2,16进制位数为0.3,起始格式错误.4,十进制位数为0.
static rt_uint8_t str2nnum(rt_uint8_t*src,rt_uint8_t *dest)
{
	rt_uint32_t t;
	rt_uint8_t bnum=0;	//数字的位数
	rt_uint8_t *p;		  
	rt_uint8_t hexdec=10;//默认为十进制数据
	p=src;
	*dest=0;//清零.
	while(1)
	{
		if((*p<='9'&&*p>='0')||(*p<='F'&&*p>='A')||(*p=='X'&&bnum==1))//参数合法
		{
			if(*p>='A') hexdec=16;	//字符串中存在字母,为16进制格式.
			bnum++;					//位数增加.
		}
		else if(*p=='\0')
		    break;	                //碰到结束符,退出.
		else 
			return 1;				//不全是十进制或者16进制数据.
		p++; 
	} 
	p=src;			    //重新定位到字符串开始的地址.
	hexdec=16;		    //16进制数据
	  
	while(1)
	{
		if(bnum) bnum--;
		if(*p<='9'&&*p>='0') t=*p-'0';	//得到数字的值
		else t=*p-'A'+10;				//得到A~F对应的值	    
		*dest += t*pow_df(hexdec,bnum);		   
		p++;
		if(*p=='\0') break;//数据都查完了.	
	}
	return 0;//成功转换
}
/*********************************************************************************************************
** Function name:		Log_Process
** Descriptions:		LOG函数
** input parameters:	 
** 						
** return value:		
** Created by:			LCF		  
** Created Date:		20170511	  
**-------------------------------------------------------------------------------------------------------
** Modified by:		  		
** Modified date:	  		
**-------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
//m^n函数
//返回值:m^n次方
static rt_uint32_t pow_df(rt_uint8_t m,rt_uint8_t n)
{
	rt_uint32_t result=1;
	while(n--)result*=m;
	return result;
}
