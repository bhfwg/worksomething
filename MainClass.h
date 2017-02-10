/*
 * MainClass.h
 *
 *  Created on: 2016-9-2
 *      Author: lenovo
 */

#ifndef MAINCLASS_H_
#define MAINCLASS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "HeadFileForAll.h"
#include "TrainState.h"
#include "ParamOperation.h"
#include "LogInstance.h"

#include "SerialHandle.h"
#include "EthernetHandle.h"
#include "CanHandle.h"
#include "MrmGprsHandle.h"
#include "DataProcesser.h"
#include "FrameGenerate.h"
#include "DispatchCommandFileHandle.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <thread>

class MainClass
{
public:
	MainClass();
	virtual ~MainClass();
	long lCurrentTime ;

	TrainState* mTrainState ;
	ParamOperation* mParamOperation ;
	LogInstance* mLogInstance;

	//SerialHandle mSerialHandle;
	EthernetHandle mEthernetHandle;
	CanHandle mCanHandle0;
	CanHandle mCanHandle1;
	MrmGprsHandle mMrmGprsHandle0;
	MrmGprsHandle mMrmGprsHandle1;

	DataProcesser mDataProcesser ;
	FrameGenerate mFrameGenerate;

	DispatchCommandFileHandle mDispatchCommandFileHandle;

	void InitWorks();
	void CheckFoldsAndFiles();
	void CheckZhuKongFlag();
	void WatchDogThread();

	void StartService();
};

#endif /* MAINCLASS_H_ */
