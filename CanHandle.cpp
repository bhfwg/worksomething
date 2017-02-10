/*
 * CanHandle.cpp
 *
 *  Created on: 2016-8-5
 *      Author: lenovo
 *  can�Ƿ��1002 1003 1010? ������ӣ���ctc�����ݴ�����ôת�ӵ�Ԫ���ͽ���ʱ��Ҫ��������
 *  �ڲ��������Ƿ��1002 1003 1010?����������֡����֡�Ƿ��������ֽ�
 *	����ֻ֡��һ��8�ֽڣ��ֶ����ʶ��ô��?���ֶ�?
 *
 */

#include "CanHandle.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/shm.h>
#include <net/if.h>
#include <signal.h>
CanHandle::CanHandle()
{
	tag = "CanHandle: ";
	mTrainState = TrainState::GetInstance();
	mParamOperation = ParamOperation::GetInstance();
	mLogInstance = LogInstance::GetInstance();
}
void CanHandle::Init(const int port)
{
	fd_can = -1;
	ibytes_read = -1;

	bPort = port;
	std::stringstream ss;
	ss  <<"can" << port<<" ";
	name = ss.str();

	if (!find_can(port))
	{
		mLogInstance->Log(tag+name+" socket find fail");
		exit(EXIT_FAILURE);
	}
	else
	{
		if(bLogPrint2)
			mLogInstance->Log(tag+name+" socket find successful");
	}
	close_can(port);// �����ȹر�CAN�����ܳɹ�����CAN������
	set_bitrate(port, CANBAUDRATE);// ����CAN֮ǰ����Ҫ���ò�����
	open_can(port);
	fd_can = socket_connect(port);
	if (fd_can < 0 )
	{
		disconnect(&fd_can);
		mLogInstance->Log(tag+name+"open socket can failed");
		exit(EXIT_FAILURE);
	}
	bRead = true;
	bHandleRawInfo = true;
	bWriteThreadGps = true;
	bWriteThreadOtherBoard = true;
	bWriteThreadNoAck = true;

	mblResponse_OK_Gps= false;
	mblResponse_OK_OtherBoard= false;

	if(bLogPrint2)
		mLogInstance->Log(tag+name+" is already");
}
void CanHandle::StartService()
{
	bRead = true;
	bHandleRawInfo = true;
	bWriteThreadGps = true;
	bWriteThreadOtherBoard = true;
	bWriteThreadNoAck = true;

	thdReceive = std::thread(std::mem_fn(&CanHandle::CanReceiveThread),this);
	thdHandleRawInfo = std::thread(std::mem_fn(&CanHandle::HandleRawInfoThread),this);
	thdWriteThreadGps = std::thread(std::mem_fn(&CanHandle::CanWriteThreadGps),this);
	thdWriteThreadOtherBoard = std::thread(std::mem_fn(&CanHandle::CanWriteThreadOtherBoard),this);
	thdWriteThreadNoAck = std::thread(std::mem_fn(&CanHandle::CanWriteThreadNoAck),this);
	mLogInstance->Log(tag+name+"StartService");
}

CanHandle::~CanHandle()
{

}
bool CanHandle::find_can(const int port)
{
    char buf[128] = {0};
    //sprintf(buf, "/sys/class/net/can%d/can_bittiming/bitrate", port);
    sprintf(buf, "/sys/class/net/can%d/dev_id", port);
    return  ((access(buf, 0) == 0));
}

void CanHandle::close_can(const int port)
{
    char	l_c8Command[64] = {0};
    sprintf(l_c8Command, "ifconfig can%d down", port);
    system(l_c8Command);

    if(bLogPrint2)
    	mLogInstance->Log(tag+name+" close successful");
}
void CanHandle::set_bitrate(const int port, const int bitrate)
{
	char	l_c8Command[128] = {0};
	//sprintf(l_c8Command, "echo %d > /sys/class/net/can%d/can_bittiming/bitrate", bitrate, port);
	sprintf(l_c8Command, "ip link set can%d type can bitrate %d", port, bitrate);
	system(l_c8Command);

	if(bLogPrint2)
		mLogInstance->Log(tag+name+" set bitrate successful");
}
void CanHandle::open_can(const int port)
{
    char	l_c8Command[64] = {0};
    sprintf(l_c8Command, "ifconfig can%d up", port);
    system(l_c8Command);

    if(bLogPrint2)
    	mLogInstance->Log(tag+name+" open successful");
}

