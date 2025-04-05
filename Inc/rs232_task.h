/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             :  rs232_task.h
 * 版本                 :  0.0.1
 * 作者                 :  lvchunhao
 * 创建日期             :　2017年11月14日
 * 描述                 :  Rs232接收及处理线程
                           1. 本版本采用UART4
													 2. 作为默认的调试终端
 ****************************************************************************/
#ifndef _RS232_TASK_H
#define _RS232_TASK_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "stdint.h"
	
	
extern uint8_t g_u8Rs232RecvLen; 	 
extern uint8_t g_u8Rs232RecvStep;
extern uint8_t u8Rs232RecvCompeletFlag; 
	 
	 
uint32_t GetRs232WriteIndex(void);	 
void Rs232RecvOneData(uint8_t u8Bytes);
void vTaskRs232Communicate(void *pArgs);
int32_t Rs232RecvPTLData(uint8_t u8Data);
	 
#ifdef __cplusplus
}
#endif
#endif /* _RS232_TASK_H */


