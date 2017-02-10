/*
 * MainClass.cpp
 *
 *  Created on: 2016-9-2
 *      Author: lenovo
 */

#include "MainClass.h"
#include <time.h>
#include <linux/rtc.h>
MainClass::MainClass()
{
	mTrainState = TrainState::GetInstance();
	mParamOperation = ParamOperation::GetInstance();
	mLogInstance = LogInstance::GetInstance();
	lCurrentTime = time((time_t*)NULL);
}

MainClass::~MainClass()
{

}

void MainClass::StartService()
{
	mLogInstance->StartLogFile();

	mLogInstance->Log("tieke_newcir project start: " + GetCurrentTime() + " version: " + mTrainState->strSoftwareVersion);
//	mLogInstance->Log("version: " + mTrainState->strSoftwareVersion);

	MyEnableWtd(WATCHDOGTIME);
	MyFeedWtd();

	MySetGpioDirection(GPIO_ZHUKONG1,1);
	MySetGpioDirection(GPIO_ZHUKONG2,1);
	MySetGpioDirection(GPIO_ZHUKONG3,1);

	MySetGpioDirection(GPIO_ZHUANHUAN1,0);//设置转换单元的复位控制管脚
	MySetGpioDirection(GPIO_ZHUANHUAN2,0);//设置转换单元的复位控制管脚

	CheckZhuKongFlag();

	if(mTrainState->bZhuKongFlag == (byte)BOARD_ZK_LEFT)
	{//初始状态:左侧为主用
		mTrainState->bMainUsed = true;
	}
	else
	{
		mTrainState->bMainUsed = false;
	}

	//MySetLocalIp(ETH0,mTrainState->bZhuKongFlag);//暂时注释
	//CheckLocalIp(mTrainState->iZhuKongFlag);

	CheckFoldsAndFiles();

	mParamOperation->ReadConfigFile(PATHPARAMETERFILE);
	mParamOperation->HandleParamFile();

	//mSerialHandle = new SerialHandle();//取消了串口
	//mEthernetHandle = new EthernetHandle();
	//mCanHandle0 = new CanHandle(CAN0);
	//mCanHandle1 = new CanHandle(CAN1);

	//mSerialHandle.Init();
	mEthernetHandle.Init();
	mCanHandle0.Init(CAN0);
	mCanHandle1.Init(CAN1);
	mMrmGprsHandle0.Init(BOARD_ZH_LEFT,MRM_MODE);
	mMrmGprsHandle1.Init(BOARD_ZH_RIGHT,GPRS_MODE);


	mDataProcesser.SetMembers(&mEthernetHandle,&mCanHandle0,&mCanHandle1,&mMrmGprsHandle0,&mMrmGprsHandle1, &mDispatchCommandFileHandle);
	mEthernetHandle.mDataProcesser.SetMembers(&mEthernetHandle,&mCanHandle0,&mCanHandle1,&mMrmGprsHandle0,&mMrmGprsHandle1, &mDispatchCommandFileHandle);
	mCanHandle0.mDataProcesser.SetMembers(&mEthernetHandle,&mCanHandle0,&mCanHandle1,&mMrmGprsHandle0,&mMrmGprsHandle1, &mDispatchCommandFileHandle);
	mCanHandle1.mDataProcesser.SetMembers(&mEthernetHandle,&mCanHandle0,&mCanHandle1,&mMrmGprsHandle0,&mMrmGprsHandle1, &mDispatchCommandFileHandle);
	mMrmGprsHandle0.mDataProcesser.SetMembers(&mEthernetHandle,&mCanHandle0,&mCanHandle1,&mMrmGprsHandle0,&mMrmGprsHandle1, &mDispatchCommandFileHandle);
	mMrmGprsHandle1.mDataProcesser.SetMembers(&mEthernetHandle,&mCanHandle0,&mCanHandle1,&mMrmGprsHandle0,&mMrmGprsHandle1, &mDispatchCommandFileHandle);

	std::thread thdWatchDog = thread(std::mem_fn(&MainClass::WatchDogThread),this);//喂狗线程

	//mSerialHandle.StartService();
	mEthernetHandle.StartService();
	mCanHandle0.StartService();
	mCanHandle1.StartService();
	mMrmGprsHandle0.StartService();
	mMrmGprsHandle1.StartService();
	mDispatchCommandFileHandle.StartService();

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	InitWorks();
	//mSerialHandle->thdWriteWithAck.join();
	//mSerialHandle->thdWriteNoAck.join();
	//mSerialHandle->thdReceive.join();
	//mSerialHandle->thdHandleRawInfo.join();

	mEthernetHandle.thdWrite.join();
	mEthernetHandle.thdReceive.join();
	mEthernetHandle.thdHandleRawInfo.join();

	mCanHandle0.thdWriteThreadGps.join();
	mCanHandle0.thdWriteThreadOtherBoard.join();
	mCanHandle0.thdWriteThreadNoAck.join();
	mCanHandle0.thdReceive.join();
	mCanHandle0.thdHandleRawInfo.join();

	mCanHandle1.thdWriteThreadGps.join();
	mCanHandle1.thdWriteThreadOtherBoard.join();
	mCanHandle1.thdWriteThreadNoAck.join();
	mCanHandle1.thdReceive.join();
	mCanHandle1.thdHandleRawInfo.join();

	mMrmGprsHandle0.thdDoWork.join();
	mMrmGprsHandle0.thdTimer.join();
	mMrmGprsHandle1.thdDoWork.join();
	mMrmGprsHandle1.thdTimer.join();

	mLogInstance->thdLog.join();

	thdWatchDog.join();

	mLogInstance->Log("newcir project end: " + GetCurrentTime());
}

