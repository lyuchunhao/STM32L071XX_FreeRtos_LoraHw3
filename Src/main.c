/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l0xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "common.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
#define RS232_TASK    1
#define WRITE_TASK    1
#define DATA_TASK     1
#define LORA_TASK     1
#define RS485_TASK    1
#define ADC_TASK      0


//ע��������
StTaskObj g_stTaskObjs[] =
{
#if RS232_TASK
    {
			"Rs232Communicate",
			vTaskRs232Communicate,
			configMAX_PRIORITIES - 3,
			100,
			NULL,
    },
#endif
		
#if WRITE_TASK
    {
			"WriteTask",
			vTaskUartWrite,
			configMAX_PRIORITIES - 3,
			100,
			NULL,
    },
#endif
		
#if DATA_TASK
    {
			"DataTask",
			vTaskDataProcess,
			configMAX_PRIORITIES - 2,
			100,
			NULL,
    },
#endif
		
#if LORA_TASK
		{
			"LoraTask",
			vTaskLoraCommunicate,
			configMAX_PRIORITIES - 2,
			100,
			NULL,
		},
#endif
		
#if RS485_TASK
    {
			"RS485Communicate",
			vTaskRS485Communicate,
			configMAX_PRIORITIES  - 3,
			100,
			NULL,
    },
#endif
		
#if ADC_TASK
    {
			"ADCTask",
			vTaskADCollection,
			configMAX_PRIORITIES  - 3,
			100,
			NULL,
    },
#endif

};
/*
 * ������      : RegisterMessage
 * ����        : ע����Ϣ����
 * ����        : s32Index [in] : ��������
 *             : boMsgQueue [in] : �Ƿ���Ҫ��Ϣ����
 * ����        : ��ȷ����0, ���󷵻�-1
 * ����        : lvchunhao 2017-7-25
 */
static void RegisterMessage(int s32Index, unsigned char u8IsMsgQueue)
{
    if(1 == u8IsMsgQueue)
    {
        g_stTaskObjs[s32Index].s32MsgQueID = xQueueCreate( 5, sizeof(StMsgPkg));
        if(NULL == g_stTaskObjs[s32Index].s32MsgQueID)
        {
            return;
        }
    }

    return;
}

/*
 * ������      : RegisterTaskMessages
 * ����        : ����������Ϣ����
 * ����        : ��
 * ����        : ��Щ������Ҫ��Ϣ����,Ϊ��ȷ������Ӧg_stTaskObjs[s32Index]��ֵ,
 * 				 ��������RegisterMessage(i ++, false), ע���Ӧ��ϵ
 * ����        : ��ȷ����0, ���󷵻�-1
 * ����        : lvchunhao 2017-7-25
 */
static int RegisterTaskMessages(void)
{
    int i = 0;

#if RS232_TASK
    RegisterMessage(i ++, 0);
#endif
	
#if WRITE_TASK
    RegisterMessage(i ++, 1);
#endif
	
#if DATA_TASK
    RegisterMessage(i ++, 1);
#endif
	
#if LORA_TASK
    RegisterMessage(i ++, 0);
#endif
	
#if RS485_TASK
    RegisterMessage(i ++, 0);
#endif

#if ADC_TASK
    RegisterMessage(i ++, 0);
#endif

    return 0;
}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
  portBASE_TYPE s32Ret = 0;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART4_UART_Init();
  MX_USART5_UART_Init();
  MX_RTC_Init();

  /* USER CODE BEGIN 2 */
	PRINT("LoRa main process is start\n");
	
  //����������Ϣ����
	if(RegisterTaskMessages() < 0)
	{
		return -1;
	}
	
		//��������
	for(unsigned int i = 0; i < sizeof(g_stTaskObjs) / sizeof(g_stTaskObjs[0]); i ++)
	{

		s32Ret = xTaskCreate(g_stTaskObjs[i].pTaskFxn,       /* ָ����������ָ�� */
												 g_stTaskObjs[i].pTaskName,      /* ������ı�����,ֻ���ڵ������õ� */
												 g_stTaskObjs[i].u16StackSize,   /* ջ��� */
												 &g_stTaskObjs[i],               /* ������� */
								         g_stTaskObjs[i].u16TaskPrio,    /* ���ȼ� */
							           &g_stTaskObjs[i].stTask);       /* ������ */

		
		//��������ʧ��
		if(errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == s32Ret)
		{
				return -1;
		}

	}
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  //MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;    //24M
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;            //24*4=96M
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_3;            //96/3=22M  
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;         //32M/4=8M

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 3, 0);
}

/* USER CODE BEGIN 4 */
/*
 * ������      : SystemClock_StopMode
 * ����        : ϵͳ��Stopģʽ�˳�����Ҫ��������ʱ��
 * ����        : void
 * ����        : void
 * ����        : lvchunhao 2017-11-10
 */
void SystemClock_StopMode(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 3, 0);
}
/*
 * ������      : HAL_RTC_AlarmAEventCallback
 * ����        : ����A�жϻص�����
 * ����        : hrtc[in]: rtc���
 * ����        : void
 * ����        : lvchunhao 2017-11-10 
 */	
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
		#if PRINT_STRING
		PRINT("******** I am in RTC Alarm A *********\n");
	  #endif
	  //SetRTCAlarmSeconds(0, 1, 1);
}
/*
 * ������      : HAL_RTCEx_AlarmBEventCallback
 * ����        : ����B�жϻص�����
 * ����        : hrtc[in]: rtc���
 * ����        : void
 * ����        : lvchunhao 2017-11-10 
 */	
void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
{
		#if PRINT_STRING
		PRINT("******** I am in RTC Alarm B *********\n");
	  #endif
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
/* USER CODE BEGIN Callback 0 */

/* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
/* USER CODE BEGIN Callback 1 */

/* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
