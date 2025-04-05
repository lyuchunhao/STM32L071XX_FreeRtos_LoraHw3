/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             :  write_task.h
 * 版本                 :  0.0.1
 * 作者                 :  lvchunhao
 * 创建日期             :　2017年8月2日
 * 描述                 :  Uart写线程
                           1. 所有的下发都将通过此任务完成
													 2. 默认Uart2接收放在此文件下
 ****************************************************************************/
#ifndef _WRITE_TASK_H
#define _WRITE_TASK_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "stdint.h"
	
	 
	 
	 
void vTaskUartWrite(void *pArgs);
int32_t PostUartWriteMsg(uint8_t *pData, uint32_t u32DataLen, uint8_t u8MsgType);
	 
#ifdef __cplusplus
}
#endif
#endif /* _WRITE_TASK_H */

