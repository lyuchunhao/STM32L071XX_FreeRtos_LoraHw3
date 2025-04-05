/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  lora_task.c
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��8��2��
 * ����                 :  LoraͨѶ 
                           1. ����STM32����Uart5-Lora
													 2. Ĭ�ϳ�ʼ���˽����ж�/����DMA�Լ�����DMA
													 3. ���Է��֣����͹��춼�����һ���̶ȵĶ���
													 4. FreeRTOS���ͺͽ�����ò�Ҫ���ò�ѯ��ʽ���������������
													 5. �Ƽ����жϽ���/DMA����
													 6. Uart5����-�жϽ���/��ѯ���� (vTaskUart1Read)
                              HAL_UART_Receive_IT��HAL_UART_Transmit�����ƺ���Щ����,
															ͬʱ���û���ɽ���ȥ�����жϡ���˸ĳ��Լ�����Ľ��պ�
															���ͺ�����������
													 7. 2017-8-5�޸ĳ��Զ����жϽ���/��ѯ����
 ****************************************************************************/

#include "common.h"


uint8_t g_u8LoraRecvCompleteFlag = 0;         //1-������� 0-û����ɼ�������
uint8_t u8LoraRxBuff[MAX_RECV_SIZE];
uint8_t u8LoraTxBuff[MAX_SEND_SIZE];
uint8_t u8LoraKpBuff[MAX_RECV_SIZE];
static uint32_t u32LoraWriteIndex = 0;

uint8_t g_u8LoraSlaveConfig[32];              //�ӻ�������Ϣ
uint8_t g_u8LoraRecvSlaveStateReply = RESET;  //�Ƿ��յ��ӻ�����ͨѶ����״̬�Ļظ�
uint8_t g_u8LoraModuleReplyOK = RESET;        //�յ�Loraģ��"OK"�ظ���־
uint8_t g_u8LoraConfigResultOK= RESULT_INIT;  //Loraģ�����ý��,��Ҫ���ݴ˱�������λ���ظ�
uint8_t g_u8LoraReplyAppFlag = REPLY_CMD_NONE;//�ӻ��յ���������Lora������Ϣ��ѯ��,�ظ����ܷ���Lora�̴߳���


/**
  * ��������: Loraģ�鸴λ
	* �������: void
  * �� �� ֵ: ��
  * ˵    ����������Ҫ200ms
  */
void LoraModuleReset(void)
{
		LORA_RST_RESET;
	  vTaskDelay(250 / portTICK_RATE_MS);
	  LORA_RST_SET;
}
/*
 * ������      : Rs232WriteIndexClearZero
 * ����        : Rs232дָ������
 * ����        : void
 * ����        : void
 * ����        : lvchunhao  2017-11-14
 */
void LoraWriteIndexClearZero(void)
{
		u32LoraWriteIndex = 0;
}
/*
 * ������      : GetRs232WriteIndex
 * ����        : Rs232ָ���ȡ
 * ����        : void
 * ����        : ָ��ֵ
 * ����        : lvchunhao  2017-11-14
 */
uint32_t GetLoraWriteIndex(void)
{
		return u32LoraWriteIndex;
}
/*
 * ������      : Rs232RecvOneData
 * ����        : ���жϴ�����One�ֽڴ��뻺����
 * ����        : u8Bytes [in] : ����
 * ����        : ��
 * ����        : lvchunhao  2017-11-14
 */
void LoraRecvOneData(uint8_t u8Bytes)
{
		if(u32LoraWriteIndex == MAX_RECV_SIZE)
		{
				u32LoraWriteIndex = 0;
		}
		
		u8LoraRxBuff[u32LoraWriteIndex++] = u8Bytes;
}
/*
 * ������      : GetUnReadDataFromRecvBuff
 * ����        : ����ѭ���������ж�ȡ����(ָ���������ں������)
 * ����        : u8Read [in] : ��ָ��
 * ����				 ��u8Write[in] : дָ��
 * ����	       ��u8RvBufStartAddr[in]: ���ջ�������ʼ��ַ
 * ����				 ��u32RvMAXSize[in]:���ջ�������С
 * ����				 ��u8Data[out]: ���������
 *             : u32DataLen [in] : ���ݳ���
 * ����        : >=0�����ֽ���, ���󷵻�-1
 * ����        : lvchunhao 2017-7-27
 */
