/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  data_task.c
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��9��12��
 * ����                 :  ���ݴ����߳�
 ****************************************************************************/

#include "common.h"


uint8_t u8DataTxBuff[MAX_SEND_SIZE];
static xQueueHandle s_s32MsgQueID = NULL;


StLoraAttr s_StLoraConfig[] =
{
		//ѯ��Loraģ�����ò���
		{
				"AT+IPR?",
				"AT+DBL?",
				"AT+NID?",
				"AT+MOD?",
				"AT+LRS?",
				"AT+CHA?",
				"AT+RID?",
				"AT+TID?",
				"AT+ITV?",
				"AT+PAR?",
				"AT+STO?",
				"AT+SLE?",
		},
		
		//��ʼ�����ò���(�ӻ�1)
		{
				"AT+IPR=9",                 //������ 9-115200
				"AT+DBL=1",                 //���Եȼ� 1
				"AT+NID=8888",              //�豸ID
				"AT+MOD=0",                 //����ģʽ 0-͸��
				"AT+LRS=3",                 //�������� 3
				"AT+CHA=24",                //����Ƶ�� 24-433MHz
				"AT+RID=1000",              //�м̵�ַ
				"AT+TID=1001",              //͸����ַ
				"AT+ITV=20",                //����ʱ����
				"AT+PAR=0",                 //У��λ 0-NONE
				"AT+STO=0",                 //ֹͣλ 0-1��ֹͣλ 1-2
				"AT+SLE=0",                 //����ģʽ 0-NONE 1-TIME 2-DEEP
		},
		
		//��ǰloraģ������
		{
			  "+IPR:",                    //������ 9-115200
			  "+DBL:",                    //���Եȼ� 1
				"+NID:",                    //�豸ID
				"+MOD:",                    //����ģʽ 0-͸��
				"+LRS:",                    //�������� 3
				"+CHA:",                    //����Ƶ�� 24-433MHz
				"+RID:",                    //�м̵�ַ
				"+TID:",                    //͸����ַ
				"+ITV:",                    //����ʱ����
				"+PAR:",                    //У��λ 0-NONE
				"+STO:",                    //ֹͣλ 0-1��ֹͣλ 1-2
				"+SLE:",                    //����ģʽ 0-NONE
		},
};

StLoraParameter g_stLoraParameter[] =
{
		{9, 1, 8888, 0, 3, 24, 1000, 1001, 20, 0, 0, 0},  //Ĭ������
		{0},                                              //���洢��ǰ����
	
};




/*
 * ������      : GetLoraConfigInfo
 * ����        : ��������Ϣ���ַ�������ʽ��β����һ����������
 * ����        : u8Data[out]: ������Ӧ���㹻��(128����)
 * ����        : -1ʧ�� >0 ����
 * ����        : lvchunhao 2017-11-07
 */
int32_t GetLoraConfigInfo(uint8_t *u8Data)
{
		if(u8Data == NULL)
		{
				return -1;
		}
		
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8IPRCmd);
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8DBLCmd);
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8NIDCmd);
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8MODCmd);
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8LRSCmd);
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8CHACmd);
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8RIDCmd);
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8TIDCmd);
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8ITVCmd);
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8PARCmd);
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8STOCmd);
		strcat((char *)u8Data, (char *)s_StLoraConfig[2].u8SLECmd);
		
		return(strlen((char *)u8Data));
		
}
/*
 * ������      : LoraConfigPrintIndex
 * ����        : s_StLoraConfig��ӡ����
 * ����        : u8Index[in]: ����
 * ����        : void
 * ����        : lvchunhao 2017-11-07
 */
