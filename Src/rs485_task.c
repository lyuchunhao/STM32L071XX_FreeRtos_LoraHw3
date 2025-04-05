/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  rs485_task.c
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��11��21��
 * ����                 :  Uart2-RS485ͨѶ
                           1. �жϽ���/��ѯ����
													 2. ���շ��͵�ʹ���Ѿ���Ӳ�������
 ****************************************************************************/
 
 #include "common.h"
 
 
 
uint8_t g_u8Rs485RecvCompleteFlag = 0;           //1-������� 0-û����ɼ�������
uint8_t u8Rs485RxBuff[MAX_RECV_SIZE];
uint8_t u8Rs485TxBuff[MAX_SEND_SIZE];
uint8_t u8Rs485KpBuff[MAX_RECV_SIZE];
static uint32_t u32Rs485WriteIndex = 0;

/*
 * ������      : Rs485WriteIndexClearZero
 * ����        : Rs232дָ������
 * ����        : void
 * ����        : void
 * ����        : lvchunhao  2017-11-14
 */
void Rs485WriteIndexClearZero(void)
{
		u32Rs485WriteIndex = 0;
}
/*
 * ������      : GetRs485WriteIndex
 * ����        : Rs232ָ���ȡ
 * ����        : void
 * ����        : ָ��ֵ
 * ����        : lvchunhao  2017-11-14
 */
uint32_t GetRs485WriteIndex(void)
{
		return u32Rs485WriteIndex;
}
/*
 * ������      : Rs485RecvOneData
 * ����        : ���жϴ�����One�ֽڴ��뻺����
 * ����        : u8Bytes [in] : ����
 * ����        : ��
 * ����        : lvchunhao  2017-11-14
 */
void Rs485RecvOneData(uint8_t u8Bytes)
{
		if(u32Rs485WriteIndex == MAX_RECV_SIZE)
		{
				u32Rs485WriteIndex = 0;
		}
		
		u8Rs485RxBuff[u32Rs485WriteIndex++] = u8Bytes;
}
 /*
 * ������      : vTaskRS485Communicate
 * ����        : RS485-Uart2���մ���
 * ����        : pArgs [in] : �̲߳���
 * ����        : ��
 * ����        : lvchunhao  2017-8-2
 */
void vTaskRS485Communicate(void *pArgs)
{
		int32_t s32RecvLen = 0;
	
		#if 0
		RTC_TimeTypeDef sTime;
		RTC_DateTypeDef sDate;
		#endif
	
	  RS485_IT_ENABLE;
	
		while(1)
		{
				#if 0
				/* ���밴��Time+Data��˳���ȡ,���򲻶�Data�Ļ�,����ֵ�ᱻ��ס */
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
				PRINT("Now: %02d - %02d - %02d - %02d\n", sDate.Year, sDate.Month, sDate.Date, sDate.WeekDay);
				PRINT("Now: %02d:%02d:%02d\n", sTime.Hours, sTime.Minutes, sTime.Seconds); 
				vTaskDelay(1000 / portTICK_RATE_MS);
				#endif
			
			 	/******************** 115200������ÿmsԼ����12Bytes ****************/
				s32RecvLen = GetRs485WriteIndex();
			  if(s32RecvLen == 0)
				{
						vTaskDelay(8 / portTICK_RATE_MS);
				}
				else
				{
						vTaskDelay(1 / portTICK_RATE_MS);
					  if(s32RecvLen == GetRs485WriteIndex())
						{
								/* ��ʱ1ms���дָ��û�иı���Ϊ������� */
							  g_u8Rs485RecvCompleteFlag = 1;
						}
						else
						{
								/* �������� */
							  s32RecvLen = GetRs485WriteIndex();
							  if(s32RecvLen > MAX_PTL_BYTES)
								{
										g_u8Rs485RecvCompleteFlag = 1;
								}
						}
					
				}
				/*******************************************************************/
				
				/*********************** ������յ������� **************************/
				if(g_u8Rs485RecvCompleteFlag == 1)
				{
						/* �����ж��н��յ������ݸ��Ƶ����������� */
					  memset(u8Rs485KpBuff, 0, MAX_RECV_SIZE);
					  memcpy(u8Rs485KpBuff, u8Rs485RxBuff, s32RecvLen);
					
					  /* ���дָ�����ɱ�־�ٴν��� */
					  Rs485WriteIndexClearZero();
					  g_u8Rs485RecvCompleteFlag = 0;
					
					  /* ���յ������ݽ��д��� */
					  PostUartWriteMsg(u8Rs485KpBuff, s32RecvLen, TYPE_UART_RS232);
					  PostUartWriteMsg(u8Rs485KpBuff, s32RecvLen, TYPE_UART_RS485);
					
				}
		
				 
		}
	
	
}