int CanHandle::socket_connect(const int port)
{
    return  socket_listen(port);
}
void CanHandle::disconnect(int *sockfd)
{
    if (!sockfd || *sockfd == -1)
    {
        return ;
    }
    close_socket(*sockfd);
    *sockfd = -1;
}
int CanHandle::socket_listen(const int port)
{// ��sock��Ȼ������˿� ���ؼ��� �׽��� �ļ�������
    int sockfd = -1;

    struct sockaddr_can _addr;
    struct ifreq _ifreq;
    char buf[256];
    int ret = 0;

     /* �����׽��֣�����Ϊԭʼ�׽��֣�ԭʼCANЭ�� */
    sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sockfd < 0)
    {
        mLogInstance->Log(tag+name+"create socket error");
        return  -1;
    }

     /* �����Ƕ�CAN�ӿڽ��г�ʼ����������CAN�ӿ���������������ifconfig����ʱ��ʾ������ */
    sprintf(buf, "can%d", port);
    strcpy(_ifreq.ifr_name, buf);

    ret = ioctl(sockfd, SIOCGIFINDEX, &_ifreq);

    if (ret < 0)
    {
        mLogInstance->Log(tag+name+"ioctl socket error");
        return  -1;
    }

    /* ����CANЭ�� */
    _addr.can_family = AF_CAN;
    _addr.can_ifindex = _ifreq.ifr_ifindex;


    /* disable default receive filter on this RAW socket */
    /* This is obsolete as we do not read from the socket at all, but for */
    /* this reason we can remove the receive list in the Kernel to save a */
    /* little (really a very little!) CPU usage.                          */
    //	setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

    set_can_filter(sockfd);//

    /* �������ɵ��׽�����CAN�׽��ֵ�ַ���а� */
    ret = bind(sockfd, (struct sockaddr *)&_addr, sizeof(_addr));



    if ( ret < 0)
    {
        close_socket(sockfd);
        mLogInstance->Log(tag+name+"bind socket error");
        return  -1;
    }

    if(bLogPrint2)
    	mLogInstance->Log(tag+name+"connect and listen successful");

    return  sockfd;
}
void CanHandle::close_socket(const int sockfd)
{
    if (sockfd != -1)
    {
        close(sockfd);
    }
}
int CanHandle::set_can_filter(int sock)
{
    const int n = 3;
    struct can_filter rfilter[n];

    // ���˹��򣺵�<received_can_id> & mask == can_id & maskʱ�����ձ��ģ�������˵�. ����ͬʱ��Ӷ������˹���

    if(bLogPrint2)
    	mLogInstance->Log(tag+name+"set_can_filter begin");

    // ����������CAN֡��CAN_RAW�׽����ϲ������κ�CAN֡
    rfilter[0].can_mask = 0x04FF0000U;
    rfilter[0].can_id   = 0x04FF0000U;//���ý��յ�id

    rfilter[1].can_mask = 0x04FF0000U;
    rfilter[1].can_id   = 0x04120000U;//���ý��յ�id

    if(mTrainState->bZhuKongFlag == (byte)BOARD_ZK_LEFT)
    {
		rfilter[2].can_mask = 0x04FF0000U;
		rfilter[2].can_id   = 0x04130000U;//���ý��յ�id
    }
    else if(mTrainState->bZhuKongFlag == (byte)BOARD_ZK_RIGHT)
    {
		rfilter[2].can_mask = 0x04FF0000U;
		rfilter[2].can_id   = 0x04140000U;//���ý��յ�id
    }
    else
    {
    	rfilter[2].can_mask = 0x04000000U;
    	rfilter[2].can_id   = 0x04000000U;//���ý��յ�id
    	mLogInstance->Log("please attention, zhukong flag is not set");
    }
    setsockopt(sock, SOL_CAN_RAW, CAN_RAW_FILTER, rfilter, n * sizeof(struct can_filter));

    //can_err_mask_t err_mask = ( CAN_ERR_MASK );
    //setsockopt(fd_can, SOL_CAN_RAW, CAN_RAW_ERR_FILTER,&err_mask, sizeof(err_mask)); //���δ���֡

    int loopback = 0; /* 0 = disabled, 1 = enabled (default) */
    setsockopt(sock, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback)); //�رջػ�

    // ����������CAN֡��CAN_RAW�׽����Ͻ��ý��չ��˹���
    //(void)setsockopt(recv_socket_fd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
    if(bLogPrint2)
    	mLogInstance->Log(tag+name+"set_can_filter end");

    return  0;
}

