/*
 * DispatchCommandFileHandle.cpp
 *
 *  Created on: 2016-8-17
 *      Author: lenovo
 */

#include "DispatchCommandFileHandle.h"

using namespace std;

//DispatchCommandFileHandle* DispatchCommandFileHandle::mInstance = NULL;

//DispatchCommandFileHandle* DispatchCommandFileHandle::GetInstance()
//{
//	if(mInstance == NULL)
//		mInstance = new DispatchCommandFileHandle();
//	return mInstance;
//}

DispatchCommandFileHandle::DispatchCommandFileHandle()
{
	tag= "DispatchCommandFileHandle";
	mTrainState = TrainState::GetInstance();
	mLogInstance = LogInstance::GetInstance();
}

DispatchCommandFileHandle::~DispatchCommandFileHandle()
{

}

void DispatchCommandFileHandle::StartService()
{
	bDoWork = true;
	thdDoWork = std::thread(std::mem_fn(&DispatchCommandFileHandle::DoWorkThread),this);
}

void DispatchCommandFileHandle::DoWorkThread()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+"start do work process");

	while (bDoWork)
	{
		try
		{
			{
				std::unique_lock <std::mutex> lck(mtxFileEvent);
				mLogInstance->Log(tag+"mtxFileEvent wait" );
				mwaitFileEvent.wait(lck);//
			}
			while (!listStore.empty())
			{
				mLogInstance->Log(tag+"listStore be handled" );
				_InfoToFile itf= listStore.front();
				listStore.pop_front();
			}
			while (!listSignIn.empty())
			{
				mLogInstance->Log(tag+"listSignIn be handled" );
				_InfoToFile itf= listSignIn.front();
				listSignIn.pop_front();
			}
			while (!listAutoAck.empty())
			{
				mLogInstance->Log(tag+"listAutoAck be handled" );
				_RawInfo PackageInfo= listAutoAck.front();
				listAutoAck.pop_front();

				std::string strDestinationIp;
				strDestinationIp += PackageInfo.arrSourAddre[0];
				strDestinationIp += PackageInfo.arrSourAddre[1];
				strDestinationIp += PackageInfo.arrSourAddre[2];
				strDestinationIp += PackageInfo.arrSourAddre[3];
				int iPort = 20001 ;
				std::cout<<"strDestinationIp="<<strDestinationIp<<" iPort="<<iPort<<std::endl;

				_SendDataNet  pSendDataNet;
				mFrameGenerate.GetFrame_DispatchCommandAutoAck(pSendDataNet, PackageInfo, strDestinationIp, iPort);
				mDataProcesser.SendInfoForGprs(pSendDataNet);
			}
			while (!listPrintMute.empty())
			{
				mLogInstance->Log(tag+"listPrintMute be handled" );
				_InfoToFile itf= listPrintMute.front();
				listPrintMute.pop_front();
			}
			while (!listGetOne.empty())
			{
				mLogInstance->Log(tag+"listGetOne be handled" );
				_InfoToFile itf= listGetOne.front();
				listGetOne.pop_front();
			}
			while (!listGetTen.empty())
			{
				mLogInstance->Log(tag+"listGetTen be handled" );
				_InfoToFile itf= listGetTen.front();
				listGetTen.pop_front();
			}
			while (!listGetCertain.empty())
			{
				mLogInstance->Log(tag+"listGetCertain be handled" );
				_InfoToFile itf= listGetCertain.front();
				listGetCertain.pop_front();
			}
		}
		catch(...)
		{

		}
	}
}

void DispatchCommandFileHandle::AddStoreFile(_InfoToFile& itf)
{
	try
	{
		listStore.push_back(itf);
		std::unique_lock <std::mutex> lck(mtxFileEvent);
		mwaitFileEvent.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log("AddStoreFile error");
	}
}

void DispatchCommandFileHandle::AddSignInFile(_InfoToFile& itf)
{
	try
	{
		listSignIn.push_back(itf);
		std::unique_lock <std::mutex> lck(mtxFileEvent);
		mwaitFileEvent.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log("AddSignInFile error");
	}
}

//void DispatchCommandFileHandle::AddAutoAckFile(_InfoToFile& itf)
//{
//	try
//	{
//		listAutoAck.push_back(itf);
//		std::unique_lock <std::mutex> lck(mtxFileEvent);
//		mwaitFileEvent.notify_all();
//	}
//	catch(...)
//	{
//		mLogInstance->Log("AddAutoAckFile error");
//	}
//}
void DispatchCommandFileHandle::AddAutoAckFile(_RawInfo& PackageInfo)
{
	try
	{
		listAutoAck.push_back(PackageInfo);
		std::unique_lock <std::mutex> lck(mtxFileEvent);
		mwaitFileEvent.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log("AddAutoAckFile error");
	}
}


void DispatchCommandFileHandle::AddPrintMuteFile(_InfoToFile& itf)
{
	try
	{
		listPrintMute.push_back(itf);
		std::unique_lock <std::mutex> lck(mtxFileEvent);
		mwaitFileEvent.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log("AddPrintMuteFile error");
	}
}

void DispatchCommandFileHandle::AddGetOneFile(_InfoToFile& itf)
{
	try
	{
		listGetOne.push_back(itf);
		std::unique_lock <std::mutex> lck(mtxFileEvent);
		mwaitFileEvent.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log("AddGetOneFile error");
	}
}

void DispatchCommandFileHandle::AddGetTenFile(_InfoToFile& itf)
{
	try
	{
		listGetTen.push_back(itf);
		std::unique_lock <std::mutex> lck(mtxFileEvent);
		mwaitFileEvent.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log("AddGetTenFile error");
	}
}

void DispatchCommandFileHandle::AddGetCertainFile(_InfoToFile& itf)
{
	try
	{
		listGetCertain.push_back(itf);
		std::unique_lock <std::mutex> lck(mtxFileEvent);
		mwaitFileEvent.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log("AddGetCertainFile error");
	}
}