void MainClass::InitWorks()
{
	//初始化mrm
	mMrmGprsHandle0.SetMrmGprsStatus(BIT0,true,true,0x00);//标记g_MRMBooting_Event
	//初始化gprs
	mMrmGprsHandle1.SetMrmGprsStatus(BIT0,true,true,0x01);//标记g_MRMBooting_Event
	//读取450M串号
	mDataProcesser.ChuanHaoFor450M(BOARD_MMI_ALL);
	//发送综合信息
	mDataProcesser.IntegrateInfoTx(BOARD_MMI_ALL);
}

void MainClass::CheckFoldsAndFiles()
{
	if(IsFolderExist(PATHFILEDIR.c_str()) < 0)
	{
		if (mkdir(PATHFILEDIR.c_str(), 0755) < 0)
		{
			if(IsFolderExist(PATHFILEDIR.c_str()) < 0)
			{
				cout<<"create folder: "<< PATHFILEDIR <<" failed"<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	if(IsFolderExist(PATHDISPATCHCOMMAND.c_str()) < 0)
	{
		if (mkdir(PATHDISPATCHCOMMAND.c_str(), 0755) < 0)
		{
			if(IsFolderExist(PATHDISPATCHCOMMAND.c_str()) < 0)
			{
				cout<<"create folder: "<< PATHDISPATCHCOMMAND <<" failed"<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	if(IsFolderExist(PATHRUNINGTOKEN.c_str()) < 0)
	{
		if (mkdir(PATHRUNINGTOKEN.c_str(), 0755) < 0)
		{
			if(IsFolderExist(PATHRUNINGTOKEN.c_str()) < 0)
			{
				cout<<"create folder: "<< PATHRUNINGTOKEN <<" failed"<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	if(IsFolderExist(PATHSHUNTINGOPERATION.c_str()) < 0)
	{
		if (mkdir(PATHSHUNTINGOPERATION.c_str(), 0755) < 0)
		{
			if(IsFolderExist(PATHSHUNTINGOPERATION.c_str()) < 0)
			{
				cout<<"create folder: "<< PATHSHUNTINGOPERATION <<" failed"<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	if(IsFolderExist(PATHADVANCENOTICE.c_str()) < 0)
	{
		if (mkdir(PATHADVANCENOTICE.c_str(), 0755) < 0)
		{
			if(IsFolderExist(PATHADVANCENOTICE.c_str()) < 0)
			{
				cout<<"create folder: "<< PATHADVANCENOTICE <<" failed"<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	if(IsFolderExist(PATHOTHERCOMMAND.c_str()) < 0)
	{
		if (mkdir(PATHOTHERCOMMAND.c_str(), 0755) < 0)
		{
			if(IsFolderExist(PATHOTHERCOMMAND.c_str()) < 0)
			{
				cout<<"create folder: "<< PATHOTHERCOMMAND <<" failed"<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	if(IsFolderExist(PATHLOSSPACKAGE.c_str()) < 0)
	{
		if (mkdir(PATHLOSSPACKAGE.c_str(), 0755) < 0)
		{
			if(IsFolderExist(PATHLOSSPACKAGE.c_str()) < 0)
			{
				cout<<"create folder: "<< PATHLOSSPACKAGE <<" failed"<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	if(IsFolderExist(PATHOTHERRECORD.c_str()) < 0)
	{
		if (mkdir(PATHOTHERRECORD.c_str(), 0755) < 0)
		{
			if(IsFolderExist(PATHOTHERRECORD.c_str()) < 0)
			{
				cout<<"create folder: "<< PATHOTHERRECORD <<" failed"<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	ofstream outfile;
	outfile.open(PATHLOGFILE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHLOGFILE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	outfile.open(PATHPARAMETERFILE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHPARAMETERFILE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	outfile.open(PATHALLCOMMANDFILE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHALLCOMMANDFILE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	outfile.open(PATHDISPATCHCOMMANDFILE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHDISPATCHCOMMANDFILE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	outfile.open(PATHRUNINGTOKENFILE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHRUNINGTOKENFILE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	outfile.open(PATHSHUNTINGOPERATIONFILE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHSHUNTINGOPERATIONFILE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	outfile.open(PATHADVANCENOTICEFILE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHADVANCENOTICEFILE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	outfile.open(PATHOTHERCOMMANDFILE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHOTHERCOMMANDFILE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	outfile.open(PATHLOSSPACKAGEFILE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHLOSSPACKAGEFILE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	outfile.open(PATHOTHERRECORDTRAIN.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHOTHERRECORDTRAIN<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	outfile.open(PATHOTHERRECORDENGINE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHOTHERRECORDENGINE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	outfile.open(PATHOTHERRECORDWORKMODE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHOTHERRECORDWORKMODE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	//cout<<"folder or files is ok"<<endl;
	//mLogInstance->Log("folder or files is ok");
}

void MainClass::CheckZhuKongFlag()
{
	int iGpio1 = -1;
	int iGpio2 = -1;
	int iGpio3 = -1;

	if( MySetIoState(GPIO_ZHUANHUAN1,1) != ERROR_OK)
	{
		mLogInstance->Log("can not set gpio for zhuanzhuan1 flag");
		exit(EXIT_FAILURE);//
	}
	else
	{
		//mLogInstance->Log("set gpio for zhuanzhuan1 ok");
	}
	if( MySetIoState(GPIO_ZHUANHUAN2,1) != ERROR_OK)
	{
		mLogInstance->Log("can not set gpio for zhuanzhuan2 flag");
		exit(EXIT_FAILURE);//
	}
	else
	{
		//mLogInstance->Log("set gpio for zhuanzhuan2 ok");
	}

	if( MyReadIoState(GPIO_ZHUKONG1,&iGpio1) != ERROR_OK)
	{
		mLogInstance->Log("can not read gpio for zhukong1 flag");
		exit(EXIT_FAILURE);//
	}
	if( MyReadIoState(GPIO_ZHUKONG2,&iGpio2) != ERROR_OK)
	{
		mLogInstance->Log("can not read gpio for zhukong2 flag");
		exit(EXIT_FAILURE);//
	}
	if( MyReadIoState(GPIO_ZHUKONG3,&iGpio3) != ERROR_OK)
	{
		mLogInstance->Log("can not read gpio for zhukong3 flag");
		exit(EXIT_FAILURE);//
	}
//	stringstream ss;
//	ss<<"gpio_zhukong1:"<<iGpio1<<", gpio_zhukong2:"<<iGpio2<<", gpio_zhukong3:"<<iGpio3;
//	mLogInstance->Log(ss.str());

	char tCurTime[6];
	GetCurrentTime2(tCurTime);
	if(tCurTime[0]<16)
	{//设置新时间
		mLogInstance->Log("system time is not right, need be modified");
		MySetRtcTime2(2016,9,8,11,15,30);
	}
	//

	if(iGpio1 == 1 && iGpio2 == 0 && iGpio3 == 0 )
	{
		mTrainState->bZhuKongFlag = (byte)BOARD_ZK_LEFT;
		mTrainState->bOtherBoardFlag = (byte)BOARD_ZK_RIGHT;
		mLogInstance->Log("this board is left");
	}
	else if(iGpio1 == 1 && iGpio2 == 1 && iGpio3 == 0 )
	{
		mTrainState->bZhuKongFlag = (byte)BOARD_ZK_RIGHT;
		mTrainState->bOtherBoardFlag = (byte)BOARD_ZK_LEFT;
		mLogInstance->Log("this board is right");
	}
	else
	{
		mLogInstance->Log("zhu kong flag is not right");
	}
}


void MainClass::WatchDogThread()
{
	//开机启动报告
	if(bLogPrint2)
		mLogInstance->Log("WatchDogThread start");

	mDataProcesser.StartReport(0x01);
	int iSeq = 0;
	while(WATCHDOG)
	{
		if(iSeq % 2 == 0)
		{//1s喂狗
			MyFeedWtd();
			//mLogInstance->Log("feed watch dog successful");
		}
		if(iSeq % 5 == 0)
		{//2.5s检测can通道状态
			lCurrentTime = time((time_t*)NULL);
			bool bZongXianState0,bZongXianState1;
			if((lCurrentTime - mTrainState->lCan0RecvTime) > CAN_TIMEOUT)
			{
				bZongXianState0 = false;
				//std::cout<<"bZongXianState0 = false  lCurrentTime="<<lCurrentTime<<" lCan0RecvTime="<<mTrainState->lCan0RecvTime<<" diff="<<lCurrentTime - mTrainState->lCan0RecvTime<<std::endl;
			}
			else
				bZongXianState0 = true;

			if((lCurrentTime - mTrainState->lCan1RecvTime) > CAN_TIMEOUT)
			{
				bZongXianState1 = false;
				//std::cout<<"bZongXianState1 = false  lCurrentTime="<<lCurrentTime<<" lCan1RecvTime="<<mTrainState->lCan1RecvTime<<" diff="<<lCurrentTime - mTrainState->lCan1RecvTime<<std::endl;
			}
			else
				bZongXianState1 = true;

			if(bZongXianState0 && bZongXianState1)
				mTrainState->bZongXianState = 0x00;// 数据
			else if(!bZongXianState0 && bZongXianState1)
				mTrainState->bZongXianState = 0x01;//
			else if(bZongXianState0 && !bZongXianState1)
				mTrainState->bZongXianState = 0x02;//
			else if(!bZongXianState0 && !bZongXianState1)
				mTrainState->bZongXianState = 0x03;//

			byte bOldData = mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag];
			mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] = mTrainState->bZongXianState;

			if((lCurrentTime - mTrainState->lOtherBoardRecvTime) > OTHERBOARD_TIMEOUT)
			{
				mTrainState->bOtherboardState = 0x00;

				if(mTrainState->bOtherBoardFlag == (byte)BOARD_ZK_LEFT)
					mTrainState->arrDeviceState[0] &= (~BIT0);
				else
					mTrainState->arrDeviceState[0] &= (~BIT1);
				//std::cout<<"bOtherboardState = 0x00  lCurrentTime="<<lCurrentTime<<" lOtherBoardRecvTime="<<mTrainState->lOtherBoardRecvTime<<" diff="<<lCurrentTime - mTrainState->lOtherBoardRecvTime<<std::endl;

				mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag] = 0x03;
			}
			else
			{
				mTrainState->bOtherboardState = 0x01;

				if(mTrainState->bOtherBoardFlag == (byte)BOARD_ZK_LEFT)
					mTrainState->arrDeviceState[0] |= (BIT0);
				else
					mTrainState->arrDeviceState[0] |= (BIT1);
			}

			if(mTrainState->arrCanState[(int)mTrainState->bZhuKongFlag] == 0x03)
			{//收不到任何数据
//				//通知切换主控
//				_SendData  pSendData;
//				mFrameGenerate.GetFrame_ChangeMainUsedState(pSendData,mTrainState->bOtherBoardFlag,mTrainState->bZongXian,PRIORITY0,ACK0,SHORTFRAME,0x01);//ACK2
//				mDataProcesser.SendInfoForCan(pSendData);


				if(mTrainState->bMainUsed)
				{
					mTrainState->bMainUsed = false;
					mLogInstance->Log("mainused change to false because can is down");
				}
			}
			else
			{//有数据
				if( mTrainState->bOtherboardState == 0x00)//对方板卡故障
				{
					if(!mTrainState->bMainUsed ) //
					{
						//通知切换主控
						_SendData  pSendData;
						mFrameGenerate.GetFrame_ChangeMainUsedState(pSendData,mTrainState->bOtherBoardFlag,mTrainState->bZongXian,PRIORITY0,ACK0,SHORTFRAME,0x00);//ACK2//暂时注释
						mDataProcesser.SendInfoForCan(pSendData);

						mTrainState->bMainUsed = true;
						mLogInstance->Log("mainused change to true because otherboard is down");//mLogInstance->Log("mainused change to true because otherboard is down");
					}
				}
			}

			mDataProcesser.TestForCan();

			if(bOldData != mTrainState->bZongXianState)
			{
				stringstream ssTemp;
				ssTemp<<"old zongxian state is "<<(int)bOldData<<", now is "<<(int)mTrainState->bZongXianState;
				mLogInstance->Log(ssTemp.str());//mLogInstance->Log(ssTemp.str());

				mDataProcesser.CheckCanRoad();//总线状态有变化
			}
		}
		if(iSeq == 60)
		{
			mDataProcesser.StartReport(0x02);
		}

		if(iSeq % 20 == 0)
		{//10s设备状态信息广播
			if(mTrainState->bMainUsed)
			{
				mDataProcesser.SendDeviceState();
				mDataProcesser.SendIntegeInfo();
			}

			if(bLogPrint1)
			{
				stringstream ssTemp;
				ssTemp<<"bWorkState="<<(int)mTrainState->bWorkState<<", iWorkMode="<<(int)mTrainState->iWorkMode
					<<", bMainUsed="<<(int)mTrainState->bMainUsed
					<<", bZongXian="<<(int)mTrainState->bZongXian<<", bZongXianState="<<(int)mTrainState->bZongXianState
					<<", bZongXianState2="<<(int)mTrainState->arrCanState[(int)mTrainState->bOtherBoardFlag]
					<<", bOtherboardState="<<(int)mTrainState->bOtherboardState
					<<", bMainMMI="<<(int)mTrainState->bMainMMI
//					<<", bSgm1State="<<(int)mTrainState->bSgm1State
//					<<", bSgm2State="<<(int)mTrainState->bSgm2State
//					<<", bSgm1Actort="<<(int)mTrainState->bSgm1Actort
//					<<", bSgm2Actort="<<(int)mTrainState->bSgm2Actort
					<<", bActor1="<<(int)mMrmGprsHandle0.bActor
					<<", bActor2="<<(int)mMrmGprsHandle1.bActor
					<<", bMrmStatus1="<<(int)mMrmGprsHandle0.bMrmStatus
					<<", bGprsStatus1="<<(int)mMrmGprsHandle0.bGprsStatus
					<<", bMrmStatus2="<<(int)mMrmGprsHandle1.bMrmStatus
					<<", bGprsStatus2="<<(int)mMrmGprsHandle1.bGprsStatus
					<<", bMrmStrength1="<<(int)mTrainState->iSignalStrenghthenRealSgm1
					<<", bMrmStrength2="<<(int)mTrainState->iSignalStrenghthenRealSgm2
					<<", MrmLac="<<mTrainState->strMrmLac
					<<", MrmCi="<<mTrainState->strMrmCi
					<<", GprsLac="<<mTrainState->strGprsLac
					<<", GprsCi="<<mTrainState->strGprsCi
					<<", SourAddr="<<mTrainState->strSourAddre
					;//
				mLogInstance->Log(ssTemp.str());
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(WATCHDOGINTERVAL));
		iSeq++;
	}
	mLogInstance->Log("WatchDogThread end");
}

