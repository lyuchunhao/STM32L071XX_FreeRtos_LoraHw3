/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             :  lora_task.c
 * 版本                 :  0.0.1
 * 作者                 :  lvchunhao
 * 创建日期             :　2017年8月2日
 * 描述                 :  Lora通讯 
                           1. 采用STM32串口Uart5-Lora
													 2. 默认初始化了接收中断/接收DMA以及发送DMA
													 3. 测试发现：发送过快都会造成一定程度的丢包
													 4. FreeRTOS发送和接收最好不要采用查询方式，不利于任务调度
													 5. 推荐：中断接收/DMA发送
													 6. Uart5测试-中断接收/查询发送 (vTaskUart1Read)
                              HAL_UART_Receive_IT与HAL_UART_Transmit函数似乎有些问题,
															同时调用会造成进不去接收中断。因此改成自己定义的接收和
															发送函数，问题解决
													 7. 2017-8-5修改成自定义中断接收/查询发送
 ****************************************************************************/

#include "common.h"


uint8_t g_u8LoraRecvCompleteFlag = 0;         //1-接收完成 0-没有完成继续接收
uint8_t u8LoraRxBuff[MAX_RECV_SIZE];
uint8_t u8LoraTxBuff[MAX_SEND_SIZE];
uint8_t u8LoraKpBuff[MAX_RECV_SIZE];
static uint32_t u32LoraWriteIndex = 0;

uint8_t g_u8LoraSlaveConfig[32];              //从机配置信息
uint8_t g_u8LoraRecvSlaveStateReply = RESET;  //是否收到从机建立通讯连接状态的回复
uint8_t g_u8LoraModuleReplyOK = RESET;        //收到Lora模块"OK"回复标志
uint8_t g_u8LoraConfigResultOK= RESULT_INIT;  //Lora模块配置结果,需要根据此变量对上位机回复
uint8_t g_u8LoraReplyAppFlag = REPLY_CMD_NONE;//从机收到主机关于Lora配置信息的询问,回复不能放在Lora线程处理


/**
  * 函数功能: Lora模块复位
	* 输入参数: void
  * 返 回 值: 无
  * 说    明：至少需要200ms
  */
void LoraModuleReset(void)
{
		LORA_RST_RESET;
	  vTaskDelay(250 / portTICK_RATE_MS);
	  LORA_RST_SET;
}
/*
 * 函数名      : Rs232WriteIndexClearZero
 * 功能        : Rs232写指针清零
 * 参数        : void
 * 返回        : void
 * 作者        : lvchunhao  2017-11-14
 */
void LoraWriteIndexClearZero(void)
{
		u32LoraWriteIndex = 0;
}
/*
 * 函数名      : GetRs232WriteIndex
 * 功能        : Rs232指针获取
 * 参数        : void
 * 返回        : 指针值
 * 作者        : lvchunhao  2017-11-14
 */
uint32_t GetLoraWriteIndex(void)
{
		return u32LoraWriteIndex;
}
/*
 * 函数名      : Rs232RecvOneData
 * 功能        : 从中断处接收One字节存入缓冲区
 * 参数        : u8Bytes [in] : 数据
 * 返回        : 无
 * 作者        : lvchunhao  2017-11-14
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
 * 函数名      : GetUnReadDataFromRecvBuff
 * 功能        : 接收循环缓冲区中读取数据(指针操作需放在函数外边)
 * 参数        : u8Read [in] : 读指针
 * 参数				 ：u8Write[in] : 写指针
 * 参数	       ：u8RvBufStartAddr[in]: 接收缓冲区起始地址
 * 参数				 ：u32RvMAXSize[in]:接收缓冲区大小
 * 参数				 ：u8Data[out]: 输出缓冲区
 *             : u32DataLen [in] : 数据长度
 * 返回        : >=0返回字节数, 错误返回-1
 * 作者        : lvchunhao 2017-7-27
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
 * 函数名      : vTaskLoraCommunicate
 * 功能        : Uart5读函数
 * 参数        : pArgs [in] : 线程参数
 * 返回        : 无
 * 作者        : lvchunhao  2017-7-25
 */
