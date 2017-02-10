

#ifndef DATAPROCESSER_H_
#define DATAPROCESSER_H_

#include "HeadFileForAll.h"
#include "_RawInfo.h"
#include "TrainState.h"
#include "ParamOperation.h"
#include "LogInstance.h"
#include "FrameGenerate.h"
#include "_SendData.h"
#include "_SendDataNet.h"



//#include "SerialHandle.h"
//#include "EthernetHandle.h"
//#include "CanHandle.h"
//#include "MrmGprsHandle.h"
//#include "DispatchCommandFileHandle.h"

#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//class SerialHandle;
class EthernetHandle;
class CanHandle;
class MrmGprsHandle;
class DispatchCommandFileHandle;

class DataProcesser {
public:
	DataProcesser();
	virtual ~DataProcesser();
	bool ParseRawInfo(char* buf, int len, _RawInfo& PackageInfo);
	void HandleRawInfo(_RawInfo& PackageInfo);
	std::stringstream ss_log;

	byte arrLastBufRx[1500];
	byte arrLastCommandCode[6];

	TrainState *mTrainState;
	ParamOperation *mParamOperation;
	LogInstance* mLogInstance;

	//SerialHandle* mSerialHandle;
	EthernetHandle* mEthernetHandle;
	CanHandle* mCanHandle0;
	CanHandle* mCanHandle1;
	MrmGprsHandle* mMrmGprsHandle0;
	MrmGprsHandle* mMrmGprsHandle1;
	DispatchCommandFileHandle* mDispatchCommandFileHandle;

	int GetCan0Size();
	int GetCan1Size();
	FrameGenerate mFrameGenerate;
	void SetMembers(EthernetHandle* _EthernetHandle,CanHandle* _CanHandle0, CanHandle* _CanHandle1
			,MrmGprsHandle* _MrmGprsHandle0, MrmGprsHandle* _MrmGprsHandle1, DispatchCommandFileHandle* _DispatchCommandFileHandle);

	void SendInfoForCan(_SendData& pSendData);
	void SendInfoForGprs(_SendDataNet& pSendDataNet);
	void HandleAckInfo(_RawInfo& PackageInfo);
	void SendAckInfo(_RawInfo& PackageInfo);

	void TestForCan();
	void StartReport(byte iTimes);
	void IntegrateInfoTx(byte port);
	void ChuanHaoFor450M(byte port);

	void CheckCanRoad();
	void SendDeviceState();
	void SendIntegeInfo();
	void subApplyForDataFromOtherBoard(_RawInfo& PackageInfo);
	void subApplyForDataFromMMI(_RawInfo& PackageInfo);
	void subApplyForDataFromZhuanHuan(_RawInfo& PackageInfo);
	void subApplyForDataFromGprs(_RawInfo& PackageInfo, byte bComingPort);
	void subApplyForDispatchCommand(_RawInfo& PackageInfo, byte bComingPort);

};

#endif /* DATAPROCESSER_H_ */
