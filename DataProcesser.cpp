

#include "DataProcesser.h"

#include "SerialHandle.h"
#include "EthernetHandle.h"
#include "CanHandle.h"
#include "DispatchCommandFileHandle.h"

#include "_InfoPackage.h"
#include "_InfoToFile.h"
#include "MrmGprsHandle.h"


#include <time.h>
#include "string.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <chrono>
using namespace std;

DataProcesser::DataProcesser()
{
	mTrainState = TrainState::GetInstance();
	mParamOperation = ParamOperation::GetInstance();
	mLogInstance = LogInstance::GetInstance();

	memset(arrLastBufRx, 0 , sizeof(arrLastBufRx));
}

DataProcesser::~DataProcesser()
{

}

bool DataProcesser::ParseRawInfo(byte* buf, int len, _RawInfo& PackageInfo)
{//�������ݻ��RawInfo�ṹ
	try
	{
		if(PackageInfo.bZongXian == CAN0)
		{//��¼�����յ�can����ʱ��,�����ж�canͨ���Ƿ�����,����3��δ�յ�����Ϊcanͨ��������
			mTrainState->lCan0RecvTime = time((time_t*)NULL);
		}
		else
		{
			mTrainState->lCan1RecvTime = time((time_t*)NULL);
		}

		int iStart = 0;
		if((buf[0]>6) != 0)
		{//3.0֡
			PackageInfo.bFeature = (buf[0]>>6);
			PackageInfo.bType = (buf[0]>>5) & BIT0;
			PackageInfo.bNeedAck = (buf[0]>>4) & BIT0;
			PackageInfo.bSequence = buf[0] & 0x07;
			iStart = 1;
		}
		else
		{//2.0֡
			PackageInfo.bFeature = 0;
			iStart = 0;
			mLogInstance->Log("cir2.0 data frame, please attention");
		}

		byte CrcResult[2];
		int rxdatalength = len;
		if(PackageInfo.bFeature == 0)
		{
			PackageInfo.iInfoLenth = buf[0+iStart] * 256 + buf[1+iStart] - iStart  ;//3.0�����ֽڰ��������ֽ�,���Լ���istart��Ȼ��Ҫ����2�ֽ����ݳ����ֶ�
		}
		else
		{
			if( PackageInfo.bType  == 0)//2.0֡,3.0����֡
			{
				PackageInfo.iInfoLenth = buf[0+iStart] * 256 + buf[1+iStart] - iStart - 2 ;//3.0�����ֽڰ��������ֽ�,���Լ���istart��Ȼ��Ҫ����2�ֽ����ݳ����ֶ�
			}																		//Դ�˿ڵ�crc
			else
			{
				PackageInfo.iInfoLenth = rxdatalength - iStart;//����֡//Դ�˿ڵ�crc
			}
		}
//		if(bLogPrint3)
//		{//����յ���ԭʼ�ֽ�
//			ss_log.str("");
//			ss_log<<"can"<<(int)(PackageInfo.bZongXian)<<" recv data2:";
//			for(int i =0; i <len; i++)
//			{
//				ss_log<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(buf[i])<< " ";
//			}
//			string strPrint = ss_log.str();
//			std::cout<<strPrint<<std::endl;
//		}

		if((PackageInfo.bFeature == 0) || (PackageInfo.bFeature != 0  && PackageInfo.bType == 0))
		{//2.0 ��//3.0����֡
			CrcFunc16(buf, rxdatalength, CrcResult);// crcУ��,rxdatalength�������������crc�ֽ�,��˼�����Ϊ0
			if (!((CrcResult[0] == 0) && (CrcResult[1] == 0)))
			{
				std::cout<<"HandleReceiveData: crc16 error, can:"<<(int)PackageInfo.bZongXian<<", sourcecode:"<<std::hex<<std::setw(2)<<std::setfill('0')<<(int)buf[2+iStart]<<std::endl;//mLogInstance->Log("HandleReceiveData: crc16 error");

				return false;
			}

			PackageInfo.bSourCode = buf[2+iStart] ;
			PackageInfo.bSourAddreLenth = buf[3+iStart] ;
			memset(PackageInfo.arrSourAddre,0,sizeof(PackageInfo.arrSourAddre));
			if ((PackageInfo.bSourAddreLenth != 0) && (PackageInfo.bSourAddreLenth != 4) && (PackageInfo.bSourAddreLenth != 6))
			{// ������ʾ:Ӧ����ʾ�յ���Э��
				mLogInstance->Log("HandleReceiveData: sour address lenth is wrong");
				return false;
			}
			memcpy(PackageInfo.arrSourAddre,buf+4+iStart,PackageInfo.bSourAddreLenth);
			PackageInfo.bDectCode = buf[4 +iStart+ PackageInfo.bSourAddreLenth] ;
			PackageInfo.bDectAddreLenth = buf[5 +iStart+ PackageInfo.bSourAddreLenth] ;
			memset(PackageInfo.arrDectAddre,0,sizeof(PackageInfo.arrDectAddre));

			if ((PackageInfo.bDectAddreLenth != 0) && (PackageInfo.bDectAddreLenth != 4) && (PackageInfo.bDectAddreLenth != 6))
			{// ������ʾ:Ӧ����ʾ�յ���Э��
				mLogInstance->Log("HandleReceiveData: dest address lenth is wrong");
				return false;
			}
			memcpy(PackageInfo.arrDectAddre,buf +6 + iStart + PackageInfo.bSourAddreLenth,PackageInfo.bDectAddreLenth);
			PackageInfo.bServiceType = buf[6 + iStart+ PackageInfo.bSourAddreLenth + PackageInfo.bDectAddreLenth] ;
			PackageInfo.bCommand = buf[7 + iStart+ PackageInfo.bSourAddreLenth + PackageInfo.bDectAddreLenth] ;
			PackageInfo.iDataLenth = PackageInfo.iInfoLenth - 8  - PackageInfo.bSourAddreLenth - PackageInfo.bDectAddreLenth;
			if (PackageInfo.iDataLenth > 0)
			{
				PackageInfo.vRawInfoData.clear();
				for(int i = 0; i<PackageInfo.iDataLenth; i++)
				{
					PackageInfo.vRawInfoData.push_back(buf[8 + iStart + PackageInfo.bSourAddreLenth + PackageInfo.bDectAddreLenth + i]);
					//memcpy(PackageInfo.vRawInfoData,  buf + 8 + iStart + PackageInfo.bSourAddreLenth + PackageInfo.bDectAddreLenth, PackageInfo.iDataLenth);
				}
			}
		}
		else if(PackageInfo.bFeature == 1 && PackageInfo.bType == 1)
		{//����֡
			CrcFunc8(buf, rxdatalength, CrcResult);// crcУ��,rxdatalength���������1��crc�ֽ�,��˼�����Ϊ0
			if (!(CrcResult[0] == 0))
			{
				std::cout<<"HandleReceiveData: crc8 error, can:"<<(int)PackageInfo.bZongXian<<", sourcecode:"<<(int)PackageInfo.bSourCode<<std::endl;//mLogInstance->Log("HandleReceiveData: crc8 error");
				if(bLogPrint2)
				{
					ss_log.str("");
					for (int j = 0; j < len; j++)
					{
						ss_log<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(buf[j])<< " ";
					}
					std::string strPrint = ss_log.str();
					std::cout<<strPrint<<std::endl;//mLogInstance->Log(strPrint);
				}
				return false;
			}

			PackageInfo.bSourCode = buf[0+iStart] ;
			PackageInfo.bSourAddreLenth = 0 ;

			PackageInfo.bDectCode = buf[1 +iStart+ PackageInfo.bSourAddreLenth] ;
			PackageInfo.bDectAddreLenth = 0 ;

			PackageInfo.bServiceType = buf[2 + iStart+ PackageInfo.bSourAddreLenth + PackageInfo.bDectAddreLenth] ;
			PackageInfo.bCommand = buf[3 + iStart+ PackageInfo.bSourAddreLenth + PackageInfo.bDectAddreLenth] ;
			PackageInfo.iDataLenth = PackageInfo.iInfoLenth - 5 - PackageInfo.bSourAddreLenth - PackageInfo.bDectAddreLenth;
			if (PackageInfo.iDataLenth > 0)
			{
				PackageInfo.vRawInfoData.clear();
				for(int i = 0; i<PackageInfo.iDataLenth; i++)
				{
					PackageInfo.vRawInfoData.push_back(buf[4 + iStart + PackageInfo.bSourAddreLenth + PackageInfo.bDectAddreLenth + i]);
					//memcpy(PackageInfo.vRawInfoData,  buf + 8 + iStart + PackageInfo.bSourAddreLenth + PackageInfo.bDectAddreLenth, PackageInfo.iDataLenth);
				}
			}
		}


		return true;
	}
	catch(...)
	{
		mLogInstance->Log("HandleReceiveData: catch exception");
		return false;
	}
	return true;
}

