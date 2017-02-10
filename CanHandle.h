/*
 * CanHandle.h
 *
 *  Created on: 2016-8-5
 *      Author: lenovo
 */

#ifndef CANHANDLE_H_
#define CANHANDLE_H_

#include "HeadFileForAll.h"
#include "TrainState.h"
#include "ParamOperation.h"
#include "DataProcesser.h"
#include "_RawInfo.h"
#include "CanPackage.h"
#include "_SendData.h"
#include "LogInstance.h"
#include "FrameGenerate.h"

#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <map>
#include <chrono>
#include <sstream>

class CanHandle {
public:
	byte bPort;//0=can0为1号can总线, 1=can1为2号can总线
	int fd_can;
	int ibytes_read ;

	CanHandle();
	virtual ~CanHandle();
	void Init(const int port);
	void StartService();

	bool find_can(const int port);
	void close_can(const int port);
	void set_bitrate(const int port, const int bitrate);
	void open_can(const int port);
	int socket_connect(const int port);
	void disconnect(int *sockfd);
	int socket_listen(const int port);
	void close_socket(const int sockfd);
	int set_can_filter(int sock);

	std::string tag ;
	std::string name ;
	TrainState *mTrainState;
	ParamOperation *mParamOperation;
	LogInstance* mLogInstance;
	DataProcesser mDataProcesser ;
	FrameGenerate mFrameGenerate;

	std::map<byte,CanPackage> mapData;
	std::stringstream ss_recv;
	std::stringstream ss_send;

	bool bRead;
	std::thread thdReceive;
	void CanReceiveThread();

	bool bHandleRawInfo ;
	std::thread thdHandleRawInfo;
	std::list<_RawInfo> g_RawInfoData ;//数据帧列表
	void HandleRawInfoThread();
	std::mutex mtx_HaveRawInfo; //
	std::condition_variable mWaitHaveRawInfo ; // 控制写线程,没有数据可写时则通过mWaitSendRJ422_NoAck来阻塞

	bool bWriteThreadGps ;
	std::thread thdWriteThreadGps;
	void CanWriteThreadGps();
	std::list<_SendData> g_SendData_List_Gps;
	std::mutex mtx_WaitSend_Gps; //
	std::condition_variable mWaitSend_Gps ; // 控制写线程,没有数据可写时则通过mWaitSendRJ422来阻塞
	bool mblResponse_OK_Gps ; // 写的数据是否有回复
	std::mutex mtx_WaitResponseAck_Gps; //
	std::condition_variable mWaitResponseAck_Gps;// 等待对方的回复
	void CanWrite_Gps_Notify();
	void CanWrite_Gps_AckNotify();

	bool bWriteThreadOtherBoard ;
	std::thread thdWriteThreadOtherBoard;
	void CanWriteThreadOtherBoard();
	std::list<_SendData> g_SendData_List_OtherBoard;
	std::mutex mtx_WaitSend_OtherBoard; //
	std::condition_variable mWaitSend_OtherBoard ; // 控制写线程,没有数据可写时则通过mWaitSendRJ422来阻塞
	bool mblResponse_OK_OtherBoard ; // 写的数据是否有回复
	std::mutex mtx_WaitResponseAck_OtherBoard; //
	std::condition_variable mWaitResponseAck_OtherBoard;// 等待对方的回复
	void CanWrite_OtherBoard_Notify();
	void CanWrite_OtherBoard_AckNotify();

	bool bWriteThreadNoAck ;
	std::thread thdWriteThreadNoAck;
	void CanWriteThreadNoAck();
	std::list<_SendData> g_SendData_List_NoAck ;
	std::mutex mtx_WaitSend_NoAck; //
	std::condition_variable mWaitSend_NoAck ; // 控制写线程,没有数据可写时则通过mWaitSendRJ422_NoAck来阻塞
	void CanWrite_NoAckData_Notify();

	std::mutex mtx_CanWriteCIR ; //同步多个线程消费数据
	std::mutex mtx_CanAddData ; // 同步多个线程产生数据

	void CanWrite(_SendData& pSendData,bool bReSend);
};

#endif /* CANHANDLE_H_ */
