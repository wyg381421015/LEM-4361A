#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDEL_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 1024
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 4
#define RT_TIMER_THREAD_STACK_SIZE 1024
#define RT_DEBUG

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE
#define RT_USING_SIGNALS

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 1024
#define RT_CONSOLE_DEVICE_NAME "uart1"
#define RT_VER_NUM 0x40000
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M4

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 2048
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */


/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 2048
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_ARG_MAX 10

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 2
#define DFS_FILESYSTEM_TYPES_MAX 2
#define DFS_FD_MAX 16
#define RT_USING_DFS_DEVFS
#define RT_USING_DFS_UFFS
#define RT_UFFS_ECC_MODE_3
#define RT_UFFS_ECC_MODE 3

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ 512
#define RT_USING_SERIAL
#define RT_SERIAL_USING_DMA
#define RT_USING_CAN
#define RT_USING_HWTIMER
#define RT_USING_I2C
#define RT_USING_I2C_BITOPS
#define RT_USING_PIN
#define RT_USING_ADC
#define RT_USING_MTD_NAND
#define RT_USING_PM
#define RT_USING_SPI

/* Using WiFi */


/* Using USB */


/* POSIX layer and C standard library */

#define RT_USING_LIBC
#define RT_USING_POSIX

/* Network */

/* Socket abstraction layer */


/* light weight TCP/IP stack */


/* Modbus master and slave stack */


/* AT commands */


/* VBUS(Virtual Software BUS) */


/* Utilities */


/* ARM CMSIS */


/* RT-Thread online packages */

/* IoT - internet of things */


/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */


/* IoT Cloud */


/* security packages */


/* language packages */


/* multimedia packages */


/* tools packages */


/* system packages */


/* peripheral libraries and drivers */

/* sensors drivers */

#define PKG_USING_MPU6XXX
#define PKG_USING_MPU6XXX_LATEST_VERSION

/* miscellaneous packages */


/* samples: kernel and components samples */

#define SOC_FAMILY_STM32
#define SOC_SERIES_STM32F4

/* Hardware Drivers Config */

#define SOC_STM32F407VG

/* Onboard Peripheral Drivers */

#define BSP_USING_COM1
#define BSP_USING_COM2
#define BSP_USING_COM3
#define BSP_USING_COM4
#define BSP_USING_COM5
#define BSP_USING_COM6
#define BSP_USING_EEPROM
#define BSP_USING_FMC

/* On-chip Peripheral Drivers */

#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_UART1
#define BSP_UART1_RX_USING_DMA
#define BSP_USING_UART2
#define BSP_UART2_RX_USING_DMA
#define BSP_USING_UART3
#define BSP_UART3_RX_USING_DMA
#define BSP_USING_UART4
#define BSP_UART4_RX_USING_DMA
#define BSP_USING_UART5
#define BSP_UART5_RX_USING_DMA
#define BSP_USING_UART6
#define BSP_UART6_RX_USING_DMA
#define BSP_USING_TIM
#define BSP_USING_TIM14
#define BSP_USING_SPI
#define BSP_USING_SPI1
#define BSP_USING_ADC
#define BSP_USING_ADC1
#define BSP_USING_I2C1
#define BSP_I2C1_SCL_PIN 24
#define BSP_I2C1_SDA_PIN 25
#define BSP_USING_CAN1
#define RT_USING_PCF8563
#define RT_USING_LCD
#define RT_USING_LCD_TOUCH
#define BSP_USING_EXT_FMC_IO
#define RT_USING_SPI_ESAM

/* Board extended module Drivers */

#define RT_ANALOG_AUTORUN
#define RT_ANALOG_ADC "adc1"
#define RT_METER_AUTORUN
#define RT_METER_USART "uart6"
#define RT_BLUETOOTH_AUTORUN
#define RT_BLUETOOTH_USART "uart3"
#define RT_MONITOR_AUTORUN
#define RT_SCREEN_AUTORUN
#define RT_STORAGE_AUTORUN
#define RT_STRATEGY_AUTORUN
#define RT_CHARGEOILE_AUTORUN
#define RT_HPLC_AUTORUN
#define RT_ESAM_AUTORUN

#endif
