/*
 * SerialHandle.h
 *
 *  Created on: 2016-8-11
 *      Author: lenovo
 */

#ifndef SERIALHANDLE_H_
#define SERIALHANDLE_H_

#include "HeadFileForAll.h"
#include "TrainState.h"
#include "ParamOperation.h"
#include "DataProcesser.h"
#include "_RawInfo.h"
#include "_SendData.h"
#include "LogInstance.h"
#include "FrameGenerate.h"


#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <chrono>
using namespace std;


class SerialHandle {
public:

	SerialHandle();
	virtual ~SerialHandle();
	void Init();
	int Uart_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
	void StartService();


	std::string tag ;
	LogInstance* mLogInstance;
	TrainState *mTrainState;
	ParamOperation *mParamOperation;
	DataProcesser mDataProcesser ;
	FrameGenerate mFrameGenerate;

	int fd_serial;
	unsigned int rx_len;
	unsigned int tx_len;
	byte RecvDataBuf[BUF_SIZ];

	bool mblCIR_DLE_FLAG;
	bool mblCIR_Head_Flag;
	int mnCIR_COM_R_Index;
	byte mnCIR_COM_R_Data[BUF_SIZ] ;

	bool bRead ;
	std::thread thdReceive;
	void SerialReceiveThread();

	bool bHandleRawInfo ;
	std::thread thdHandleRawInfo;
	std::list<_RawInfo> g_RawInfoData ;//数据帧列表
	void HandleRawInfoThread();
	std::mutex mtx_HaveRawInfo; //
	std::condition_variable mWaitHaveRawInfo ; // 控制写线程,没有数据可写时则通过mWaitSendRJ422_NoAck来阻塞

	bool bWriteThreadWithAck ;
	std::thread thdWriteWithAck;
	void SerialWriteWithAckThread();

	bool bWriteThreadNoAck ;
	std::thread thdWriteNoAck;
	void SerialWriteNoAckThread();

	std::list<_SendData> g_SendData_List_WithAck;
	std::mutex mtx_mWaitSend_WithAck; //
	std::condition_variable mWaitSend_WithAck ; // 控制写线程,没有数据可写时则通过mWaitSendRJ422来阻塞
	bool mblResponse_OK ; // 写的数据是否有回复
	std::mutex mtx_WaitResponse_OK; //
	std::condition_variable mWaitResponse_OK;// 等待对方的回复

	std::list<_SendData> g_SendData_List_NoAck ;
	std::mutex mtx_WaitSend_NoAck; //
	std::condition_variable mWaitSend_NoAck ; // 控制写线程,没有数据可写时则通过mWaitSendRJ422_NoAck来阻塞

	std::mutex mtx_CanWriteCIR ; // 同步两个写线程
};

#endif /* SERIALHANDLE_H_ */
