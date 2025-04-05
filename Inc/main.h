/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define STM32_UART4_Tx_Pin GPIO_PIN_0
#define STM32_UART4_Tx_GPIO_Port GPIOA
#define STM32_UART4_Rx_Pin GPIO_PIN_1
#define STM32_UART4_Rx_GPIO_Port GPIOA
#define STM32_UART2_Tx_Pin GPIO_PIN_2
#define STM32_UART2_Tx_GPIO_Port GPIOA
#define STM32_UART2_Rx_Pin GPIO_PIN_3
#define STM32_UART2_Rx_GPIO_Port GPIOA
#define Voltage_ADCIN6_Pin GPIO_PIN_6
#define Voltage_ADCIN6_GPIO_Port GPIOA
#define Current_ADCIN7_Pin GPIO_PIN_7
#define Current_ADCIN7_GPIO_Port GPIOA
#define Hart_DATA_Pin GPIO_PIN_12
#define Hart_DATA_GPIO_Port GPIOB
#define Hart_CLOCK_Pin GPIO_PIN_13
#define Hart_CLOCK_GPIO_Port GPIOB
#define Hart_LATCH_Pin GPIO_PIN_15
#define Hart_LATCH_GPIO_Port GPIOB
#define Hart_OCD_Pin GPIO_PIN_8
#define Hart_OCD_GPIO_Port GPIOA
#define STM32_UART1_Tx_Pin GPIO_PIN_9
#define STM32_UART1_Tx_GPIO_Port GPIOA
#define STM32_UART1_Rx_Pin GPIO_PIN_10
#define STM32_UART1_Rx_GPIO_Port GPIOA
#define Hart_INRTS_Pin GPIO_PIN_11
#define Hart_INRTS_GPIO_Port GPIOA
#define Hart_INRESET_Pin GPIO_PIN_12
#define Hart_INRESET_GPIO_Port GPIOA
#define Lora_RQ_Pin GPIO_PIN_15
#define Lora_RQ_GPIO_Port GPIOA
#define STM32_UART5_Tx_Pin GPIO_PIN_3
#define STM32_UART5_Tx_GPIO_Port GPIOB
#define STM32_UART5_Rx_Pin GPIO_PIN_4
#define STM32_UART5_Rx_GPIO_Port GPIOB
#define Lora_RST_Pin GPIO_PIN_5
#define Lora_RST_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
void SystemClock_StopMode(void);
/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
