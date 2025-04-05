/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  lora_task.h
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��8��2��
 * ����                 :  LoraͨѶ 
                           1. ����STM32����Uart5-Lora
													 2. Ĭ�ϳ�ʼ���˽����ж�/����DMA�Լ�����DMA
													 3. ���Է��֣����͹��춼�����һ���̶ȵĶ���
													 4. FreeRTOS���ͺͽ�����ò�Ҫ���ò�ѯ��ʽ���������������
													 5. �Ƽ����жϽ���/DMA����
													 6. Uart5����-�жϽ���/��ѯ���� (vTaskUart1Read)
                              HAL_UART_Receive_IT��HAL_UART_Transmit�����ƺ���Щ����,
														  ͬʱ���û���ɽ���ȥ�����жϡ���˸ĳ��Լ�����Ľ��պ�
															���ͺ�����������
													 7. 2017-8-5�޸ĳ��Զ����жϽ���/��ѯ����
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

