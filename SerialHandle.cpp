/*
 * SerialHandle.cpp
 *
 *  Created on: 2016-8-11
 *      Author: lenovo
 */

#include "SerialHandle.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

SerialHandle::SerialHandle():fd_serial(-1),rx_len(0),tx_len(0)
{
	tag = "SerialHandle: ";
	mTrainState = TrainState::GetInstance();
	mParamOperation = ParamOperation::GetInstance();
	mLogInstance = LogInstance::GetInstance();
}
void SerialHandle::Init()
{
	bRead = true;
	bHandleRawInfo = true;
	bWriteThreadWithAck = true;
	bWriteThreadNoAck = true;
	mblCIR_DLE_FLAG = false;
	mblCIR_Head_Flag = false;
	mnCIR_COM_R_Index = 0;

	fd_serial = open(SERIALPROT_NAME.c_str(),O_RDWR);
	if(fd_serial < 0)
	{
		//std::cout<< "can't open serial port, will exit" <<std::endl;
		mLogInstance->Log(tag+"can't open serial port, will exit");
		exit(EXIT_FAILURE);
	}
	if(Uart_Set(fd_serial,UARTBAUDRATE,0,8,1,'N') != 0)
	{
		//std::cout<< "can't set serial port, will exit" <<std::endl;
		mLogInstance->Log(tag+"can't set serial port, will exit");
		exit(EXIT_FAILURE);
	}
	mLogInstance->Log(tag+"serial is already");
}
void SerialHandle::StartService()
{
	bRead = true;
	bHandleRawInfo = true;
	bWriteThreadWithAck = true;
	bWriteThreadNoAck = true;
	thdReceive = thread(std::mem_fn(&SerialHandle::SerialReceiveThread),this);
	thdHandleRawInfo = thread(std::mem_fn(&SerialHandle::HandleRawInfoThread),this);
	thdWriteWithAck = thread(std::mem_fn(&SerialHandle::SerialWriteWithAckThread),this);
	thdWriteNoAck = thread(std::mem_fn(&SerialHandle::SerialWriteNoAckThread),this);
	mLogInstance->Log(tag+"StartService");
}

SerialHandle::~SerialHandle()
{

}

void SerialHandle::SerialReceiveThread()
{
	//std::cout<< "start serial read process" <<std::endl;
	mLogInstance->Log(tag+"start serial read process");
	unsigned int nCount;
	while(bRead)
	{
		nCount = BUF_SIZ/4;
		if(ReadCom(fd_serial,RecvDataBuf,&nCount,10000) == ERROR_OK)
		{
			rx_len = nCount;
			if(rx_len > 0)
			{
				for (unsigned int i = 0; i < rx_len; i++)
				{
					if (mblCIR_DLE_FLAG)
					{
						if (mblCIR_DLE_FLAG)
							mblCIR_DLE_FLAG = false;
						switch (RecvDataBuf[i])
						{
						case 0x02:
							mblCIR_Head_Flag = true;
							mblCIR_DLE_FLAG = false;
							mnCIR_COM_R_Index = 0;
							break;
						case 0x03:
							mblCIR_Head_Flag = false;
							mnCIR_COM_R_Index = mnCIR_COM_R_Index - 1;
							{
								_RawInfo PackageInfo;
								bool flag = mDataProcesser.ParseRawInfo(mnCIR_COM_R_Data, mnCIR_COM_R_Index,PackageInfo);// 0 ~ mnCIR_COM_R_Index-1
								if(flag)
								{
									try
									{
										std::unique_lock <std::mutex> lck(mtx_HaveRawInfo);
										while(g_RawInfoData.size()>=BUF_SIZ)
										{
											g_RawInfoData.pop_front();
											//cout<<"too much package need to be handled, so remove older instances"<<endl;
											mLogInstance->Log(tag+"too much package need to be handled, so remove older instances");
										}
										g_RawInfoData.push_back(PackageInfo);
										//notify
										mWaitHaveRawInfo.notify_all();
									}
									catch(...)
									{

									}
								}
							}
							mnCIR_COM_R_Index = 0;
							break;
						default:
							break;
						}
					}
					else
					{
						if (RecvDataBuf[i] == 0x10)
							mblCIR_DLE_FLAG = true;
						if (mblCIR_Head_Flag)
							mnCIR_COM_R_Data[(mnCIR_COM_R_Index)++] = RecvDataBuf[i] ;
						if (mnCIR_COM_R_Index >= 1023)
						{
							mnCIR_COM_R_Index = 0;
							mblCIR_Head_Flag = false;
						}
					}
				}
			}
			else
			{
				//std::cout<< "read serial device error" <<std::endl;
				mLogInstance->Log(tag+"read serial device error");
				//exit(EXIT_FAILURE);
			}
		}
	}
}