void DataProcesser::HandleRawInfo(_RawInfo& PackageInfo)
{
	if(bLogPrint1 )
	{
		if(!(PackageInfo.bServiceType == 0xE0 && PackageInfo.bCommand == 0x07) || bLogPrint2)
		{
			if(mTrainState->arrCheckIp3[2] == 2)
			{
				ss_log.str("");
				ss_log<<"{"<<(int)(PackageInfo.bZongXian)<<"}<--"
					<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bSourCode)<<"]:"
					<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bDectCode)<<"]<--"
					<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bServiceType)<<"]:"
					<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bCommand)<<"]<--{"
					<<PackageInfo.iDataLenth<<"} ";
				for(int i =0; i< PackageInfo.iDataLenth; i++)
				{
					int tm = PackageInfo.vRawInfoData[i];
					ss_log << std::hex << std::setw(2) <<std::setfill('0') << tm << " ";
				}
				string strPrint = ss_log.str();
				mLogInstance->Log(strPrint);
			}
		}
	}

	switch (PackageInfo.bDectCode)
	{
	case (byte) BOARD_BROADCAST:// �㲥0xFF
	case (byte) BOARD_ZK_ALL:// Ŀ��0x12
	case (byte) BOARD_ZK_LEFT:// Ŀ��0x13
	case (byte) BOARD_ZK_RIGHT:// Ŀ��0x14
		{
			if (PackageInfo.bCommand == 0x41)
			{// �յ�Ӧ����Ϣ
				HandleAckInfo(PackageInfo);
			}
			else
			{
				if(PackageInfo.bNeedAck != ACK0)
				{//����Ӧ����Ϣ
					SendAckInfo(PackageInfo);
				}
				//��������
				switch (PackageInfo.bSourCode)
				{// Դ�˿ں�
				case (byte) BOARD_BROADCAST:
					{
						mLogInstance->Log("source code should not be 0x12");
					}
					break;
				case (byte) BOARD_ZK_LEFT:
				case (byte) BOARD_ZK_RIGHT:
					{//��һ������
						subApplyForDataFromOtherBoard(PackageInfo);
					}
					break;
				case (byte) BOARD_MMI_ALL:
				case (byte) BOARD_MMI_LEFT:
				case (byte) BOARD_MMI_RIGHT:
					{//mmi
						subApplyForDataFromMMI(PackageInfo);
					}
					break;
				case (byte) BOARD_ZH_ALL:
				case (byte) BOARD_ZH_LEFT:
				case (byte) BOARD_ZH_RIGHT:
					{//ת����Ԫ
					subApplyForDataFromZhuanHuan(PackageInfo);
					}
					break;
				case (byte)BOARD_GPS:
					{//GPS
						//subApplyForDataFromOtherBoard(PackageInfo);
					}
					break;
				case (byte)BOARD_JL:
					{//��¼��Ԫ
						//subApplyForDataFromOtherBoard(PackageInfo);
					}
					break;
				case (byte)BOARD_WX:
					{//���߿����Ԫ
						//subApplyForDataFromOtherBoard(PackageInfo);
					}
					break;
				case (byte)BOARD_JH:
					{//���߽�����Ԫ
						//subApplyForDataFromOtherBoard(PackageInfo);
					}
					break;
				case (byte) BOARD_400M:
					{//450M��̨
						//subApplyForDataFromOtherBoard(PackageInfo);
					}
					break;
				case (byte) BOARD_LBJ:
					{//800M��̨ LBJ
						//subApplyForDataFromOtherBoard(PackageInfo);
					}
					break;
				case (byte) BOARD_JK:
					{//�ӿڵ�Ԫ
						//subApplyForDataFromOtherBoard(PackageInfo);
					}
					break;
				case (byte) BOARD_450M:
					{//450M��̨
						//subApplyForDataFromOtherBoard(PackageInfo);
					}
					break;
				case (byte) BOARD_DL:
					{//��Դ��Ԫ
						//subApplyForDataFromOtherBoard(PackageInfo);
					}
					break;
				default:
					mLogInstance->Log("source code is not be defined");
					break;
				}
			}
		}
		break;
	default:
		mLogInstance->Log("dest code is not zhukong or broadcast");
		break;
	}
}

void DataProcesser::SetMembers(EthernetHandle* _EthernetHandle,CanHandle* _CanHandle0,CanHandle* _CanHandle1
		,MrmGprsHandle* _MrmGprsHandle0, MrmGprsHandle* _MrmGprsHandle1, DispatchCommandFileHandle* _DispatchCommandFileHandle)
{
	mEthernetHandle = _EthernetHandle;
	mCanHandle0 = _CanHandle0;
	mCanHandle1 = _CanHandle1;
	mMrmGprsHandle0 = _MrmGprsHandle0;
	mMrmGprsHandle1 = _MrmGprsHandle1;
	mDispatchCommandFileHandle = _DispatchCommandFileHandle;
}
int DataProcesser::GetCan0Size()
{
	return mCanHandle0->g_SendData_List_NoAck.size();
}
int DataProcesser::GetCan1Size()
{
	return mCanHandle1->g_SendData_List_NoAck.size();
}
void DataProcesser::SendInfoForGprs(_SendDataNet& pSendDataNet)
{
	if(pSendDataNet.vSendData.size() < 14)
	{
		mLogInstance->Log("vSendData is too short, please attention");
	}
	//pSendDataNet.vSendData.erase(pSendDataNet.vSendData.begin());//ɾ����һ����ʶ�ֽ�
	byte CrcResult[2];
	CrcFunc16(&(pSendDataNet.vSendData[0]), pSendDataNet.vSendData.size()-2, CrcResult);// crcУ��,rxdatalength�������������crc�ֽ�,��˼�����Ϊ0
	pSendDataNet.vSendData[pSendDataNet.vSendData.size() - 2] =CrcResult[0];
	pSendDataNet.vSendData[pSendDataNet.vSendData.size() - 1] =CrcResult[1];
	pSendDataNet.vSendData.insert(pSendDataNet.vSendData.begin(),0x02);
	pSendDataNet.vSendData.insert(pSendDataNet.vSendData.begin(),0x10);
	pSendDataNet.vSendData.push_back(0x10);
	pSendDataNet.vSendData.push_back(0x03);

	std::cout<<std::endl<<"gprs send data:"<<std::endl;
	std::vector<byte>::iterator it = pSendDataNet.vSendData.begin();
	while(it != pSendDataNet.vSendData.end())
	{
		int tm = *it;
		std::cout<< std::hex << std::setw(2) <<std::setfill('0') << tm << " ";
		it++;
	}
	std::cout<<std::endl<<std::endl;

	MrmGprsHandle* mMrmGprsHandle ;
	if(mMrmGprsHandle0->bActor == GPRS_MODE)
	{
		mMrmGprsHandle = mMrmGprsHandle0;

		if(mMrmGprsHandle1->bActor == GPRS_MODE)
			mLogInstance->Log("gprs mode is not right");
	}
	else if(mMrmGprsHandle1->bActor == GPRS_MODE)
	{
		mMrmGprsHandle = mMrmGprsHandle1;
	}
	else
		mLogInstance->Log("gprs mode is not right");

	try
	{
		std::unique_lock <std::mutex> lck(mMrmGprsHandle->mtx_GprsData);
		mMrmGprsHandle->listGprsData.push_back(pSendDataNet);
		mMrmGprsHandle->mwaitGprsData.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log("push atcommand list to wrong");
	}
}
void DataProcesser::SendInfoForCan(_SendData& pSendData)
{
	switch (pSendData.bDestCode)
	{
	case (byte) BOARD_BROADCAST:
		{//�㲥֡����ҪӦ��
			if(pSendData.bZongXian == CAN0)
			{
				try
				{
					std::unique_lock <std::mutex> lck(mCanHandle0->mtx_CanAddData);
					mCanHandle0->g_SendData_List_NoAck.push_back(pSendData);
					mCanHandle0->CanWrite_NoAckData_Notify();
				}
				catch(...)
				{
					mLogInstance->Log("add wrong mtx_CanAddData");
				}
			}
			else if(pSendData.bZongXian == CAN1)
			{
				try
				{
					std::unique_lock <std::mutex> lck(mCanHandle1->mtx_CanAddData);
					mCanHandle1->g_SendData_List_NoAck.push_back(pSendData);
					mCanHandle1->CanWrite_NoAckData_Notify();
				}
				catch(...)
				{
					mLogInstance->Log("add wrong mtx_CanAddData");
				}
			}
		}
		break;
	case (byte) BOARD_ZK_ALL:
		{
			mLogInstance->Log("should not use 0x12 for send data to zhukong");
		}
		break;
	case (byte) BOARD_ZK_LEFT:
	case (byte) BOARD_ZK_RIGHT:
		{
			if(pSendData.bZongXian == CAN0)
			{
				if(pSendData.bNeedAckFlag == ACK0)
				{
					try
					{
						std::unique_lock <std::mutex> lck(mCanHandle0->mtx_CanAddData);
						mCanHandle0->g_SendData_List_NoAck.push_back(pSendData);
						mCanHandle0->CanWrite_NoAckData_Notify();
					}
					catch(...)
					{
						mLogInstance->Log("add wrong mtx_CanAddData");
					}
				}
				else
				{
					mCanHandle0->g_SendData_List_OtherBoard.push_back(pSendData);
					mCanHandle0->CanWrite_OtherBoard_Notify();
				}
			}
			else if(pSendData.bZongXian == CAN1)
			{
				if(pSendData.bNeedAckFlag == ACK0)
				{
					try
					{
						std::unique_lock <std::mutex> lck(mCanHandle1->mtx_CanAddData);
						mCanHandle1->g_SendData_List_NoAck.push_back(pSendData);
						mCanHandle1->CanWrite_NoAckData_Notify();
					}
					catch(...)
					{
						mLogInstance->Log("add wrong mtx_CanAddData");
					}
				}
				else
				{
					mCanHandle1->g_SendData_List_OtherBoard.push_back(pSendData);
					mCanHandle1->CanWrite_OtherBoard_Notify();
				}
			}
		}
		break;
	case (byte) BOARD_ZH_ALL:
		{
			mLogInstance->Log("should not use 0x22 for send data to zhuanhuan");
		}
		break;
	case (byte) BOARD_ZH_RIGHT:
		{
			if(pSendData.bZongXian == CAN0)
			{
				if(pSendData.bNeedAckFlag == ACK0)
				{
					try
					{
						std::unique_lock <std::mutex> lck(mCanHandle0->mtx_CanAddData);
						mCanHandle0->g_SendData_List_NoAck.push_back(pSendData);
						mCanHandle0->CanWrite_NoAckData_Notify();
					}
					catch(...)
					{
						mLogInstance->Log("add wrong mtx_CanAddData");
					}
				}
				else
				{
					//mCanHandle0->g_SendData_List_MmiRight.push_back(pSendData);
					//mCanHandle0->CanWrite_MmiRight_Notify();
				}
			}
			else if(pSendData.bZongXian == CAN1)
			{
				if(pSendData.bNeedAckFlag == ACK0)
				{
					try
					{
						std::unique_lock <std::mutex> lck(mCanHandle1->mtx_CanAddData);
						mCanHandle1->g_SendData_List_NoAck.push_back(pSendData);
						mCanHandle1->CanWrite_NoAckData_Notify();
					}
					catch(...)
					{
						mLogInstance->Log("add wrong mtx_CanAddData");
					}
				}
				else
				{
					//mCanHandle1->g_SendData_List_MmiRight.push_back(pSendData);
					//mCanHandle1->CanWrite_MmiRight_Notify();
				}
			}
		}
		break;
	case (byte) BOARD_ZH_LEFT:
		{
			if(pSendData.bZongXian == CAN0)
			{
				if(pSendData.bNeedAckFlag == ACK0)
				{
					try
					{
						std::unique_lock <std::mutex> lck(mCanHandle0->mtx_CanAddData);
						mCanHandle0->g_SendData_List_NoAck.push_back(pSendData);
						mCanHandle0->CanWrite_NoAckData_Notify();
					}
					catch(...)
					{
						mLogInstance->Log("add wrong mtx_CanAddData");
					}
				}
				else
				{
//					mCanHandle0->g_SendData_List_ZhLeft.push_back(pSendData);
//					mCanHandle0->CanWrite_ZhLeft_Notify();
				}
			}
			else if(pSendData.bZongXian == CAN1)
			{
				if(pSendData.bNeedAckFlag == ACK0)
				{
					try
					{
						std::unique_lock <std::mutex> lck(mCanHandle1->mtx_CanAddData);
						mCanHandle1->g_SendData_List_NoAck.push_back(pSendData);
						mCanHandle1->CanWrite_NoAckData_Notify();
					}
					catch(...)
					{
						mLogInstance->Log("add wrong mtx_CanAddData");
					}
				}
				else
				{
//					mCanHandle1->g_SendData_List_ZhRight.push_back(pSendData);
//					mCanHandle1->CanWrite_ZhRight_Notify();
				}
			}
		}
		break;
//	case (byte) BOARD_GPS:
//		{
//			if(pSendData.bZongXian == CAN0)
//			{
//				if(pSendData.bNeedAckFlag == ACK0)
//				{
//					mCanHandle0->g_SendData_List_NoAck.push_back(pSendData);
//					mCanHandle0->CanWrite_NoAckData_Notify();
//				}
//				else
//				{
//					mCanHandle0->g_SendData_List_Gps.push_back(pSendData);
//					mCanHandle0->CanWrite_OtherBoard_Notify();
//				}
//			}
//			else if(pSendData.bZongXian == CAN1)
//			{
//				if(pSendData.bNeedAckFlag == ACK0)
//				{
//					mCanHandle1->g_SendData_List_NoAck.push_back(pSendData);
//					mCanHandle1->CanWrite_NoAckData_Notify();
//				}
//				else
//				{
//					mCanHandle1->g_SendData_List_Gps.push_back(pSendData);
//					mCanHandle1->CanWrite_OtherBoard_Notify();
//				}
//			}
//		}
//		break;
	case (byte) BOARD_MMI_ALL:
		{
			mLogInstance->Log("should not use 0x02 for send data to mmi");
		}
		break;
	case (byte) BOARD_MMI_LEFT:
		{
			if(pSendData.bZongXian == CAN0)
			{
				if(pSendData.bNeedAckFlag == ACK0)
				{
					mCanHandle0->g_SendData_List_NoAck.push_back(pSendData);
					mCanHandle0->CanWrite_NoAckData_Notify();
				}
				else
				{
					//mCanHandle0->g_SendData_List_MmiLeft.push_back(pSendData);
					//mCanHandle0->CanWrite_MmiLeft_Notify();
					mCanHandle0->g_SendData_List_NoAck.push_back(pSendData);
					mCanHandle0->CanWrite_NoAckData_Notify();
				}
			}
			else if(pSendData.bZongXian == CAN1)
			{
				if(pSendData.bNeedAckFlag == ACK0)
				{
					mCanHandle1->g_SendData_List_NoAck.push_back(pSendData);
					mCanHandle1->CanWrite_NoAckData_Notify();
				}
				else
				{
					//mCanHandle1->g_SendData_List_MmiLeft.push_back(pSendData);
					//mCanHandle1->CanWrite_MmiLeft_Notify();
					mCanHandle0->g_SendData_List_NoAck.push_back(pSendData);
					mCanHandle0->CanWrite_NoAckData_Notify();
				}
			}
		}
		break;
	case (byte) BOARD_MMI_RIGHT:
		{
			if(pSendData.bZongXian == CAN0)
			{
				if(pSendData.bNeedAckFlag == ACK0)
				{
					mCanHandle0->g_SendData_List_NoAck.push_back(pSendData);
					mCanHandle0->CanWrite_NoAckData_Notify();
				}
				else
				{
					//mCanHandle0->g_SendData_List_MmiLeft.push_back(pSendData);
					//mCanHandle0->CanWrite_MmiLeft_Notify();
					mCanHandle0->g_SendData_List_NoAck.push_back(pSendData);
					mCanHandle0->CanWrite_NoAckData_Notify();
				}
			}
			else if(pSendData.bZongXian == CAN1)
			{
				if(pSendData.bNeedAckFlag == ACK0)
				{
					mCanHandle1->g_SendData_List_NoAck.push_back(pSendData);
					mCanHandle1->CanWrite_NoAckData_Notify();
				}
				else
				{
					//mCanHandle1->g_SendData_List_MmiLeft.push_back(pSendData);
					//mCanHandle1->CanWrite_MmiLeft_Notify();
					mCanHandle0->g_SendData_List_NoAck.push_back(pSendData);
					mCanHandle0->CanWrite_NoAckData_Notify();
				}
			}
		}
		break;
	default:
		std::cout<<"dest code not define: "<<std::hex << std::setw(2) <<std::setfill('0')<<(int)pSendData.bDestCode<<std::endl;//mLogInstance->Log("dest code not define");
		break;
	}
}