int32_t GetUnReadDataFromRecvBuff(uint8_t *u8Read, uint8_t *u8Write, uint8_t *u8RvBufStartAddr, 
                                  uint32_t u32RvMAXSize, uint8_t *u8Data)
{
		int32_t  s32Return = 0;
	  uint32_t u32TmpLen = 0;
	  uint32_t u32RecvLen= 0;
	
		if((u8Read == NULL) || (u8Write == NULL) || (u8RvBufStartAddr == NULL) || (u8Data == NULL))
		{
				return -1;
		}
		
		if(u8Read == u8Write)
		{
				s32Return = 0;
		}
		else if(u8Write > u8Read)
		{
				u32RecvLen = u8Write - u8Read;
			  memcpy(u8Data, u8Read, u32RecvLen);
			  s32Return = u32RecvLen;
		}
		else
		{
				u32TmpLen = u8RvBufStartAddr + u32RvMAXSize - u8Read;
			  memcpy(u8Data, u8Read, u32TmpLen);
			  memcpy(u8Data + u32TmpLen, u8RvBufStartAddr, u8Write - u8RvBufStartAddr);
			  u32RecvLen = u32TmpLen + (u8Write - u8RvBufStartAddr);
			  s32Return = u32RecvLen;
		}
		
		return s32Return;
}
/*
 * ������      : vTaskLoraCommunicate
 * ����        : Uart5������
 * ����        : pArgs [in] : �̲߳���
 * ����        : ��
 * ����        : lvchunhao  2017-7-25
 */
