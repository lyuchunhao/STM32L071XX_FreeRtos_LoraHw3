/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             :  rs232_task.h
 * 版本                 :  0.0.1
 * 作者                 :  lvchunhao
 * 创建日期             :　2017年11月14日
 * 描述                 :  Rs232接收及处理线程
                           1. 本版本采用UART4
													 2. 作为默认的调试终端
 ****************************************************************************/
 
 #include "common.h"
 
 
 
//中断接收处理相关参数 
uint8_t g_u8Rs232RecvStep = STEP_FIND_HEAD;
uint8_t g_u8Rs232RecvLen  = 0; 

uint8_t u8Rs232RecvCompeletFlag = 0;         //1-接收完成 0-没有完成继续接收
static uint32_t u32Rs232WriteIndex = 0; 

uint8_t u8Rs232RxBuff[MAX_RECV_SIZE];        //接收缓冲区
uint8_t u8Rs232TxBuff[MAX_SEND_SIZE];        //发送缓冲区
uint8_t u8Rs232KpBuff[MAX_RECV_SIZE];        //处理临时缓冲区



/*
 * 函数名      : GetRs232WriteIndex
 * 功能        : Rs232指针获取
 * 参数        : void
 * 返回        : 指针值
 * 作者        : lvchunhao  2017-11-14
 */
uint32_t GetRs232WriteIndex(void)
{
		return u32Rs232WriteIndex;
}
/*
 * 函数名      : Rs232WriteIndexClearZero
 * 功能        : Rs232写指针清零
 * 参数        : void
 * 返回        : void
 * 作者        : lvchunhao  2017-11-14
 */
void Rs232WriteIndexClearZero(void)
{
		u32Rs232WriteIndex = 0;
}
/*
 * 函数名      : Rs232RecvOneData
 * 功能        : 从中断处接收One字节存入缓冲区
 * 参数        : u8Bytes [in] : 数据
 * 返回        : 无
 * 作者        : lvchunhao  2017-11-14
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
 * 函数名      : Rs232RecvPTLData
 * 功能        : 判断是否接收到完整的一条协议
 * 参数        : u8Data [in] : 数据
 * 参数        ：*u8RunStep[in/out]:接收协议步骤
 * 参数        ：*u8DataLen[in/out]:需要接收的数据长度
 * 返回        : 0 - 成功 -1 - 失败
 * 作者        : lvchunhao  2017-11-14
 */