void DataProcesser::HandleAckInfo(_RawInfo& PackageInfo)
{
	switch (PackageInfo.bSourCode)
	{
	case (byte) BOARD_ZK_ALL: //
		{
			mLogInstance->Log("zhukong should not use 0x12 for ack");
		}
		break;
	case (byte) BOARD_ZK_LEFT: // ��һ������
	case (byte) BOARD_ZK_RIGHT: // ��һ������
		{
			if(PackageInfo.bZongXian == CAN0)
			{
				mCanHandle0->CanWrite_OtherBoard_AckNotify();
			}
			else if(PackageInfo.bZongXian == CAN1)
			{
				mCanHandle1->CanWrite_OtherBoard_AckNotify();
			}
			else
			{
				mLogInstance->Log("otherboard packageInfo zongxian is not be set");
			}
		}
		break;
	case (byte) BOARD_GPS: // GPS
		{
			if(PackageInfo.bZongXian == CAN0)
			{
				mCanHandle0->CanWrite_Gps_AckNotify();
			}
			else if(PackageInfo.bZongXian == CAN1)
			{
				mCanHandle1->CanWrite_Gps_AckNotify();
			}
			else
			{
				mLogInstance->Log("gps packageInfo zongxian is not be set");
			}
		}
		break;
	case (byte) BOARD_MMI_ALL: //
		{
			mLogInstance->Log("mmi should not use 0x02 for ack");
		}
		break;
	case (byte) BOARD_MMI_LEFT: // MMI
		{
			if(PackageInfo.bZongXian == CAN0)
			{
				//mCanHandle0->CanWrite_MmiLeft_AckNotify();
			}
			else if(PackageInfo.bZongXian == CAN1)
			{
				//mCanHandle1->CanWrite_MmiLeft_AckNotify();
			}
			else
			{
				mLogInstance->Log("mmileft packageInfo zongxian is not be set");
			}
		}
		break;
	case (byte) BOARD_MMI_RIGHT: // MMI
		{
			if(PackageInfo.bZongXian == CAN0)
			{
				//mCanHandle0->CanWrite_MmiRight_AckNotify();
			}
			else if(PackageInfo.bZongXian == CAN1)
			{
				//mCanHandle1->CanWrite_MmiRight_AckNotify();
			}
			else
			{
				mLogInstance->Log("mmiright packageInfo zongxian is not be set");
			}
		}
		break;
	default:
		std::cout<<"source code not define: "<<std::hex << std::setw(2) <<std::setfill('0')<<(int)PackageInfo.bSourCode<<std::endl;//mLogInstance->Log("source code not define");
		break;
	}
}

void DataProcesser::SendAckInfo(_RawInfo& PackageInfo)
{
	_SendData  pSendData;
	mFrameGenerate.GetFrame_AckInfo(pSendData,PackageInfo.bSourCode,PackageInfo.bZongXian,PRIORITY3,ACK0,SHORTFRAME,PackageInfo);
	SendInfoForCan(pSendData);
}