void LoraConfigPrintIndex(uint8_t u8Index)
{
		
	  PRINT("===> u8Index = %d \n", u8Index);
	
	  if(u8Index > (sizeof(s_StLoraConfig) / sizeof(s_StLoraConfig[0])))
		{
				return;
		}
	
		PRINT("%s\n", s_StLoraConfig[u8Index].u8IPRCmd);
	  PRINT("%s\n", s_StLoraConfig[u8Index].u8DBLCmd);
		PRINT("%s\n", s_StLoraConfig[u8Index].u8NIDCmd);
		PRINT("%s\n", s_StLoraConfig[u8Index].u8MODCmd);
		PRINT("%s\n", s_StLoraConfig[u8Index].u8LRSCmd);
		PRINT("%s\n", s_StLoraConfig[u8Index].u8CHACmd);
		PRINT("%s\n", s_StLoraConfig[u8Index].u8RIDCmd);
		PRINT("%s\n", s_StLoraConfig[u8Index].u8TIDCmd);
		PRINT("%s\n", s_StLoraConfig[u8Index].u8ITVCmd);
		PRINT("%s\n", s_StLoraConfig[u8Index].u8PARCmd);
		PRINT("%s\n", s_StLoraConfig[u8Index].u8STOCmd);
		PRINT("%s\n", s_StLoraConfig[u8Index].u8SLECmd);
}
/*
 * ������      : UpgradeLoraConfig
 * ����        : s_StLoraConfig[2]��ΪLora���õ�ǰ����
 * ����        : u8Data[in]: ��Ҫ���µ�����,���ʽ���ݼ���
 * ����        : 0 - success -1 - failed
 * ����        : lvchunhao 2017-11-07
 */
