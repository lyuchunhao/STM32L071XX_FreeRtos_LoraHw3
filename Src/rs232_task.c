/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  rs232_task.h
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��11��14��
 * ����                 :  Rs232���ռ������߳�
                           1. ���汾����UART4
													 2. ��ΪĬ�ϵĵ����ն�
 ****************************************************************************/
 
 #include "common.h"
 
 
 
//�жϽ��մ�����ز��� 
uint8_t g_u8Rs232RecvStep = STEP_FIND_HEAD;
uint8_t g_u8Rs232RecvLen  = 0; 

uint8_t u8Rs232RecvCompeletFlag = 0;         //1-������� 0-û����ɼ�������
static uint32_t u32Rs232WriteIndex = 0; 

uint8_t u8Rs232RxBuff[MAX_RECV_SIZE];        //���ջ�����
uint8_t u8Rs232TxBuff[MAX_SEND_SIZE];        //���ͻ�����
uint8_t u8Rs232KpBuff[MAX_RECV_SIZE];        //������ʱ������



/*
 * ������      : GetRs232WriteIndex
 * ����        : Rs232ָ���ȡ
 * ����        : void
 * ����        : ָ��ֵ
 * ����        : lvchunhao  2017-11-14
 */
uint32_t GetRs232WriteIndex(void)
{
		return u32Rs232WriteIndex;
}
/*
 * ������      : Rs232WriteIndexClearZero
 * ����        : Rs232дָ������
 * ����        : void
 * ����        : void
 * ����        : lvchunhao  2017-11-14
 */
void Rs232WriteIndexClearZero(void)
{
		u32Rs232WriteIndex = 0;
}
/*
 * ������      : Rs232RecvOneData
 * ����        : ���жϴ�����One�ֽڴ��뻺����
 * ����        : u8Bytes [in] : ����
 * ����        : ��
 * ����        : lvchunhao  2017-11-14
 */
void Rs232RecvOneData(uint8_t u8Bytes)
{
		if(u32Rs232WriteIndex == MAX_RECV_SIZE)
		{
				u32Rs232WriteIndex = 0;
		}
		
		u8Rs232RxBuff[u32Rs232WriteIndex++] = u8Bytes;
}
/*
 * ������      : Rs232RecvPTLData
 * ����        : �ж��Ƿ���յ�������һ��Э��
 * ����        : u8Data [in] : ����
 * ����        ��*u8RunStep[in/out]:����Э�鲽��
 * ����        ��*u8DataLen[in/out]:��Ҫ���յ����ݳ���
 * ����        : 0 - �ɹ� -1 - ʧ��
 * ����        : lvchunhao  2017-11-14
 */
int32_t Rs232RecvPTLData(uint8_t u8Data)
{
		int32_t s32Ret = -1;
	
		switch(g_u8Rs232RecvStep)
		{
				case STEP_FIND_HEAD:
						 //����ҵ�Э��ͷ0xF5
						 if(u8Data == PTL_HEAD_RS232)
						 {
								g_u8Rs232RecvStep = STEP_TYPE_OK;
								LoraWriteIndexClearZero();
								Rs232RecvOneData(u8Data);
						 }
						 break;
						 
				case STEP_TYPE_OK:
						 //�������Ϊ0xF0,��OK����
						 if(u8Data == PTL_TYPE_RS232)
						 {
								g_u8Rs232RecvStep = STEP_LENGTH_GET;
								Rs232RecvOneData(u8Data);
						 }
						 else
						 {
							 //������Ͳ���,������Ϊ0xF5��Ϊ���µİ�ͷ,������������
							 if(u8Data == PTL_HEAD_RS232)
							 {
									g_u8Rs232RecvStep = STEP_TYPE_OK;
									LoraWriteIndexClearZero();
									Rs232RecvOneData(u8Data);
							 }
							 else
							 {
									g_u8Rs232RecvStep = STEP_FIND_HEAD;
							 }
						 }
						 break;
						 
				case STEP_LENGTH_GET:
						 //������֤,ĿǰЭ����û�г������ֽڵ�
						 if(u8Data > 20)
						 {
								g_u8Rs232RecvStep = STEP_FIND_HEAD;
						 }
						 else
						 {								  
								Rs232RecvOneData(u8Data);
								g_u8Rs232RecvLen = u8Data;
								g_u8Rs232RecvStep = STEP_DATA_GET;
						 }
						 break;
						 
				case STEP_DATA_GET:
						 (g_u8Rs232RecvLen)--;
						 Rs232RecvOneData(u8Data);
						 if(g_u8Rs232RecvLen == 0)
						 {
								s32Ret = 0;
								g_u8Rs232RecvStep = STEP_FIND_HEAD;
						 }
						 break;
						 
				default :
						 g_u8Rs232RecvStep = STEP_FIND_HEAD;
						 break;
		}
		
		return s32Ret;
}