void CanHandle::CanReceiveThread()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+name+"start can read process");

	struct can_frame frame;
	frame.can_id = 1;

	int iSeq = 0;

	//deal receive
	while (bRead)
	{
		if ((ibytes_read = read(fd_can, &frame, sizeof(struct can_frame))) < 0)
		{
			mLogInstance->Log(tag+name+"can read error");
			//close(fd_can);
			//exit(EXIT_FAILURE);
		}
		else
		{
			if(bLogPrint3)
			{
				iSeq++;
				std::cout<<"iSeq="<<iSeq<<" fd_can:"<<(int)fd_can<<std::endl;
			}
			byte bSeqence = (byte)((frame.can_id ) &0x00000007);//CAN֡��ˮ��
			byte bFenDuanFlag = (byte)((frame.can_id >> 3) &0x00000003);//00-���ֶ� 01-��һ���ֶ� 10-�м�ֶ�	11-���ֶ�
			byte bSource = (byte)((frame.can_id >> 8) &0x000000FF);//Դ
			byte bDestion = (byte)((frame.can_id >> 16) &0x000000FF);//Ŀ��
			byte bZongXian = (byte)((frame.can_id >> 24) &0x00000001);//0-����1 1-����2
			byte bZhuanFa = (byte)((frame.can_id >> 25) &0x00000001);//0-����֡ 1-ת��֡

			if(bZhuanFa == 0)
			{//����֡
				if(bDestion != mTrainState->bZhuKongFlag && bDestion != BOARD_BROADCAST && bDestion != BOARD_ZK_ALL)
				{
					std::cout<<(tag+name+"destion code is not zhukong or broadcast port")<<std::endl;//std::cout<<tag<<name<<"destion code is not zhukong or broadcast port"<<std::endl;
					ss_recv.str("");

					struct timeval tv;
					gettimeofday(&tv,NULL);
					ss_recv<<std::dec<<(tv.tv_sec*1000000+tv.tv_usec)<<" ";  //΢��
					ss_recv<<"id:can_id=";
					ss_recv<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.can_id)<<" bSource="
						<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bSource)<<" bDestion="
						<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bDestion)<<" bZongXian="
						<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bZongXian)<<" bZhuanFa="
						<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bZhuanFa)<<" bFenDuanFlag="
						<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bFenDuanFlag)<<" bSeqence="
						<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bSeqence)<<" bLen="
						<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.can_dlc);
					ss_recv<<" data:";
					for(int i=0; i < frame.can_dlc; i++)
						ss_recv<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.data[i]);
					std::string strPrint = ss_recv.str();
					std::cout<<strPrint<<std::endl;//mLogInstance->Log(strPrint);
					continue;
				}
				else
				{
					if(bLogPrint3)
					{//������ǰԭʼ��can֡
						ss_recv.str("");
						ss_recv<<"can"<<(int)bPort<<" recv    id:can_id=";
						ss_recv<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.can_id)<<" bSource="
							<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bSource)<<" bDestion="
							<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bDestion)<<" bZongXian="
							<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bZongXian)<<" bZhuanFa="
							<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bZhuanFa)<<" bFenDuanFlag="
							<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bFenDuanFlag)<<" bSeqence="
							<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bSeqence)<<" bLen="
							<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.can_dlc);
						ss_recv<<" data:";
						for(int i=0; i < frame.can_dlc; i++)
							ss_recv<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.data[i]);
						std::string strPrint = ss_recv.str();
						std::cout<<strPrint<<std::endl;//mLogInstance->Log(strPrint);
					}

					if(bFenDuanFlag == 1)
					{//��һ��,����Ƿ���
						std::map<byte ,CanPackage >::iterator l_it_tmp;
						l_it_tmp=mapData.find(bSource);
						if(l_it_tmp != mapData.end())
						{
							mapData.erase(l_it_tmp);
							std::cout<<("have erase a frame, because of no tail package, please attention")<<std::endl;
						}
					}

					CanPackage *aCanPackage ;
					std::map<byte ,CanPackage >::iterator l_it;
					l_it=mapData.find(bSource);
					if(l_it == mapData.end())
					{
						CanPackage tmpCanPackage;
						tmpCanPackage.bZongXian = bZongXian;
						mapData[bSource] = tmpCanPackage;

						l_it=mapData.find(bSource);//�¼����Ҳ��Ҫ�ҵ�������
						aCanPackage = &(l_it->second);
					}
					else
					{//����
						aCanPackage = &(l_it->second);
					}


					for (int i = 0; i < frame.can_dlc; i++)
						aCanPackage->vData.push_back(frame.data[i]);

					if(bFenDuanFlag == 3 || bFenDuanFlag == 0 )
					{// ĩ��
						//for (unsigned int i = 2; i < aCanPackage->vData.size()-2; i++)
						//{//ȥ��˫10
						//	if (i == aCanPackage->vData.size() - 3)
						//	{
						//		break;
						//	}
						//	if (aCanPackage->vData[i] == 0x10 && aCanPackage->vData[i + 1] == 0x10)
						//	{
						//		aCanPackage->vData.erase(aCanPackage->vData.begin()+i+1);
						//	}
						//}
						if(!aCanPackage->vData.empty())
						{
							_RawInfo PackageInfo;
							PackageInfo.bZongXian = aCanPackage->bZongXian;
							bool flag = mDataProcesser.ParseRawInfo(&(aCanPackage->vData[0]), aCanPackage->vData.size(),PackageInfo);//
							if(flag)
							{
								try
								{
									std::unique_lock <std::mutex> lck(mtx_HaveRawInfo);
									while(g_RawInfoData.size()>=BUF_SIZ)
									{
										g_RawInfoData.pop_front();
										mLogInstance->Log(tag+name+"too much package need to be handled, so remove older instances");
									}
									g_RawInfoData.push_back(PackageInfo);

									mWaitHaveRawInfo.notify_all();
								}
								catch(...)
								{

								}
							}
							else
							{
								std::cout<<("can not prase can data, please attention")<<std::endl;
							}
						}
						else
						{
							std::cout<<("can data is null, please attention")<<std::endl;
						}
						mapData.erase(l_it);
					}
					else
					{
					}
				}
			}
			else
			{//ת��֡
				//��ʱ������
				std::cout<<"zhuan fa can data, please attention"<<std::endl;//mLogInstance->Log("zhuan fa can data, please attention");
				ss_recv.str("");
				ss_recv<<"id:can_id=";
				ss_recv<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.can_id)<<" bSource="
					<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bSource)<<" bDestion="
					<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bDestion)<<" bZongXian="
					<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bZongXian)<<" bZhuanFa="
					<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bZhuanFa)<<" bFenDuanFlag="
					<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bFenDuanFlag)<<" bSeqence="
					<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bSeqence)<<" bLen="
					<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.can_dlc);
				ss_recv<<" data:";
				for(int i=0; i < frame.can_dlc; i++)
					ss_recv<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.data[i]);
				std::string strPrint = ss_recv.str();
				std::cout<<strPrint<<std::endl;//mLogInstance->Log(strPrint);
			}
		}
	}
}

