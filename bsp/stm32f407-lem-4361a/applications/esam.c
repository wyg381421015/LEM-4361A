#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include <board.h>
#include <stdio.h>
#include <global.h>
#include <stdlib.h>
#include <esam.h>

#define DEVICE_NAME			"spi10"//ESAM设备名称  挂载在SPI1下

#define THREAD_ESAM_PRIORITY     7
#define THREAD_ESAM_STACK_SIZE   1024*2
#define THREAD_ESAM_TIMESLICE    5

#define ESAM_PWR_PIN    	GET_PIN(E, 4)//ESAM芯片电源控制引脚

static rt_uint8_t esam_stack[THREAD_ESAM_STACK_SIZE];//

static struct rt_thread esam_thread;
struct rt_spi_device* spi_esam;

//static rt_uint8_t g_ucEsam_cmd;

ScmEsam_Comm stEsam_Comm;
//SCMEsam_Info stEsam_Info;

rt_uint8_t ESAM_Check(rt_uint8_t *pData, rt_uint16_t Len);
//rt_err_t ESAM_Send_and_Recv(struct rt_spi_device *device,rt_uint8_t* send_buf,rt_uint16_t send_len,rt_uint8_t* recv_buf,rt_uint16_t recv_len);

rt_err_t ESAM_Send_and_Recv(struct rt_spi_device *device,ScmEsam_Comm* l_stEsam_Comm);

static void esam_thread_entry(void *parameter)
{
	rt_err_t ret = RT_EOK;
	
	rt_pin_mode(ESAM_PWR_PIN, PIN_MODE_OUTPUT);
	rt_pin_write(ESAM_PWR_PIN, PIN_LOW);
	
	spi_esam = (struct rt_spi_device *)rt_device_find(DEVICE_NAME);
	
	if(spi_esam != RT_NULL)
	{
		struct rt_spi_configuration cfg;
		
		cfg.data_width = 8;
		cfg.max_hz = 22000000;
		cfg.mode = RT_SPI_MODE_3 | RT_SPI_MSB;
		
		rt_spi_configure(spi_esam, &cfg);
	}
	
	rt_thread_mdelay(1000);	
	
//	g_ucEsam_cmd = RD_INFO;
//	
//	ESAM_Communicattion(RD_INFO_07,&stEsam_Comm);
	while (1)
	{
//		if(g_ucEsam_cmd != 0xff)
//		{
//			ret = ESAM_Communicattion(RD_INFO,&stEsam_Comm);
//			if(ret == RT_EOK)
//			{
//				if((stEsam_Comm.Rx_data[0] == 0x90)&&(stEsam_Comm.Rx_data[1] == 0x00))
//				{
//					memcpy(&stEsam_Info,&stEsam_Comm.Rx_data[4],sizeof(SCMEsam_Info));
//					g_ucEsam_cmd = 0xff;
//				}
//			}			
//		}
		rt_thread_mdelay(2000);
	}
}

