/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  adc_task.c
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��8��3��
 * ����                 :  AD�ɼ���IN6(��ѹ)/IN7(4-20ma����)
 ****************************************************************************/

#include "common.h"



// ADת�����ֵ
uint32_t DMA_Transfer_Complete_Count = 0;
uint32_t ADC_ConvertedValue[ADC_NUMOFCHANNEL];


// ���ڱ���ת�������ĵ�ѹֵ	 
__IO float ADC_ConvertedValueLocal[ADC_NUMOFCHANNEL];


/*
 * ������      : Float2String
 * ����        : ��Floatת�����ַ���(����sprintf������%fת������bug)
 * ����        : fValue [in] : ��Ҫת���ĸ�����
 * ����        ��u8Data [out]: ת������ַ���
 * ����        : ��
 * ����        : lvchunhao  2017-8-2
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
 * ������      : GetADCValue
 * ����        : ����һ��ת�����Value
 * ����        : hadc [in] : ADC���
 * ����        ��u32ADCValue [out]: ת�����Value
 * ����        : 0�ɹ� -1ʧ��
 * ����        : lvchunhao  2017-8-4
 */
int32_t GetADCValue(ADC_HandleTypeDef* hadc, uint32_t u32ADCValue[ADC_NUMOFCHANNEL])
{
		uint8_t u8CNT;
	
		/* ����CubeMx�������ɵĹ�����Ҫ�˺���ADCУ��,����������ƫ�� */
	  HAL_ADCEx_Calibration_Start(hadc, ADC_SINGLE_ENDED);
#if 0	
		/* ����ADת����ʹ��DMA������ж� */
		HAL_ADC_Start_DMA(hadc, ADC_ConvertedValue, 2);
	
    vTaskDelay(50 / portTICK_RATE_MS);	
	  for(u8CNT = 0; u8CNT < ADC_NUMOFCHANNEL; u8CNT ++)
    {
				u32ADCValue[u8CNT] = ADC_ConvertedValue[u8CNT];
		}
	
		/* �ر�AD-DMA��ʽת�� */
	  HAL_ADC_Stop_DMA(hadc);
#else
    /* ���ò�ѯ��ʽ�����´����ƺ�ֻ�ܵõ�u32ADCValue[0] */
	  /* ���βɼ���һ����ʱ,����12.5+7.5=20������ʱ��*/
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
 * ������      : vTaskRS485Communicate
 * ����        : RS485-Uart4���մ���
 * ����        : pArgs [in] : �̲߳���
 * ����        : ��
 * ����        : lvchunhao  2017-8-2
 */
void vTaskADCollection(void *pArgs)
{
		uint8_t u8ADCSendBuf[64];
	
	  /* ��������ʼ�� */
	  ADC_ConvertedValue[0] = 0;
		ADC_ConvertedValue[0] = 1;
	
	  /* ����CubeMx�������ɵĹ�����Ҫ�˺���ADCУ��,����������ƫ�� */
	  HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED);
	
	  /* ����ADת����ʹ��DMA������ж� */
	  vTaskDelay(50);
		HAL_ADC_Start_DMA(&hadc, ADC_ConvertedValue, ADC_NUMOFCHANNEL);  

	  while(1)
		{
				vTaskDelay(1500 / portTICK_RATE_MS);
			  /* 3.3ΪADת���Ĳο���ѹֵ��stm32��ADת��Ϊ12bit��2^12=4096��
					 ��������Ϊ3.3Vʱ��ADת�����Ϊ4096 */ 
        //ADC_ConvertedValue[0]ֻȡ���12��Ч����	
        //�����ɼ�����ѹֵ/R25(100��)			
				ADC_ConvertedValueLocal[0] =(float)(ADC_ConvertedValue[0]&0xFFF)*3.3/4096*3; 
				ADC_ConvertedValueLocal[1] =(float)(ADC_ConvertedValue[1]&0xFFF)*3.3/4096/100*1000; 
			

#if 0			
			  /* ���,printf���������︡�����ִ�ӡ������,����send_task��Ϊʲô������? */
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
  * ��������: ADCת����ɻص�����
  * �������: hadc��ADC�����豸���
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
void  HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
		/* Enable conversion of regular group.                                  */
    /* If software start has been selected, conversion starts immediately.  */
    /* If external trigger has been selected, conversion will start at next */
	  //ֱ�Ӵ�HAL_ADC_Start_DMA()����ȡ,�����д��ָ��,��ֻ��ADת��1��

		DMA_Transfer_Complete_Count++; 
		//hadc->Instance->CR |= ADC_CR_ADSTART;
	  HAL_ADC_Start(hadc);
}


