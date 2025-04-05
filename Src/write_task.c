/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs ��Ȩ����
 ****************************************************************************
 * �ļ�����             :  write_task.h
 * �汾                 :  0.0.1
 * ����                 :  lvchunhao
 * ��������             :��2017��8��2��
 * ����                 :  Uartд�߳�
                           1. ���е��·�����ͨ�����������
													 2. Ĭ��Uart2���շ��ڴ��ļ���
 ****************************************************************************/
 
 #include "common.h"
 
 
 
 static  xQueueHandle s_s32MsgQueID = NULL;
 
 
 
 
 /*
 * ������      : PostUartWriteMsg
 * ����        : Ͷ��һ��Uartд��Ϣ
 * ����        : pData [in] : ��Ҫд������ָ��
 *             : u32DataLen [in] : ���ݳ���
 * ����        : ��ȷ����0, ���󷵻�-1
 * ����        : lvchunhao 2017-7-27
 */
int32_t PostUartWriteMsg(uint8_t *pData, uint32_t u32DataLen, uint8_t u8MsgType)
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
 * ������      : vTaskUartSend
 * ����        : Uart����
 * ����        : pArgs [in] : �̲߳���
 * ����        : ��
 * ����        : lvchunhao  2017-8-2
 */
void vTaskUartWrite(void *pArgs)
{
		StTaskObj *pTaskObj = (StTaskObj *)pArgs;
		StMsgPkg stMsg;
		portBASE_TYPE s32Ret;

		s_s32MsgQueID = pTaskObj->s32MsgQueID;
	
		while(1)
		{		
				s32Ret = xQueueReceive(s_s32MsgQueID, &stMsg, portMAX_DELAY);
				if(s32Ret != pdPASS)
				{
					continue;
				}
				
				switch(stMsg.u8MsgType)
				{
						case TYPE_UART_HART:	
                 /* UART1-Hart NoUsed */							
							   break;
						
						case TYPE_UART_RS485:
							   /* UART2-Rs485 */
						     STM32L071xx_UART_Transmit(&huart2, stMsg.pData, stMsg.u32DataLen);
							   break;
												
						case TYPE_UART_NONE:
							   /* UART3-NOUsed */
							   break;
						
						case TYPE_UART_RS232:
							   /* UART2-Rs232 */
						     STM32L071xx_UART_Transmit(&huart4, stMsg.pData, stMsg.u32DataLen);
							   break;
												
						case TYPE_UART_LORA:
							   /* UART5-LoRa */
                 STM32L071xx_UART_Transmit(&huart5, stMsg.pData, stMsg.u32DataLen);							
							   break;
						
						default :
							   break;
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