void CanHandle::HandleRawInfoThread()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+name+"start can handle process");

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
					mLogInstance->Log(tag+name+"wait wrong mWaitHaveRawInfo");
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
			mLogInstance->Log(tag+name+"wait wrong HandleRawInfoThread");
		}
	}
}

void CanHandle::CanWrite_Gps_Notify()
{
	try
	{
		std::unique_lock<std::mutex> lck(mtx_WaitSend_Gps);
		mWaitSend_Gps.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log(tag+name+"notify_all wrong mtx_WaitSend_Gps");
	}
}
void CanHandle::CanWrite_Gps_AckNotify()
{
	try
	{
		mblResponse_OK_Gps = true;
		std::unique_lock <std::mutex> lck(mtx_WaitResponseAck_Gps);
		mWaitResponseAck_Gps.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log(tag+name+"notify_all wrong mWaitResponseAck_Gps");
	}
}
void CanHandle::CanWriteThreadGps()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+name+"start can write jiekou process");

	int sendtimes = 0;
	while(bWriteThreadGps)
	{
		try
		{
			if (g_SendData_List_Gps.empty())
			{
				try
				{
					std::unique_lock <std::mutex> lck(mtx_WaitSend_Gps);
					mWaitSend_Gps.wait(lck);
				}
				catch(...)
				{
					mLogInstance->Log(tag+name+"wait wrong mWaitSend_JieKou");
				}
			}
			else
			{
				_SendData pSendData = g_SendData_List_Gps.front();


				//byte* tmpData_NeedAck = pSendData.arrInfo;
				//int tmpData_NeedAck_Length = pSendData.iLength;
				//if (tmpData_NeedAck == NULL || tmpData_NeedAck_Length < 1)
				//{
				//	if(!g_SendData_List_Gps.empty())
				//		g_SendData_List_Gps.pop_front();// ���ɾ��
				//	continue;
				//}

				try
				{
					std::unique_lock <std::mutex> lck2(mtx_CanWriteCIR);
					//д��
					if(!pSendData.vSendData.empty())
					{
						if(sendtimes == 0)
							CanWrite(pSendData,false);
						else
							CanWrite(pSendData,true);
					}
				}
				catch(...)
				{
					mLogInstance->Log(tag+name+"send wrong mtx_CanWriteCIR");
				}

				try
				{
					std::unique_lock <std::mutex> lck3(mtx_WaitResponseAck_Gps);
					mWaitResponseAck_Gps.wait_for(lck3, std::chrono::milliseconds(500));// ��ȴ�500ms,�����ݵ�������ǰ������mblResponse_OK������
				}
				catch(...)
				{
					mLogInstance->Log(tag+name+"wait wrong mtx_WaitResponseAck_JieKou");
				}

				sendtimes++;
				if (mblResponse_OK_Gps || sendtimes >= 3)
				{
					sendtimes = 0;
					mblResponse_OK_Gps = false;
					if(!g_SendData_List_Gps.empty())
						g_SendData_List_Gps.pop_front();
				}
			}
		}
		catch(...)
		{
			mLogInstance->Log(tag+name+"something wrong CanWriteThreadJieKou");
		}
	}
}