void vTaskLoraCommunicate(void *pArgs)
{

	  uint8_t u8RunCmd = 0;
	  int32_t s32SendLen = 0;
	  int32_t s32RecvLen = 0;
	
	  //Lora串口接收中断使能
	  LORA_IT_ENABLE;
	  //LoraModuleReset(); 
	
	  uint8_t u8Index = 0;
	  uint8_t u8State = 0;
	
		uint32_t ADC_CnvtValue[ADC_NUMOFCHANNEL];
	  __IO float ADC_CnvtValueLocal[ADC_NUMOFCHANNEL];
	
		while(1)
		{
				/******************** 115200波特率每ms约接收12Bytes ****************/
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
								/* 延时1ms如果写指针没有改变认为接收完成 */
							  g_u8LoraRecvCompleteFlag = 1;
						}
						else
						{
								/* 继续接收 */
							  s32RecvLen = GetLoraWriteIndex();
							  if(s32RecvLen > MAX_PTL_BYTES)
								{
										g_u8LoraRecvCompleteFlag = 1;
								}
						}
					
				}
				/*******************************************************************/
				
				/*********************** 处理接收到的数据 **************************/
				if(g_u8LoraRecvCompleteFlag == 1)
				{
						/* 将从中断中接收到的数据复制到处理缓冲区中 */
					  memset(u8LoraKpBuff, 0, MAX_RECV_SIZE);
					  memcpy(u8LoraKpBuff, u8LoraRxBuff, s32RecvLen);
					
					  /* 清除写指针和完成标志再次接收 */
					  LoraWriteIndexClearZero();
					  g_u8LoraRecvCompleteFlag = 0;
					
					  /* 具体协议解析 */
						if((strlen((char *)u8LoraKpBuff) >= 6) 
							&& (strncmp((char *)u8LoraKpBuff + strlen((char *)u8LoraKpBuff) - 4, "OK", 2) == 0))
						{
								//LoRa配置时判断AT命令的回复(最后两个字节为OK)
							  /************************************************************************  
								查询某个参数的返回值为：u8Data
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
										/* 接收到的数据类型为:0xB0,则认为是从机发过来的数据*/
									  switch(u8RunCmd)
										{
											  case 0xCA:
													   //收到从机0xCA则置标志位,在Rs232中回复上位机
											       g_u8LoraRecvSlaveStateReply = SET;
												     break;
												
												case 0xCB:
													   //主机收到从机配置的回复data:state+NID+TID+LRS+MOD+SLE+CHA
												     g_u8LoraRecvSlaveStateReply = SET;
												     u8State = u8LoraKpBuff[4];
												     memset(g_u8LoraSlaveConfig, 0xFF, 32);
												     if(u8State == 0)
														 {
																memcpy(g_u8LoraSlaveConfig, u8LoraKpBuff + 4, 9);
														 }									   
												     break;
														 
												case 0xCC:
													   //主机收到从机电压的回复
												     g_u8LoraRecvSlaveStateReply = SET;
												     memset(g_u8LoraSlaveConfig, 0xFF, 32);
												     memcpy(g_u8LoraSlaveConfig, u8LoraKpBuff + 4, s32RecvLen - 4);
													   break;
												
												case 0xCD:
													   //主机收到从机电流的回复
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
									  /* 接收到的数据类型为:0xA0,则认为是从主机发过来的数据 */
										switch(u8RunCmd)
										{
												case 0xCA:
													   //接收到主机建立通讯连接的命令
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
													   //接收到主机配置请求->回复
												     //发送到处理线程查询Lora配置
														 u8Index = 0;
														 PostDataProcessMsg(&u8Index, 1, TYPE_LORA_SET_MULTIP_CFG);
														 #if PRINT_STRING
														 PRINT("Read Lora config:\n");
														 #endif
												
												 		 //等待配置结果回复上位机,放在Data线程回复(如果放在此处配置Lora时会出错阻塞)
														 g_u8LoraConfigResultOK= RESULT_INIT;
														 g_u8LoraReplyAppFlag = REPLY_CMD_XXCB;
												
												     break;
														 
												case 0xCC:
													   //接收到主机对从机电压的询问
														 /* 启动一次ADC转换,获取Value值 */
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
													   //接收到主机对从机电流的询问
														 /* 启动一次ADC转换,获取Value值 */
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
													   //接收到主机向Rs485接口数据的发送
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
								/* 如果都不是我们想要的数据,可以不做任何处理或者发送到Rs232查看 */
							  PostUartWriteMsg(u8LoraKpBuff, s32RecvLen, TYPE_UART_RS232);
						}
				}
				/*******************************************************************/
				
		}
	
	
}