void SerialHandle::HandleRawInfoThread()
{
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
					//std::cout<< "wait wrong 3-1" <<std::endl;
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
			//std::cout<< "something wrong 3" <<std::endl;
			mLogInstance->Log(tag+"something wrong HandleRawInfoThread");
		}
	}
}

void SerialHandle::SerialWriteWithAckThread()
{
	int sendtimes = 0;
	while(bWriteThreadWithAck)
	{
		try
		{
			if (g_SendData_List_WithAck.empty())
			{
				try
				{
					std::unique_lock <std::mutex> lck(mtx_mWaitSend_WithAck);
					mWaitSend_WithAck.wait(lck);
				}
				catch(...)
				{
					//std::cout<< "wait wrong 1-1" <<std::endl;
					mLogInstance->Log(tag+"wait wrong mWaitSend_WithAck");
				}
			}
			else
			{
				_SendData pSendData = g_SendData_List_WithAck.front();

				try
				{
					std::unique_lock <std::mutex> lck2(mtx_CanWriteCIR);
					//if(WriteCom(fd_serial,tmpData_NeedAck,&tmpData_NeedAck_Length,10000) != ERROR_OK)
					//{
					//	mLogInstance->Log(tag+"write serial device error");
					//	//exit(EXIT_FAILURE);
					//}
					unsigned int tmpData_NeedAck_Length = pSendData.vSendData.size();
					if(WriteCom(fd_serial,&(pSendData.vSendData[0]),&tmpData_NeedAck_Length,10000) != ERROR_OK)
					{
						mLogInstance->Log(tag+"write serial device error");
						//exit(EXIT_FAILURE);
					}
				}
				catch(...)
				{
					//std::cout<< "send wrong 1" <<std::endl;
					mLogInstance->Log(tag+"send wrong mtx_CanWriteCIR");
				}

				try
				{
					std::unique_lock <std::mutex> lck3(mtx_WaitResponse_OK);
					mWaitResponse_OK.wait_for(lck3, std::chrono::milliseconds(500));// 最长等待500ms,若数据到达则被提前唤醒且mblResponse_OK被设置
				}
				catch(...)
				{
					//std::cout<< "wait wrong 1-2" <<std::endl;
					mLogInstance->Log(tag+"wait wrong mWaitResponse_OK");
				}

				sendtimes++;
				if (mblResponse_OK || sendtimes >= 3)
				{
					sendtimes = 0;
					mblResponse_OK = false;
					if(!g_SendData_List_WithAck.empty())
						g_SendData_List_WithAck.pop_front();
				}
			}
		}
		catch(...)
		{
			//std::cout<< "something wrong 1" <<std::endl;
			mLogInstance->Log(tag+"something wrong SerialWriteWithAckThread");
		}
	}
}

void SerialHandle::SerialWriteNoAckThread()
{
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
					//std::cout<< "wait wrong 2-1" <<std::endl;
					mLogInstance->Log(tag+"wait wrong mWaitSend_NoAck");
				}
			}
			else
			{
					_SendData pSendData = g_SendData_List_NoAck.front();


					//byte* tmpData_NeedAck = pSendData.arrInfo;
					//unsigned int tmpData_NeedAck_Length = (unsigned int )pSendData.iLength;
					//if (tmpData_NeedAck == NULL || tmpData_NeedAck_Length < 1)
					//{
					//	if(!g_SendData_List_NoAck.empty())
					//		g_SendData_List_NoAck.pop_front();// 最后删除
					//	continue;
					//}

					try
					{
						std::unique_lock <std::mutex> lck2(mtx_CanWriteCIR);
						//if(WriteCom(fd_serial,tmpData_NeedAck,&tmpData_NeedAck_Length,10000) != ERROR_OK)
						//{
						//	mLogInstance->Log(tag+"write serial device error");
						//	//exit(EXIT_FAILURE);
						//}
						unsigned int tmpData_NeedAck_Length = pSendData.vSendData.size();
						if(WriteCom(fd_serial,&(pSendData.vSendData[0]),&tmpData_NeedAck_Length,10000) != ERROR_OK)
						{
							mLogInstance->Log(tag+"write serial device error");
							//exit(EXIT_FAILURE);
						}
					}
					catch(...)
					{
						//std::cout<< "send wrong 2" <<std::endl;
						mLogInstance->Log(tag+"send wrong mtx_CanWriteCIR");
					}

					if(!g_SendData_List_NoAck.empty())
						g_SendData_List_NoAck.pop_front();
			}
		}
		catch(...)
		{
			//std::cout<< "something wrong 2" <<std::endl;
			mLogInstance->Log(tag+"something wrong SerialWriteNoAckThread");
		}
	}
}

