/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             :  adc_task.c
 * 版本                 :  0.0.1
 * 作者                 :  lvchunhao
 * 创建日期             :　2017年8月3日
 * 描述                 :  AD采集：IN6(电压)/IN7(4-20ma电流)
 ****************************************************************************/

#include "common.h"



// AD转换结果值
uint32_t DMA_Transfer_Complete_Count = 0;
uint32_t ADC_ConvertedValue[ADC_NUMOFCHANNEL];


// 用于保存转换计算后的电压值	 
__IO float ADC_ConvertedValueLocal[ADC_NUMOFCHANNEL];


/*
 * 函数名      : Float2String
 * 功能        : 将Float转换成字符串(由于sprintf函数中%f转换存在bug)
 * 参数        : fValue [in] : 需要转换的浮点型
 * 参数        ：u8Data [out]: 转换后的字符串
 * 返回        : 无
 * 作者        : lvchunhao  2017-8-2
 */
void Float2String(float fValue, uint8_t *u8Data)
{
		uint32_t u32Value;
	  char  c8TmpBuf[32];
		
		if(u8Data == NULL)
		{
			 return;
		}
		
		sprintf((char *)u8Data, "%d.", (uint32_t)fValue);
		u32Value = (uint32_t)(fValue * 1000);
		sprintf(c8TmpBuf, "%03d", u32Value);
		if(strlen(c8TmpBuf) >= 3)
		{
				strcat((char *)u8Data, c8TmpBuf + strlen(c8TmpBuf) - 3);
		}
		
		strcat((char *)u8Data, "\n");

}

/*
 * 函数名      : GetADCValue
 * 功能        : 启动一次转换获得Value
 * 参数        : hadc [in] : ADC句柄
 * 参数        ：u32ADCValue [out]: 转换后的Value
 * 返回        : 0成功 -1失败
 * 作者        : lvchunhao  2017-8-4
 */
int32_t GetADCValue(ADC_HandleTypeDef* hadc, uint32_t u32ADCValue[ADC_NUMOFCHANNEL])
{
		uint8_t u8CNT;
	
		/* 经过CubeMx配置生成的工程需要此函数ADC校验,否则结果会有偏差 */
	  HAL_ADCEx_Calibration_Start(hadc, ADC_SINGLE_ENDED);
#if 0	
		/* 启动AD转换并使能DMA传输和中断 */
		HAL_ADC_Start_DMA(hadc, ADC_ConvertedValue, 2);
	
    vTaskDelay(50 / portTICK_RATE_MS);	
	  for(u8CNT = 0; u8CNT < ADC_NUMOFCHANNEL; u8CNT ++)
    {
				u32ADCValue[u8CNT] = ADC_ConvertedValue[u8CNT];
		}
	
		/* 关闭AD-DMA方式转换 */
	  HAL_ADC_Stop_DMA(hadc);
#else
    /* 采用查询方式：以下代码似乎只能得到u32ADCValue[0] */
	  /* 两次采集加一个延时,最少12.5+7.5=20个周期时间*/
		if(HAL_ADC_Start(hadc) != HAL_OK)
		{ 
				return -1; 
		}

		HAL_ADC_PollForConversion(hadc, 100);
		if((HAL_ADC_GetState(hadc) & 0x00000200U) == HAL_ADC_STATE_EOC_REG)
		{
				for(u8CNT = 0; u8CNT < ADC_NUMOFCHANNEL; u8CNT ++)
				{
						u32ADCValue[u8CNT] = HAL_ADC_GetValue(hadc);
					  vTaskDelay(5 / portTICK_RATE_MS);	
				}
			
			  HAL_ADC_Stop(hadc);
		}
#endif	
		return 0;
}
/*
 * 函数名      : vTaskRS485Communicate
 * 功能        : RS485-Uart4接收处理
 * 参数        : pArgs [in] : 线程参数
 * 返回        : 无
 * 作者        : lvchunhao  2017-8-2
 */