int32_t Rs232RecvPTLData(uint8_t u8Data)
{
		int32_t s32Ret = -1;
	
		switch(g_u8Rs232RecvStep)
		{
				case STEP_FIND_HEAD:
						 //如果找到协议头0xF5
						 if(u8Data == PTL_HEAD_RS232)
						 {
								g_u8Rs232RecvStep = STEP_TYPE_OK;
								LoraWriteIndexClearZero();
								Rs232RecvOneData(u8Data);
						 }
						 break;
						 
				case STEP_TYPE_OK:
						 //如果类型为0xF0,则OK继续
						 if(u8Data == PTL_TYPE_RS232)
						 {
								g_u8Rs232RecvStep = STEP_LENGTH_GET;
								Rs232RecvOneData(u8Data);
						 }
						 else
						 {
							 //如果类型不对,则数据为0xF5认为是新的包头,其他则重新找
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
						 //长度验证,目前协议中没有超过该字节的
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
 * 函数名      : vTaskRs232Communicate
 * 功能        : Rs232任务处理
 * 参数        : pArgs [in] : 线程参数
 * 返回        : 无
 * 作者        : lvchunhao  2017-11-14
 */
void vTaskRs232Communicate(void *pArgs)
{
		int32_t s32RecvLen = 0;
	  int32_t s32SendLen = 0;
	  uint8_t u8RunStepFlag = 0;
	
	  uint32_t u32DevID = 0;                    //设备ID
	  uint32_t u32TouID = 0;                    //透传ID
	  uint8_t  u8SkySpd = 0;                    //空中速率
	  uint8_t  u8SleepMode = 0;                 //休眠模式
	  uint8_t  u8TmpData[MSG_BUFF_32BYTE];      //临时缓冲
	
	  int32_t s32Err = 0;
	  uint8_t u8Index = 0;
	
		RS232_IT_ENABLE;
	
			uint32_t ADC_CnvtValue[ADC_NUMOFCHANNEL];
	  __IO float ADC_CnvtValueLocal[ADC_NUMOFCHANNEL];
	
		while(1)
		{
				if(u8Rs232RecvCompeletFlag == 1)
				{
						/* 将从中断中接收到的数据复制到处理缓冲区中 */
					  s32RecvLen = GetRs232WriteIndex();
					  memset(u8Rs232KpBuff, 0, MAX_RECV_SIZE);
					  memcpy(u8Rs232KpBuff, u8Rs232RxBuff, s32RecvLen);
					
					  /* 清除写指针和完成标志再次接收 */
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
						
						/* 具体协议解析 */
						switch(u8RunStepFlag)
						{
	
							  case 0xC0: 		
                     //打开串口：上位机串口连接							
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
								     //配置主机：设备ID+透传ID+空中速率(其他默认)
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
										 
										 //将设备ID配置存入全局变量s_StLoraConfig[1]
										 memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+NID=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u32DevID);
										 #if PRINT_STRING
										 PRINT("u32DevID:%s\n", u8TmpData);
										 #endif
										 memset(s_StLoraConfig[1].u8NIDCmd, 0, 16);
										 strcat((char *)s_StLoraConfig[1].u8NIDCmd, (char *)u8TmpData);							 
										 
										 //将透传ID配置存入全局变量s_StLoraConfig[1]
										 memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+TID=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u32TouID);
										 #if PRINT_STRING
										 PRINT("u32TouID:%s\n", u8TmpData);
										 #endif
										 memset(s_StLoraConfig[1].u8TIDCmd, 0, 16);
										 strcat((char *)s_StLoraConfig[1].u8TIDCmd, (char *)u8TmpData);
										 
										 //将空中速率存入全局变量s_StLoraConfig[1]
										 memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+LRS=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u8SkySpd);
										 #if PRINT_STRING
										 PRINT("u8SkySpd:%s\n", u8TmpData);
										 #endif
										 memset(s_StLoraConfig[1].u8LRSCmd, 0, 16);
										 strcat((char *)s_StLoraConfig[1].u8LRSCmd, (char *)u8TmpData);
										 
										 //将休眠模式SLE存入全局变量s_StLoraConfig[1]
										 memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+SLE=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u8SleepMode);
										 #if PRINT_STRING
										 PRINT("u8SleepMode:%s\n", u8TmpData);
										 #endif
										 memset(s_StLoraConfig[1].u8SLECmd, 0, 16);
										 strcat((char *)s_StLoraConfig[1].u8SLECmd, (char *)u8TmpData);
										 
										 //发送数据处理线程配置Lora模块
										 u8Index = 1;
										 PostDataProcessMsg(&u8Index, 1, TYPE_LORA_SET_MULTIP_CFG);
										 #if PRINT_STRING
										 PRINT("Set Lora config:\n");
										 #endif
										 
										 //等待配置结果回复上位机
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
											 
											   //如果配置成功保存参数
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
									   //发送到处理线程查询Lora配置
										 u8Index = 0;
										 PostDataProcessMsg(&u8Index, 1, TYPE_LORA_SET_MULTIP_CFG);
										 #if PRINT_STRING
										 PRINT("Read Lora config:\n");
										 #endif
								
								     //等待配置结果回复上位机
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
											  u8Rs232TxBuff[0] = 0xF5;     //协议头
											  u8Rs232TxBuff[1] = 0xA0;     //协议类型
											  u8Rs232TxBuff[2] = 0x0A;     //长度
											  u8Rs232TxBuff[3] = 0xC2;     //命令ID
											 
											  u8Rs232TxBuff[4] = 0x00;     //状态 0 - 成功
											  u8Rs232TxBuff[5] = g_stLoraParameter[1].u32NID / 256;   //设备ID
											  u8Rs232TxBuff[6] = g_stLoraParameter[1].u32NID % 256;
											 	u8Rs232TxBuff[7] = g_stLoraParameter[1].u32TID / 256;   //透传ID
											  u8Rs232TxBuff[8] = g_stLoraParameter[1].u32TID % 256;
											  u8Rs232TxBuff[9] = g_stLoraParameter[1].u32LRS;         //空中速率
											  u8Rs232TxBuff[10]= g_stLoraParameter[1].u32MOD;         //工作协议
											  u8Rs232TxBuff[11]= g_stLoraParameter[1].u32SLE;         //睡眠模式
											  u8Rs232TxBuff[12]= g_stLoraParameter[1].u32CHA;         //物理信道
											 
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
									   //读取主机电压值
										 /* 启动一次ADC转换,获取Value值 */
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
									   //读取主机电流值:ADC_IN7/R25(100Ω)= N*V = N * 1000 mv			
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
									   //向Rs485接口发送请求数据
								     PostUartWriteMsg(u8Rs232KpBuff, s32RecvLen, TYPE_UART_RS485);
								     break;
								
								case 0xC6:
									   //直接向从机透传数据
								     u8Rs232KpBuff[1] = 0xA0;
								     PostUartWriteMsg(u8Rs232KpBuff, s32RecvLen, TYPE_UART_LORA);
								     break;
								
								case 0xCA:
									   //连接从机命令(2字节透传ID)
									   u32TouID = u8Rs232KpBuff[4] * 256 + u8Rs232KpBuff[5];
								     memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+TID=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u32TouID);
								     #if PRINT_STRING
										 PRINT("CMD: %s\n", u8TmpData);
										 #endif
										 PostDataProcessMsg(u8TmpData, strlen((char *)u8TmpData), TYPE_LORA_SET_SINGLE_CFG);
								
											//等待配置结果回复上位机
										 g_u8LoraConfigResultOK= RESULT_INIT;
										 s32Err = LoRa_WaitOnCFGResultUntilTimeout(4);
										 if(s32Err != RESULT_OK)
										 {
												//配置失败直接回复上位机
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
												//配置透传ID成功后,则发送CA与从机建立通讯
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
											 
											  //设置超时等待从机对于0xCA建立连接的回复
											  g_u8LoraRecvSlaveStateReply = RESET;
											  s32Err = LoRa_WaitOnSlaveReplayFlagUntilTimeout(500);
											  if(s32Err == 0)
												{
														//收到回复标志
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
														//没收到回复或者超时
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
									   //读取从机配置信息
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
								
								     //等待从机配置信息的回复
										 g_u8LoraRecvSlaveStateReply = RESET;
										 s32Err = LoRa_WaitOnSlaveReplayFlagUntilTimeout(3000);
										 if(s32Err == 0)
										 {
												//收到回复标志
												#if PRINT_STRING
												PRINT("Recv slave config Success \n");
												#else
											  //g_u8LoraSlaveConfig初始化时全为0xFF,如果得到从机配置
											  //会在此缓冲区中存储state+2NID+2TID+LRS+MOD+SLE+CHA共9字节
												memset(u8Rs232TxBuff, 0, MAX_SEND_SIZE); 
											  if((g_u8LoraSlaveConfig[0] == 0xFF) && (g_u8LoraSlaveConfig[1] == 0xFF))
												{
														//认为缓冲区g_u8LoraSlaveConfig中无配置数据
														u8Rs232TxBuff[0] = 0xF5;
														u8Rs232TxBuff[1] = 0xA0;
														u8Rs232TxBuff[2] = 0x02;
														u8Rs232TxBuff[3] = 0xCB;
													
													  u8Rs232TxBuff[4] = 0x01;
													  s32SendLen = 5;
												}
												else
												{
														//缓冲区已经得到从机传过来的配置信息
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
												//没收到回复标志或者超时
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
									   //读取从机AD采集到的电压值
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
								
										 //等待从机配置信息的回复
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
												u8Rs232TxBuff[4] = 0x00;   //成功
											 
											  memcpy(u8Rs232TxBuff + 5, g_u8LoraSlaveConfig, 4);
											 
												s32SendLen = 9;
												PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
												#endif	
										 }
										 else
										 {
												//没收到回复标志或者超时
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
									   //读取从机AD采集到的电流值
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
								
										 //等待从机配置信息的回复
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
												u8Rs232TxBuff[4] = 0x00;   //成功
											 
											  memcpy(u8Rs232TxBuff + 5, g_u8LoraSlaveConfig, 4);
											 
												s32SendLen = 9;
												PostUartWriteMsg(u8Rs232TxBuff, s32SendLen, TYPE_UART_RS232);
												#endif	
										 }
										 else
										 {
												//没收到回复标志或者超时
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
									   //向从机Rs485接口发送请求数据
								     PostUartWriteMsg(u8Rs232KpBuff, s32RecvLen, TYPE_UART_LORA);
								     break;
										 
										 
										 
								case 0xA0:
									   //Lora模块的相关控制操作
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
									   //设备ID单项设置
									   u32DevID = u8Rs232KpBuff[4] * 256 + u8Rs232KpBuff[5];
								     memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+NID=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u32DevID);
								     #if PRINT_STRING
										 PRINT("CMD: %s\n", u8TmpData);
										 PostDataProcessMsg(u8TmpData, strlen((char *)u8TmpData), TYPE_LORA_SET_SINGLE_CFG);
								     #endif
								
								     //等待配置结果回复上位机
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
									   //透传ID单项设置
									   u32TouID = u8Rs232KpBuff[4] * 256 + u8Rs232KpBuff[5];
								     memset(u8TmpData, 0, MSG_BUFF_32BYTE);
										 strcat((char *)u8TmpData, "AT+TID=");
										 sprintf((char *)u8TmpData + strlen((char *)u8TmpData), "%d", u32TouID);
								     #if PRINT_STRING
										 PRINT("CMD: %s\n", u8TmpData);
										 PostDataProcessMsg(u8TmpData, strlen((char *)u8TmpData), TYPE_LORA_SET_SINGLE_CFG);
								     #endif
								
								     //等待配置结果回复上位机
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
									   //空中速率单项设置
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
								
								     //等待配置结果回复上位机
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
									   //休眠模式单项设置
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
								
								     //等待配置结果回复上位机
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
									   //进入Stop休眠模式
									   PRINT("STM32 system will in Stop sleep mode \n");	
                     SetRTCAlarmSeconds(0, 1, 11);	//睡眠60+11=71s后退出						 
										 vTaskDelay(100 / portTICK_RATE_MS);
									
										 /*****************************************************************
										 * 1. 停止模式下电压调节器可以运行在正常/低功耗模式,此时1.8V供电区
												  所有时钟停止,PLL/HSI/HSE的RC振荡器功能被禁止,但SRAM和寄存器内
												  被保存下来了。
										 * 2. 在停止模式下,所有的IO引脚都保持在他们运行时的状态
										 * 3. 一定要关闭滴答定时器,因为测试中发现滴答定时器中断有可能唤醒停
												  机模式
										 ******************************************************************/
										 SysTick->CTRL &= (~SysTick_CTRL_ENABLE_Msk); 
										 HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI | PWR_STOPENTRY_WFE);	
										
										 /* 该函数确保该代码区不被任何中断打断 */					
										 portENTER_CRITICAL(); 
									
										 /* 由于时钟问题,此条信息必然不会被打印 */
										 PRINT("STM32 stop mode will printf here?\n");
									
										 /*****************************************************************
										 * 1. 当一个中断或者唤醒事件导致退出停止模式时,HSI-RC振荡器被选为系
										 		  统时钟
										 * 2. 退出低功耗的停止模式后,需要重新配置使用HSE
									 	 * 3. 2017-11-9用PB5外部中断的方法可以唤醒并成功执行(用USB转串口发送
												  提供上升沿或者下降沿)
										 ******************************************************************/
										 SystemClock_StopMode();
										 SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
										 PRINT("STM32 will be start here from sleep mode\n");									
									
										 /* 该函数确保该代码区不被任何中断打断 */			
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