/*
 * ������      : vTaskRs232Communicate
 * ����        : Rs232������
 * ����        : pArgs [in] : �̲߳���
 * ����        : ��
 * ����        : lvchunhao  2017-11-14
 */
void vTaskRs232Communicate(void *pArgs)
{
		int32_t s32RecvLen = 0;
	  int32_t s32SendLen = 0;
	  uint8_t u8RunStepFlag = 0;
	
	  uint32_t u32DevID = 0;                    //�豸ID
	  uint32_t u32TouID = 0;                    //͸��ID
	  uint8_t  u8SkySpd = 0;                    //��������
	  uint8_t  u8SleepMode = 0;                 //����ģʽ
	  uint8_t  u8TmpData[MSG_BUFF_32BYTE];      //��ʱ����
	
	  int32_t s32Err = 0;
	  uint8_t u8Index = 0;
	
		RS232_IT_ENABLE;
	
			uint32_t ADC_CnvtValue[ADC_NUMOFCHANNEL];
	  __IO float ADC_CnvtValueLocal[ADC_NUMOFCHANNEL];
	
		while(1)
		{
				if(u8Rs232RecvCompeletFlag == 1)
				{
						/* �����ж��н��յ������ݸ��Ƶ����������� */
					  s32RecvLen = GetRs232WriteIndex();
					  memset(u8Rs232KpBuff, 0, MAX_RECV_SIZE);
					  memcpy(u8Rs232KpBuff, u8Rs232RxBuff, s32RecvLen);
					
					  /* ���дָ�����ɱ�־�ٴν��� */
					  Rs232WriteIndexClearZero();
					  u8Rs232RecvCompeletFlag = 0;
					
					  if(s32RecvLen >= 4)
						{
								u8RunStepFlag = u8Rs232KpBuff[PTL_CMD_OFFSET];
						}
						else
						{
								u8RunStepFlag = 0x00;
						}
						
						/* ����Э����� */
						switch(u8RunStepFlag)
						{
	
							  case 0xC0: 		
                     //�򿪴��ڣ���λ����������							
									   #if PRINT_STRING
									   PRINT("Recv the cmd 0xc0 \n");
								     #else
								     memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
								     u8Rs232TxBuff[0] = 0xF5;
								     u8Rs232TxBuff[1] = 0xA0;
										 u8Rs232TxBuff[2] = 0x01;
								     u8Rs232TxBuff[3] = 0xC0;
								     
								     s32SendLen = 4;
								     PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
								     #endif
								     break;
							 
							  case 0xC1:
								     //�����������豸ID+͸��ID+��������(����Ĭ��)
								     u32DevID = u8Rs232KpBuff[4] * 256 + u8Rs232KpBuff[5];
								     u32TouID = u8Rs232KpBuff[6] * 256 + u8Rs232KpBuff[7];
								     u8SkySpd = u8Rs232KpBuff[8];
								     u8SleepMode = u8Rs232KpBuff[9];
								     if(u8SkySpd > 6)
										 {
												#if PRINT_STRING
									      PRINT("LRS para error (%d > 6) \n", u8SkySpd);
											  #endif
												break;
										 }
										 
										 //���豸ID���ô���ȫ�ֱ���s_StLoraConfig[1]
										 memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+NID=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u32DevID);
										 #if PRINT_STRING
										 PRINT("u32DevID:%s\n", u8TmpData);
										 #endif
										 memset(s_StLoraConfig[1].u8NIDCmd, 0, 16);
										 strcat((char *)s_StLoraConfig[1].u8NIDCmd, (char *)u8TmpData);							 
										 
										 //��͸��ID���ô���ȫ�ֱ���s_StLoraConfig[1]
										 memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+TID=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u32TouID);
										 #if PRINT_STRING
										 PRINT("u32TouID:%s\n", u8TmpData);
										 #endif
										 memset(s_StLoraConfig[1].u8TIDCmd, 0, 16);
										 strcat((char *)s_StLoraConfig[1].u8TIDCmd, (char *)u8TmpData);
										 
										 //���������ʴ���ȫ�ֱ���s_StLoraConfig[1]
										 memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+LRS=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u8SkySpd);
										 #if PRINT_STRING
										 PRINT("u8SkySpd:%s\n", u8TmpData);
										 #endif
										 memset(s_StLoraConfig[1].u8LRSCmd, 0, 16);
										 strcat((char *)s_StLoraConfig[1].u8LRSCmd, (char *)u8TmpData);
										 
										 //������ģʽSLE����ȫ�ֱ���s_StLoraConfig[1]
										 memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+SLE=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u8SleepMode);
										 #if PRINT_STRING
										 PRINT("u8SleepMode:%s\n", u8TmpData);
										 #endif
										 memset(s_StLoraConfig[1].u8SLECmd, 0, 16);
										 strcat((char *)s_StLoraConfig[1].u8SLECmd, (char *)u8TmpData);
										 
										 //�������ݴ����߳�����Loraģ��
										 u8Index = 1;
										 PostDataProcessMsg(&u8Index, 1, TYPE_LORA_SET_MULTIP_CFG);
										 #if PRINT_STRING
										 PRINT("Set Lora config:\n");
										 #endif
										 
										 //�ȴ����ý���ظ���λ��
										 g_u8LoraConfigResultOK= RESULT_INIT;
										 s32Err = LoRa_WaitOnCFGResultUntilTimeout(45);
										 if(s32Err == RESULT_OK)
										 {
												 #if PRINT_STRING
												 PRINT("Set config success \n");
												 #else
												 memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
												 u8Rs232TxBuff[0] = 0xF5;
												 u8Rs232TxBuff[1] = 0xA0;
												 u8Rs232TxBuff[2] = 0x02;
												 u8Rs232TxBuff[3] = 0xC1;
											   u8Rs232TxBuff[4] = 0x00;
												 
												 s32SendLen = 5;
												 PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											   #endif
											 
											   //������óɹ��������
											   g_stLoraParameter[0].u32NID = u32DevID;
											   g_stLoraParameter[0].u32TID = u32TouID;
											   g_stLoraParameter[0].u32LRS = u8SkySpd;
										 }
										 else
										 {
												 #if PRINT_STRING
												 PRINT("Set config failed \n");
												 #else
												 memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
												 u8Rs232TxBuff[0] = 0xF5;
												 u8Rs232TxBuff[1] = 0xA0;
												 u8Rs232TxBuff[2] = 0x02;
												 u8Rs232TxBuff[3] = 0xC1;
											   u8Rs232TxBuff[4] = 0x01;
												 
												 s32SendLen = 5;
												 PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											   #endif
										 }	 
							       break;
										 
								case 0xC2:
									   //���͵������̲߳�ѯLora����
										 u8Index = 0;
										 PostDataProcessMsg(&u8Index, 1, TYPE_LORA_SET_MULTIP_CFG);
										 #if PRINT_STRING
										 PRINT("Read Lora config:\n");
										 #endif
								
								     //�ȴ����ý���ظ���λ��
										 g_u8LoraConfigResultOK= RESULT_INIT;
										 s32Err = LoRa_WaitOnCFGResultUntilTimeout(45);
										 if(s32Err == RESULT_OK)
										 {
												#if PRINT_STRING
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  s32SendLen = GetLoraConfigInfo(u8Rs232TxBuff);
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;     //Э��ͷ
											  u8Rs232TxBuff[1] = 0xA0;     //Э������
											  u8Rs232TxBuff[2] = 0x0A;     //����
											  u8Rs232TxBuff[3] = 0xC2;     //����ID
											 
											  u8Rs232TxBuff[4] = 0x00;     //״̬ 0 - �ɹ�
											  u8Rs232TxBuff[5] = g_stLoraParameter[1].u32NID / 256;   //�豸ID
											  u8Rs232TxBuff[6] = g_stLoraParameter[1].u32NID % 256;
											 	u8Rs232TxBuff[7] = g_stLoraParameter[1].u32TID / 256;   //͸��ID
											  u8Rs232TxBuff[8] = g_stLoraParameter[1].u32TID % 256;
											  u8Rs232TxBuff[9] = g_stLoraParameter[1].u32LRS;         //��������
											  u8Rs232TxBuff[10]= g_stLoraParameter[1].u32MOD;         //����Э��
											  u8Rs232TxBuff[11]= g_stLoraParameter[1].u32SLE;         //˯��ģʽ
											  u8Rs232TxBuff[12]= g_stLoraParameter[1].u32CHA;         //�����ŵ�
											 
											  s32SendLen = 13;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #endif
										 }
										 else
										 {
												#if PRINT_STRING
											  PRINT("Get config failed \n");
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;
											  u8Rs232TxBuff[1] = 0xA0;
											  u8Rs232TxBuff[2] = 0x02;
											  u8Rs232TxBuff[3] = 0xC2;
											  u8Rs232TxBuff[4] = 0x01;
											 
											  s32SendLen = 5;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #endif
										 }
									  
								     break;
										 
								case 0xC3:
									   //��ȡ������ѹֵ
										 /* ����һ��ADCת��,��ȡValueֵ */
										 GetADCValue(&hadc, ADC_CnvtValue);
								     ADC_CnvtValueLocal[0] =(float)(ADC_CnvtValue[0]&0xFFF)*3.3/4096*3; 
										 ADC_CnvtValueLocal[1] =(float)(ADC_CnvtValue[1]&0xFFF)*3.3/4096*10; 
								     #if PRINT_STRING
								     PRINT("ADC_CH6 value = %d --> %fV\n", ADC_CnvtValue[0]&0xFFF, 
								                   ADC_CnvtValueLocal[0]);
								     #else
										 memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
										 u8Rs232TxBuff[0] = 0xF5;
										 u8Rs232TxBuff[1] = 0xA0;
										 u8Rs232TxBuff[2] = 0x01 + sizeof(ADC_CnvtValueLocal[0]);
										 u8Rs232TxBuff[3] = 0xC3;
								
										 memcpy(u8Rs232TxBuff + 4, (char *)&ADC_CnvtValueLocal[0], sizeof(ADC_CnvtValueLocal[0]));
									 
										 s32SendLen = 4 + sizeof(ADC_CnvtValueLocal[0]);
										 PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
										 #endif
								     break;
								
							  case 0xC4:
									   //��ȡ��������ֵ:ADC_IN7/R25(100��)= N*V = N * 1000 mv			
										 GetADCValue(&hadc, ADC_CnvtValue);
								     ADC_CnvtValueLocal[0] =(float)(ADC_CnvtValue[0]&0xFFF)*3.3/4096*3; 
										 ADC_CnvtValueLocal[1] =(float)(ADC_CnvtValue[1]&0xFFF)*3.3/4096*10; 
								     #if PRINT_STRING
								     PRINT("ADC_CH7 value = %d --> %fmA\n", ADC_CnvtValue[1]&0xFFF, ADC_CnvtValueLocal[1]);
								     #else
										 memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
										 u8Rs232TxBuff[0] = 0xF5;
										 u8Rs232TxBuff[1] = 0xA0;
										 u8Rs232TxBuff[2] = 0x01 + sizeof(ADC_CnvtValueLocal[1]);
										 u8Rs232TxBuff[3] = 0xC4;
								
										 memcpy(u8Rs232TxBuff + 4, (char *)&ADC_CnvtValueLocal[1], sizeof(ADC_CnvtValueLocal[1]));
									 
										 s32SendLen = 4 + sizeof(ADC_CnvtValueLocal[1]);
										 PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
								     #endif
								     break;
								
								case 0xC5:
									   //��Rs485�ӿڷ�����������
								     PostUartWriteMsg(u8Rs232KpBuff, s32RecvLen, TYPE_UART_RS485);
								     break;
								
								case 0xC6:
									   //ֱ����ӻ�͸������
								     u8Rs232KpBuff[1] = 0xA0;
								     PostUartWriteMsg(u8Rs232KpBuff, s32RecvLen, TYPE_UART_LORA);
								     break;
								
								case 0xCA:
									   //���Ӵӻ�����(2�ֽ�͸��ID)
									   u32TouID = u8Rs232KpBuff[4] * 256 + u8Rs232KpBuff[5];
								     memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+TID=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u32TouID);
								     #if PRINT_STRING
										 PRINT("CMD: %s\n", u8TmpData);
										 #endif
										 PostDataProcessMsg(u8TmpData, strlen((char *)u8TmpData), TYPE_LORA_SET_SINGLE_CFG);
								
											//�ȴ����ý���ظ���λ��
										 g_u8LoraConfigResultOK= RESULT_INIT;
										 s32Err = LoRa_WaitOnCFGResultUntilTimeout(4);
										 if(s32Err != RESULT_OK)
										 {
												//����ʧ��ֱ�ӻظ���λ��
												#if PRINT_STRING
											  PRINT("Set TID failed \n");
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;
											  u8Rs232TxBuff[1] = 0xA0;
											  u8Rs232TxBuff[2] = 0x02;
											  u8Rs232TxBuff[3] = 0xCA;
											  u8Rs232TxBuff[4] = 0x01;
											 
											  s32SendLen = 5;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #endif
										 }
										 else
										 {
												//����͸��ID�ɹ���,����CA��ӻ�����ͨѶ
											 	#if PRINT_STRING
											  PRINT("Set TID Success \n");
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;
											  u8Rs232TxBuff[1] = 0xA0;
											  u8Rs232TxBuff[2] = 0x02;
											  u8Rs232TxBuff[3] = 0xCA;
											  u8Rs232TxBuff[4] = 0x00;
											 
											  s32SendLen = 5;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_LORA);
											  #endif
											 
											  //���ó�ʱ�ȴ��ӻ�����0xCA�������ӵĻظ�
											  g_u8LoraRecvSlaveStateReply = RESET;
											  s32Err = LoRa_WaitOnSlaveReplayFlagUntilTimeout(500);
											  if(s32Err == 0)
												{
														//�յ��ظ���־
												  	#if PRINT_STRING
														PRINT("Recv slave state Success \n");
														#else
														memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
														u8Rs232TxBuff[0] = 0xF5;
														u8Rs232TxBuff[1] = 0xA0;
														u8Rs232TxBuff[2] = 0x02;
														u8Rs232TxBuff[3] = 0xCA;
														u8Rs232TxBuff[4] = 0x00;  //0-success 1-failed
													 
														s32SendLen = 5;
														PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
														#endif
												}
												else
											  {
														//û�յ��ظ����߳�ʱ
												  	#if PRINT_STRING
														PRINT("Recv slave state failed \n");
														#else
														memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
														u8Rs232TxBuff[0] = 0xF5;
														u8Rs232TxBuff[1] = 0xA0;
														u8Rs232TxBuff[2] = 0x02;
														u8Rs232TxBuff[3] = 0xCA;
														u8Rs232TxBuff[4] = 0x02;
													 
														s32SendLen = 5;
														PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
														#endif													
												}
										 }
								
								     break;
								
								case 0xCB:
									   //��ȡ�ӻ�������Ϣ
										 #if PRINT_STRING
										 PRINT("Read slave config \n");
										 #else
										 memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
										 u8Rs232TxBuff[0] = 0xF5;
										 u8Rs232TxBuff[1] = 0xA0;
										 u8Rs232TxBuff[2] = 0x01;
										 u8Rs232TxBuff[3] = 0xCB;
									 
										 s32SendLen = 4;
										 PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_LORA);
										 #endif
								
								     //�ȴ��ӻ�������Ϣ�Ļظ�
										 g_u8LoraRecvSlaveStateReply = RESET;
										 s32Err = LoRa_WaitOnSlaveReplayFlagUntilTimeout(3000);
										 if(s32Err == 0)
										 {
												//�յ��ظ���־
												#if PRINT_STRING
												PRINT("Recv slave config Success \n");
												#else
											  //g_u8LoraSlaveConfig��ʼ��ʱȫΪ0xFF,����õ��ӻ�����
											  //���ڴ˻������д洢state+2NID+2TID+LRS+MOD+SLE+CHA��9�ֽ�
												memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE); 
											  if((g_u8LoraSlaveConfig[0] == 0xFF) && (g_u8LoraSlaveConfig[1] == 0xFF))
												{
														//��Ϊ������g_u8LoraSlaveConfig������������
														u8Rs232TxBuff[0] = 0xF5;
														u8Rs232TxBuff[1] = 0xA0;
														u8Rs232TxBuff[2] = 0x02;
														u8Rs232TxBuff[3] = 0xCB;
													
													  u8Rs232TxBuff[4] = 0x01;
													  s32SendLen = 5;
												}
												else
												{
														//�������Ѿ��õ��ӻ���������������Ϣ
														u8Rs232TxBuff[0] = 0xF5;
														u8Rs232TxBuff[1] = 0xA0;
														u8Rs232TxBuff[2] = 0x0A;
														u8Rs232TxBuff[3] = 0xCB;
													
													  u8Rs232TxBuff[4] = 0x00;  //0-success 1-failed
														memcpy(u8Rs232TxBuff + 5, g_u8LoraSlaveConfig + 1, 8);
													 
														s32SendLen = 13;
													
												}
												PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
												#endif
										 }
									   else
										 {
												//û�յ��ظ���־���߳�ʱ
												#if PRINT_STRING
												PRINT("Recv slave config failed \n");
												#else
												memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
												u8Rs232TxBuff[0] = 0xF5;
												u8Rs232TxBuff[1] = 0xA0;
												u8Rs232TxBuff[2] = 0x02;
												u8Rs232TxBuff[3] = 0xCB;
												u8Rs232TxBuff[4] = 0x01;
											 
												s32SendLen = 5;
												PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
												#endif													
										 }
								     break;
								
								case 0xCC:
									   //��ȡ�ӻ�AD�ɼ����ĵ�ѹֵ
										 #if PRINT_STRING
										 PRINT("Get slave AD Voltage \n");
										 #else
										 memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
										 u8Rs232TxBuff[0] = 0xF5;
										 u8Rs232TxBuff[1] = 0xA0;
										 u8Rs232TxBuff[2] = 0x01;
										 u8Rs232TxBuff[3] = 0xCC;
									 
										 s32SendLen = 4;
										 PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_LORA);
								
										 //�ȴ��ӻ�������Ϣ�Ļظ�
										 g_u8LoraRecvSlaveStateReply = RESET;
										 s32Err = LoRa_WaitOnSlaveReplayFlagUntilTimeout(500);
										 if(s32Err == 0)
										 {
												#if PRINT_STRING
												PRINT("Recv slave voltage reply success \n");
												#else
												memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
												u8Rs232TxBuff[0] = 0xF5;
												u8Rs232TxBuff[1] = 0xA0;
												u8Rs232TxBuff[2] = 0x06;
												u8Rs232TxBuff[3] = 0xCC;
												u8Rs232TxBuff[4] = 0x00;   //�ɹ�
											 
											  memcpy(u8Rs232TxBuff + 5, g_u8LoraSlaveConfig, 4);
											 
												s32SendLen = 9;
												PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
												#endif	
										 }
										 else
										 {
												//û�յ��ظ���־���߳�ʱ
												#if PRINT_STRING
												PRINT("Recv slave voltage reply failed \n");
												#else
												memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
												u8Rs232TxBuff[0] = 0xF5;
												u8Rs232TxBuff[1] = 0xA0;
												u8Rs232TxBuff[2] = 0x02;
												u8Rs232TxBuff[3] = 0xCC;
												u8Rs232TxBuff[4] = 0x01;
											 
												s32SendLen = 5;
												PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
												#endif	
										 }
										 #endif
								     break;
								
								case 0xCD:
									   //��ȡ�ӻ�AD�ɼ����ĵ���ֵ
										 #if PRINT_STRING
										 PRINT("Get slave AD Current \n");
										 #else
										 memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
										 u8Rs232TxBuff[0] = 0xF5;
										 u8Rs232TxBuff[1] = 0xA0;
										 u8Rs232TxBuff[2] = 0x01;
										 u8Rs232TxBuff[3] = 0xCD;
									 
										 s32SendLen = 4;
										 PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_LORA);
								
										 //�ȴ��ӻ�������Ϣ�Ļظ�
										 g_u8LoraRecvSlaveStateReply = RESET;
										 s32Err = LoRa_WaitOnSlaveReplayFlagUntilTimeout(500);
										 if(s32Err == 0)
										 {
												#if PRINT_STRING
												PRINT("Recv slave current reply success \n");
												#else
												memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
												u8Rs232TxBuff[0] = 0xF5;
												u8Rs232TxBuff[1] = 0xA0;
												u8Rs232TxBuff[2] = 0x06;
												u8Rs232TxBuff[3] = 0xCD;
												u8Rs232TxBuff[4] = 0x00;   //�ɹ�
											 
											  memcpy(u8Rs232TxBuff + 5, g_u8LoraSlaveConfig, 4);
											 
												s32SendLen = 9;
												PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
												#endif	
										 }
										 else
										 {
												//û�յ��ظ���־���߳�ʱ
												#if PRINT_STRING
												PRINT("Recv slave current reply failed \n");
												#else
												memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
												u8Rs232TxBuff[0] = 0xF5;
												u8Rs232TxBuff[1] = 0xA0;
												u8Rs232TxBuff[2] = 0x02;
												u8Rs232TxBuff[3] = 0xCD;
												u8Rs232TxBuff[4] = 0x01;
											 
												s32SendLen = 5;
												PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
												#endif	
										 }
										 #endif
								     break;
								
								case 0xCE:
									   //��ӻ�Rs485�ӿڷ�����������
								     PostUartWriteMsg(u8Rs232KpBuff, s32RecvLen, TYPE_UART_LORA);
								     break;
										 
										 
										 
								case 0xA0:
									   //Loraģ�����ؿ��Ʋ���
									   if(u8Rs232KpBuff[4] == 0)
										 {
											  LORA_RQ_RESET;
											  #if PRINT_STRING
											  PRINT("LoRa module RQ = 0\n");
											  #endif
										 }
										 
										 if(u8Rs232KpBuff[4] == 1)
										 {
											  LORA_RQ_SET;
											  #if PRINT_STRING
											  PRINT("LoRa module RQ = 1\n");
											  #endif
										 }
										 
										 if(u8Rs232KpBuff[4] == 2)
										 {
											  LoraModuleReset();
											  #if PRINT_STRING
											  PRINT("LoRa module Reset\n");
											  #endif
										 }
										 
										 #if PRINT_STRING
										 
										 #else
										 memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
										 u8Rs232TxBuff[0] = 0xF5;
										 u8Rs232TxBuff[1] = 0xA0;
										 u8Rs232TxBuff[2] = 0x02;
										 u8Rs232TxBuff[3] = 0xA0;
										 u8Rs232TxBuff[4] = u8Rs232KpBuff[4];
									 
										 s32SendLen = 5;
										 PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
										 #endif
										 
									   break;
								
								case 0xA1:
									   //�豸ID��������
									   u32DevID = u8Rs232KpBuff[4] * 256 + u8Rs232KpBuff[5];
								     memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+NID=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u32DevID);
								     #if PRINT_STRING
										 PRINT("CMD: %s\n", u8TmpData);
										 PostDataProcessMsg(u8TmpData, strlen((char *)u8TmpData), TYPE_LORA_SET_SINGLE_CFG);
								     #endif
								
								     //�ȴ����ý���ظ���λ��
										 g_u8LoraConfigResultOK= RESULT_INIT;
										 s32Err = LoRa_WaitOnCFGResultUntilTimeout(4);
										 if(s32Err == RESULT_OK)
										 {
												#if PRINT_STRING
											  PRINT("Set NID success \n");
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;
											  u8Rs232TxBuff[1] = 0xA0;
											  u8Rs232TxBuff[2] = 0x01;
											  u8Rs232TxBuff[3] = 0xC2;
											  u8Rs232TxBuff[4] = 0x00;
											 
											  s32SendLen = 5;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #endif
										 }
										 else
										 {
											 	#if PRINT_STRING
											  PRINT("Get NID failed \n");
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;
											  u8Rs232TxBuff[1] = 0xA0;
											  u8Rs232TxBuff[2] = 0x01;
											  u8Rs232TxBuff[3] = 0xC2;
											  u8Rs232TxBuff[4] = 0x01;
											 
											  s32SendLen = 5;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #endif
											 
										 }
								     break;
										 
								case 0xA2:
									   //͸��ID��������
									   u32TouID = u8Rs232KpBuff[4] * 256 + u8Rs232KpBuff[5];
								     memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+TID=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u32TouID);
								     #if PRINT_STRING
										 PRINT("CMD: %s\n", u8TmpData);
										 PostDataProcessMsg(u8TmpData, strlen((char *)u8TmpData), TYPE_LORA_SET_SINGLE_CFG);
								     #endif
								
								     //�ȴ����ý���ظ���λ��
										 g_u8LoraConfigResultOK= RESULT_INIT;
										 s32Err = LoRa_WaitOnCFGResultUntilTimeout(4);
										 if(s32Err == RESULT_OK)
										 {
												#if PRINT_STRING
											  PRINT("Set TID success \n");
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;
											  u8Rs232TxBuff[1] = 0xA0;
											  u8Rs232TxBuff[2] = 0x01;
											  u8Rs232TxBuff[3] = 0xC2;
											  u8Rs232TxBuff[4] = 0x00;
											 
											  s32SendLen = 5;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #endif
										 }
										 else
										 {
											 	#if PRINT_STRING
											  PRINT("Get TID failed \n");
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;
											  u8Rs232TxBuff[1] = 0xA0;
											  u8Rs232TxBuff[2] = 0x01;
											  u8Rs232TxBuff[3] = 0xC2;
											  u8Rs232TxBuff[4] = 0x01;
											 
											  s32SendLen = 5;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #endif
											 
										 }
								     break;
										 
								case 0xA3:
									   //�������ʵ�������
									   u8SkySpd = u8Rs232KpBuff[4];
								     if(u8SkySpd > 6)
										 {
												#if PRINT_STRING
									      PRINT("LRS para error (%d > 6) \n", u8SkySpd);
											  #endif
												break;
										 }
								     memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+LRS=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u8SkySpd);
								     #if PRINT_STRING
										 PRINT("CMD: %s\n", u8TmpData);
										 PostDataProcessMsg(u8TmpData, strlen((char *)u8TmpData), TYPE_LORA_SET_SINGLE_CFG);
								     #endif
								
								     //�ȴ����ý���ظ���λ��
										 g_u8LoraConfigResultOK= RESULT_INIT;
										 s32Err = LoRa_WaitOnCFGResultUntilTimeout(4);
										 if(s32Err == RESULT_OK)
										 {
												#if PRINT_STRING
											  PRINT("Set LRS success \n");
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;
											  u8Rs232TxBuff[1] = 0xA0;
											  u8Rs232TxBuff[2] = 0x01;
											  u8Rs232TxBuff[3] = 0xC2;
											  u8Rs232TxBuff[4] = 0x00;
											 
											  s32SendLen = 5;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #endif
										 }
										 else
										 {
											 	#if PRINT_STRING
											  PRINT("Get LRS failed \n");
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;
											  u8Rs232TxBuff[1] = 0xA0;
											  u8Rs232TxBuff[2] = 0x01;
											  u8Rs232TxBuff[3] = 0xC2;
											  u8Rs232TxBuff[4] = 0x01;
											 
											  s32SendLen = 5;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #endif
											 
										 }
								     break;
								
								case 0xA4:
									   //����ģʽ��������
									   u8SleepMode = u8Rs232KpBuff[4];
								     if(u8SleepMode > 2)
										 {
												#if PRINT_STRING
									      PRINT("SLE para error (%d > 2) \n", u8SkySpd);
											  #endif
												break;
										 }
								     memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+SLE=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u8SleepMode);
								     #if PRINT_STRING
										 PRINT("CMD: %s\n", u8TmpData);
										 PostDataProcessMsg(u8TmpData, strlen((char *)u8TmpData), TYPE_LORA_SET_SINGLE_CFG);
								     #endif
								
								     //�ȴ����ý���ظ���λ��
										 g_u8LoraConfigResultOK= RESULT_INIT;
										 s32Err = LoRa_WaitOnCFGResultUntilTimeout(4);
										 if(s32Err == RESULT_OK)
										 {
												#if PRINT_STRING
											  PRINT("Set SLE success \n");
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;
											  u8Rs232TxBuff[1] = 0xA0;
											  u8Rs232TxBuff[2] = 0x01;
											  u8Rs232TxBuff[3] = 0xC2;
											  u8Rs232TxBuff[4] = 0x00;
											 
											  s32SendLen = 5;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #endif
										 }
										 else
										 {
											 	#if PRINT_STRING
											  PRINT("Get SLE failed \n");
											  #else
											  memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE);
											  u8Rs232TxBuff[0] = 0xF5;
											  u8Rs232TxBuff[1] = 0xA0;
											  u8Rs232TxBuff[2] = 0x01;
											  u8Rs232TxBuff[3] = 0xC2;
											  u8Rs232TxBuff[4] = 0x01;
											 
											  s32SendLen = 5;
											  PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
											  #endif
											 
										 }
								     break;
										 
								case 0xAF:
									   //����Stop����ģʽ
									   PRINT("STM32 system will in Stop sleep mode \n");	
                     SetRTCAlarmSeconds(0, 1, 11);	//˯��60+11=71s���˳�						 
										 vTaskDelay(100 / portTICK_RATE_MS);
									
										 /*****************************************************************
										 * 1. ֹͣģʽ�µ�ѹ��������������������/�͹���ģʽ,��ʱ1.8V������
												  ����ʱ��ֹͣ,PLL/HSI/HSE��RC�������ܱ���ֹ,��SRAM�ͼĴ�����
												  �����������ˡ�
										 * 2. ��ֹͣģʽ��,���е�IO���Ŷ���������������ʱ��״̬
										 * 3. һ��Ҫ�رյδ�ʱ��,��Ϊ�����з��ֵδ�ʱ���ж��п��ܻ���ͣ
												  ��ģʽ
										 ******************************************************************/
										 SysTick->CTRL &= (~SysTick_CTRL_ENABLE_Msk); 
										 HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI | PWR_STOPENTRY_WFE);	
										
										 /* �ú���ȷ���ô����������κ��жϴ�� */					
										 portENTER_CRITICAL(); 
									
										 /* ����ʱ������,������Ϣ��Ȼ���ᱻ��ӡ */
										 PRINT("STM32 stop mode will printf here?\n");
									
										 /*****************************************************************
										 * 1. ��һ���жϻ��߻����¼������˳�ֹͣģʽʱ,HSI-RC������ѡΪϵ
										 		  ͳʱ��
										 * 2. �˳��͹��ĵ�ֹͣģʽ��,��Ҫ��������ʹ��HSE
									 	 * 3. 2017-11-9��PB5�ⲿ�жϵķ������Ի��Ѳ��ɹ�ִ��(��USBת���ڷ���
												  �ṩ�����ػ����½���)
										 ******************************************************************/
										 SystemClock_StopMode();
										 SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
										 PRINT("STM32 will be start here from sleep mode\n");									
									
										 /* �ú���ȷ���ô����������κ��жϴ�� */			
										 portEXIT_CRITICAL(); 
								     break;
								
								default :
									   break;
						}
					
				}
				else
				{
						vTaskDelay(10 / portTICK_RATE_MS);
				}
		}
	
}

