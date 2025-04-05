/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             :  adc_task.h
 * 版本                 :  0.0.1
 * 作者                 :  lvchunhao
 * 创建日期             :　2017年8月3日
 * 描述                 :  AD采集：IN6(电压)/IN7(4-20ma电流)
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


