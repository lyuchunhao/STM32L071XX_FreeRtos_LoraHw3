/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             :  rs485_task.c
 * 版本                 :  0.0.1
 * 作者                 :  lvchunhao
 * 创建日期             :　2017年11月21日
 * 描述                 :  Uart2-RS485通讯
                           1. 中断接收/查询发送
													 2. 接收发送的使能已经在硬件上完成
 ****************************************************************************/
 
 #include "common.h"
 
 
 
uint8_t g_u8Rs485RecvCompleteFlag = 0;           //1-接收完成 0-没有完成继续接收
uint8_t u8Rs485RxBuff[MAX_RECV_SIZE];
uint8_t u8Rs485TxBuff[MAX_SEND_SIZE];
uint8_t u8Rs485KpBuff[MAX_RECV_SIZE];
static uint32_t u32Rs485WriteIndex = 0;

/*
 * 函数名      : Rs485WriteIndexClearZero
 * 功能        : Rs232写指针清零
 * 参数        : void
 * 返回        : void
 * 作者        : lvchunhao  2017-11-14
 */
void Rs485WriteIndexClearZero(void)
{
		u32Rs485WriteIndex = 0;
}
/*
 * 函数名      : GetRs485WriteIndex
 * 功能        : Rs232指针获取
 * 参数        : void
 * 返回        : 指针值
 * 作者        : lvchunhao  2017-11-14
 */
uint32_t GetRs485WriteIndex(void)
{
		return u32Rs485WriteIndex;
}
/*
 * 函数名      : Rs485RecvOneData
 * 功能        : 从中断处接收One字节存入缓冲区
 * 参数        : u8Bytes [in] : 数据
 * 返回        : 无
 * 作者        : lvchunhao  2017-11-14
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
 * 函数名      : vTaskRS485Communicate
 * 功能        : RS485-Uart2接收处理
 * 参数        : pArgs [in] : 线程参数
 * 返回        : 无
 * 作者        : lvchunhao  2017-8-2
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
				/* 必须按照Time+Data的顺序读取,否则不读Data的话,日历值会被锁住 */
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
				PRINT("Now: %02d - %02d - %02d - %02d\n", sDate.Year, sDate.Month, sDate.Date, sDate.WeekDay);
				PRINT("Now: %02d:%02d:%02d\n", sTime.Hours, sTime.Minutes, sTime.Seconds); 
				vTaskDelay(1000 / portTICK_RATE_MS);
				#endif
			
			 	/******************** 115200波特率每ms约接收12Bytes ****************/
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
								/* 延时1ms如果写指针没有改变认为接收完成 */
							  g_u8Rs485RecvCompleteFlag = 1;
						}
						else
						{
								/* 继续接收 */
							  s32RecvLen = GetRs485WriteIndex();
							  if(s32RecvLen > MAX_PTL_BYTES)
								{
										g_u8Rs485RecvCompleteFlag = 1;
								}
						}
					
				}
				/*******************************************************************/
				
				/*********************** 处理接收到的数据 **************************/
				if(g_u8Rs485RecvCompleteFlag == 1)
				{
						/* 将从中断中接收到的数据复制到处理缓冲区中 */
					  memset(u8Rs485KpBuff, 0, MAX_RECV_SIZE);
					  memcpy(u8Rs485KpBuff, u8Rs485RxBuff, s32RecvLen);
					
					  /* 清除写指针和完成标志再次接收 */
					  Rs485WriteIndexClearZero();
					  g_u8Rs485RecvCompleteFlag = 0;
					
					  /* 接收到的数据进行处理 */
					  PostUartWriteMsg(u8Rs485KpBuff, s32RecvLen, TYPE_UART_RS232);
					  PostUartWriteMsg(u8Rs485KpBuff, s32RecvLen, TYPE_UART_RS485);
					
				}
		
				 
		}
	
	
}