void DataProcesser::CheckCanRoad()
{
	if((mTrainState->bZhuKongFlag == (byte)BOARD_ZK_LEFT || mTrainState->bZhuKongFlag == (byte)BOARD_ZK_RIGHT ) && mTrainState->bMainUsed)
	{//�������ص�Ԫ��Ҫ�����ж�,�����Ƿ���������л�

		mLogInstance->Log("main board check ZongXian state");//mLogInstance->Log("main board check ZongXian state");

		byte bOldCan = mTrainState->bZongXian;
		bool bNeedChangeBoard = false;

		bool bCan0 = true;
		bool bCan1 = true;
		for(int i=0; i <512; i++)
		{
			if(mTrainState->arrCanState[i] == 0x01 || mTrainState->arrCanState[i] == 0x03)
			{
				bCan0 = false;
				std::cout<<"bCan0 = false  i="<< std::hex << std::setw(2) <<std::setfill('0')<< i <<" arrCanState[i]="<< std::hex << std::setw(2) <<std::setfill('0') <<(int)(mTrainState->arrCanState[i])<<std::endl;
			}
			if(mTrainState->arrCanState[i] == 0x02 || mTrainState->arrCanState[i] == 0x03)
			{
				bCan1 = false;
				std::cout<<"bCan1 = false  i="<< std::hex << std::setw(2) <<std::setfill('0')<<i<<" arrCanState[i]="<< std::hex << std::setw(2) <<std::setfill('0') <<(int)(mTrainState->arrCanState[i])<<std::endl;
			}
			if(!bCan0 && !bCan1)
			{
				break;
			}
		}

		if(bCan0 && bCan1 )
		{
			mTrainState->bZongXian = CAN1;
		}
		else if(bCan0 && !bCan1)
		{
			mTrainState->bZongXian = CAN0;
			if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x02 && mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] ==0x00)
			{
				//֪ͨ������
				bNeedChangeBoard = true;
			}
		}
		else if(!bCan0 && bCan1)
		{
			mTrainState->bZongXian = CAN1;
			if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x01 && mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] ==0x00)
			{
				//֪ͨ������
				bNeedChangeBoard = true;
			}
		}
		else
		{//�������߶�������,����Ҫ�����ص�Ԫ������״̬
			if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x00)
			{
				if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x00)
				{
					mTrainState->bZongXian = CAN_DEFAULT;//CAN1
				}
				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x01)
				{
					mTrainState->bZongXian = CAN1;
				}
				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x02 )
				{
					mTrainState->bZongXian = CAN0;
				}
				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x03)
				{
					mTrainState->bZongXian = CAN_DEFAULT;//CAN1
				}
			}
			else if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x01 )
			{
				if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x00)
				{
					mTrainState->bZongXian = CAN1;//֪ͨ������
					bNeedChangeBoard = true;
				}
				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x01)
				{
					mTrainState->bZongXian = CAN1;
				}
				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x02 )
				{
					mTrainState->bZongXian = CAN1;//����һ��,�е����õ�can
				}
				else if( mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x03 )
				{
					mTrainState->bZongXian = CAN1;//����һ��,�е����õ�can
				}
			}
			else if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x02 )
			{
				if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x00)
				{
					mTrainState->bZongXian = CAN0;//֪ͨ������
					bNeedChangeBoard = true;
				}
				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x01)
				{
					mTrainState->bZongXian = CAN0;//����һ��,���仯
				}
				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x02 )
				{
					mTrainState->bZongXian = CAN0;//ά�ֲ���
				}
				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x03)
				{
					mTrainState->bZongXian = CAN0;//ά�ֲ���
				}
			}
			else if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x03 )
			{
				if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x00)
				{
					mTrainState->bZongXian = CAN_DEFAULT;//CAN1
					bNeedChangeBoard = true;
				}
				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x01)
				{
					mTrainState->bZongXian = CAN1;
					bNeedChangeBoard = true;
				}
				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x02 )
				{
					mTrainState->bZongXian = CAN0;
					bNeedChangeBoard = true;
				}
				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x03)
				{
					mTrainState->bZongXian = CAN_DEFAULT;//CAN1
					bNeedChangeBoard = true;
				}
			}
		}

		if(bNeedChangeBoard)
		{
			//֪ͨ�л�����
			_SendData  pSendData;
			mFrameGenerate.GetFrame_ChangeMainUsedState(pSendData,mTrainState->bOtherBoardFlag,mTrainState->bZongXian,PRIORITY0,ACK0,SHORTFRAME,0x01);//ACK2//��ʱע��
			SendInfoForCan(pSendData);

			mTrainState->bMainUsed = false;
			mLogInstance->Log("mainused change to false because bZongXian is something wrong");
		}

		if(bOldCan != mTrainState->bZongXian || bNeedChangeBoard)
		{
			if(bOldCan != mTrainState->bZongXian)
			{
				stringstream ssTemp;
				ssTemp<<"change my zongxian select, old zongxian is "<<(int)bOldCan<<", now is "<<(int)mTrainState->bZongXian;
				mLogInstance->Log(ssTemp.str());
			}
			if(mTrainState->bMainUsed)
				SendDeviceState();
		}
	}
	else
	{
	}
}
//void DataProcesser::CheckCanRoad()
//{//Ĭ��CAN0
//	if((mTrainState->bZhuKongFlag == (byte)BOARD_ZK_LEFT || mTrainState->bZhuKongFlag == (byte)BOARD_ZK_RIGHT ) && mTrainState->bMainUsed)
//	{//�������ص�Ԫ��Ҫ�����ж�,�����Ƿ���������л�
//
//		mLogInstance->Log("main board check ZongXian state");
//
//		byte bOldCan = mTrainState->bZongXian;
//		bool bNeedChangeBoard = false;
//
//		bool bCan0 = true;
//		bool bCan1 = true;
//		for(int i=0; i <512; i++)
//		{
//			if(mTrainState->arrCanState[i] == 0x01 || mTrainState->arrCanState[i] == 0x03)
//			{
//				bCan0 = false;
//				std::cout<<"bCan0 = false  i="<< std::hex << std::setw(2) <<std::setfill('0')<< i <<" arrCanState[i]="<< std::hex << std::setw(2) <<std::setfill('0') <<(int)(mTrainState->arrCanState[i])<<std::endl;
//			}
//			if(mTrainState->arrCanState[i] == 0x02 || mTrainState->arrCanState[i] == 0x03)
//			{
//				bCan1 = false;
//				std::cout<<"bCan1 = false  i="<< std::hex << std::setw(2) <<std::setfill('0')<<i<<" arrCanState[i]="<< std::hex << std::setw(2) <<std::setfill('0') <<(int)(mTrainState->arrCanState[i])<<std::endl;
//			}
//			if(!bCan0 && !bCan1)
//			{
//				break;
//			}
//		}
//
//		if(bCan0 && bCan1 )
//		{
//			mTrainState->bZongXian = CAN0;
//		}
//		else if(bCan0 && !bCan1)
//		{
//			mTrainState->bZongXian = CAN0;
//			if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x02 && mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] ==0x00)
//			{
//				//֪ͨ������
//				bNeedChangeBoard = true;
//			}
//		}
//		else if(!bCan0 && bCan1)
//		{
//			mTrainState->bZongXian = CAN1;
//			if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x01 && mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] ==0x00)
//			{
//				//֪ͨ������
//				bNeedChangeBoard = true;
//			}
//		}
//		else
//		{//�������߶�������,����Ҫ�����ص�Ԫ������״̬
//			if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x00)
//			{
//				if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x00)
//				{
//					mTrainState->bZongXian = CAN0;
//				}
//				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x01)
//				{
//					mTrainState->bZongXian = CAN1;
//				}
//				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x02 )
//				{
//					mTrainState->bZongXian = CAN0;//
//				}
//				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x03)
//				{
//					mTrainState->bZongXian = CAN0;//
//				}
//			}
//			else if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x01 )
//			{
//				if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x00)
//				{
//					mTrainState->bZongXian = CAN1;//֪ͨ������
//					bNeedChangeBoard = true;
//				}
//				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x01)
//				{
//					mTrainState->bZongXian = CAN1;
//				}
//				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x02 )
//				{
//					mTrainState->bZongXian = CAN1;//����һ��,�е����õ�can
//				}
//				else if( mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x03 )
//				{
//					mTrainState->bZongXian = CAN1;//����һ��,�е����õ�can
//				}
//			}
//			else if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x02 )
//			{
//				if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x00)
//				{
//					mTrainState->bZongXian = CAN0;//֪ͨ������
//					bNeedChangeBoard = true;
//				}
//				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x01)
//				{
//					mTrainState->bZongXian = CAN0;//����һ��,���仯
//				}
//				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x02 )
//				{
//					mTrainState->bZongXian = CAN0;//ά�ֲ���
//				}
//				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x03)
//				{
//					mTrainState->bZongXian = CAN0;//ά�ֲ���
//				}
//			}
//			else if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x03 )
//			{
//				if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x00)
//				{
//					mTrainState->bZongXian = CAN0;//֪ͨ������,��Ȼ�޷�֪ͨ
//					bNeedChangeBoard = true;
//				}
//				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x01)
//				{
//					mTrainState->bZongXian = CAN1;//֪ͨ������,��Ȼ�޷�֪ͨ
//					bNeedChangeBoard = true;
//				}
//				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x02 )
//				{
//					mTrainState->bZongXian = CAN0;////֪ͨ������,��Ȼ�޷�֪ͨ
//					bNeedChangeBoard = true;
//				}
//				else if(mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] == 0x03)
//				{
//					mTrainState->bZongXian = CAN0;////֪ͨ������,��Ȼ�޷�֪ͨ
//					bNeedChangeBoard = true;
//				}
//			}
//		}
//
//		if(bNeedChangeBoard)
//		{
//			//֪ͨ�л�����
//			_SendData  pSendData;
//			mFrameGenerate.GetFrame_ChangeMainUsedState(pSendData,mTrainState->bOtherBoardFlag,mTrainState->bZongXian,PRIORITY0,ACK0,SHORTFRAME,0x01);//ACK2//��ʱע��
//			SendInfoForCan(pSendData);
//
//			mTrainState->bMainUsed = false;
//			mLogInstance->Log("mainused change to false because bZongXian is something wrong");
//		}
//
//		if(bOldCan != mTrainState->bZongXian || bNeedChangeBoard)
//		{
//			if(bOldCan != mTrainState->bZongXian)
//			{
//				stringstream ssTemp;
//				ssTemp<<"change my zongxian select, old zongxian is "<<(int)bOldCan<<", now is "<<(int)mTrainState->bZongXian;
//				mLogInstance->Log(ssTemp.str());
//			}
//			if(mTrainState->bMainUsed)
//				SendDeviceState();
//		}
//	}
//	else
//	{
//	}
//}
void DataProcesser::SendDeviceState()
{
	if(mTrainState->bZongXian == CAN0)
	{
		_SendData  pSendData0;
		mFrameGenerate.GetFrame_DeviceState(pSendData0,BOARD_BROADCAST,CAN0,PRIORITY0,ACK0,BASEFRAME);
		SendInfoForCan(pSendData0);
	}
	else if(mTrainState->bZongXian == CAN1)
	{
		_SendData  pSendData1;
		mFrameGenerate.GetFrame_DeviceState(pSendData1,BOARD_BROADCAST,CAN1,PRIORITY0,ACK0,BASEFRAME);
		SendInfoForCan(pSendData1);
	}
	else
	{
		mLogInstance->Log("zongxian be setted");
	}
}
void DataProcesser::SendIntegeInfo()
{
	if(mTrainState->bZongXian == CAN0)
	{
		_SendData  pSendData0;
		mFrameGenerate.GetFrame_IntegeInfo(pSendData0,BOARD_BROADCAST,CAN0,PRIORITY0,ACK0,BASEFRAME);
		SendInfoForCan(pSendData0);
	}
	else if(mTrainState->bZongXian == CAN1)
	{
		_SendData  pSendData1;
		mFrameGenerate.GetFrame_IntegeInfo(pSendData1,BOARD_BROADCAST,CAN1,PRIORITY0,ACK0,BASEFRAME);
		SendInfoForCan(pSendData1);
	}
	else
	{
		mLogInstance->Log("zongxian be setted");
	}
}

