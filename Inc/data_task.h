/****************************************************************************
 * Copyright(c), 2001-2060, ihealthlabs 版权所有
 ****************************************************************************
 * 文件名称             :  data_task.h
 * 版本                 :  0.0.1
 * 作者                 :  lvchunhao
 * 创建日期             :　2017年9月12日
 * 描述                 :  数据处理线程
 ****************************************************************************/
#ifndef _DATA_TASK_H
#define _DATA_TASK_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "common.h"
	

typedef struct _tagStLoraParameter
{
		uint32_t u32IPR;
	  uint32_t u32DBL;
		uint32_t u32NID;
	  uint32_t u32MOD;
    uint32_t u32LRS;
	  uint32_t u32CHA;
	  uint32_t u32RID;
		uint32_t u32TID;
		uint32_t u32ITV;
	  uint32_t u32PAR;
	  uint32_t u32STO;
	  uint32_t u32SLE;	
}StLoraParameter;
	 
	 
typedef struct _tagStLoraAttr
{
		uint8_t u8IPRCmd[16];                 //波特率,取值范围0-9 0=300bps/s 1=600 2=1200 3=2400 4=4800 5=9600 6=19200 7=38400 8=57600 9=115200bps/s
		uint8_t u8DBLCmd[16];                 //调试等级 0-2
	  uint8_t u8NIDCmd[16];                 //设备ID   0-65535
		uint8_t u8MODCmd[16];                 //工作模式 0-TRNS 1-AT 2-API
		uint8_t u8LRSCmd[16];                 //空中速率 1-6 数值越大传输速率越高距离越近
		uint8_t u8CHACmd[16];                 //物理信道/工作频率 1-32 (24大概是433MHz)
	  uint8_t u8RIDCmd[16];                 //中继地址 0-65535
		uint8_t u8TIDCmd[16];                 //透传地址 0-65535
		uint8_t u8ITVCmd[16];                 //数据帧时间间隔 1-65535
		uint8_t u8PARCmd[16];                 //校验位 0-NONE 1-EVEN 2-ODD
		uint8_t u8STOCmd[16];                 //停止位 1 2
	  uint8_t u8SLECmd[16];                 //休眠模式 0-NONE 1=TIME 2=DEEP
	
}StLoraAttr;



extern StLoraAttr s_StLoraConfig[];
extern StLoraParameter g_stLoraParameter[];

	 
void vTaskDataProcess(void *pArgs);
int32_t PostDataProcessMsg(uint8_t *pData, uint32_t u32DataLen, uint8_t u8MsgType);

int32_t GetLoraConfigInfo(uint8_t *u8Data);
void LoraConfigPrintIndex(uint8_t u8Index);
int32_t LoRa_WaitOnCFGResultUntilTimeout(uint32_t Timeout);
int32_t UpgradeLoraConfig(uint8_t *u8Data, uint32_t u32DataLen);
int32_t LoRa_WaitOnSlaveReplayFlagUntilTimeout(uint32_t Timeout);

#ifdef __cplusplus
}
#endif
#endif /* _DATA_TASK_H */

