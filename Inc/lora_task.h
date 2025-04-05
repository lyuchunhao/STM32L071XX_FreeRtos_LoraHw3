/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             :  lora_task.h
 * 版本                 :  0.0.1
 * 作者                 :  lvchunhao
 * 创建日期             :　2017年8月2日
 * 描述                 :  Lora通讯 
                           1. 采用STM32串口Uart5-Lora
													 2. 默认初始化了接收中断/接收DMA以及发送DMA
													 3. 测试发现：发送过快都会造成一定程度的丢包
													 4. FreeRTOS发送和接收最好不要采用查询方式，不利于任务调度
													 5. 推荐：中断接收/DMA发送
													 6. Uart5测试-中断接收/查询发送 (vTaskUart1Read)
                              HAL_UART_Receive_IT与HAL_UART_Transmit函数似乎有些问题,
														  同时调用会造成进不去接收中断。因此改成自己定义的接收和
															发送函数，问题解决
													 7. 2017-8-5修改成自定义中断接收/查询发送
 ****************************************************************************/
#ifndef _LORA_TASK_H
#define _LORA_TASK_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "stdint.h"
	 
	 
typedef enum _tagEmCFGResult
{
	 RESULT_INIT = 0x00,
	 RESULT_OK,
	 RESULT_FAILED,
	 RESULT_TIMEOUT,

}EmCFGResult;

typedef enum _tagEmAppReplyCMD
{
	 REPLY_CMD_NONE = 0x00,
	 REPLY_CMD_XXCB,
}EmAppReplyCMD;


extern uint8_t g_u8LoraSlaveConfig[32];	 
extern uint8_t g_u8LoraRecvCompleteFlag;	 
extern uint8_t g_u8LoraModuleReplyOK;
extern uint8_t g_u8LoraConfigResultOK;
extern uint8_t g_u8LoraRecvSlaveStateReply;
extern uint8_t g_u8LoraReplyAppFlag;
	 
void LoraModuleReset(void);	 
void LoraWriteIndexClearZero(void);
void LoraRecvOneData(uint8_t u8Bytes);
void vTaskLoraCommunicate(void *pArgs);
	 
	 
#ifdef __cplusplus
}
#endif
#endif /* _LORA_TASK_H */

