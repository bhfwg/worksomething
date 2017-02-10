/*
 * EthernetHandle.cpp
 *
 *  Created on: 2016-8-11
 *      Author: lenovo
 */
#include "EthernetHandle.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


EthernetHandle::EthernetHandle()
{
	tag = "EthernetHandle: ";
	mTrainState = TrainState::GetInstance();
	mParamOperation = ParamOperation::GetInstance();
	mLogInstance = LogInstance::GetInstance();
}
void EthernetHandle::Init()
{
	iRecvNumber = 0;
	iRecvLen=0;
	fd_ethernet = -1;
	server_addr_len = -1;
	send_addr_len = -1;
	iSendLen = 0;

	bRead = true;
	bWriteThread = true;
	bHandleRawInfo = true;

	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr(serverIP);
	server_addr.sin_port = htons(LOCALPORT);
	server_addr_len = sizeof(server_addr);
	fd_ethernet = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd_ethernet < 0)
	{
		mLogInstance->Log(tag+"socket create fail");
		exit(EXIT_FAILURE);
	}
	if(bind(fd_ethernet,(struct sockaddr *)&server_addr,sizeof(server_addr)) == -1)
	{
		mLogInstance->Log(tag+"socket bind fail");
		exit(EXIT_FAILURE);
	}

	if(bLogPrint2)
		mLogInstance->Log(tag+"ethernet is already");
}
EthernetHandle::~EthernetHandle()
{
}
void EthernetHandle::StartService()
{
	bRead = true;
	bHandleRawInfo = true;
	thdReceive = std::thread(std::mem_fn(&EthernetHandle::EthernetReceiveThread),this);
	thdWrite = std::thread(std::mem_fn(&EthernetHandle::EthernetWriteThread),this);
	thdHandleRawInfo = std::thread(std::mem_fn(&EthernetHandle::HandleRawInfoThread),this);
	mLogInstance->Log(tag+"StartService");
}
void EthernetHandle::EthernetReceiveThread()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+"start ethernet receive process");

	while(bRead)
	{
		try
		{
			iRecvLen = recvfrom(fd_ethernet,buf_read,sizeof(buf_read),0,(struct sockaddr *)&server_addr,&server_addr_len);
			if(iRecvLen > 0)
			{
				//int len = FormValidData(buf_read,iRecvLen);
				//if(len != -1)// 去除10 02 10 03 及10 10
				//{
					_RawInfo PackageInfo;
					bool flag = mDataProcesser.ParseRawInfo(buf_read, iRecvLen ,PackageInfo);//mDataProcesser.ParseRawInfo(tmpReader, len ,PackageInfo);
					if(flag)
					{
						try
						{
							std::unique_lock <std::mutex> lck(mtx_HaveRawInfo);
							while(g_RawInfoData.size()>=BUF_SIZ)
							{
								g_RawInfoData.pop_front();
								mLogInstance->Log(tag+"too much package need to be handled, so remove older instances");
							}
							g_RawInfoData.push_back(PackageInfo);
							mWaitHaveRawInfo.notify_all();
						}
						catch(...)
						{

						}
					}
				//}
			}
			else
			{
				std::stringstream ss;
				ss <<tag <<"recvfrom error, recvLen=" << iRecvLen;
				mLogInstance->Log(ss.str());
				//exit(EXIT_FAILURE);
			}
		}
		catch(...)
		{

		}
	}
}

void EthernetHandle::EthernetWriteThread()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+"start ethernet write process");

	while(bWriteThread)
	{
		try
		{
			if (g_SendData_List.empty())
			{
				try
				{
					std::unique_lock <std::mutex> lck(mtx_WaitSend);
					mWaitSend.wait(lck);
				}
				catch(...)
				{
					mLogInstance->Log(tag+"wait wrong mWaitSend");
				}
			}
			else
			{
					_SendDataNet pSendDataNet = g_SendData_List.front();


					//byte* tmpData_NeedAck = pSendDataNet.arrInfo;
					//int tmpData_Length = pSendDataNet.iLength;
					//if (tmpData_NeedAck == NULL || tmpData_Length < 1)
					//{
					//	if(!g_SendData_List.empty())
					//		g_SendData_List.pop_front();// 最后删除
					//	continue;
					//}

					//send
					if(!pSendDataNet.vSendData.empty())
					{
						bzero(&send_addr,sizeof(send_addr));
						send_addr.sin_family = AF_INET;
						send_addr.sin_addr.s_addr = inet_addr(pSendDataNet.strDestinationIPAddress.c_str());

						std::stringstream ss; ss<<pSendDataNet.strDestinationPort; int iDestinationPort=20001; ss>>iDestinationPort;
						send_addr.sin_port = htons(iDestinationPort);
						send_addr_len = sizeof(send_addr);

						int nRealSend = -1;
						if((nRealSend = sendto(fd_ethernet, &(pSendDataNet.vSendData[0]), pSendDataNet.vSendData.size(),0,(struct sockaddr*)&send_addr,sizeof(send_addr_len))) < 0)
						{
							std::stringstream ss;
							ss <<tag <<"sendto error, nrealsend=" << nRealSend;
							mLogInstance->Log(ss.str());
							//exit(EXIT_FAILURE);
						}
					}
					else
					{
						mLogInstance->Log("ethernet data is null, please attention");
					}

					if(!g_SendData_List.empty())
						g_SendData_List.pop_front();
			}
		}
		catch(...)
		{
			mLogInstance->Log(tag+"something wrong EthernetWriteThread");
		}
	}
}

void EthernetHandle::HandleRawInfoThread()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+"start ethernet handle process");

	while(bHandleRawInfo)
	{
		try
		{
			if (g_RawInfoData.empty())
			{
				try
				{
					std::unique_lock <std::mutex> lck(mtx_HaveRawInfo);
					mWaitHaveRawInfo.wait(lck);
				}
				catch(...)
				{
					mLogInstance->Log(tag+"wait wrong mWaitHaveRawInfo");
				}
			}
			else
			{
					_RawInfo PackageInfo = g_RawInfoData.front();

					//handle
					mDataProcesser.HandleRawInfo(PackageInfo);
					//
					if(!g_RawInfoData.empty())
						g_RawInfoData.pop_front();
			}
		}
		catch(...)
		{
			mLogInstance->Log(tag+"something wrong HandleRawInfoThread");
		}
	}
}

//int EthernetHandle::FormValidData(byte *tmpBuf, int tmpBufLen)
//{//去除1002 1003,去除多10
//	int len = 0;
//	byte bLastData = 0x00;
//	if (tmpBufLen < 22)
//	{
//		mLogInstance->Log(tag+"data length is to short");
//		return -1;
//	}
//	if (tmpBuf[0] != 0x10 || tmpBuf[1] != 0x02 || tmpBuf[tmpBufLen - 2] != 0x10 || tmpBuf[tmpBufLen - 1] != 0x03)
//	{
//		mLogInstance->Log(tag+"not begin 10 02 or not end 10 03");
//		return -1;
//	}
//	for (int i = 2; i < tmpBufLen - 2; i++)
//	{
//		if (tmpBuf[i] == 0x10 && bLastData == 0x10)
//		{
//			bLastData = 0x00;// 防止出现2个10以上(10 10 10 10)的情况
//			continue;
//		}
//		else
//		{
//			tmpReader[len++] = tmpBuf[i] ;
//			bLastData = tmpBuf[i] ;
//		}
//	}
//	return len;
//}