void CanHandle::CanWrite_OtherBoard_Notify()
{
	try
	{
		std::unique_lock <std::mutex> lck(mtx_WaitSend_OtherBoard);
		mWaitSend_OtherBoard.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log(tag+name+"notify_all wrong mWaitSend_OtherBoard");
	}
}
void CanHandle::CanWrite_OtherBoard_AckNotify()
{
	try
	{
		mblResponse_OK_OtherBoard = true;
		std::unique_lock <std::mutex> lck(mtx_WaitResponseAck_OtherBoard);
		mWaitResponseAck_OtherBoard.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log(tag+name+"notify_all wrong mWaitResponseAck_OtherBoard");
	}
}
void CanHandle::CanWriteThreadOtherBoard()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+name+"start can write otherboard process");

	int sendtimes = 0;
	while(bWriteThreadOtherBoard)
	{
		try
		{
			if (g_SendData_List_OtherBoard.empty())
			{
				try
				{
					std::unique_lock <std::mutex> lck(mtx_WaitSend_OtherBoard);
					mWaitSend_OtherBoard.wait(lck);
				}
				catch(...)
				{
					mLogInstance->Log(tag+name+"wait wrong mWaitSend_OtherBoard");
				}
			}
			else
			{
				while(!g_SendData_List_OtherBoard.empty())
				{
					if(bLogPrint3)
						std::cout<<std::endl<<tag<<name<< "g_SendData_List_OtherBoard number begin :" << g_SendData_List_OtherBoard.size() <<std::endl;

					_SendData pSendData = g_SendData_List_OtherBoard.front();
					try
					{
						if(bLogPrint3)
							std::cout<<tag<<name<<"otherboard mtx_CanWriteCIR:1"<<std::endl;

						std::unique_lock <std::mutex> lck2(mtx_CanWriteCIR);
						//д��
						if(!pSendData.vSendData.empty())
						{
							if(bLogPrint3)
								std::cout<<tag<<name<<"otherboard mtx_CanWriteCIR:2"<<std::endl;

							if(sendtimes == 0)
								CanWrite(pSendData,false);
							else
								CanWrite(pSendData,true);
						}

						if(bLogPrint3)
							std::cout<<tag<<name<<"otherboard mtx_CanWriteCIR:3"<<std::endl;
					}
					catch(...)
					{
						mLogInstance->Log(tag+name+"send wrong mtx_CanWriteCIR");
					}

					try
					{
						std::unique_lock <std::mutex> lck3(mtx_WaitResponseAck_OtherBoard);
						mWaitResponseAck_OtherBoard.wait_for(lck3, std::chrono::milliseconds(500));// ��ȴ�500ms,�����ݵ�������ǰ������mblResponse_OK������
					}
					catch(...)
					{
						mLogInstance->Log(tag+name+"wait wrong mtx_WaitResponseAck_OtherBoard");
					}

					sendtimes++;
					if (mblResponse_OK_OtherBoard || sendtimes >= 3)
					{
						sendtimes = 0;
						mblResponse_OK_OtherBoard = false;
						if(!g_SendData_List_OtherBoard.empty())
							g_SendData_List_OtherBoard.pop_front();
					}

					if(bLogPrint3)
						std::cout<<tag<<name<< "g_SendData_List_OtherBoard number end :" << g_SendData_List_OtherBoard.size() <<std::endl<<std::endl;
				}
			}
		}
		catch(...)
		{
			mLogInstance->Log(tag+name+"something wrong CanWriteThreadOtherBoard");
		}
	}

	std::cout<< "please attention, why this thread exit????????????????????????" <<std::endl;
}

