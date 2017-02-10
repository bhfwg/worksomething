/*
 * FrameGenerate.h
 *
 *  Created on: 2016-8-31
 *      Author: lenovo
 */

#ifndef FRAMEGENERATE_H_
#define FRAMEGENERATE_H_

#include "HeadFileForAll.h"
#include "TrainState.h"
#include "_SendData.h"
#include "_SendDataNet.h"
#include "_RawInfo.h"
#include "StructsForMrm.h"
class FrameGenerate {
public:
	FrameGenerate();
	virtual ~FrameGenerate();
	TrainState *mTrainState;
	void GetFrame_AckInfo(_SendData& pSendData,byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType, _RawInfo& PackageInfo);
	void GetFrame_RestBoard(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType,byte bDeviceCode);
	void GetFrame_CanState(_SendData& pSendData,byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType);
	void GetFrame_ChangeMainUsedState(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType ,byte bData);
	void GetFrame_StartReport(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType,byte bData);
	void GetFrame_StopPermit(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType);
	void GetFrame_DeviceState(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType);
	void GetFrame_IntegeInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType);
	void GetFrame_SetIoForGsmrModel(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType,byte bData);
	void GetFrame_FunctionEngineNumberResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
			,TrainState* mTrainState,byte bResult, byte bReason,byte bNumber);
	void GetFrame_FunctionTrainNumberResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
				,TrainState* mTrainState,byte bResult, byte bReason,byte bNumber);
	void GetFrame_CallListToMmi(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
			,std::vector<_PToPCallStruct>& vPToPbuffer,std::vector<_GroupCallStruct>& vVGCbuffer);
	void GetFrame_CallResponse(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
			,byte bFrameType,byte bData);
	void GetFrame_PttState(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
				,byte bFrameType,byte bData);
	void GetFrame_SearchNetResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
			,bool bHaveFound,std::vector<std::string> & mAvailableOperatorNameList);
	void GetFrame_SearchOperatorResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
			,int iResult, std::string operatorname);
	void GetFrame_ATCommandToZhuanHuan(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
			,std::string strCommand);
	void GetFrame_GprsDataToZhuanHuan(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
			,_SendDataNet& pSendDataNet);
	void GetFrame_InitNetworkInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
			,byte bFrameType);
	void GetFrame_DispatchCommand(_SendData& pSendData, byte bDestCode, byte bZongXian, byte bPriority, byte bNeedAckFlag,byte bFrameType, _RawInfo& PackageInfo);
//	void GetFrame_DispatchCommandAutoAck(_SendDataNet& pSendDataNet, byte bZongXian, byte bPriority, byte bNeedAckFlag,byte bFrameType, _RawInfo& PackageInfo);
	void GetFrame_DispatchCommandAutoAck(_SendDataNet& pSendDataNet, _RawInfo& PackageInfo, std::string strDestinationIp, int iPort);
};

#endif /* FRAMEGENERATE_H_ */