void DataProcesser::subApplyForDataFromOtherBoard(_RawInfo& PackageInfo)
{
	try
	{
		mTrainState->lOtherBoardRecvTime = time((time_t*)NULL);

		switch (PackageInfo.bServiceType)
		{
		case (byte) 0xE0:
			{
				switch (PackageInfo.bCommand)
				{
				case 0x02://��������
					{
						mLogInstance->Log("receive device start info command from otherboard");
						mLogInstance->Log("otherboard have started");
					}
					break;
				case 0x05://ģʽ�л�
					{
						mLogInstance->Log("receive workstate change command from otherboard");
						byte bData = PackageInfo.vRawInfoData[0];
						if(bData == 0x01)
							mTrainState->bWorkState = WORKSTATE_NORMAL;
						else if(bData == 0x02)
							mTrainState->bWorkState = WORKSTATE_TEST;
						else if(bData == 0x03)
							mTrainState->bWorkState = WORKSTATE_REPAIR;
						else
							mLogInstance->Log("workstate code is not defined");
					}
					break;
				case 0x06://�л�������
						{
							mLogInstance->Log("receive mainused zhukong change command from otherboard");
							byte bData = PackageInfo.vRawInfoData[0];
							if(bData == 0x01)
							{
								mTrainState->bMainUsed = true;
								mLogInstance->Log("mainused change to true because be set by otherboard");//mLogInstance->Log("mainused change to true because be set by otherboard");
							}
							else
							{
								mTrainState->bMainUsed = false;
								mLogInstance->Log("mainused change to false because be set by otherboard");
							}
							CheckCanRoad();
							if(mTrainState->bMainUsed)
								SendDeviceState();
						}
						break;
				case 0x07://ͨ��״̬��Ϣ
					{
						if(bLogPrint2)
							mLogInstance->Log("receive tongxin state info command from otherboard");

						byte bData = mTrainState->arrCanState[(int)PackageInfo.bSourCode];
						mTrainState->arrCanState[(int)PackageInfo.bSourCode] = PackageInfo.vRawInfoData[0];
						if(bData != PackageInfo.vRawInfoData[0])
						{//�����仯�������+����can״̬�㲥ʱ���

							mLogInstance->Log("otherboard can zongxian state changed");
							CheckCanRoad();
						}
					}
					break;
				case 0xFF://XXXXX
					{

					}
					break;
				default:
					mLogInstance->Log("command code is not defined");
					break;
				}
			}
			break;
		case (byte) 0xE3://XXXXX
			{
				switch (PackageInfo.bCommand)
				{
				case 0x02://�豸״̬ͨ��
					{
						if(bLogPrint2)
							mLogInstance->Log("receive device state info command from otherboard");

						//�����û��ܵ�,���ð�ֻ����
						if((PackageInfo.vRawInfoData[11] & BIT2) != 0  && mTrainState->bZongXian != CAN0)
						{
							mTrainState->bZongXian = CAN0;
							mLogInstance->Log("bZongXian is set to CAN0 by main board");
						}
						if((PackageInfo.vRawInfoData[11] & BIT3) != 0  && mTrainState->bZongXian != CAN1)
						{
							mTrainState->bZongXian = CAN1;
							mLogInstance->Log("bZongXian is set to CAN1 by main board");
						}

						if(mTrainState->bMainUsed)
						{
							mLogInstance->Log("should not receive this meassage1");
							//��ͻ
						}
						else
						{
							if((PackageInfo.vRawInfoData[11] & BIT0) != 0 && mTrainState->bZhuKongFlag ==(byte)BOARD_ZK_LEFT )
							{
								//��������
								mLogInstance->Log("should not receive this meassage2");
							}
							else if((PackageInfo.vRawInfoData[11] & BIT0) != 0 && mTrainState->bZhuKongFlag ==(byte)BOARD_ZK_RIGHT )
							{
								//ok
							}
							else if((PackageInfo.vRawInfoData[11] & BIT1) != 0 && mTrainState->bZhuKongFlag ==(byte)BOARD_ZK_LEFT )
							{
								//ok
							}
							else if((PackageInfo.vRawInfoData[11] & BIT1) != 0 && mTrainState->bZhuKongFlag ==(byte)BOARD_ZK_RIGHT )
							{
								//��������
								mLogInstance->Log("should not receive this meassage3");
							}
						}
					}
					break;
				case 0x03:
					{//�ۺ���Ϣ,������
					}
					break;
				case 0xFF://XXXXX
					{

					}
					break;
				default:
					mLogInstance->Log("command code is not defined");
					break;
				}
			}
			break;
		case (byte) 0xE1://XXXXX
			{

			}
			break;
		default:
			std::cout<<"servicetype code is not defined:";//mLogInstance->Log("servicetype code is not defined");
			std::cout<<"{"<<(int)(PackageInfo.bZongXian)<<"}<--"
							<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bSourCode)<<"]:"
							<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bDectCode)<<"]<--"
							<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bServiceType)<<"]:"
							<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bCommand)<<"]<--{"
							<<PackageInfo.iDataLenth<<"} ";
			for(int i =0; i< PackageInfo.iDataLenth; i++)
			{
				int tm = PackageInfo.vRawInfoData[i];
				std::cout<< std::hex << std::setw(2) <<std::setfill('0') << tm << " ";
			}
			std::cout<<std::endl;
			break;
		}
	}
	catch(...)
	{

	}
}