rt_err_t ESAM_Communicattion(ESAM_CMD cmd,ScmEsam_Comm* l_stEsam_Comm)
{
	rt_err_t res;
	rt_uint16_t ptr,lenth;
  rt_uint8_t sendbuf[1024];
	
	ptr = 0;
	lenth = l_stEsam_Comm->DataTx_len;
	
	sendbuf[ptr++] = ESAM_HEAD;
	switch(cmd)
	{
		case RD_INFO_FF:
			sendbuf[ptr++] = 0x80;
			sendbuf[ptr++] = 0x36;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0xff;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x00;
			break;
		case RD_INFO_01:
		case RD_INFO_02:
		case RD_INFO_03:
		case RD_INFO_04:
		case RD_INFO_05:
		case RD_INFO_06:
		case RD_INFO_07:
		case RD_INFO_08:
		case RD_INFO_09:
		case RD_INFO_10:
		case RD_INFO_11:
		case RD_INFO_12:
			sendbuf[ptr++] = 0x80;
			sendbuf[ptr++] = 0x36;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = cmd;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x00;
		break;
		
		case HOST_KEY_AGREE:
			sendbuf[ptr++] = 0x81;
			sendbuf[ptr++] = 0x02;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x24;
		break;
		case HOST_KEY_UPDATE:
			break;
		case HOST_CERT_UPDATE:
			break;
		case HOST_SESS_CALC_MAC_11:
			sendbuf[ptr++] = 0x81;
			sendbuf[ptr++] = 0x1C;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x11;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
			break;
		case HOST_SESS_CALC_MAC_A2:
			sendbuf[ptr++] = 0x81;
			sendbuf[ptr++] = 0x1C;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0xA2;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
			break;
		case HOST_SESS_CALC_MAC_A7:
			sendbuf[ptr++] = 0x81;
			sendbuf[ptr++] = 0x1C;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0xA7;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
			break;
		case HOST_SESS_VERI_MAC:
			break;
		case CON_KEY_AGREE:
			sendbuf[ptr++] = 0x82;
			sendbuf[ptr++] = 0x02;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x24;
		break;
		case CON_KEY_UPDATE:
			break;
		case CON_CERT_UPDATE:
			break;
		case CON_SESS_CALC_MAC_11:
			sendbuf[ptr++] = 0x82;
			sendbuf[ptr++] = 0x24;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x11;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
		
			memcpy(sendbuf+ptr,l_stEsam_Comm->Tx_data,lenth);
			sendbuf[ptr++] = 0x80;
			sendbuf[ptr++] = 0x24;
			sendbuf[ptr++] = 0x11;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
			
			break;
		case CON_SESS_CALC_MAC_12:
			sendbuf[ptr++] = 0x82;
			sendbuf[ptr++] = 0x24;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x12;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
			break;
		case CON_SESS_CALC_MAC_13:
			sendbuf[ptr++] = 0x82;
			sendbuf[ptr++] = 0x24;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x13;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
		
			memcpy(sendbuf+ptr,l_stEsam_Comm->Tx_data,lenth);
		
			sendbuf[ptr++] = 0x80;
			sendbuf[ptr++] = 0x24;
			sendbuf[ptr++] = 0x13;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
			break;
		case CON_SESS_VERI_MAC_11:
			sendbuf[ptr++] = 0x82;
			sendbuf[ptr++] = 0x24;
			sendbuf[ptr++] = 0x31;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
			sendbuf[ptr++] = 0x82;
			sendbuf[ptr++] = 0x24;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x31;
			sendbuf[ptr++] = ((lenth-2)>>8)&0xff;
			sendbuf[ptr++] = ((lenth-2)&0xff);
			break;
		case CON_SESS_VERI_MAC_12:
			sendbuf[ptr++] = 0x82;
			sendbuf[ptr++] = 0x24;
			sendbuf[ptr++] = 0x32;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
			sendbuf[ptr++] = 0x82;
			sendbuf[ptr++] = 0x24;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x32;
			sendbuf[ptr++] = ((lenth-2)>>8)&0xff;
			sendbuf[ptr++] = ((lenth-2)&0xff);
			break;
		case CON_SESS_VERI_MAC_13:
			sendbuf[ptr++] = 0x82;
			sendbuf[ptr++] = 0x24;
			sendbuf[ptr++] = 0x33;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
			sendbuf[ptr++] = 0x82;
			sendbuf[ptr++] = 0x24;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x33;
			sendbuf[ptr++] = ((lenth-2)>>8)&0xff;
			sendbuf[ptr++] = ((lenth-2)&0xff);
			break;
		case APP_KEY_AGREE_ONE://app会话协商 第一步
			sendbuf[ptr++] = 0x80;
			sendbuf[ptr++] = 0x04;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x10;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x00;
			break;
		case APP_KEY_AGREE_TWO://app会话协商 第二步804A00800015
			sendbuf[ptr++] = 0x80;
			sendbuf[ptr++] = 0x4A;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x80;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x15;
			break;
		case APP_KEY_AGREE_THREE://app会话协商 第三步804A00000030
			sendbuf[ptr++] = 0x80;
			sendbuf[ptr++] = 0x4A;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x30;
			break;
		case APP_SESS_AGREE_ONE://主站会话密钥协商
			sendbuf[ptr++] = 0x80;
			sendbuf[ptr++] = 0x4A;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0xC0;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x20;
			break;
		case APP_SESS_AGREE_TWO://主站会话密钥协商
			sendbuf[ptr++] = 0x80;
			sendbuf[ptr++] = 0x4C;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = 0x00;
			sendbuf[ptr++] = (lenth>>8)&0xff;
			sendbuf[ptr++] = (lenth&0xff);
			break;
		case APP_SESS_VERI_MAC:
			break;
		case HOST_READ:
//			sendbuf[ptr++] = 0x81;
//			sendbuf[ptr++] = 0x1c;
//			sendbuf[ptr++] = 0x31;     //811C311002000C00   10 D9 DB 9F E9 19 8A 96 26 DE 19 FD AE 39 83 1F E6 03
//			sendbuf[ptr++] = 0x10;
//			sendbuf[ptr++] = 0x00;
//			sendbuf[ptr++] = 0x12;
//			sendbuf[ptr++] = 0x00;
//			sendbuf[ptr++] = 0x0c;
//			sendbuf[ptr++] = 0xd9;
//			sendbuf[ptr++] = 0xdb;
//			sendbuf[ptr++] = 0x9f;
//			sendbuf[ptr++] = 0xe9;
//			sendbuf[ptr++] = 0x19;
//			sendbuf[ptr++] = 0x8a;
//			sendbuf[ptr++] = 0x96;
//			sendbuf[ptr++] = 0x26;
//			sendbuf[ptr++] = 0xde;
//			sendbuf[ptr++] = 0x19;
//			sendbuf[ptr++] = 0xfd;
//			sendbuf[ptr++] = 0xae;
//			sendbuf[ptr++] = 0x39;
//			sendbuf[ptr++] = 0x83;
//			sendbuf[ptr++] = 0x1f;
//			sendbuf[ptr++] = 0xe6;
			break;
		default:
			break;
	}
	if((cmd != CON_SESS_CALC_MAC_11)&&(cmd != CON_SESS_CALC_MAC_13))
		memcpy(sendbuf+ptr,l_stEsam_Comm->Tx_data,lenth);
	
	sendbuf[ptr+lenth] = ESAM_Check(&sendbuf[1],ptr+lenth-1);
	
	stEsam_Comm.DataTx_len = ptr+lenth+1;
	memcpy(stEsam_Comm.Tx_data,sendbuf,stEsam_Comm.DataTx_len);
	
	my_printf((char*)stEsam_Comm.Tx_data,stEsam_Comm.DataTx_len,MY_HEX,1,"[Esam]:TX:");
	
	
	res = ESAM_Send_and_Recv(spi_esam,&stEsam_Comm);
	if(res == RT_EOK)
	{
		memcpy(l_stEsam_Comm->Rx_data,stEsam_Comm.Rx_data,stEsam_Comm.DataRx_len);
		l_stEsam_Comm->DataRx_len = stEsam_Comm.DataRx_len;
	}

	return res;
}