void CanHandle::CanWrite_NoAckData_Notify()
{
	try
	{
		std::unique_lock <std::mutex> lck(mtx_WaitSend_NoAck);
		mWaitSend_NoAck.notify_all();
	}
	catch(...)
	{
		mLogInstance->Log(tag+name+"notify_all wrong mWaitSend_NoAck");
	}
}

void CanHandle::CanWriteThreadNoAck()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+name+"start can write no ack process");

	while(bWriteThreadNoAck)
	{
		try
		{
			if (g_SendData_List_NoAck.empty())
			{
				try
				{
					std::unique_lock <std::mutex> lck(mtx_WaitSend_NoAck);
					mWaitSend_NoAck.wait(lck);
				}
				catch(...)
				{
					mLogInstance->Log(tag+name+"wait wrong mWaitSend_NoAck");
				}
			}
			else
			{
				while(!g_SendData_List_NoAck.empty())
				{
					if(bLogPrint3)
						std::cout<<std::endl<<tag<<name<< "g_SendData_List_NoAck number begin :" << g_SendData_List_NoAck.size() <<std::endl;

					_SendData pSendData = g_SendData_List_NoAck.front();
					try
					{
						if(bLogPrint3)
							std::cout<<tag<<name<<"noack mtx_CanWriteCIR:1"<<std::endl;

						std::unique_lock <std::mutex> lck2(mtx_CanWriteCIR);

						if(!pSendData.vSendData.empty())
						{
							if(bLogPrint3)
								std::cout<<tag<<name<<"noack mtx_CanWriteCIR:2"<<std::endl;

							CanWrite(pSendData,false);
						}

						if(bLogPrint3)
							std::cout<<tag<<name<<"noack mtx_CanWriteCIR:3"<<std::endl;
					}
					catch(...)
					{
						mLogInstance->Log(tag+name+"send wrong mtx_CanWriteCIR");
					}

					if(!g_SendData_List_NoAck.empty())
						g_SendData_List_NoAck.pop_front();

					if(bLogPrint3)
						std::cout<<tag<<name<< "g_SendData_List_NoAck number end :" << g_SendData_List_NoAck.size() <<std::endl<<std::endl;
				}
			}
		}
		catch(...)
		{
			mLogInstance->Log(tag+name+"something wrong  CanWriteThreadNoAck");
		}
	}

	std::cout<< "please attention, why this thread exit????????????????????????" <<std::endl;
}


