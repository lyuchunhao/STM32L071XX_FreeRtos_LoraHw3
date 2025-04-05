/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  data_task.h
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��9��12��
 * ����                 :  ���ݴ����߳�
 ****************************************************************************/
#ifndef _DATA_TASK_H
#define _DATA_TASK_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "common.h"
	

typedef struct _tagStLoraParameter
{
		uint32_t u32IPR;
	  uint32_t u32DBL;
		uint32_t u32NID;
	  uint32_t u32MOD;
    uint32_t u32LRS;
	  uint32_t u32CHA;
	  uint32_t u32RID;
		uint32_t u32TID;
		uint32_t u32ITV;
	  uint32_t u32PAR;
	  uint32_t u32STO;
	  uint32_t u32SLE;	
}StLoraParameter;
	 
	 
typedef struct _tagStLoraAttr
{
		uint8_t u8IPRCmd[16];                 //������,ȡֵ��Χ0-9 0=300bps/s 1=600 2=1200 3=2400 4=4800 5=9600 6=19200 7=38400 8=57600 9=115200bps/s
		uint8_t u8DBLCmd[16];                 //���Եȼ� 0-2
	  uint8_t u8NIDCmd[16];                 //�豸ID   0-65535
		uint8_t u8MODCmd[16];                 //����ģʽ 0-TRNS 1-AT 2-API
		uint8_t u8LRSCmd[16];                 //�������� 1-6 ��ֵԽ��������Խ�߾���Խ��
		uint8_t u8CHACmd[16];                 //�����ŵ�/����Ƶ�� 1-32 (24�����433MHz)
	  uint8_t u8RIDCmd[16];                 //�м̵�ַ 0-65535
		uint8_t u8TIDCmd[16];                 //͸����ַ 0-65535
		uint8_t u8ITVCmd[16];                 //����֡ʱ���� 1-65535
		uint8_t u8PARCmd[16];                 //У��λ 0-NONE 1-EVEN 2-ODD
		uint8_t u8STOCmd[16];                 //ֹͣλ 1 2
	  uint8_t u8SLECmd[16];                 //����ģʽ 0-NONE 1=TIME 2=DEEP
	
}StLoraAttr;



extern StLoraAttr s_StLoraConfig[];
extern StLoraParameter g_stLoraParameter[];

	 
void vTaskDataProcess(void *pArgs);
int32_t PostDataProcessMsg(uint8_t *pData, uint32_t u32DataLen, uint8_t u8MsgType);

int32_t GetLoraConfigInfo(uint8_t *u8Data);
void LoraConfigPrintIndex(uint8_t u8Index);
int32_t LoRa_WaitOnCFGResultUntilTimeout(uint32_t Timeout);
int32_t UpgradeLoraConfig(uint8_t *u8Data, uint32_t u32DataLen);
int32_t LoRa_WaitOnSlaveReplayFlagUntilTimeout(uint32_t Timeout);

#ifdef __cplusplus
}
#endif
#endif /* _DATA_TASK_H */