void vTaskLoraCommunicate(void *pArgs)
{

	  uint8_t u8RunCmd = 0;
	  int32_t s32SendLen = 0;
	  int32_t s32RecvLen = 0;
	
	  //Lora���ڽ����ж�ʹ��
	  LORA_IT_ENABLE;
	  //LoraModuleReset(); 
	
	  uint8_t u8Index = 0;
	  uint8_t u8State = 0;
	
		uint32_t ADC_CnvtValue[ADC_NUMOFCHANNEL];
	  __IO float ADC_CnvtValueLocal[ADC_NUMOFCHANNEL];
	
		while(1)
		{
				/******************** 115200������ÿmsԼ����12Bytes ****************/
				s32RecvLen = GetLoraWriteIndex();
			  if(s32RecvLen == 0)
				{
						vTaskDelay(8 / portTICK_RATE_MS);
				}
				else
				{
						vTaskDelay(1 / portTICK_RATE_MS);
					  if(s32RecvLen == GetLoraWriteIndex())
						{
								/* ��ʱ1ms���дָ��û�иı���Ϊ������� */
							  g_u8LoraRecvCompleteFlag = 1;
						}
						else
						{
								/* �������� */
							  s32RecvLen = GetLoraWriteIndex();
							  if(s32RecvLen > MAX_PTL_BYTES)
								{
										g_u8LoraRecvCompleteFlag = 1;
								}
						}
					
				}
				/*******************************************************************/
				
				/*********************** ������յ������� **************************/
				if(g_u8LoraRecvCompleteFlag == 1)
				{
						/* �����ж��н��յ������ݸ��Ƶ����������� */
					  memset(u8LoraKpBuff, 0, MAX_RECV_SIZE);
					  memcpy(u8LoraKpBuff, u8LoraRxBuff, s32RecvLen);
					
					  /* ���дָ�����ɱ�־�ٴν��� */
					  LoraWriteIndexClearZero();
					  g_u8LoraRecvCompleteFlag = 0;
					
					  /* ����Э����� */
						if((strlen((char *)u8LoraKpBuff) >= 6) 
							&& (strncmp((char *)u8LoraKpBuff + strlen((char *)u8LoraKpBuff) - 4, "OK", 2) == 0))
						{
								//LoRa����ʱ�ж�AT����Ļظ�(��������ֽ�ΪOK)
							  /************************************************************************  
								��ѯĳ�������ķ���ֵΪ��u8Data
								AT+IPR?  0D 0A 2B 49 50 52 3A 39 0D 0A 4F 4B 0D 0A   /r/n+IPR:9/r/nOK/r/n
								AT+DBL?  0D 0A 2B 44 42 4C 3A 31 0D 0A 4F 4B 0D 0A   /r/n+DBL:1/r/nOK/r/n
							  *************************************************************************/

							  #if PRINT_STRING
								PRINT("%s",u8LoraKpBuff);
								#endif
							  g_u8LoraModuleReplyOK = SET;
							  UpgradeLoraConfig(u8LoraKpBuff, s32RecvLen);
							
						}
						else if((s32RecvLen >= 4) && (u8LoraKpBuff[PTL_HEAD_OFFSET] == PTL_HEAD_RS232))
						{
								u8RunCmd = u8LoraKpBuff[PTL_CMD_OFFSET];
							
								if(u8LoraKpBuff[PTL_TYPE_OFFSET] == PTL_TYPE_SLAVE)
								{
										/* ���յ�����������Ϊ:0xB0,����Ϊ�Ǵӻ�������������*/
									  switch(u8RunCmd)
										{
											  case 0xCA:
													   //�յ��ӻ�0xCA���ñ�־λ,��Rs232�лظ���λ��
											       g_u8LoraRecvSlaveStateReply = SET;
												     break;
												
												case 0xCB:
													   //�����յ��ӻ����õĻظ�data:state+NID+TID+LRS+MOD+SLE+CHA
												     g_u8LoraRecvSlaveStateReply = SET;
												     u8State = u8LoraKpBuff[4];
												     memset(g_u8LoraSlaveConfig, 0xFF, 32);
												     if(u8State == 0)
														 {
																memcpy(g_u8LoraSlaveConfig, u8LoraKpBuff + 4, 9);
														 }									   
												     break;
														 
												case 0xCC:
													   //�����յ��ӻ���ѹ�Ļظ�
												     g_u8LoraRecvSlaveStateReply = SET;
												     memset(g_u8LoraSlaveConfig, 0xFF, 32);
												     memcpy(g_u8LoraSlaveConfig, u8LoraKpBuff + 4, s32RecvLen - 4);
													   break;
												
												case 0xCD:
													   //�����յ��ӻ������Ļظ�
												     g_u8LoraRecvSlaveStateReply = SET;
												     memset(g_u8LoraSlaveConfig, 0xFF, 32);
												     memcpy(g_u8LoraSlaveConfig, u8LoraKpBuff + 4, s32RecvLen - 4);
													   break;
												
												case 0xCE:
												     break;
															 
												default :
													   break;
										}
								}
								else if(u8LoraKpBuff[PTL_TYPE_OFFSET] == PTL_TYPE_MASTER)
								{
									  /* ���յ�����������Ϊ:0xA0,����Ϊ�Ǵ����������������� */
										switch(u8RunCmd)
										{
												case 0xCA:
													   //���յ���������ͨѶ���ӵ�����
														 #if PRINT_STRING
														 PRINT("Recv build connect cmd 0xCA \n");
														 #else
														 memset(u8LoraTxBuff, 0, MAX_SEND_SIZE);
														 u8LoraTxBuff[0] = 0xF5;
														 u8LoraTxBuff[1] = 0xB0;
														 u8LoraTxBuff[2] = 0x02;
														 u8LoraTxBuff[3] = 0xCA;
														 u8LoraTxBuff[4] = 0x00;
													 
														 s32SendLen = 5;
														 PostUartWriteMsg(u8LoraTxBuff, s32SendLen, TYPE_UART_LORA);
														 #endif
														 break;
												
												case 0xCB:
													   //���յ�������������->�ظ�
												     //���͵������̲߳�ѯLora����
														 u8Index = 0;
														 PostDataProcessMsg(&u8Index, 1, TYPE_LORA_SET_MULTIP_CFG);
														 #if PRINT_STRING
														 PRINT("Read Lora config:\n");
														 #endif
												
												 		 //�ȴ����ý���ظ���λ��,����Data�̻߳ظ�(������ڴ˴�����Loraʱ���������)
														 g_u8LoraConfigResultOK= RESULT_INIT;
														 g_u8LoraReplyAppFlag = REPLY_CMD_XXCB;
												
												     break;
														 
												case 0xCC:
													   //���յ������Դӻ���ѹ��ѯ��
														 /* ����һ��ADCת��,��ȡValueֵ */
														 GetADCValue(&hadc, ADC_CnvtValue);
														 ADC_CnvtValueLocal[0] =(float)(ADC_CnvtValue[0]&0xFFF)*3.3/4096*3; 
														 ADC_CnvtValueLocal[1] =(float)(ADC_CnvtValue[1]&0xFFF)*3.3/4096*10; 
												
												     #if PRINT_STRING
														 PRINT("Recv valtage ask cmd 0xCC \n");
														 PRINT("ADC_CH6 value = %d --> %fV\n", ADC_CnvtValue[0]&0xFFF, 
																				ADC_CnvtValueLocal[0]);
														 #else
														 memset(u8LoraTxBuff, 0, MAX_SEND_SIZE);
														 u8LoraTxBuff[0] = 0xF5;
														 u8LoraTxBuff[1] = 0xB0;
														 u8LoraTxBuff[2] = 0x01 + sizeof(ADC_CnvtValueLocal[0]);
														 u8LoraTxBuff[3] = 0xCC;
														 
												     memcpy(u8LoraTxBuff + 4, (char *)&ADC_CnvtValueLocal[0], sizeof(ADC_CnvtValueLocal[0]));
													 
														 s32SendLen = 4 + sizeof(ADC_CnvtValueLocal[0]);
														 PostUartWriteMsg(u8LoraTxBuff, s32SendLen, TYPE_UART_LORA);
														 #endif
													   break;
												
												case 0xCD:
													   //���յ������Դӻ�������ѯ��
														 /* ����һ��ADCת��,��ȡValueֵ */
														 GetADCValue(&hadc, ADC_CnvtValue);
														 ADC_CnvtValueLocal[0] =(float)(ADC_CnvtValue[0]&0xFFF)*3.3/4096*3; 
														 ADC_CnvtValueLocal[1] =(float)(ADC_CnvtValue[1]&0xFFF)*3.3/4096*10; 
												
												     #if PRINT_STRING
														 PRINT("Recv valtage ask cmd 0xCC \n");
														 PRINT("ADC_CH7 value = %d --> %fmA\n", ADC_CnvtValue[1]&0xFFF, 
																								ADC_CnvtValueLocal[1]);
														 #else
														 memset(u8LoraTxBuff, 0, MAX_SEND_SIZE);
														 u8LoraTxBuff[0] = 0xF5;
														 u8LoraTxBuff[1] = 0xB0;
														 u8LoraTxBuff[2] = 0x01 + sizeof(ADC_CnvtValueLocal[1]);
														 u8LoraTxBuff[3] = 0xCC;
														 
												     memcpy(u8LoraTxBuff + 4, (char *)&ADC_CnvtValueLocal[1], sizeof(ADC_CnvtValueLocal[1]));
													 
														 s32SendLen = 4 + sizeof(ADC_CnvtValueLocal[1]);
														 PostUartWriteMsg(u8LoraTxBuff, s32SendLen, TYPE_UART_LORA);
														 #endif
													   break;
												
												case 0xCE:
													   //���յ�������Rs485�ӿ����ݵķ���
												     PostUartWriteMsg(u8LoraKpBuff, s32RecvLen, TYPE_UART_RS485);
													   break;
											
												case 0xC6:
													   PostUartWriteMsg(u8LoraKpBuff, s32RecvLen, TYPE_UART_RS232);
												     break;
												
												default:
													   break;
											
										}
									 
								}
						}
						else
						{
								/* ���������������Ҫ������,���Բ����κδ�����߷��͵�Rs232�鿴 */
							  PostUartWriteMsg(u8LoraKpBuff, s32RecvLen, TYPE_UART_RS232);
						}
				}
				/*******************************************************************/
				
		}
	
	
}

