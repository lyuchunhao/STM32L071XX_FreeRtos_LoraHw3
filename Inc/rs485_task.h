/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             :  rs485_task.c
 * 版本                 :  0.0.1
 * 作者                 :  lvchunhao
 * 创建日期             :　2017年11月21日
 * 描述                 :  Uart2-RS485通讯
                           1. 中断接收/查询发送
													 2. 接收发送的使能已经在硬件上完成
 ****************************************************************************/
#ifndef _RS485_TASK_H
#define _RS485_TASK_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "stdint.h"
	 
extern uint8_t g_u8Rs485RecvCompleteFlag;	 
	 
void vTaskRS485Communicate(void *pArgs);
	 
	 
#ifdef __cplusplus
}
#endif
#endif /* _RS485_TASK_H */