void CanHandle::CanWrite(_SendData& pSendData,bool bReSend)
{
	if(bLogPrint3)
		std::cout<<tag<<name<<"CanWrite():begin"<<std::endl;

	byte* buffertx = &(pSendData.vSendData[0]);
	int len = pSendData.vSendData.size();

	int iPackageNum = 0;
	if (len % 8 == 0)
		iPackageNum = len / 8;
	else
		iPackageNum = len / 8 + 1;

	int curLength = 0;

	if(!bReSend)
	{//����֡
		pSendData.vSendData[0] &= MSK3;//0������֡

		if(pSendData.bZongXian == CAN0)
		{
			byte bFrameNumFrame = mTrainState->arrFrameNumFrame0[(int)pSendData.bDestCode];
			pSendData.vSendData[0] |= bFrameNumFrame;
			mTrainState->arrFrameNumFrame0[(int)pSendData.bDestCode] = (mTrainState->arrFrameNumFrame0[(int)pSendData.bDestCode] + 1)%8;
		}
		else
		{
			byte bFrameNumFrame = mTrainState->arrFrameNumFrame1[(int)pSendData.bDestCode];
			pSendData.vSendData[0] |= bFrameNumFrame;
			mTrainState->arrFrameNumFrame1[(int)pSendData.bDestCode] = (mTrainState->arrFrameNumFrame1[(int)pSendData.bDestCode] + 1)%8;
		}
	}
	else
	{//�ط�֡
		pSendData.vSendData[0] |= BIT3;//1���ط�֡
	}

	//����crc
	if(pSendData.bType == (byte)SHORTFRAME)
	{
		byte crc_result8;
		CrcFunc8(&(pSendData.vSendData[0]),(pSendData.vSendData.size()-1),&crc_result8);
		pSendData.vSendData[pSendData.vSendData.size()-1] = crc_result8;
	}
	else
	{
		byte crc_result16[2];
		CrcFunc16(&(pSendData.vSendData[0]),(pSendData.vSendData.size()-2),crc_result16);
		pSendData.vSendData[pSendData.vSendData.size()-2] = crc_result16[0];
		pSendData.vSendData[pSendData.vSendData.size()-1] = crc_result16[1];
	}
	//
	for (int i = 0; i < iPackageNum; i++)
	{
		struct can_frame frame;
		memset(&frame,0,sizeof(struct can_frame));
		frame.can_id |= CAN_EFF_FLAG;//1��չ֡
		frame.can_id &= 0x9FFFFFFFU; //��Զ��,�Ǵ���
		frame.can_id |=(pSendData.bPriority<<28); //���ȼ�
		frame.can_id |= 0x04000000U; //1-����

		//frame.can_id &= 0xFDFFFFFFU; //0-����֡
		if(pSendData.bZongXian == CAN0)
		{
			//frame.can_id &= 0xFEFFFFFFU; //0-����1
		}
		else
		{
			frame.can_id |= 0x01000000U; //1-����2
		}
		frame.can_id |= (pSendData.bDestCode << 16);
		frame.can_id |= (mTrainState->bZhuKongFlag << 8);
		if(iPackageNum == 1)
		{
			//00b-���ֶ�
		}
		else
		{
			if (i == 0)
			{//01b-��һ���ֶ�
				frame.can_id |= 0x00000008U;
			}
			else if(i == iPackageNum - 1)
			{//11b-���ֶ�
				frame.can_id |= 0x00000018U;
			}
			else
			{//10b-�м�ֶ�
				frame.can_id |= 0x00000010U;
			}
		}
		if(pSendData.bZongXian == CAN0)
		{
			byte bFrameNumId = mTrainState->arrFrameNumId0[(int)pSendData.bDestCode];
			frame.can_id |= bFrameNumId;
			mTrainState->arrFrameNumId0[(int)pSendData.bDestCode] = (mTrainState->arrFrameNumId0[(int)pSendData.bDestCode] + 1)%8;
		}
		else
		{
			byte bFrameNumId = mTrainState->arrFrameNumId1[(int)pSendData.bDestCode];
			frame.can_id |= bFrameNumId;
			mTrainState->arrFrameNumId1[(int)pSendData.bDestCode] = (mTrainState->arrFrameNumId1[(int)pSendData.bDestCode] + 1)%8;
		}

		if (i < iPackageNum - 1)
			curLength = 8;
		else
			curLength = len - 8*i;
		for (int j = 0; j < curLength; j++)
		{
			frame.data[j] = buffertx[8*i+j];
		}
		frame.can_dlc = curLength;

		if(bLogPrint3)
		{//������͵�ԭʼcan�ֽ�
			ss_send.str("");
			ss_send<<name<<" send   id:can_id=";
			ss_send<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.can_id)<<" bSource="
				<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(((frame.can_id >> 8) &0x000000FF))<<" bDestion="
				<< std::hex << std::setw(2) <<std::setfill('0')<<(int)((frame.can_id >> 16) &0x000000FF)<<" bZongXian="
				<< std::hex << std::setw(2) <<std::setfill('0')<<(int)((frame.can_id >> 24) &0x00000001)<<" bZhuanFa="
				<< std::hex << std::setw(2) <<std::setfill('0')<<(int)((frame.can_id >> 25) &0x00000001)<<" bFenDuanFlag="
				<< std::hex << std::setw(2) <<std::setfill('0')<<(int)((frame.can_id >> 3) &0x00000003)<<" bSeqence="
				<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.can_id & 0x00000007);
			std::string strPrint = ss_send.str();
			std::cout<<strPrint<<std::endl;//mLogInstance->Log(strPrint);

			ss_send.str("");
			ss_send<<name<<" send data:";
			for (int j = 0; j < frame.can_dlc; j++)
			{
				ss_send<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(frame.data[j])<< " ";
			}
			strPrint = ss_send.str();
			std::cout<<strPrint<<std::endl;//mLogInstance->Log(strPrint);
		}

		if(bLogPrint3)
			std::cout<<tag<<name<<"real write:1"<<" fd:"<< fd_can<<std::endl;

		int iNWrite = write(fd_can, &frame, sizeof(frame));

		if(bLogPrint3)
			std::cout<<tag<<name<<"real write:2"<<std::endl;

		if (iNWrite == -1)
		{
			mLogInstance->Log(tag+name+"can write error  CanWrite");
			//close(fd_can);
			//exit(EXIT_FAILURE);
		}
	}

	if(bLogPrint1 )
	{
		byte bServiceType;
		byte bCommand;
		byte iDataLength;
		if(pSendData.bType == SHORTFRAME)
		{
			bServiceType = pSendData.vSendData[3];
			bCommand = pSendData.vSendData[4];
			iDataLength = 2;
		}
		else
		{
			int iStart = 1;
			if((pSendData.vSendData[0]>6) != 0)//3.0֡����֡
				iStart = 1;
			else//2.0֡
				iStart = 0;
			bServiceType = pSendData.vSendData[6 + iStart+ pSendData.vSendData[3+iStart]  + pSendData.vSendData[5 +iStart+ pSendData.vSendData[3+iStart]]];
			bCommand = pSendData.vSendData[7 + iStart+ pSendData.vSendData[3+iStart] + pSendData.vSendData[5 +iStart+ pSendData.vSendData[3+iStart]]];
			iDataLength = pSendData.vSendData.size() -iStart - 2  - 8  - pSendData.vSendData[3+iStart] - pSendData.vSendData[5 +iStart+ pSendData.vSendData[3+iStart]];
		}

		if(!(bServiceType == 0xE0 && bCommand == 0x07) || bLogPrint2)
		{
			if(mTrainState->arrCheckIp3[2] == 2)
			{
				ss_send.str("");
				ss_send<<"{"<<(int)(pSendData.bZongXian)<<"}-->"
					<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(mTrainState->bZhuKongFlag)<<"]:"
					<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(pSendData.bDestCode)<<"]-->"
					<<"["<< std::hex << std::setw(2) <<std::setfill('0')<<(int)(bServiceType)<<"]:"
					<<"["<<std::hex << std::setw(2) <<std::setfill('0')<<(int)(bCommand)<<"]-->"
					<<"{"<<(int)iDataLength<<"} ";
				if(pSendData.bType == SHORTFRAME)
				{
					for(int i = 5,  j=0; i< pSendData.vSendData.size() && j<iDataLength; i++,j++) //
					{
						int tm = pSendData.vSendData[i];
						ss_send << std::hex << std::setw(2) <<std::setfill('0') << tm << " ";
					}
				}
				else
				{
					for(int i = pSendData.vSendData.size()-iDataLength - 2 ,  j=0; i< pSendData.vSendData.size() && j<iDataLength; i++,j++) //
					{
						int tm = pSendData.vSendData[i];
						ss_send << std::hex << std::setw(2) <<std::setfill('0') << tm << " ";
					}
				}

				if(bReSend)
					ss_send << "resend";

				std::string strPrint = ss_send.str();
				mLogInstance->Log(strPrint);//std::cout<<strPrint<<std::endl;
			}
		}
	}

	if(bLogPrint3)
		std::cout<<tag<<name<<"CanWrite():end"<<std::endl;
}


