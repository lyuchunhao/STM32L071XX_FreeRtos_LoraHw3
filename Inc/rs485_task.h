/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  rs485_task.c
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��11��21��
 * ����                 :  Uart2-RS485ͨѶ
                           1. �жϽ���/��ѯ����
													 2. ���շ��͵�ʹ���Ѿ���Ӳ�������
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

