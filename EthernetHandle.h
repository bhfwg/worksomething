/*
 * EthernetHandle.h
 *
 *  Created on: 2016-8-11
 *      Author: lenovo
 */

#ifndef ETHERNETHANDLE_H_
#define ETHERNETHANDLE_H_

#include "HeadFileForAll.h"
#include "TrainState.h"
#include "ParamOperation.h"
#include "DataProcesser.h"
#include "_RawInfo.h"
#include "_SendDataNet.h"
#include "LogInstance.h"
#include "FrameGenerate.h"

#include <netinet/in.h>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <chrono>
#include <sstream>
class EthernetHandle {
public:
	EthernetHandle();
	virtual ~EthernetHandle();
	void Init();
	void StartService();

	std::string tag ;
	LogInstance* mLogInstance;
	TrainState *mTrainState;
	ParamOperation *mParamOperation;

	DataProcesser mDataProcesser ;
	FrameGenerate mFrameGenerate;

	byte buf_read[BUF_SIZ];
	//byte buf_send[BUF_SIZ];
    //byte tmpReader[BUF_SIZ];

	int iRecvNumber ;
	int iRecvLen;
	int fd_ethernet;
	struct sockaddr_in server_addr;
	socklen_t server_addr_len;

	struct sockaddr_in send_addr;
	socklen_t send_addr_len;
	int iSendLen;

	bool bRead ;
	std::thread thdReceive;
	void EthernetReceiveThread();


	bool bWriteThread ;
	std::thread thdWrite;
	void EthernetWriteThread();

	std::list<_SendDataNet> g_SendData_List ;
	std::mutex mtx_WaitSend; //
	std::condition_variable mWaitSend ; // 控制写线程,没有数据可写时则通过mWaitSendRJ422_NoAck来阻塞


	bool bHandleRawInfo ;
	std::thread thdHandleRawInfo;
	std::list<_RawInfo> g_RawInfoData ;//数据帧列表
	void HandleRawInfoThread();
	std::mutex mtx_HaveRawInfo; //
	std::condition_variable mWaitHaveRawInfo ; // 控制写线程,没有数据可写时则通过mWaitSendRJ422_NoAck来阻塞


	//int FormValidData(byte *tmpBuf, int tmpBufLen);
};

#endif /* ETHERNETHANDLE_H_ */
