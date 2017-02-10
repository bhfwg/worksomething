/*
 * DispatchCommandFileHandle.h
 *
 *  Created on: 2016-8-17
 *      Author: lenovo
 */

#ifndef DISPATCHCOMMANDFILEHANDLE_H_
#define DISPATCHCOMMANDFILEHANDLE_H_


#include "HeadFileForAll.h"
#include "ParamOperation.h"
#include "LogInstance.h"
#include "FrameGenerate.h"

#include "DataProcesser.h"

#include "_InfoToFile.h"
#include "_RawInfo.h"
#include "_SendDataNet.h"

#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <chrono>
#include <sstream>

class DispatchCommandFileHandle
{
public:
	//static DispatchCommandFileHandle* GetInstance();
	virtual ~DispatchCommandFileHandle();


	std::string tag ;
	TrainState *mTrainState;
	LogInstance* mLogInstance;

	FrameGenerate mFrameGenerate;
	DataProcesser mDataProcesser ;

	std::mutex mtxFileEvent;//
	std::condition_variable mwaitFileEvent;//

	std::list<_InfoToFile> listDispatchPackage;//多包调度命令缓存

	std::list<_InfoToFile> listStore;
	std::list<_InfoToFile> listSignIn;

	//std::list<_InfoToFile> listAutoAck;
	std::list<_RawInfo> listAutoAck;

	std::list<_InfoToFile> listPrintMute;
	std::list<_InfoToFile> listGetOne;
	std::list<_InfoToFile> listGetTen;
	std::list<_InfoToFile> listGetCertain;

	void StartService();

	bool bDoWork;
	std::thread thdDoWork;
	void DoWorkThread();

	void AddGetCertainFile(_InfoToFile& itf);
	void AddGetTenFile(_InfoToFile& itf);
	void AddGetOneFile(_InfoToFile& itf);
	void AddPrintMuteFile(_InfoToFile& itf);

//	void AddAutoAckFile(_InfoToFile& itf);
	void AddAutoAckFile(_RawInfo& PackageInfo);

	void AddSignInFile(_InfoToFile& itf);
	void AddStoreFile(_InfoToFile& itf);

//private:
	//static DispatchCommandFileHandle* mInstance;
	DispatchCommandFileHandle();

};

#endif /* DISPATCHCOMMANDFILEHANDLE_H_ */
