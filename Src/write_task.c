/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             :  write_task.h
 * 版本                 :  0.0.1
 * 作者                 :  lvchunhao
 * 创建日期             :　2017年8月2日
 * 描述                 :  Uart写线程
                           1. 所有的下发都将通过此任务完成
													 2. 默认Uart2接收放在此文件下
 ****************************************************************************/
 
 #include "common.h"
 
 
 
 static  xQueueHandle s_s32MsgQueID = NULL;
 
 
 
 
 /*
 * 函数名      : PostUartWriteMsg
 * 功能        : 投递一个Uart写消息
 * 参数        : pData [in] : 需要写的数据指针
 *             : u32DataLen [in] : 数据长度
 * 返回        : 正确返回0, 错误返回-1
 * 作者        : lvchunhao 2017-7-27
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

    pDataCopy = (uint8_t *)pvPortMalloc(MSG_BUFF_SIZE);   //申请内存256Bytes
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
        //发送端动态分配空间，如果转发失败，则释放空间
        if(stPkg.pData)
        {
            vPortFree(stPkg.pData);
        }
        return -1;
    }

    return 0;
}
/*
 * 函数名      : vTaskUartSend
 * 功能        : Uart发送
 * 参数        : pArgs [in] : 线程参数
 * 返回        : 无
 * 作者        : lvchunhao  2017-8-2
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

				/* 释放缓冲区 */
				if(stMsg.pData)
				{
						vPortFree(stMsg.pData);
						stMsg.pData = NULL;
						stMsg.u32DataLen = 0;
				}

		}
}