//rt_err_t ESAM_Send_and_Recv(struct rt_spi_device *device,rt_uint8_t* send_buf,rt_uint16_t send_len,rt_uint8_t* recv_buf,rt_uint16_t recv_len)
rt_err_t ESAM_Send_and_Recv(struct rt_spi_device *device,ScmEsam_Comm* l_stEsam_Comm)
{
	rt_err_t ret = RT_EOK;
//	rt_uint8_t Tx_buf[1024],Rx_buf[1024];
	rt_uint16_t lenth,count;
	
	l_stEsam_Comm->DataRx_len = 0;
	
	rt_enter_critical();
	ret = rt_spi_take_bus(device);
	if(ret == RT_EOK)
	{
		rt_spi_take(device);
		
		rt_thread_delay(20);
		
//		Tx_buf[0] = ESAM_HEAD;
//		
//		memcpy(Tx_buf+1,send_buf,send_len);
		
//		Tx_buf[send_len+1] = ESAM_Check(&l_stEsam_Comm->Tx_data[1],send_len);
		
//		lenth = send_len+2;
		
		rt_spi_send(device,l_stEsam_Comm->Tx_data,l_stEsam_Comm->DataTx_len);
		
		rt_thread_delay(5);
		rt_spi_release(device);
		rt_thread_delay(10);
		rt_spi_take(device);
		rt_thread_delay(20);
		
		count = 0;
		lenth = 0xffff;
		while(count <50000)
		{
			rt_spi_recv(device,l_stEsam_Comm->Rx_data,1);
			if(l_stEsam_Comm->Rx_data[0] == 0x55)
			{
				rt_spi_recv(device,l_stEsam_Comm->Rx_data,4);
				lenth = l_stEsam_Comm->Rx_data[2];
				lenth = (rt_size_t)((lenth<<8)|l_stEsam_Comm->Rx_data[3]+1);
				if(lenth < 1023)
				{
					rt_spi_recv(device,l_stEsam_Comm->Rx_data+4,lenth);
					rt_thread_delay(5);
					rt_spi_release(device);
					rt_thread_delay(10);
					
					if(l_stEsam_Comm->Rx_data[lenth+3] == ESAM_Check(&l_stEsam_Comm->Rx_data[0],lenth+3))//校验正确
					{
//						memcpy(recv_buf,Rx_buf,lenth+4);
						
						rt_thread_delay(5);
						rt_spi_release(device);
						rt_thread_delay(10);
						rt_spi_release_bus(device);
						rt_exit_critical();
						
						l_stEsam_Comm->DataRx_len = lenth+4;
									
//						rt_kprintf("[Esam]:RX:");
//						for(count = 0; count <lenth+4; count++)
//						{
//							rt_kprintf("%02X ",l_stEsam_Comm->Rx_data[count]);
//						}
//						rt_kprintf("  \n");
						
						my_printf((char*)l_stEsam_Comm->Rx_data,l_stEsam_Comm->DataRx_len,MY_HEX,1,"[Esam]:RX:");
						
						return RT_EOK;
					}
					else//校验错误  再次读取
					{
						rt_spi_take(device);
						rt_thread_delay(20);
						rt_lprintf("Esam]:spi esam recv data check error!!!\n");
					}
					
					my_printf((char*)l_stEsam_Comm->Rx_data,l_stEsam_Comm->DataRx_len,MY_HEX,1,"[Esam]:RX:");
					
//					rt_kprintf("[Esam]:RX:");
//					for(count = 0; count <lenth+4; count++)
//					{
//						rt_kprintf("%02X ",l_stEsam_Comm->Rx_data[count]);
//					}
//					rt_kprintf("  \n");
				}
			}
			else
			{
				count++;
				rt_thread_delay(20);
			}
		}
		if(count>=50000)
		{
			rt_thread_delay(5);
			rt_spi_release(device);
			rt_thread_delay(10);
		}
		rt_spi_release_bus(device);				
	}
	rt_exit_critical();	
	l_stEsam_Comm->DataRx_len = 0;
	
	return RT_ERROR;
}


rt_uint8_t ESAM_Check(rt_uint8_t *pData, rt_uint16_t Len)
{
	rt_uint16_t i;
	rt_uint8_t Check_Data;
	
	Check_Data = 0;

	for(i=0;i<Len;i++)
	{
		Check_Data^=*pData;
		pData++;	
	}
	return (~Check_Data)&0xff;	
}

int esam_thread_init(void)
{
	rt_err_t res;
	
	res=rt_thread_init(&esam_thread,
											"esam",
											esam_thread_entry,
											RT_NULL,
											esam_stack,
											THREAD_ESAM_STACK_SIZE,
											THREAD_ESAM_PRIORITY,
											THREAD_ESAM_TIMESLICE);
	if (res == RT_EOK) 
	{
			rt_thread_startup(&esam_thread);
	}
	return res;
}


#if defined (RT_ESAM_AUTORUN) && defined(RT_USING_COMPONENTS_INIT)
	INIT_APP_EXPORT(esam_thread_init);
#endif
MSH_CMD_EXPORT(esam_thread_init, esam thread run);