void DataProcesser::subApplyForDataFromMMI(_RawInfo& PackageInfo)
{
	try
	{
		if(mTrainState->bMainMMI == BOARD_MMI_ALL)
		{//˭���ȷ�����Ϣ˭������MMI,�ݶ��Ĳ���
			mTrainState->bMainMMI = PackageInfo.bSourCode;
		}

		switch (PackageInfo.bServiceType)
		{
		case (byte) 0xE0:
			{
				switch (PackageInfo.bCommand)
				{
				case 0x02://��������
					{
						mLogInstance->Log("receive device start info command from mmi");
						mLogInstance->Log("mmi have started");
					}
					break;
				case 0x07://ͨ��״̬��Ϣ
					{
						if(bLogPrint2)
							mLogInstance->Log("receive tongxin state info command from mmi");

						byte bData = mTrainState->arrCanState[(int)PackageInfo.bSourCode];
						mTrainState->arrCanState[(int)PackageInfo.bSourCode] = PackageInfo.vRawInfoData[0];
						if(bData != PackageInfo.vRawInfoData[0])
						{//�����仯�������+����can״̬�㲥ʱ���
							mLogInstance->Log("mmi can zongxian state changed");
							CheckCanRoad();
						}
					}
					break;
				case 0xFF://XXXXX
					{

					}
					break;
				default:
					mLogInstance->Log("command code is not defined");
					break;
				}
			}
			break;
		case (byte) 0x03://XXXXX
			{
				MrmGprsHandle* mMrmGprsHandle ;
				if(mMrmGprsHandle0->bActor == MRM_MODE)
				{
					mMrmGprsHandle = mMrmGprsHandle0;

					if(mMrmGprsHandle1->bActor == MRM_MODE)
						mLogInstance->Log("mrm mode is not right");
				}
				else if(mMrmGprsHandle1->bActor == MRM_MODE)
				{
					mMrmGprsHandle = mMrmGprsHandle1;
				}
				else
					mLogInstance->Log("mrm mode is not right");

				switch (PackageInfo.bCommand)
				{
					case 0x03:////MMIժ�Ҷ�
						{
							mLogInstance->Log("receive zhai/gua hook command from mmi");

							mTrainState->bHookFlag = PackageInfo.vRawInfoData[0];
							if((mTrainState->iWorkMode == 0x65)||((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))
							{
								if(mTrainState->bHookFlag == 0x00 && (mMrmGprsHandle->bMrmStatus == 0x21 ||mMrmGprsHandle->bMrmStatus == 0x31 ||mMrmGprsHandle->bMrmStatus == 0x41))
								{//�һ�
									mMrmGprsHandle->SetMrmGprsStatus(BIT3,true,true,0x00);
								}
								else if(mTrainState->bHookFlag == 0x01)
								{//ժ��
									int i=0;
									for(i=0; i<mMrmGprsHandle->vPToPbuffer.size();i++)
										if(mMrmGprsHandle->vPToPbuffer[i].strPtpcsStat.compare("4") ==0)
											break;
									if(i<mMrmGprsHandle->vPToPbuffer.size())
									{
										mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcType = "1";
										mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcNumber =mMrmGprsHandle->vPToPbuffer[i].strPtpcsNumber;

										mMrmGprsHandle->SetMrmGprsStatus(BIT8,true,true,0x00);
									}
									else
									{
										for(i=0; i<mMrmGprsHandle->vVGCbuffer.size();i++)
											if(mMrmGprsHandle->vVGCbuffer[i].strGcStat.compare("2") ==0)
												break;
										if(i<mMrmGprsHandle->vVGCbuffer.size())
										{
											mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcType = "17";
											mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcGID = mMrmGprsHandle->vVGCbuffer[i].strGcGID;
											mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcGCA = mMrmGprsHandle->vVGCbuffer[i].strGcGCA;
											mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcIs17Or18 = mMrmGprsHandle->vVGCbuffer[i].strGcIs17Or18;
											mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcPriority = mMrmGprsHandle->vVGCbuffer[i].strGcPriority;


											mMrmGprsHandle->SetMrmGprsStatus(BIT8,true,true,0x00);
										}
									}
									std::cout<<GetCurrentTime()<<mMrmGprsHandle->tag<<"mMMISelectCallNumber.strMmiSlcType="<<mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcType
									<<" mMMISelectCallNumber.strMmiSlcNumber="<<mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcNumber
									<<" mMMISelectCallNumber.strMmiSlcGID="<<mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcGID
									<<" mMMISelectCallNumber.strMmiSlcGCA="<<mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcGCA
									<<" mMMISelectCallNumber.strMmiSlcIs17Or18="<<mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcIs17Or18
									<<" mMMISelectCallNumber.strMmiSlcPriority="<<mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcPriority<<std::endl;
								}

								//if(mTrainState->iWorkMode == 0x65)
								//{//ֻ����GSM-Rģʽ�£�����450MHz��̨ת��ժ�һ���Ϣ��450Mģʽ�£�MMI��������450MHz��̨����ժ�һ���
								//	//����Ҫ??????
								//}
							}
						}
						break;
					case 0x04://MMI(PTT)����
						{
							mLogInstance->Log("receive ptt command from mmi");

							mTrainState->bPttFlag = PackageInfo.vRawInfoData[0];
							if((mTrainState->iWorkMode == 0x65)||((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))
							{
								if(mMrmGprsHandle->bMrmStatus == 0x41)
								{
									if(mTrainState->bPttFlag == 0)
									{
										mMrmGprsHandle->SetMrmGprsStatus(BIT9,true,true,0x00);//����PTT����������£�g_MRMPushPTT_Event,���������ŵ�
									}
									else
									{
										mMrmGprsHandle->SetMrmGprsStatus(BIT10,true,true,0x00);//�ͷ�PTT����������£�g_MRMReleasePTT_Event���ͷ������ŵ�
									}
								}
							}
							else if(mTrainState->iWorkMode != 0x00)
							{
								//2.0ʱҪת��,3.0ʱMMIֱ�ӷ�����̨��ת��
							}
						}
						break;
				case 0x0C://����
					{
						mLogInstance->Log("receive dial a number command from mmi");

						if(mMrmGprsHandle->bMrmStatus == 0x01)
						{//ģ�����
							if(PackageInfo.vRawInfoData[0] == 0x01 || PackageInfo.vRawInfoData[0] == 0x03 )
								mMrmGprsHandle->strDialType = "1";
							else
								mMrmGprsHandle->strDialType = "17";

							mMrmGprsHandle->strDialPriority = "";
							mMrmGprsHandle->strDialPriority += PackageInfo.vRawInfoData[1];

							mMrmGprsHandle->strDialNumber ="";
							for(int i=2;PackageInfo.vRawInfoData[i]!=';' && i<PackageInfo.iDataLenth; i++)
							{
								mMrmGprsHandle->strDialNumber += PackageInfo.vRawInfoData[i];
							}

							if(mMrmGprsHandle->strDialNumber.size() >= 2 && mMrmGprsHandle->strDialNumber.substr(0,2).compare("*#") == 0)
							{
								std::string strTempString = mMrmGprsHandle->strDialNumber.substr(2);
								if(strTempString.compare("101") == 0)
								{
									mMrmGprsHandle->SendAT("AT+COPS?");
									mLogInstance->Log(mMrmGprsHandle->tag + "test send AT+COPS?");
								}
								else
								{

								}
								mMrmGprsHandle->CallListToMMI(0);
							}
							else
							{
								mLogInstance->Log("1.strDialNumber="+mMrmGprsHandle->strDialNumber+" strDialType="+mMrmGprsHandle->strDialType+" strDialPriority="+mMrmGprsHandle->strDialPriority);
								mMrmGprsHandle->SetMrmGprsStatus(BIT5,true,true,0x00);
							}
						}
						else
						{
							//����ǽ�������,�ҵ����ͨ��,���ȱ�֤��������
							if(PackageInfo.vRawInfoData[0] == 0x02 && (mMrmGprsHandle->bMrmStatus == 0x31 || mMrmGprsHandle->bMrmStatus == 0x41))
							{
								mMrmGprsHandle->strDialType = "17";
								mMrmGprsHandle->strDialNumber ="";
								for(int i=2;PackageInfo.vRawInfoData[i]!=';' && i<PackageInfo.iDataLenth; i++)
								{
									mMrmGprsHandle->strDialNumber += PackageInfo.vRawInfoData[i];
								}
							}
							mMrmGprsHandle->strDialPriority = "";
							mMrmGprsHandle->strDialPriority += PackageInfo.vRawInfoData[1];
							mLogInstance->Log("2.strDialNumber="+mMrmGprsHandle->strDialNumber+" strDialType="+mMrmGprsHandle->strDialType+" strDialPriority"+mMrmGprsHandle->strDialPriority);
							if(mMrmGprsHandle->strDialNumber.compare("299")==0)
							{
								mMrmGprsHandle->SetMrmGprsStatus(BIT3,true,true,0x00);
								mMrmGprsHandle->bNoListToMMIFlag =1;//�˳������ʱ��,��Ҫ��MMI�����б�,����,����ʱ���ϵĲ��,�������˳��漴�����õĽ�������,�ص�ԭ���˳������
								mMrmGprsHandle->SetMrmGprsStatus(BIT5,true,true,0x00);
							}
						}
					}
					break;
				case 0x0D:////MMI�Ҷϡ�����ָ��ͨ��
					{
						mLogInstance->Log("receive open/close a number command from mmi");

						if(PackageInfo.vRawInfoData[0] == 0x00)
						{//�Ҷ�
							mLogInstance->Log("hang up the call");
							mMrmGprsHandle->SetMrmGprsStatus(BIT7,true,true,0x00);
						}
						else if(PackageInfo.vRawInfoData[0] == 0x01)
						{//����ָ��
							if(PackageInfo.vRawInfoData[1] <= mMrmGprsHandle->vPToPbuffer.size() + mMrmGprsHandle->vVGCbuffer.size())
							{
								if(PackageInfo.vRawInfoData[1] - 1 < mMrmGprsHandle->vPToPbuffer.size())//��Ҫ��ͨ����
								{
									mLogInstance->Log("open up the signal call");
									mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcType = "1";
									mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcNumber =mMrmGprsHandle->vPToPbuffer[PackageInfo.vRawInfoData[1] - 1 ].strPtpcsNumber;
								}
								else //��Ҫ��ͨ���
								{
									mLogInstance->Log("open up the group call");
									mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcType = "17";
									mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcGID = mMrmGprsHandle->vVGCbuffer[PackageInfo.vRawInfoData[1] - 1 - mMrmGprsHandle->vPToPbuffer.size()].strGcGID;
									mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcGCA = mMrmGprsHandle->vVGCbuffer[PackageInfo.vRawInfoData[1] - 1 - mMrmGprsHandle->vPToPbuffer.size()].strGcGCA;
									mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcIs17Or18 = mMrmGprsHandle->vVGCbuffer[PackageInfo.vRawInfoData[1] - 1 - mMrmGprsHandle->vPToPbuffer.size()].strGcIs17Or18;
									mMrmGprsHandle->mMMISelectCallNumber.strMmiSlcPriority = mMrmGprsHandle->vVGCbuffer[PackageInfo.vRawInfoData[1] - 1 - mMrmGprsHandle->vPToPbuffer.size()].strGcPriority;
								}
								mMrmGprsHandle->SetMrmGprsStatus(BIT5,true,true,0x00);
							}
						}
						else
						{
							mLogInstance->Log("hang up the call");
						}
					}
					break;
				case 0x23://MMI��������
					{
						mLogInstance->Log("receive request mainused mmi command from mmi");

						mTrainState->bMainMMI = PackageInfo.bSourCode;
						std::cout<<"bMainMMI=" <<(int)mTrainState->bMainMMI;

						SendDeviceState();
					}
					break;
				case 0xFF://XXXXX
					{

					}
					break;
				default:
					mLogInstance->Log("command code is not defined");
					break;
				}
			}
			break;
		case (byte) 0x06: ////����MMI���͵ĵ����������
			{
				MrmGprsHandle* mMrmGprsHandle ;
				if(mMrmGprsHandle0->bActor == GPRS_MODE)
				{
					mMrmGprsHandle = mMrmGprsHandle0;

					if(mMrmGprsHandle1->bActor == GPRS_MODE)
						mLogInstance->Log("gprs mode is not right");
				}
				else if(mMrmGprsHandle1->bActor == GPRS_MODE)
				{
					mMrmGprsHandle = mMrmGprsHandle1;
				}
				else
					mLogInstance->Log("gprs mode is not right");

				switch (PackageInfo.bCommand)
				{
				case 0x51://�Զ�ȷ��
					{
						if(bLogPrint2)
							mLogInstance->Log("receive zi dong que ren from mmi");
						if(PackageInfo.vRawInfoData[0] == 0x81)
						{
							mDispatchCommandFileHandle->AddAutoAckFile(PackageInfo);
						}
						else
						{

						}
					}
					break;
				case 0xFF://XXXXX
					{

					}
					break;
				default:
					mLogInstance->Log("command code is not defined");
					break;
				}
			}
			break;
		default:
			std::cout<<"servicetype code is not defined:";//mLogInstance->Log("servicetype code is not defined");
			std::cout<<"{"<<(int)(PackageInfo.bZongXian)<<"}<--"
							<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bSourCode)<<"]:"
							<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bDectCode)<<"]<--"
							<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bServiceType)<<"]:"
							<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bCommand)<<"]<--{"
							<<PackageInfo.iDataLenth<<"} ";
			for(int i =0; i< PackageInfo.iDataLenth; i++)
			{
				int tm = PackageInfo.vRawInfoData[i];
				std::cout<< std::hex << std::setw(2) <<std::setfill('0') << tm << " ";
			}
			std::cout<<std::endl;
			break;
		}
	}
	catch(...)
	{

	}
}