int32_t UpgradeLoraConfig(uint8_t *u8Data, uint32_t u32DataLen)
{
		char c8Data[8] = {0};
		uint32_t u32Value = 0;
	
		if(u8Data == NULL)
		{
				return -1;
		}
		
		/**********************************************************************  
		��ѯĳ�������ķ���ֵΪ��u8Data
		AT+IPR?  0D 0A 2B 49 50 52 3A 39 0D 0A 4F 4B 0D 0A   /r/n+IPR:9/r/nOK/r/n
		AT+DBL?  0D 0A 2B 44 42 4C 3A 31 0D 0A 4F 4B 0D 0A   /r/n+DBL:1/r/nOK/r/n

		�ڶ��֣� AT+TID=8888
		�˺�����������
		***********************************************************************/
		
		if((u8Data[0] == 0x0D) && (u8Data[1] == 0x0A))
		{
				if(u32DataLen <= 13)
			  {
						return -1;
				}
			
			  if(strncmp((char *)u8Data + 2, "+IPR:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8IPRCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8IPRCmd, "+IPR:");
					  memcpy((char *)s_StLoraConfig[2].u8IPRCmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32IPR = u32Value;
				}
				else if(strncmp((char *)u8Data + 2, "+DBL:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8DBLCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8DBLCmd, "+DBL:");
					  memcpy((char *)s_StLoraConfig[2].u8DBLCmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32DBL = u32Value;
				}
				else if(strncmp((char *)u8Data + 2, "+NID:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8NIDCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8NIDCmd, "+NID:");
					  memcpy((char *)s_StLoraConfig[2].u8NIDCmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32NID = u32Value;
				}
				else if(strncmp((char *)u8Data + 2, "+MOD:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8MODCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8MODCmd, "+MOD:");
					  memcpy((char *)s_StLoraConfig[2].u8MODCmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32MOD = u32Value;
				}
				else if(strncmp((char *)u8Data + 2, "+LRS:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8LRSCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8LRSCmd, "+LRS:");
					  memcpy((char *)s_StLoraConfig[2].u8LRSCmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32LRS = u32Value;
				}
				else if(strncmp((char *)u8Data + 2, "+CHA:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8CHACmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8CHACmd, "+CHA:");
					  memcpy((char *)s_StLoraConfig[2].u8CHACmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32CHA = u32Value;
				}
				else if(strncmp((char *)u8Data + 2, "+RID:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8RIDCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8RIDCmd, "+RID:");
					  memcpy((char *)s_StLoraConfig[2].u8RIDCmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32RID = u32Value;
				}
				else if(strncmp((char *)u8Data + 2, "+TID:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8TIDCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8TIDCmd, "+TID:");
					  memcpy((char *)s_StLoraConfig[2].u8TIDCmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32TID = u32Value;
				}
				else if(strncmp((char *)u8Data + 2, "+ITV:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8ITVCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8ITVCmd, "+ITV:");
					  memcpy((char *)s_StLoraConfig[2].u8ITVCmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32ITV = u32Value;
				}
				else if(strncmp((char *)u8Data + 2, "+PAR:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8PARCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8PARCmd, "+PAR:");
					  memcpy((char *)s_StLoraConfig[2].u8PARCmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32PAR = u32Value;
				}
				else if(strncmp((char *)u8Data + 2, "+STO:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8STOCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8STOCmd, "+STO:");
					  memcpy((char *)s_StLoraConfig[2].u8STOCmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32STO = u32Value;
				}
				else if(strncmp((char *)u8Data + 2, "+SLE:", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8SLECmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8SLECmd, "+SLE:");
					  memcpy((char *)s_StLoraConfig[2].u8SLECmd + 5, (char *)u8Data + 7, u32DataLen - 13);
					
					  memset(c8Data, 0, 8);
					  memcpy(c8Data, (char *)u8Data + 7, u32DataLen - 13);
					  sscanf(c8Data, "%d", &u32Value);
					  g_stLoraParameter[1].u32SLE = u32Value;
				}
				else
				{
						return -1;
				}
				
		}
		else
		{
				if(u32DataLen <= 7)
			  {
						return -1;
				}
			
			  if(strncmp((char *)u8Data + 2, "+IPR=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8IPRCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8IPRCmd, "+IPR:");
					  memcpy((char *)s_StLoraConfig[2].u8IPRCmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else if(strncmp((char *)u8Data + 2, "+DBL=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8DBLCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8DBLCmd, "+DBL:");
					  memcpy((char *)s_StLoraConfig[2].u8DBLCmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else if(strncmp((char *)u8Data + 2, "+NID=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8NIDCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8NIDCmd, "+NID:");
					  memcpy((char *)s_StLoraConfig[2].u8NIDCmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else if(strncmp((char *)u8Data + 2, "+MOD=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8MODCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8MODCmd, "+MOD:");
					  memcpy((char *)s_StLoraConfig[2].u8MODCmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else if(strncmp((char *)u8Data + 2, "+LRS=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8LRSCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8LRSCmd, "+LRS:");
					  memcpy((char *)s_StLoraConfig[2].u8LRSCmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else if(strncmp((char *)u8Data + 2, "+CHA=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8CHACmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8CHACmd, "+CHA:");
					  memcpy((char *)s_StLoraConfig[2].u8CHACmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else if(strncmp((char *)u8Data + 2, "+RID=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8RIDCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8RIDCmd, "+RID:");
					  memcpy((char *)s_StLoraConfig[2].u8RIDCmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else if(strncmp((char *)u8Data + 2, "+TID=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8TIDCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8TIDCmd, "+TID:");
					  memcpy((char *)s_StLoraConfig[2].u8TIDCmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else if(strncmp((char *)u8Data + 2, "+ITV=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8ITVCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8ITVCmd, "+ITV:");
					  memcpy((char *)s_StLoraConfig[2].u8ITVCmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else if(strncmp((char *)u8Data + 2, "+PAR=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8PARCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8PARCmd, "+PAR:");
					  memcpy((char *)s_StLoraConfig[2].u8PARCmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else if(strncmp((char *)u8Data + 2, "+STO=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8STOCmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8STOCmd, "+STO:");
					  memcpy((char *)s_StLoraConfig[2].u8STOCmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else if(strncmp((char *)u8Data + 2, "+SLE=", 5) == 0)
				{
						memset(s_StLoraConfig[2].u8SLECmd, 0, 16);
					  strcat((char *)s_StLoraConfig[2].u8SLECmd, "+SLE:");
					  memcpy((char *)s_StLoraConfig[2].u8SLECmd + 5, (char *)u8Data + 7, u32DataLen - 7);
				}
				else
				{
						return -1;
				}
		}
		
		return 0;
}

/*
 * ������      : LoRa_WaitOnCFGResultUntilTimeout
 * ����        : Loraģ�����ý���ĵȴ�
 * ����        : Timeout[in]: ��ʱʱ��1000ms
 *             : 
 * ����        : 	 RESULT_INIT = 0x00,
									 RESULT_OK,
									 RESULT_FAILED,
									 RESULT_TIMEOUT,
 * ����        : lvchunhao 2017-11-15
 */
int32_t LoRa_WaitOnCFGResultUntilTimeout(uint32_t Timeout)
{
  
  /* Wait until flag is set */
	while(Timeout--)
	{
		  /* Check for the Timeout */
			if(g_u8LoraConfigResultOK != RESULT_INIT)
			{		
					return g_u8LoraConfigResultOK;
			}
			else
			{
					vTaskDelay(1000 / portTICK_RATE_MS);
			}
	}

  return RESULT_TIMEOUT;      
}
/*
 * ������      : LoRa_WaitOnSlaveReplayFlagUntilTimeout
 * ����        : Lora��ģ�������ظ�(���ӻ�ͨѶ)
 * ����        : Timeout[in]: ��ʱʱ��10ms
 *             : 
 * ����        : ��ȷ����0, ���󷵻�-1
 * ����        : lvchunhao 2017-11-16
 */
int32_t LoRa_WaitOnSlaveReplayFlagUntilTimeout(uint32_t Timeout)
{
  
  /* Wait until flag is set */
	while(Timeout--)
	{
		  /* Check for the Timeout */
			if(g_u8LoraRecvSlaveStateReply == SET)
			{		
					return 0;
			}
			else
			{
					vTaskDelay(10 / portTICK_RATE_MS);
			}
	}

  return -1;      
}
/*
 * ������      : LoRa_WaitOnFlagUntilTimeout
 * ����        : Loraģ�������ظ�(stm32--Loraģ��)
 * ����        : Timeout[in]: ��ʱʱ��10ms
 *             : 
 * ����        : ��ȷ����0, ���󷵻�-1
 * ����        : lvchunhao 2017-9-12
 */
int32_t LoRa_WaitOnFlagUntilTimeout(uint32_t Timeout)
{
  
  /* Wait until flag is set */
	while(Timeout--)
	{
		  /* Check for the Timeout */
			if(g_u8LoraModuleReplyOK == SET)
			{		
					return 0;
			}
			else
			{
					vTaskDelay(10 / portTICK_RATE_MS);
			}
	}

  return -1;      
}
/*
 * ������      : LoraModuleSingleConfig
 * ����        : Loraģ�������
 * ����        : u8Command [in] : AT����
 *             : u32DataLen [in] : AT�����
 * ����        : ��ȷ����0, ���󷵻�-1
 * ����        : lvchunhao 2017-9-12
 */
int32_t LoraModuleSingleConfig(uint8_t *u8Command, uint32_t u32DataLen)
{
	  int32_t s32Err = 0;
    char u8ATCMD[64] = {0};
	
		if(!u8Command || 0 == u32DataLen)
    {
    	return -1;
    }
		
		/* ����+++,LoRaģ�������ʱ����ģʽ */
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)LORA_AT_MODE, strlen(LORA_AT_MODE), TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				return -1;
		}
		
		/* ��ʽ��"AT+NID=1123/r/n" */
		memcpy(u8ATCMD, u8Command, u32DataLen);
		u8ATCMD[u32DataLen]     = 0x0d;      //�س�/r
		u8ATCMD[u32DataLen + 1] = 0x0a;      //����/n
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u32DataLen + 2, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
err:
		/*  ����LoRaģ�� */
    vTaskDelay(100/portTICK_RATE_MS);		
		g_u8LoraModuleReplyOK = RESET;
		HAL_GPIO_WritePin(Lora_RST_GPIO_Port, Lora_RST_Pin, GPIO_PIN_RESET);
		vTaskDelay(250/portTICK_RATE_MS);
		HAL_GPIO_WritePin(Lora_RST_GPIO_Port, Lora_RST_Pin, GPIO_PIN_SET);
		vTaskDelay(2000/portTICK_RATE_MS);

		return s32Err;
}

/*
 * ������      : LoraModuleMultipConfig
 * ����        : Loraģ�������
 * ����        : u8Index [in] : s_StLoraConfig[u8Index]
 * ����        : ��ȷ����0, ���󷵻�-1
 * ����        : lvchunhao 2017-9-12
 */
int32_t LoraModuleMultipConfig(uint8_t u8Index)
{
		int32_t s32Err = 0;
		uint8_t u8ATCMDLen;
		char u8ATCMD[64] = {0};
		int32_t s32ATModTimes = 5;
		
		/* ����+++,LoRaģ�������ʱ����ģʽ */
		while(s32ATModTimes--)
		{
			  g_u8LoraModuleReplyOK = RESET;
				PostUartWriteMsg((uint8_t *)LORA_AT_MODE, strlen(LORA_AT_MODE), TYPE_UART_LORA);	
				if(LoRa_WaitOnFlagUntilTimeout(300) == 0)  
				{ 
						break;
				}
		}
		
		if(s32ATModTimes <= 0)
		{
				return -1;
		}
		
//		g_u8LoraModuleReplyOK = RESET;
//		PostUartWriteMsg((uint8_t *)LORA_AT_MODE, strlen(LORA_AT_MODE), TYPE_UART_LORA);	
//		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
//		{ 
//				return -1;
//		}
		
		/* 1.������ */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8IPRCmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8IPRCmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
		/* 2.���Եȼ� */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8DBLCmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8DBLCmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
		/* 3.�豸ID */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8NIDCmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8NIDCmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
	  /* 4.����ģʽ */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8MODCmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8MODCmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
		/* 5.�������� */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8LRSCmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8LRSCmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
		/* 6.����Ƶ�� */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8CHACmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8CHACmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
		/* 7.�м̵�ַ */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8RIDCmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8RIDCmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
		/* 8.͸����ַ */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8TIDCmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8TIDCmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
		/* 9.����ʱ���� */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8ITVCmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8ITVCmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
	  /* 10.У��λ */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8PARCmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8PARCmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
		/* 11.ֹͣλ */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8STOCmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8STOCmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}
		
		/* 12.����ģʽ */
		memset(u8ATCMD, 0, 64);
		u8ATCMDLen = strlen((char *)s_StLoraConfig[u8Index].u8SLECmd);
		if(u8ATCMDLen < 7)
		{
				s32Err = -1;
			  goto err;
		}	
		strcat(u8ATCMD, (char *)s_StLoraConfig[u8Index].u8SLECmd);
		u8ATCMD[u8ATCMDLen]     = 0x0d;      //�س�/r
		u8ATCMD[u8ATCMDLen + 1] = 0x0a;      //����/n
		u8ATCMDLen += 2;
		g_u8LoraModuleReplyOK = RESET;
		PostUartWriteMsg((uint8_t *)u8ATCMD, u8ATCMDLen, TYPE_UART_LORA);
		if(LoRa_WaitOnFlagUntilTimeout(300) != 0)  
		{ 
				s32Err = -1;
			  goto err;
		}	
		
err:
		/*  ����LoRaģ�� */
    vTaskDelay(100/portTICK_RATE_MS);		
		g_u8LoraModuleReplyOK = RESET;
		HAL_GPIO_WritePin(Lora_RST_GPIO_Port, Lora_RST_Pin, GPIO_PIN_RESET);
		vTaskDelay(250/portTICK_RATE_MS);
		HAL_GPIO_WritePin(Lora_RST_GPIO_Port, Lora_RST_Pin, GPIO_PIN_SET);
		vTaskDelay(3000/portTICK_RATE_MS);

		return s32Err;
}

/*
 * ������      : PostDataProcessMsg
 * ����        : Ͷ��һ��DataProcessд��Ϣ
 * ����        : pData [in] : ��Ҫд������ָ��
 *             : u32DataLen [in] : ���ݳ���
 * ����        : ��ȷ����0, ���󷵻�-1
 * ����        : lvchunhao 2017-7-27
 */
int32_t PostDataProcessMsg(uint8_t *pData, uint32_t u32DataLen, uint8_t u8MsgType)
{
    StMsgPkg stPkg;
    uint8_t *pDataCopy = NULL;
		portBASE_TYPE s32Ret;

    if(!pData || 0 == u32DataLen)
    {
    	return -1;
    }

    if(NULL == s_s32MsgQueID)
    {
        return -1;
    }

    pDataCopy = (uint8_t *)pvPortMalloc(MSG_BUFF_SIZE);   //�����ڴ�256Bytes
    if(NULL == pDataCopy)
    {
    	return -1;
    }
    else
    {
    	memcpy(pDataCopy, pData, u32DataLen);
    }

    memset(&stPkg, 0, sizeof(stPkg));
    stPkg.pData = pDataCopy;
    stPkg.u32DataLen = u32DataLen;
		stPkg.u8MsgType = u8MsgType;

		s32Ret = xQueueSendToBack(s_s32MsgQueID, (const void *)(&stPkg), 0);
    if(s32Ret != pdPASS)
    {
        //���Ͷ˶�̬����ռ䣬���ת��ʧ�ܣ����ͷſռ�
        if(stPkg.pData)
        {
            vPortFree(stPkg.pData);
        }
        return -1;
    }

    return 0;
}
/*
 * ������      : vTaskDataProcess
 * ����        : ���ݴ����߳�
 * ����        : pArgs [in] : �̲߳���
 * ����        : ��
 * ����        : lvchunhao  2017-9-12
 */
void vTaskDataProcess(void *pArgs)
{
		int32_t s32Err = 0;
		StTaskObj *pTaskObj = (StTaskObj *)pArgs;
	  StMsgPkg stMsg;
		portBASE_TYPE s32Ret;

		uint8_t u8Index = 0;
		s_s32MsgQueID = pTaskObj->s32MsgQueID;
	
		int32_t s32SendLen = 0;
	
		while(1)
		{
				s32Ret = xQueueReceive(s_s32MsgQueID, &stMsg, portMAX_DELAY);
				if(s32Ret != pdPASS)
				{
					continue;
				}
				
				/* �������ݴ��� */
				switch(stMsg.u8MsgType)
				{
						case TYPE_LORA_SET_MULTIP_CFG:
						     //ִ��Ĭ��Lora��������(���ã�u8Index = 1)  
						     u8Index = stMsg.pData[0];
						     if(u8Index >= sizeof(s_StLoraConfig) / sizeof(s_StLoraConfig[0]))
								 {
										 #if PRINT_STRING
									   PRINT("u8Index error \n");
									   #endif
									   g_u8LoraConfigResultOK = RESULT_FAILED;
								 }
								 else
								 {
										 s32Err = LoraModuleMultipConfig(u8Index);
										 if(s32Err == 0)
										 {
												#if PRINT_STRING
												PRINT("Lora Multip config success \n");
												#endif
												g_u8LoraConfigResultOK = RESULT_OK;
										 }
										 else
										 {
												#if PRINT_STRING
												PRINT("Lora Multip config failed \n");
												#endif
												g_u8LoraConfigResultOK = RESULT_FAILED;
										 }
								 }
								 
                 break;

						case TYPE_LORA_SET_SINGLE_CFG:
							   s32Err = LoraModuleSingleConfig(stMsg.pData, stMsg.u32DataLen);
						     if(s32Err == 0)
								 {
										//���óɹ�
									  #if PRINT_STRING
									  PRINT("Lora Single config success \n");
									  #endif
									  g_u8LoraConfigResultOK = RESULT_OK;
									  /* �������������,�����óɹ������Lora����ȫ�ֲ��� */
									  /* �������AT+TID=8888 ��ѯ���AT+TID? */
									  UpgradeLoraConfig(stMsg.pData, stMsg.u32DataLen);
								 }
								 else
								 {
								    #if PRINT_STRING
									  PRINT("Lora Single config failed \n");
									  #endif
									  g_u8LoraConfigResultOK = RESULT_FAILED;
								 }
							   break;
						
						default :
							   break;
						
				}
				
			  //��Щ�ظ��޷�����Lora�̴߳���,����ڴ˴��ظ�
			  if(g_u8LoraReplyAppFlag == REPLY_CMD_XXCB)
			  {
					 if(g_u8LoraConfigResultOK == RESULT_OK)
					 {
								#if PRINT_STRING
								memset(u8DataTxBuff, 0, MAX_SEND_SIZE);
								s32SendLen = GetLoraConfigInfo(u8DataTxBuff);
								PostUartWriteMsg(u8DataTxBuff, s32SendLen, TYPE_UART_RS232);
								#else
								memset(u8DataTxBuff, 0, MAX_SEND_SIZE);
								u8DataTxBuff[0] = 0xF5;     //Э��ͷ
								u8DataTxBuff[1] = 0xB0;     //Э������
								u8DataTxBuff[2] = 0x0A;     //����
								u8DataTxBuff[3] = 0xCB;     //����ID
							 
								u8DataTxBuff[4] = 0x00;     //״̬ 0 - �ɹ�
								u8DataTxBuff[5] = g_stLoraParameter[1].u32NID / 256;   //�豸ID
								u8DataTxBuff[6] = g_stLoraParameter[1].u32NID % 256;
								u8DataTxBuff[7] = g_stLoraParameter[1].u32TID / 256;   //͸��ID
								u8DataTxBuff[8] = g_stLoraParameter[1].u32TID % 256;
								u8DataTxBuff[9] = g_stLoraParameter[1].u32LRS;         //��������
								u8DataTxBuff[10]= g_stLoraParameter[1].u32MOD;         //����Э��
								u8DataTxBuff[11]= g_stLoraParameter[1].u32SLE;         //˯��ģʽ
								u8DataTxBuff[12]= g_stLoraParameter[1].u32CHA;         //�����ŵ�
							 
								s32SendLen = 13;
								PostUartWriteMsg(u8DataTxBuff, s32SendLen, TYPE_UART_LORA);
								#endif
					 }
					 else
					 {
								#if PRINT_STRING
								PRINT("Get config failed \n");
								#else
								memset(u8DataTxBuff, 0, MAX_SEND_SIZE);
								u8DataTxBuff[0] = 0xF5;
								u8DataTxBuff[1] = 0xB0;
								u8DataTxBuff[2] = 0x02;
								u8DataTxBuff[3] = 0xCB;
								u8DataTxBuff[4] = 0x01;
							 
								s32SendLen = 5;
								PostUartWriteMsg(u8DataTxBuff, s32SendLen, TYPE_UART_LORA);
								#endif
					 }
					 
					 g_u8LoraReplyAppFlag = REPLY_CMD_NONE;
				
			  }
				
				/* �ͷŻ����� */
				if(stMsg.pData)
				{
						vPortFree(stMsg.pData);
						stMsg.pData = NULL;
						stMsg.u32DataLen = 0;
				}
			
		}
		
		
}

