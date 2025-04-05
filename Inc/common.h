/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             : common.h
 * 版本                 : 0.0.1
 * 作者                 : lvchunhao
 * 创建日期             :　2017年11月14日
 * 描述                 : 
 ****************************************************************************/
#ifndef _COMMON_H
#define _COMMON_H
#ifdef __cplusplus
 extern "C" {
#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include "main.h"
#include "stm32l0xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

#include "rs232_task.h"
#include "lora_task.h"
#include "write_task.h"
#include "data_task.h"
#include "adc_task.h"
#include "rs485_task.h"



#define  PRINT_STRING       0               //打印字符串/Hex开关
#define  PRINT_DEBUG                        //调试串口打印开关
#define  USART_IRQ_USED     0               //中断函数自定义
#define  MAX_PTL_BYTES      64              //目前设置最大的协议字节数

#define  MSG_BUFF_SIZE      256
#define  MAX_RECV_SIZE      256             //串口接收缓冲区
#define  MAX_SEND_SIZE      256             //串口发送缓冲区

#define  MSG_BUFF_32BYTE    32
#define  MSG_BUFF_64BYTE    64
#define  MSG_BUFF_128BYTE   128
#define  MSG_BUFF_256BYTE   256


#define  LORA_AT_MODE      "+++" 
#define  TMPE_AT_QUIT      "AT+QUIT"
#define  LORA_GET_NID      "AT+NID?"
#define  LORA_GET_TID      "AT+TID?"
#define  LORA_SET_NID_1001 "AT+NID=1001"
#define  LORA_SET_TID_1001 "AT+TID=1001"
#define  LORA_SET_NID_1002 "AT+NID=1002"
#define  LORA_SET_TID_1002 "AT+TID=1002"
#define  LORA_SET_NID_1003 "AT+NID=1003"
#define  LORA_SET_TID_1003 "AT+TID=1003"
#define  LORA_SET_NID_1004 "AT+NID=1004"
#define  LORA_SET_TID_1004 "AT+TID=1004"

#define  LORA_SET_NID_8888 "AT+NID=8888"
#define  LORA_SET_TID_8888 "AT+TID=8888"

#define  LORA_SET_SLE_NONE "AT+SLE=0"       //正常工作模式
#define  LORA_SET_SLE_DEEP "AT+SLE=2"       //深度休眠,RQ=1唤醒 RQ=0进入休眠


#define  PTL_HEAD_RS232    0xF5
#define  PTL_TYPE_RS232    0xF0
#define  PTL_TYPE_MASTER   0xA0
#define  PTL_TYPE_SLAVE    0xB0
#define  PTL_HEAD_OFFSET   0x00
#define  PTL_TYPE_OFFSET   0x01
#define  PTL_LEN_OFFSET    0x02
#define  PTL_CMD_OFFSET    0x03



#define  LORA_RST_SET    HAL_GPIO_WritePin(Lora_RST_GPIO_Port, Lora_RST_Pin, GPIO_PIN_SET)
#define  LORA_RST_RESET  HAL_GPIO_WritePin(Lora_RST_GPIO_Port, Lora_RST_Pin, GPIO_PIN_RESET)
#define  LORA_RQ_SET     HAL_GPIO_WritePin(Lora_RQ_GPIO_Port, Lora_RQ_Pin, GPIO_PIN_SET)
#define  LORA_RQ_RESET   HAL_GPIO_WritePin(Lora_RQ_GPIO_Port, Lora_RQ_Pin, GPIO_PIN_RESET)

#define LORA_IT_ENABLE   STM32L071xx_UART_RxITEnable(&huart5, 1)
#define LORA_IT_DISABLE  STM32L071xx_UART_RxITEnable(&huart5, 0)
#define RS485_IT_ENABLE  STM32L071xx_UART_RxITEnable(&huart2, 1)
#define RS485_IT_DISABLE STM32L071xx_UART_RxITEnable(&huart2, 0)
#define RS232_IT_ENABLE  STM32L071xx_UART_RxITEnable(&huart4, 1)
#define RS232_IT_DISABLE STM32L071xx_UART_RxITEnable(&huart4, 0)




typedef struct _tagStTaskObj
{
    const char    *pTaskName;
	  TaskFunction_t	pTaskFxn;               //任务函数
    uint16_t      u16TaskPrio;              //任务优先级
    uint16_t	  u16StackSize;				        //任务栈大小
    xQueueHandle  s32MsgQueID;              //本任务用于接受消息的消息队列ID
    xTaskHandle    stTask;                  //任务数据结构(句柄)

}StTaskObj;


typedef struct _tagStMsgPkg
{
    uint8_t  *pData;
	  uint8_t  u8MsgType;
    uint32_t u32DataLen;
}StMsgPkg;

typedef enum _tagEmUartType
{
		/* For Uart write */
		TYPE_UART_HART  = 0x01,
	  TYPE_UART_RS485,
	  TYPE_UART_NONE,
	  TYPE_UART_RS232,	
	  TYPE_UART_LORA,
	
	  /* For lora config */
		TYPE_LORA_SET_SINGLE_CFG,                  //Lora模块单项配置
	  TYPE_LORA_SET_MULTIP_CFG,                  //Lora模块多项配置

}EmUartType;

typedef enum _tagEmRecvStep
{
		STEP_RECV_INIT = 0x00,                     
		STEP_FIND_HEAD,                            //寻找协议头       
	  STEP_TYPE_OK,                              //类型必为F0 A0 B0中的一种
	  STEP_LENGTH_GET,                           //获得长度
	  STEP_DATA_GET,                             //接收数据
}EmRecvStep;


#ifdef PRINT_DEBUG
#if 1
#define PRINT(x, ...) printf(" "x, ##__VA_ARGS__);
#else
#define PRINT(x, ...) printf("[%s:%d]: "x, __FILE__, __LINE__, ##__VA_ARGS__);
#endif
#else
#define PRINT(x, ...)
#endif

#ifdef __cplusplus
}
#endif
#endif /* _COMMON_H */