int SerialHandle::Uart_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
	unsigned int i;
	int speed_arr[] = {B115200,B19200,B9600,B4800,B2400,B1200,B300};
	int name_arr[] = {115200,19200,9600,4800,2400,1200,300};
	struct termios options;

	if(tcgetattr(fd,&options) != 0)
	{
		//std::cout<< "get serial port configs error" <<std::endl;
		mLogInstance->Log(tag+"get serial port configs error");
		return -1;
	}
	//设置串口输入波特率和输出波特率
	for(i=0;i<sizeof(speed_arr)/sizeof(int);i++)
	{
		if(speed == name_arr[i])
		{
			cfsetispeed(&options,speed_arr[i]);
			cfsetospeed(&options,speed_arr[i]);
		}
	}

	//设置数据流控制
	switch(flow_ctrl)
	{
		case 0 ://不使用流控制
			options.c_cflag &= ~CRTSCTS;
			break;
		case 1 ://使用硬件流控制
			options.c_cflag |= CRTSCTS;
			break;
		case 2 ://使用软件流控制
			options.c_cflag |= IXON | IXOFF | IXANY;
			break;
		default:
			//std::cout<<"unsupported flow control" <<std::endl;
			mLogInstance->Log(tag+"unsupported flow control");
	}

	//设置数据位
	options.c_cflag &= ~CSIZE;
	switch(databits)
	{
		case 5 :
			options.c_cflag |= CS5;
			break;
		case 6 :
			options.c_cflag |= CS6;
			break;
		case 7 :
			options.c_cflag |= CS7;
			break;
		case 8 :
			options.c_cflag |= CS8;
			break;
		default:
			//std::cout<< "unsupported datasize for serial port" <<std::endl;
			mLogInstance->Log(tag+"unsupported datasize for serial port");
	}

	//设置校验位
	switch(parity)
	{
		case 'n' :
		case 'N' : //无奇偶校验位
			options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 'o' :
        case 'O' : //设置为奇校验
        	options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;
        case 'e' :
        case 'E' : //设置为偶校验
        	options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        case 's' :
        case 'S' : //设置为空格
        	options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
        	//std::cout<< "unsupported parity for serial port" <<std::endl;
        	mLogInstance->Log(tag+"unsupported parity for serial port");
        	return -1;

	}
	// 设置停止位
	switch(stopbits)
	{
		case 1 :
			options.c_cflag &= ~ CSTOPB;
			break;
		case 2 :
			options.c_cflag |= CSTOPB;
			break;
		default:
			//std::cout<< "unsupported stop bits for serial port" <<std::endl;
			mLogInstance->Log(tag+"unsupported stop bits for serial port");
			return -1;
	}
	//修改控制模式，保证程序不会占用串口
	options.c_cflag |= CLOCAL;
	//修改控制模式，使得能够从串口中读取输入数据
	options.c_cflag |= CREAD;

	//修改输出模式，原始数据输出
	options.c_oflag &= ~OPOST;

	//不需要回车换行就能发送。
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	//防止将0x0D 转换成0X0A
	options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
	options.c_oflag &= ~(ONLCR | OCRNL );

	//设置等待时间和最小接收字符
    options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */
    options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */

    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    tcflush(fd,TCIFLUSH);

    //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
    	//std::cout<< "serial port  set error" <<std::endl;
    	mLogInstance->Log(tag+"serial port  set error");
    	return -1;
    }

    return 0;

}
