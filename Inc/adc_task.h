/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  adc_task.h
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��8��3��
 * ����                 :  AD�ɼ���IN6(��ѹ)/IN7(4-20ma����)
 ****************************************************************************/
#ifndef _ADC_TASK_H
#define _ADC_TASK_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "common.h"
	 
#define ADC_NUMOFCHANNEL  2
	 
	 

	 
void vTaskADCollection(void *pArgs);
void Float2String(float fValue, uint8_t *u8Data);
void GetADCConvertedValue(uint32_t u32ADCValue[ADC_NUMOFCHANNEL]);
int32_t GetADCValue(ADC_HandleTypeDef* hadc, uint32_t u32ADCValue[ADC_NUMOFCHANNEL]);	 
void vTaskADCollection(void *pArgs);
	 
#ifdef __cplusplus
}
#endif
#endif /* _ADC_TASK_H */