void DataProcesser::subApplyForDataFromZhuanHuan(_RawInfo& PackageInfo)
{
	try
	{
		if(BOARD_ZH_ALL == PackageInfo.bSourCode)
		{
			mLogInstance->Log("zhuan huan board should not use 0x22");
			return;
		}
		MrmGprsHandle* mMrmGprsHandle ;
		if(PackageInfo.bSourCode == BOARD_ZH_LEFT)
			mMrmGprsHandle = mMrmGprsHandle0;
		else
			mMrmGprsHandle = mMrmGprsHandle1;

		switch (PackageInfo.bServiceType)
		{
		case (byte) 0xE0:
			switch (PackageInfo.bCommand)
			{
			case 0x07://����״̬��Ϣ
				{
					if(bLogPrint2)
						mLogInstance->Log("receive tongxin state info command from zhuan huan");

					byte bData = mTrainState->arrCanState[(int)PackageInfo.bSourCode];
					mTrainState->arrCanState[(int)PackageInfo.bSourCode] = PackageInfo.vRawInfoData[0];
					if(bData != PackageInfo.vRawInfoData[0])
					{//�����仯�������+����can״̬�㲥ʱ���
						mLogInstance->Log("zhuanhuanboard can zongxian state changed");
						CheckCanRoad();
					}
				}
				break;
			default:
				mLogInstance->Log("command code is not defined");
				break;
			}
			break;
		case (byte) 0x01://
			{
				switch (PackageInfo.bCommand)
				{
				case 0x02://AT����
					{
						if(bLogPrint2)
							mLogInstance->Log("receive at command from zhuan huan");

						std::string strAtCommand;
						strAtCommand.assign( PackageInfo.vRawInfoData.begin(), PackageInfo.vRawInfoData.end());

						if(strAtCommand.size()>=2 && strAtCommand[0]==0x0d && strAtCommand[1]==0x0a)
						{
							if(bLogPrint3)
								mLogInstance->Log("recv a newline flag:"+strAtCommand);
						}
						else
						{
							try
							{
								std::unique_lock <std::mutex> lck(mMrmGprsHandle->mtx_AtCommand);
								mMrmGprsHandle->listAtCommand.push_back(strAtCommand);
								mMrmGprsHandle->mwaitAtCommand.notify_all();
							}
							catch(...)
							{
								mLogInstance->Log("push atcommand list to wrong");
							}
						}
					}
					break;

				case 0x03://����
					{
						if(bLogPrint2)
							mLogInstance->Log(mMrmGprsHandle->tag + "receive gprs data from zhuan huan");

						_RawInfo newPackageInfo;
						newPackageInfo.bZongXian = PackageInfo.bZongXian;
						newPackageInfo.bNeedAck =  PackageInfo.bNeedAck;
						newPackageInfo.bType = PackageInfo.bType;
						newPackageInfo.bFeature = PackageInfo.bFeature;

						for(int i=0; i<PackageInfo.vRawInfoData.size(); i++)
						{
							newPackageInfo.vRawInfoData.push_back(PackageInfo.vRawInfoData[i]);
						}
						if(newPackageInfo.vRawInfoData.size() > 0)
						{
							bool flag = ParseRawInfo(&(newPackageInfo.vRawInfoData[0]), newPackageInfo.vRawInfoData.size(),newPackageInfo);
							if(flag)
							{
								//if(bLogPrint2)
								//{
									std::cout<<std::endl<<"{"<<(int)(newPackageInfo.bZongXian)<<"}--"
													<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(newPackageInfo.bSourCode)<<"--"
													<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(newPackageInfo.bSourAddreLenth)<<"--"
													<< std::dec <<(int)(newPackageInfo.arrSourAddre[0])<<"."<<(int)(newPackageInfo.arrSourAddre[1])<<"."<<(int)(newPackageInfo.arrSourAddre[2])<<"."<<(int)(newPackageInfo.arrSourAddre[3])<<"]--"

													<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(newPackageInfo.bDectCode)<<"--"
													<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(newPackageInfo.bDectAddreLenth)<<"--"
													<< std::dec <<(int)(newPackageInfo.arrDectAddre[0])<<"."<<(int)(newPackageInfo.arrDectAddre[1])<<"."<<(int)(newPackageInfo.arrDectAddre[2])<<"."<<(int)(newPackageInfo.arrDectAddre[3])<<"]--"

													<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(newPackageInfo.bServiceType)<<"]:"
													<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(newPackageInfo.bCommand)<<"]--{"
													<<std::dec<<newPackageInfo.iDataLenth<<"} ";
									std::vector<byte>::iterator it = newPackageInfo.vRawInfoData.begin();
									while(it != newPackageInfo.vRawInfoData.end())
									{
										int tm = *it;
										std::cout<< std::hex << std::setw(2) <<std::setfill('0') << tm << " ";
										it++;
									}
									std::cout<<std::endl<<std::endl;
								//}

								//����GPRS����
								subApplyForDataFromGprs(newPackageInfo,PackageInfo.bSourCode);
							}
							else
							{
								mLogInstance->Log("can not prase can data, please attention");
							}
						}
					}
					break;
				case 0x55://�汾��Ϣ
					{
						mLogInstance->Log("receive version info from zhuan huan");

						std::string strRecv;
						strRecv.assign( PackageInfo.vRawInfoData.begin(), PackageInfo.vRawInfoData.end());
						mLogInstance->Log("zhuan huan version: "+strRecv);
					}
					break;
				default:
					mLogInstance->Log("command code is not defined");
					break;
				}
			}
			break;

		default:
			std::cout<<"servicetype code is not defined:";//mLogInstance->Log("servicetype code is not defined");
			std::cout<<"{"<<(int)(PackageInfo.bZongXian)<<"}<--"
							<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bSourCode)<<"]:"
							<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bDectCode)<<"]<--"
							<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bServiceType)<<"]:"
							<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(PackageInfo.bCommand)<<"]<--{"
							<<PackageInfo.iDataLenth<<"} ";
			for(int i =0; i< PackageInfo.iDataLenth; i++)
			{
				int tm = PackageInfo.vRawInfoData[i];
				std::cout<< std::hex << std::setw(2) <<std::setfill('0') << tm << " ";
			}
			std::cout<<std::endl;
			break;
		}
	}
	catch(...)
	{

	}
}
void DataProcesser::subApplyForDispatchCommand(_RawInfo& PackageInfo, byte bComingPort)
{
	mLogInstance->Log("subApplyForDispatchCommand 1");

	string strDestinationIp;
	strDestinationIp += PackageInfo.arrSourAddre[0];
	strDestinationIp += PackageInfo.arrSourAddre[1];
	strDestinationIp += PackageInfo.arrSourAddre[2];
	strDestinationIp += PackageInfo.arrSourAddre[3];
	int iPort = 20001 ;
	std::cout<<"strDestinationIp="<<strDestinationIp<<" iPort="<<iPort<<std::endl;

	_InfoPackage ipCommandPackage;
	_InfoToFile itfStoreInfo;

	byte arrCommandCode[6];
	byte bPackageSum,bPackageNo;
	byte bRecvFlag=0,bAckFlag=0,bTimeAdjustFlag=0;
	byte TrainNumber[7],EngineNumber[8];


	memcpy(TrainNumber,&(PackageInfo.vRawInfoData[10]),7);
	for(int i=0; i<7; i++)
		if(TrainNumber[i]>=0x61 && TrainNumber[i]<=0x7A)
			TrainNumber[i] -= 0x20;
	std::string tmpTrainNumber(TrainNumber,7);

	memcpy(EngineNumber,&(PackageInfo.vRawInfoData[17]),8);
	for(int i=0; i<8; i++)
		if(EngineNumber[i]>=0x61 && EngineNumber[i]<=0x7A)
			EngineNumber[i] -= 0x20;
	std::string tmpEngineNumber(EngineNumber,8);

	memcpy(arrCommandCode,&(PackageInfo.vRawInfoData[26]),6);
	bPackageSum = PackageInfo.vRawInfoData[46];
	bPackageNo = PackageInfo.vRawInfoData[47];
	ipCommandPackage.vInfoData.push_back(PackageInfo.bSourCode);
	for(int i=0; i<6;i++)
		ipCommandPackage.vInfoData.push_back(PackageInfo.arrSourAddre[i]);
	ipCommandPackage.vInfoData.push_back(PackageInfo.bDectCode);
	for(int i=0; i<6;i++)
		ipCommandPackage.vInfoData.push_back(PackageInfo.arrDectAddre[i]);
	for(int i=0; i<PackageInfo.iDataLenth;i++)
		ipCommandPackage.vInfoData.push_back(PackageInfo.vRawInfoData[i]);

	//�ж��Ƿ�Ϊ��Ҫ���յĵ���������Ƿ����Զ�ȷ��
	std::cout<<"tmpTrainNumber:"<<tmpTrainNumber<<", mTrainState->strTrainNumber:"<<mTrainState->strTrainNumber
		<<", tmpEngineNumber:"<<tmpEngineNumber<<", mTrainState->strEngineNumber:"<<mTrainState->strEngineNumber<<std::endl;

	bRecvFlag =0, bAckFlag =0, bTimeAdjustFlag =0 ;
	if(mTrainState->iBenBuStatus == 1)
	{
		bRecvFlag =0;
		bAckFlag =0;
		bTimeAdjustFlag =0 ;

		std::cout<<"1.bRecvFlag:"<<(int)bRecvFlag <<" bAckFlag:"<<(int)bAckFlag<<" bTimeAdjustFlag:"<<(int)bTimeAdjustFlag<<std::endl;
	}
	else if(mTrainState->strTrainNumber.compare("XXXXXXX") == 0)
	{

		 if(tmpEngineNumber.compare("XXXXXXXX") == 0)
		 {
			bRecvFlag =0;
			bAckFlag =0;
			bTimeAdjustFlag =0 ;
			std::cout<<"2.bRecvFlag:"<<(int)bRecvFlag <<" bAckFlag:"<<(int)bAckFlag<<" bTimeAdjustFlag:"<<(int)bTimeAdjustFlag<<std::endl;
		 }
		 else if(tmpEngineNumber.compare(mTrainState->strEngineNumber) == 0)
		 {
			bRecvFlag =1;
			bAckFlag =1;
			//TimeAdjust_Flag = 1;
			std::cout<<"3.bRecvFlag:"<<(int)bRecvFlag <<" bAckFlag:"<<(int)bAckFlag<<" bTimeAdjustFlag:"<<(int)bTimeAdjustFlag<<std::endl;
		 }
		 else
			 std::cout<<"4.bRecvFlag:"<<(int)bRecvFlag <<" bAckFlag:"<<(int)bAckFlag<<" bTimeAdjustFlag:"<<(int)bTimeAdjustFlag<<std::endl;
	}
	else if(mTrainState->strTrainNumber.compare("XXXXXXX") != 0)
	{
		if(tmpTrainNumber.compare("XXXXXXX") == 0 && tmpEngineNumber.compare("XXXXXXXX") == 0)
		 {
			bRecvFlag =0;
			bAckFlag =0;
			bTimeAdjustFlag =0 ;
			std::cout<<"5.bRecvFlag:"<<(int)bRecvFlag <<" bAckFlag:"<<(int)bAckFlag<<" bTimeAdjustFlag:"<<(int)bTimeAdjustFlag<<std::endl;
		 }
		else if(tmpTrainNumber.compare(mTrainState->strTrainNumber) == 0 && tmpEngineNumber.compare(mTrainState->strEngineNumber) == 0)
		 {
			bRecvFlag =1;
			bAckFlag =1;
			//TimeAdjust_Flag = 1;
			std::cout<<"6.bRecvFlag:"<<(int)bRecvFlag <<" bAckFlag:"<<(int)bAckFlag<<" bTimeAdjustFlag:"<<(int)bTimeAdjustFlag<<std::endl;
		 }
		else if(tmpTrainNumber.compare("XXXXXXX") == 0 && tmpEngineNumber.compare(mTrainState->strEngineNumber) == 0)
		 {
			bRecvFlag =1;
			bAckFlag =1;
			//TimeAdjust_Flag = 1;
			std::cout<<"7.bRecvFlag:"<<(int)bRecvFlag <<" bAckFlag:"<<(int)bAckFlag<<" bTimeAdjustFlag:"<<(int)bTimeAdjustFlag<<std::endl;
		 }
		else if(tmpTrainNumber.compare(mTrainState->strTrainNumber) == 0  && tmpEngineNumber.compare("XXXXXXXX") == 0)
		 {
			bRecvFlag =1;
			bAckFlag =1;
			//TimeAdjust_Flag = 1;
			std::cout<<"8.bRecvFlag:"<<(int)bRecvFlag <<" bAckFlag:"<<(int)bAckFlag<<" bTimeAdjustFlag:"<<(int)bTimeAdjustFlag<<std::endl;
		 }
		else
			std::cout<<"9.bRecvFlag:"<<(int)bRecvFlag <<" bAckFlag:"<<(int)bAckFlag<<" bTimeAdjustFlag:"<<(int)bTimeAdjustFlag<<std::endl;
	}
	else
		std::cout<<"10.bRecvFlag:"<<(int)bRecvFlag <<" bAckFlag:"<<(int)bAckFlag<<" bTimeAdjustFlag:"<<(int)bTimeAdjustFlag<<std::endl;

	mLogInstance->Log("subApplyForDispatchCommand 2");

	if(PackageInfo.iDataLenth>42 && PackageInfo.vRawInfoData[0]==0x20 && mTrainState->iEnPort != PackageInfo.vRawInfoData[42])
	{//������⣬�����˺Ų�һ��ʱ����Ӧ
		bRecvFlag = 0;
		bAckFlag = 0;
		bTimeAdjustFlag = 0;
	}
	if(PackageInfo.iDataLenth>0 && PackageInfo.vRawInfoData[0]==0x20)
	{//������⣬������ʱ��
		bTimeAdjustFlag = 0;
	}

	if(bRecvFlag !=0)
	{
		mLogInstance->Log("subApplyForDispatchCommand 3");

		int iIsDifferent;
		if(memcmp(arrLastBufRx,&(PackageInfo.vRawInfoData[0]),7) || memcmp(arrLastBufRx+10,&(PackageInfo.vRawInfoData[10]),PackageInfo.iDataLenth-10))
			iIsDifferent = 1;
		else
			iIsDifferent = 0;
		memcpy(arrLastBufRx,&(PackageInfo.vRawInfoData[0]),PackageInfo.iDataLenth);

		byte bDisptchCommandStatus;
		byte MsgTimeinfo[] = {"DA����ʱ�䣺XXʱXX��XX��DA���յص㣺����� XXXX.XDA"};
		char bArrNow[6];
		GetCurrentTime2(bArrNow);
		MsgTimeinfo[0] = 0x0d;
		MsgTimeinfo[1] = 0x0a;
		MsgTimeinfo[12] = (bArrNow[3]/10)+0x30;
		MsgTimeinfo[13] = (bArrNow[3]%10)+0x30;
		MsgTimeinfo[16] = (bArrNow[4]/10)+0x30;
		MsgTimeinfo[17] = (bArrNow[4]%10)+0x30;
		MsgTimeinfo[20] = (bArrNow[5]/10)+0x30;
		MsgTimeinfo[21] = (bArrNow[5]%10)+0x30;
		MsgTimeinfo[24] = 0x0d;
		MsgTimeinfo[25] = 0x0a;
		if(mTrainState->bTrainNumberStatusRxReportFlag != 0) //��ǰ�������Ч
		{
			if(
			(((mTrainState->arrCurrentKilometer[0]&0x0f)==0x0f)&&(mTrainState->arrCurrentKilometer[1]==0xff)&&(mTrainState->arrCurrentKilometer[2]==0xff))
			||(((mTrainState->arrCurrentKilometer[0]&0x0f)==0x09)&&(mTrainState->arrCurrentKilometer[1]==0x99)&&(mTrainState->arrCurrentKilometer[2]==0x99))
			)										//��ǰ�������Ч����ʾ�ص㣺----
			{
				for(int i=36;i<=39;i++)
					MsgTimeinfo[i] = '-';
				for(int i=40;i<=48;i++)
					MsgTimeinfo[i] = ' ';
			}
			else
			{
				if(mTrainState->bKilometerFlag)
					MsgTimeinfo[42] = '-';
				MsgTimeinfo[43] = (mTrainState->arrCurrentKilometer[0]&0x0f)+0x30;
				MsgTimeinfo[44] = ((mTrainState->arrCurrentKilometer[1]&0xf0)>>4)+0x30;
				MsgTimeinfo[45] = (mTrainState->arrCurrentKilometer[1]&0x0f)+0x30;
				MsgTimeinfo[46] = ((mTrainState->arrCurrentKilometer[2]&0xf0)>>4)+0x30;
				MsgTimeinfo[48] = (mTrainState->arrCurrentKilometer[2]&0x0f)+0x30;
			}
		}
		else //��ǰ�������Ч����ʾ�ص㣺----
		{
			for(int i=36;i<=39;i++)
				MsgTimeinfo[i] = '-';
			for(int i=40;i<=48;i++)
				MsgTimeinfo[i] = ' ';
		}
		MsgTimeinfo[49] = 0x0d;
		MsgTimeinfo[50] = 0x0a;

		if(bPackageSum == 1)
		{
			mLogInstance->Log("subApplyForDispatchCommand 5");

			//�õ�����״̬
			bDisptchCommandStatus = ipCommandPackage.vInfoData[54];
			//���ݱ�����״̬������״̬��ֵ
			if(mTrainState->iBenBuStatus ==0)
				bDisptchCommandStatus = bDisptchCommandStatus & (~ BIT0);
			else
				bDisptchCommandStatus = bDisptchCommandStatus | BIT0;
			mTrainState->bDisptchCommandStatus = bDisptchCommandStatus;
			ipCommandPackage.vInfoData[54] = bDisptchCommandStatus;

			for(int i=0;i<51;i++)
				ipCommandPackage.vInfoData.push_back(MsgTimeinfo[i]);

			//�洢��������
			if(iIsDifferent != 0)
			{
				mLogInstance->Log("subApplyForDispatchCommand 7");
				//�洢��������
				itfStoreInfo.vInfoData.assign(ipCommandPackage.vInfoData.begin(),ipCommandPackage.vInfoData.end());
				itfStoreInfo.iLost = 0;
				mDispatchCommandFileHandle->AddStoreFile(itfStoreInfo);

				//������������ǰ���������,��Ϊ��������
				while(!mDispatchCommandFileHandle->listDispatchPackage.empty())
				{
					_InfoToFile tmpInfoToFile = mDispatchCommandFileHandle->listDispatchPackage.front();
					tmpInfoToFile.iLost = 1;
					mDispatchCommandFileHandle->AddStoreFile(tmpInfoToFile);
					mDispatchCommandFileHandle->listDispatchPackage.pop_front();
				}

				PackageInfo.vRawInfoData[40] = PackageInfo.vRawInfoData[40]  & (~BIT1);//����Ϊ���յ��ĵ�������
				PackageInfo.vRawInfoData[40] = PackageInfo.vRawInfoData[40] | (mTrainState->iBenBuStatus);

				memcpy(mTrainState->arrCurrentDispatchCommandTrainNumber,TrainNumber,sizeof(TrainNumber));
				memcpy(mTrainState->arrCurrentDispatchCommandEngineNumber,EngineNumber,sizeof(EngineNumber));

				PackageInfo.AppendData(MsgTimeinfo,51);

				//���͵��������MMI
				_SendData  pSendData;
				mFrameGenerate.GetFrame_DispatchCommand(pSendData,mTrainState->bMainMMI,PackageInfo.bZongXian,PRIORITY3,ACK0,BASEFRAME,PackageInfo);
				SendInfoForCan(pSendData);

				mLogInstance->Log("send dispatch command");
			}
			else
			{//�յ���ͬ�ĵ����͵������������������Զ�ȷ��(��ͬʱ��MMI�����Զ�ȷ��)
				mLogInstance->Log("subApplyForDispatchCommand 8");
				if(PackageInfo.vRawInfoData[0] != 0x12)
				{//���ط����Զ�ȷ��
					_SendDataNet  pSendDataNet;
					mFrameGenerate.GetFrame_DispatchCommandAutoAck(pSendDataNet,PackageInfo,strDestinationIp,iPort);
					SendInfoForGprs(pSendDataNet);
				}
				else
				{
					//�����������յ��������Զ�ȷ��
					//����Ҫ����
				}
			}
		}
		else
		{
			mLogInstance->Log("subApplyForDispatchCommand 6");
		}
		memcpy(arrLastCommandCode,arrCommandCode,6);
	}
	else
	{
		mLogInstance->Log("subApplyForDispatchCommand 4");
	}

}
void DataProcesser::subApplyForDataFromGprs(_RawInfo& PackageInfo, byte bComingPort)
{
	try
	{
		mLogInstance->Log("subApplyForDataFromGprs in");
		std::cout<< std::hex << std::setw(2) <<std::setfill('0')<<"bSourCode:" <<(int)PackageInfo.bSourCode
				<< std::hex << std::setw(2) <<std::setfill('0')<<" bDectCode:"<<(int)PackageInfo.bDectCode
				<< std::hex << std::setw(2) <<std::setfill('0')<<" bServiceType:"<<(int)PackageInfo.bServiceType
				<< std::hex << std::setw(2) <<std::setfill('0')<<" bCommand:"<<(int)PackageInfo.bCommand<<std::endl;

		switch (PackageInfo.bDectCode)
		{
			case (byte) 0x01://����
				{
					switch (PackageInfo.bSourCode)
					{
						case (byte) 0x23://gprs
						case (byte) 0x24:
						case (byte) 0x31:
							{
								switch (PackageInfo.bServiceType)
								{
									case (byte) 0x06://��������
										{
											switch (PackageInfo.bCommand)
											{
												case (byte) 0x20://���������������
													{
														subApplyForDispatchCommand(PackageInfo, bComingPort);
													}
													break;
												default:
													{

													}
													break;
											}
										}
										break;
									case (byte) 0x05://���κ���Ϣ
										{

										}
										break;
									case (byte) 0x11://CTC ��MMI����ͼ�η�ʽ�г���·�Զ�Ԥ����Ϣ
										{

										}
										break;
									case (byte) 0x12://CTCά��������Ϣ
										{

										}
										break;
									case (byte) 0x13://�����������豸
										{

										}
										break;
									default:
										{

										}
										break;
								}
							}
							break;
						case 0x26://��β����GPRS
							{

							}
							break;
						case 0x27:////�ӿڷ�����
							{

							}
							break;
					default:
						{

						}
						break;
					}
				}
				break;
		case (byte) 0x11://DMIS�ܻ�����TAX���������������
			{

			}
			break;
		default:
			{

			}
			break;
		}
	}
	catch(...)
	{

	}
	mLogInstance->Log("subApplyForDataFromGprs out");
}

void DataProcesser::TestForCan()
{
	_SendData  pSendData0;
	mFrameGenerate.GetFrame_CanState(pSendData0,BOARD_BROADCAST,CAN0,PRIORITY0,ACK0,SHORTFRAME);
	_SendData  pSendData1;
	mFrameGenerate.GetFrame_CanState(pSendData1,BOARD_BROADCAST,CAN1,PRIORITY0,ACK0,SHORTFRAME);
	SendInfoForCan(pSendData0);
	SendInfoForCan(pSendData1);

}
void DataProcesser::StartReport(byte iTimes)
{
	_SendData  pSendData0;
	mFrameGenerate.GetFrame_StartReport(pSendData0,BOARD_BROADCAST,CAN0,PRIORITY0,ACK0,SHORTFRAME,iTimes);
	_SendData  pSendData1;
	mFrameGenerate.GetFrame_StartReport(pSendData1,BOARD_BROADCAST,CAN1,PRIORITY0,ACK0,SHORTFRAME,iTimes);
	SendInfoForCan(pSendData0);
	SendInfoForCan(pSendData1);
}
void DataProcesser::IntegrateInfoTx(byte port)
{

}
void DataProcesser::ChuanHaoFor450M(byte port)
{

}
