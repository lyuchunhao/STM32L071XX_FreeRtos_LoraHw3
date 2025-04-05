/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  rs232_task.h
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��11��14��
 * ����                 :  Rs232���ռ������߳�
                           1. ���汾����UART4
													 2. ��ΪĬ�ϵĵ����ն�
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


