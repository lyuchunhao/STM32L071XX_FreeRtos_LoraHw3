/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  write_task.h
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��8��2��
 * ����                 :  Uartд�߳�
                           1. ���е��·�����ͨ�����������
													 2. Ĭ��Uart2���շ��ڴ��ļ���
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