void vTaskADCollection(void *pArgs)
{
		uint8_t u8ADCSendBuf[64];
	
	  /* 缓冲区初始化 */
	  ADC_ConvertedValue[0] = 0;
		ADC_ConvertedValue[0] = 1;
	
	  /* 经过CubeMx配置生成的工程需要此函数ADC校验,否则结果会有偏差 */
	  HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED);
	
	  /* 启动AD转换并使能DMA传输和中断 */
	  vTaskDelay(50);
		HAL_ADC_Start_DMA(&hadc, ADC_ConvertedValue, ADC_NUMOFCHANNEL);  

	  while(1)
		{
				vTaskDelay(1500 / portTICK_RATE_MS);
			  /* 3.3为AD转换的参考电压值，stm32的AD转换为12bit，2^12=4096，
					 即当输入为3.3V时，AD转换结果为4096 */ 
        //ADC_ConvertedValue[0]只取最低12有效数据	
        //电流采集：电压值/R25(100Ω)			
				ADC_ConvertedValueLocal[0] =(float)(ADC_ConvertedValue[0]&0xFFF)*3.3/4096*3; 
				ADC_ConvertedValueLocal[1] =(float)(ADC_ConvertedValue[1]&0xFFF)*3.3/4096/100*1000; 
			

#if 0			
			  /* 妈的,printf函数在这里浮点型又打印不出来,可在send_task中为什么可以呢? */
				printf("ADC_CH6 value = %d --> %fV\n", ADC_ConvertedValue[0]&0xFFF, ADC_ConvertedValueLocal[0]);
			  printf("ADC_CH7 value = %d --> %fmA\n", ADC_ConvertedValue[1]&0xFFF, ADC_ConvertedValueLocal[1]);	
#else			
			  memset(u8ADCSendBuf, 0, 64);
				sprintf((char *)u8ADCSendBuf, "Change Times: %d \n", DMA_Transfer_Complete_Count);
			  PostUartWriteMsg(u8ADCSendBuf, strlen((char *)u8ADCSendBuf), TYPE_UART_RS232);
			
				memset(u8ADCSendBuf, 0, 64);
			  sprintf((char *)u8ADCSendBuf, "ADC_CH6 value = %d --> %d.", ADC_ConvertedValue[0]&0xFFF, (uint32_t )(ADC_ConvertedValueLocal[0]));
				sprintf((char *)u8ADCSendBuf + strlen((char *)u8ADCSendBuf), "%03dV\n", ((uint32_t )(ADC_ConvertedValueLocal[0] * 1000)) % 1000);
			  PostUartWriteMsg(u8ADCSendBuf, strlen((char *)u8ADCSendBuf), TYPE_UART_RS232);
			
			  memset(u8ADCSendBuf, 0, 64);
			  sprintf((char *)u8ADCSendBuf, "ADC_CH7 value = %d --> %d.", ADC_ConvertedValue[1]&0xFFF, (uint32_t )(ADC_ConvertedValueLocal[1]));
				sprintf((char *)u8ADCSendBuf + strlen((char *)u8ADCSendBuf), "%03dmA\n", ((uint32_t )(ADC_ConvertedValueLocal[1] * 1000)) % 1000);
				PostUartWriteMsg(u8ADCSendBuf, strlen((char *)u8ADCSendBuf), TYPE_UART_RS232);
#endif				
			  
		}
	
}

/**
  * 函数功能: ADC转换完成回调函数
  * 输入参数: hadc：ADC外设设备句柄
  * 返 回 值: 无
  * 说    明: 无
  */
void  HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
		/* Enable conversion of regular group.                                  */
    /* If software start has been selected, conversion starts immediately.  */
    /* If external trigger has been selected, conversion will start at next */
	  //直接从HAL_ADC_Start_DMA()中提取,如果不写该指令,则只能AD转换1次

		DMA_Transfer_Complete_Count++; 
		//hadc->Instance->CR |= ADC_CR_ADSTART;
	  HAL_ADC_Start(hadc);
}


