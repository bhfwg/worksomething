

#include "MrmGprsHandle.h"

using namespace std;
MrmGprsHandle::MrmGprsHandle()
{
	bPort = -1;//端口
	bActor = -1;//角色 mrm gprs
	bLastLiveTime = time((time_t*)NULL);
	bIsLive = true;

	mTrainState = TrainState::GetInstance();
	mParamOperation = ParamOperation::GetInstance();
	mLogInstance = LogInstance::GetInstance();

	iMrmForStep = 0;
	bMrmStatus = 0;
	bMrmNeedDeleteOk = false;
	//bMRMResetForbidFlag = 0;
	//bSetMRMBaudRateFlag =0;
	bMrmForbidRigisterInitFlag =0;
	bMrmRegisterAck = false;
	bMrmAcOkFlag = false;
	bMrmOk = false;
	//bMrmResponse = false;
	bMrmNoCarrier = false;
	bMrmCUSD = false;
	bMrmBusy = false;
	bMrmError= false;
	bMrmError3= false;
	bMrmError155= false;
	bMrmError156= false;
	bMrmError162= false;
	bMrmRegisterAck =false;
	bMrmCregSuccess = false;
	//bMrmCops= false;
	bMrmExitAllComplete =false;
	bMrmTrainNumberFNDeregisterComplete =false;
	bMrmEngineNumberFNDeregisterComplete = false;

	strOriginateNumber ="";
	strDialPriority = "";
	strDialType = "";
	strDialNumber="";


	iGprsForStep = 0;
	bGprsStatus =0 ;

	bGprsStatusBefore = 0xff;
	bNeedEnterCSDFlag =false;
	bNeedEnterGPRSFlag =false;
	bHaveGprsIPAddressFlag =false;
	iGPRSSendErrorCounter =0;
	strDestinationIPAddress= "";
	strDestinationPort = "";
}

MrmGprsHandle::~MrmGprsHandle()
{

}

void MrmGprsHandle::Init(byte port,byte actor)
{
	bMrmStatus = 0;

	bPort = port;
	bActor = actor;
	stringstream ss;

	if(bPort == BOARD_ZH_LEFT)
	{
		mTrainState->bSgm1Actort = actor;
		ss<<"";
	}
	else if(bPort == BOARD_ZH_RIGHT)
	{
		mTrainState->bSgm2Actort = actor;
		ss<<"........................................................................";
	}

	ss << "MrmGprsHandle" << std::hex << std::setw(2) <<std::setfill('0')<< (int)bPort<<":";

	if(actor == (byte)MRM_MODE)
	{
		ss<< "mrm: ";
	}
	else if(actor == (byte)GPRS_MODE)
	{
		ss<< "gprs: ";
	}
	else if(actor == (byte)ALL_MODE)
	{
		ss<< "all_mode: ";
	}
	else
	{
		ss<< "broken_mode: ";
	}
	tag = ss.str();
}

void MrmGprsHandle::StartService()
{
	bMrmStatus = 0;
	bHandleAtCommand = true;
	bSendGprsData = true;
	bDoWork = true;
	bTimer = true;

	bTimer1ForQueryCsq = true;//
	iTime1ForQueryCsq = 1;

	bTimer17ForReset = false;
	iTime17ForReset = 1;

	bTimer8ForQueryCallList = false;
	iTime8ForQueryCallList = 1;

	thdHandleAtCommand = std::thread(std::mem_fn(&MrmGprsHandle::HandleAtCommandThread),this);
	thdHandleSendGprsData = std::thread(std::mem_fn(&MrmGprsHandle::SendGprsDataThread),this);
	thdDoWork = std::thread(std::mem_fn(&MrmGprsHandle::DoWorkThread),this);
	thdTimer = std::thread(std::mem_fn(&MrmGprsHandle::TimerThread),this);
}
void MrmGprsHandle::TimerThread()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+"start timer process");

	while (bTimer)
	{
		if(bTimer1ForQueryCsq ) //10s查询csq
		{
			if (iTime1ForQueryCsq%100 ==0)
			{
				if(mTrainState->bMainUsed)
				{
					if(mTrainState->bMainUsed)
						SendAT("AT+CSQ");//查询信号值csq
				}
			}
			iTime1ForQueryCsq++;
		}
		else
		{
			iTime1ForQueryCsq=1;
		}

		if(bTimer8ForQueryCallList ) //10s查询呼叫列表
		{
			if (iTime8ForQueryCallList%100 ==0)
			{
				if(mTrainState->bMainUsed)
				{
					if(mTrainState->bMainUsed )
					{
						InquireCurrentVGC();
						InquirePToPCall();
						CallListToMMI(0);
						bTimer8ForQueryCallList = false;
						iTime8ForQueryCallList = 1;

					}
				}
			}
			iTime8ForQueryCallList++;
		}
		else
		{
			iTime8ForQueryCallList=1;
		}

		if(bTimer17ForReset ) //30s是否reset
		{
			if (iTime17ForReset%300 ==0)
			{
				if(mTrainState->bMainUsed)
				{
					SetMrmGprsStatus(BIT0,true,true,0x00);//标记g_MRMBooting_Event
					bTimer17ForReset = false;
					iTime17ForReset = 1;
				}
			}
			bTimer17ForReset++;
		}
		else
		{
			bTimer17ForReset=1;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void MrmGprsHandle::HandleAtCommandThread()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+"start handle atcommand process");

	while(bHandleAtCommand)
	{
		try
		{
			if (listAtCommand.empty())
			{
				try
				{
					std::unique_lock <std::mutex> lck(mtx_AtCommand);
					mwaitAtCommand.wait(lck);
				}
				catch(...)
				{
					mLogInstance->Log(tag+"wait wrong mtx_AtCommand");
				}
			}
			else
			{
				while(!listAtCommand.empty())
				{
					if(bLogPrint3)
						std::cout<<std::endl<<GetCurrentTime()<<tag<< "listAtCommand number begin :" << listAtCommand.size() <<std::endl;

					std::string strAtCommand = listAtCommand.front();

					if(strAtCommand.size() > 2)
					{//去掉换行
						mLogInstance->Log(tag+"recv: "+strAtCommand.substr(0,strAtCommand.size()-2));
					}

					if(strAtCommand.size()>=2 && strAtCommand.substr(0,2).compare("AT") == 0)
					{
						//回显命令,返回不处理
						if(!listAtCommand.empty())
							listAtCommand.pop_front();
						if(bLogPrint3)
							std::cout<<GetCurrentTime()<<tag<< "listAtCommand number end :" << listAtCommand.size() <<std::endl<<std::endl;
						continue;
					}
					else if(strAtCommand.size()>=2 && strAtCommand.substr(0,2).compare("OK") == 0)
					{
						if(bActor == MRM_MODE )
						{
							if(bMrmNeedDeleteOk)
							{
								bMrmNeedDeleteOk = false;
							}
							else
							{
								{
									bMrmOk = true;
									std::unique_lock<std::mutex> lck(mtxMrmOk);
									mwaitMrmOk.notify_all();
									mLogInstance->Log(tag+"mwaitMrmOk is notify");
								}
							}
						}
						else if(bActor == GPRS_MODE )
						{
							if(bMrmNeedDeleteOk)
							{
								bMrmNeedDeleteOk = false;
							}
							else
							{
								{
									bGprsOk = true;
									std::unique_lock<std::mutex> lck(mtxGprsOk);
									mwaitGprsOk.notify_all();
									mLogInstance->Log(tag+"mwaitGprsOk is notify");
								}
							}
						}
						else if(bActor == ALL_MODE )
						{
							mLogInstance->Log(tag+"bActor is ALL_MODE???");
						}
					}
					else if(strAtCommand.size()>=6 && strAtCommand.substr(0,6).compare("+CSQ: ") == 0)
					{
						int iStrenghthen =  0 ;
						int iReal = 0;
						if(strAtCommand.size()>=8 && strAtCommand[7] == ',')
						{
							iReal = strAtCommand[6] - 0x30;
							if(strAtCommand[6] - 0x30 < 8)
								iStrenghthen = 0;
							else
								iStrenghthen = 1;
							if(bLogPrint2)
								std::cout<<GetCurrentTime()<<tag<<"1 iSignalStrenghthenReal: "<<std::dec<<iReal<<" iSignalStrenghthen: "<<std::dec<<iStrenghthen<<std::endl;
						}
						else
						{
							if(strAtCommand.size()>=8 && strAtCommand[6] == '9' && strAtCommand[7] == '9')
							{
								iReal = 0;
								iStrenghthen = 0;

								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"2 iSignalStrenghthenReal: "<<std::dec<<iReal<<" iSignalStrenghthen: "<<std::dec<<iStrenghthen<<std::endl;
							}
							else
							{
								iReal = (strAtCommand[6]-0x30)*10 + (strAtCommand[7]-0x30);
								if(iReal >=10 && iReal<12)
									iStrenghthen = 1;
								else if(iReal >=12 && iReal<16)
									iStrenghthen = 2;
								else if(iReal >=16 && iReal<20)
									iStrenghthen = 3;
								else if(iReal >=20 && iReal<24)
									iStrenghthen = 4;
								else if(iReal >=24 && iReal<28)
									iStrenghthen = 5;
								else if(iReal >=28)
									iStrenghthen = 6;

								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"3 iSignalStrenghthenReal: "<<std::dec<<iReal<<" iSignalStrenghthen: "<<std::dec<<iStrenghthen<<std::endl;
							}
						}
						bMrmNeedDeleteOk = true;


						if(bActor == MRM_MODE )
						{
							mTrainState->iCIRSelfCheckStatus |= 0x01;
							mTrainState->iSignalStrenghthenRealSgm1 = iReal;
							mTrainState->iSignalStrenghthenSgm1 = iStrenghthen;
						}
						else if(bActor == GPRS_MODE )
						{
							mTrainState->iCIRSelfCheckStatus |= 0x02;
							mTrainState->iSignalStrenghthenRealSgm2 = iReal;
							mTrainState->iSignalStrenghthenSgm2 = iStrenghthen;
						}
						else if(bActor == ALL_MODE )
						{
							mLogInstance->Log(tag+"bActor is ALL_MODE???");
						}
					}
					else if(strAtCommand.size()>=6 && strAtCommand.substr(0,6).compare("+CREG:") == 0)
					{
						int	counter = 0;
						for(int i=0;i<strAtCommand.length();i++)
						{
							if(strAtCommand[i] == ',')
								counter++;
						}
						if(counter>=3)		//确保是查询返回的结果，如+CREG: 2,1,"1035","0EF4"格式，模块主动返回的是+CREG: 1,"1035","0EF4"格式
						{
							if((strAtCommand[9]=='1')||(strAtCommand[9]=='5'))
							{
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"-----------1"<<std::endl;

								//登录网络成功，强制设置为GSMR
								if(mTrainState->arrInitStatus[0]==0)
									mTrainState->arrInitStatus[0] = 0x02;

								if(bMrmStatus == 0)
								{
									if(!bMrmForbidRigisterInitFlag)
									{
										SetMrmGprsStatus(BIT1,true,true,0x00);//g_MRMRegisterInit_Event.SetEvent();
									}

									{
										bMrmCregSuccess = true;//m_MRMCREGSuccess_Event.SetEvent();
										std::unique_lock<std::mutex> lck(mtxMrmCregSuccess);
										mwaitMrmCregSuccess.notify_all();
										mLogInstance->Log(tag+"1 mwaitMrmCregSuccess is notify");
									}

									{
										//bMrmCops = true;
										//std::unique_lock<std::mutex> lck(mtxMrmCops);
										//mwaitMrmCops.notify_all();
										//mLogInstance->Log(tag+"1 mwaitMrmCops is notify");
										SetMrmGprsStatus(BIT2,true,true,0x00);
									}
								}
								//将位置代码,小区号等信息记录
								byte start=0,end=0;
								int i=0;
								for(start=0,end=0;i<strAtCommand.length();i++)
								{
									if(strAtCommand[i]=='"')
									{
										if(start==0)
											start = i+1;
										else
										{
											end = i-1;
											i++;
											break;
										}
									}
								}
								mTrainState->strMrmLac = strAtCommand.substr(start,end-start+1);
								for(start=0,end=0;i<strAtCommand.length();i++)
								{
									if(strAtCommand[i]=='"')
									{
										if(start==0)
											start = i+1;
										else
										{
											end = i-1;
											break;
										}
									}
								}
								mTrainState->strMrmCi = strAtCommand.substr(start,end-start+1);

								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"1 strMrmLac: "<<mTrainState->strMrmLac<<" strCi: "<<mTrainState->strMrmCi<<std::endl;

								{
									bMrmRegisterAck = true;//m_MRMRegisterACK_Event.SetEvent();
									std::unique_lock<std::mutex> lck(mtxMrmRegisterAck);
									mwaitMrmRegisterAck.notify_all();
									mLogInstance->Log(tag+"1 mwaitMrmRegisterAck is notify");
								}
							}
							else
							{
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"-----------2"<<std::endl;

								bMrmCregSuccess = false;//m_MRMCREGSuccess_Event.ResetEvent();
								if(!bMrmForbidRigisterInitFlag)
								{
									SetMrmGprsStatus(BIT1,true,true,0x00);//g_MRMRegisterInit_Event.SetEvent();
								}
								bMrmStatus = 0;//未登录状态
								KillTimer(15);
								mTrainState->iSignalStrenghthenSgm1 = 0;		//场强强度为0
								mTrainState->iSignalStrenghthenRealSgm1 = 0;
								mTrainState->arrInitStatus[0] = 0;
								/***********发送GSM-R语音模块网络注册中信息*********************/
								if((mTrainState->iWorkMode == 0x65)||((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))
								{
									_SendData  pSendData;
									mFrameGenerate.GetFrame_SearchOperatorResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,4,"");//主机向MMI报告网络搜索结果
									mDataProcesser.SendInfoForCan(pSendData);
								}
								mLogInstance->Log(tag + "1 creg not online");
							}

							bMrmNeedDeleteOk = true;
						}
						else if(counter == 1)		//查询返回的结果,不带小区号.+CREG: 0,1
						{
							if((strAtCommand[9]=='1')||(strAtCommand[9]=='5'))
							{
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"-----------3"<<std::endl;

								//登录网络成功，强制设置为GSMR
								if(mTrainState->arrInitStatus[0]==0)
									mTrainState->arrInitStatus[0] = 0x02;

								if(bMrmStatus==0)
								{
									if(!bMrmForbidRigisterInitFlag)
									{
										SetMrmGprsStatus(BIT1,true,true,0x00);//g_MRMRegisterInit_Event.SetEvent();
									}

									{
										bMrmCregSuccess = true;//m_MRMCREGSuccess_Event.SetEvent();
										std::unique_lock<std::mutex> lck(mtxMrmCregSuccess);
										mwaitMrmCregSuccess.notify_all();
										mLogInstance->Log(tag+"2 mwaitMrmCregSuccess is notify");
									}

									{	//在查询完运营商后再向MMI发送网络注册信息
										//bMrmCops = true;
										//std::unique_lock<std::mutex> lck(mtxMrmCops);
										//mwaitMrmCops.notify_all();
										//std::cout<<GetCurrentTime()<<tag<<"2 mwaitMrmCops is notify"<<std::endl;
										SetMrmGprsStatus(BIT2,true,true,0x00);
									}
								}

								{
									bMrmRegisterAck = true;//m_MRMRegisterACK_Event.SetEvent();
									std::unique_lock<std::mutex> lck(mtxMrmRegisterAck);
									mwaitMrmRegisterAck.notify_all();

									mLogInstance->Log(tag + "2 mwaitMrmRegisterAck is notify");
								}
							}
							else
							{
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"-----------4"<<std::endl;

								bMrmCregSuccess = false;//m_MRMCREGSuccess_Event.ResetEvent();
								if(!bMrmForbidRigisterInitFlag)
								{
									SetMrmGprsStatus(BIT1,true,true,0x00);//g_MRMRegisterInit_Event.SetEvent();
								}
								bMrmStatus = 0;//未登录状态
								KillTimer(15);
								mTrainState->iSignalStrenghthenSgm1 = 0;		//场强强度为0
								mTrainState->iSignalStrenghthenRealSgm1 = 0;
								mTrainState->arrInitStatus[0] = 0;
								/***********发送GSM-R语音模块网络注册中信息*********************/
								if((mTrainState->iWorkMode == 0x65)||((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))
								{
									_SendData  pSendData;
									mFrameGenerate.GetFrame_SearchOperatorResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,4,"");//主机向MMI报告网络搜索结果
									mDataProcesser.SendInfoForCan(pSendData);
								}

								mLogInstance->Log(tag + "2 creg not online");
							}

							bMrmNeedDeleteOk = true;
						}
						else
						{//主动报告的
							if((strAtCommand[7]=='1')||(strAtCommand[7]=='5'))
							{
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"-----------5"<<std::endl;

								//登录网络成功，强制设置为GSMR
								if(mTrainState->arrInitStatus[0]==0)
									mTrainState->arrInitStatus[0] = 0x02;

								if(bMrmStatus==0)
								{
									if(!bMrmForbidRigisterInitFlag)
									{
										SetMrmGprsStatus(BIT1,true,true,0x00);//g_MRMRegisterInit_Event.SetEvent();
									}

									{
										bMrmCregSuccess = true;//m_MRMCREGSuccess_Event.SetEvent();
										std::unique_lock<std::mutex> lck(mtxMrmCregSuccess);
										mwaitMrmCregSuccess.notify_all();
										mLogInstance->Log(tag + "3 mwaitMrmCregSuccess is notify");
									}

									{
										//在查询完运营商后再向MMI发送网络注册信息
										//bMrmCops = true;
										//std::unique_lock<std::mutex> lck(mtxMrmCops);
										//mwaitMrmCops.notify_all();
										//std::cout<<GetCurrentTime()<<tag<<"3 mwaitMrmCops is notify"<<std::endl;
										SetMrmGprsStatus(BIT2,true,true,0x00);
									}
								}
								//将位置代码,小区号等信息记录
								byte start,end;
								int i=0;
								for(start=0,end=0;i<strAtCommand.length();i++)
								{
									if(strAtCommand[i]=='"')
									{
										if(start==0)
											start = i+1;
										else
										{
											end = i-1;
											i++;
											break;
										}
									}
								}
								mTrainState->strMrmLac = strAtCommand.substr(start,end-start+1);
								for(start=0,end=0;i<strAtCommand.length();i++)
								{
									if(strAtCommand[i]=='"')
									{
										if(start==0)
											start = i+1;
										else
										{
											end = i-1;
											break;
										}
									}
								}
								mTrainState->strMrmCi = strAtCommand.substr(start,end-start+1);
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"2 strMrmLac: "<<mTrainState->strMrmLac<<" strCi: "<<mTrainState->strMrmCi<<std::endl;
							}
							else
							{
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"-----------6"<<std::endl;

								bMrmCregSuccess = false;//m_MRMCREGSuccess_Event.ResetEvent();
								if(!bMrmForbidRigisterInitFlag)
								{
									SetMrmGprsStatus(BIT1,true,true,0x00);//g_MRMRegisterInit_Event.SetEvent();
								}
								bMrmStatus = 0;//未登录状态
								KillTimer(15);
								mTrainState->iSignalStrenghthenSgm1 = 0;		//场强强度为0
								mTrainState->iSignalStrenghthenRealSgm1 = 0;
								mTrainState->arrInitStatus[0] = 0;
								/***********发送GSM-R语音模块网络注册中信息*********************/
								if((mTrainState->iWorkMode == 0x65)||((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))
								{
									_SendData  pSendData;
									mFrameGenerate.GetFrame_SearchOperatorResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,4,"");//主机向MMI报告网络搜索结果
									mDataProcesser.SendInfoForCan(pSendData);
								}
								mLogInstance->Log(tag + "3 creg not online");
							}
						}
					}
					else if(strAtCommand.size()>=7 && strAtCommand.substr(0,7).compare("+CGREG:") == 0)
					{
						if(strAtCommand.size()>12 && strAtCommand[11]==',' && strAtCommand[12]=='"')		//确保是查询返回的结果，如+CREG: 2,1,"1035","8510","CHINA MOBILE","0460","0000","CHINA MOBILE","0460","0000"格式，模块主动返回的是+CREG: 1,"1035","8510","","0460","0000","CHINA MOBILE","0460","0000"格式
						{
							if((strAtCommand[10]=='1')||(strAtCommand[10]=='5'))
							{
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"-----------1"<<std::endl;

								if(bGprsStatus == 0)
								{
									SetMrmGprsStatus(BIT1,true,true,0x01);//g_MRMRegisterInit_Event.SetEvent();
								}
								//将位置代码,小区号等信息记录
								byte start=0,end=0;
								int i=0;
								for(start=0,end=0;i<strAtCommand.length();i++)
								{
									if(strAtCommand[i]=='"')
									{
										if(start==0)
											start = i+1;
										else
										{
											end = i-1;
											i++;
											break;
										}
									}
								}
								mTrainState->strGprsLac = strAtCommand.substr(start,end-start+1);
								for(start=0,end=0;i<strAtCommand.length();i++)
								{
									if(strAtCommand[i]=='"')
									{
										if(start==0)
											start = i+1;
										else
										{
											end = i-1;
											break;
										}
									}
								}
								mTrainState->strGprsCi = strAtCommand.substr(start,end-start+1);

								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"1 strGprsLac: "<<mTrainState->strGprsLac<<" strGprsCi: "<<mTrainState->strGprsCi<<std::endl;

								{
									bGprsRegisterAck = true;//m_MRMRegisterACK_Event.SetEvent();
									std::unique_lock<std::mutex> lck(mtxGprsRegisterAck);
									mwaitGprsRegisterAck.notify_all();
									mLogInstance->Log(tag+"1 mwaitGprsRegisterAck is notify");
								}
							}
							else
							{
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"-----------2"<<std::endl;

								SetMrmGprsStatus(BIT1,true,true,0x01);//g_MRMRegisterInit_Event.SetEvent();


								bGprsStatus = 0;//未登录状态
								bGprsStatusBefore = 0xff;
								strDestinationIPAddress="";
								strDestinationPort="";
								mTrainState->strSourAddre="";
								memset(mTrainState->arrSourAddre, 0, sizeof(mTrainState->arrSourAddre));
								//KillTimer(14);
								mTrainState->iSignalStrenghthenSgm2 = 0;		//场强强度为0
								mTrainState->iSignalStrenghthenRealSgm2 = 0;
								/***********发送GSM-R语音模块网络注册中信息*********************/
								//if((mTrainState->iWorkMode == 0x65)||((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))
								//{
								//	_SendData  pSendData;
								//	mFrameGenerate.GetFrame_SearchOperatorResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,4,"");//主机向MMI报告网络搜索结果
								//	mDataProcesser.SendInfoForCan(pSendData);
								//}
								mLogInstance->Log(tag + "1 creg not online");
							}

							bMrmNeedDeleteOk = true;
						}
						else
						{//主动报告的
							if(strAtCommand.size()>=8 &&( (strAtCommand[8]=='1')||(strAtCommand[8]=='5')))
							{
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"-----------5"<<std::endl;

								if(bGprsStatus == 0)
								{
									SetMrmGprsStatus(BIT1,true,true,0x01);//g_MRMRegisterInit_Event.SetEvent();
								}
								//将位置代码,小区号等信息记录
								byte start,end;
								int i=0;
								for(start=0,end=0;i<strAtCommand.length();i++)
								{
									if(strAtCommand[i]=='"')
									{
										if(start==0)
											start = i+1;
										else
										{
											end = i-1;
											i++;
											break;
										}
									}
								}
								mTrainState->strGprsLac = strAtCommand.substr(start,end-start+1);
								for(start=0,end=0;i<strAtCommand.length();i++)
								{
									if(strAtCommand[i]=='"')
									{
										if(start==0)
											start = i+1;
										else
										{
											end = i-1;
											break;
										}
									}
								}
								mTrainState->strGprsCi = strAtCommand.substr(start,end-start+1);
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"2 strGprsLac: "<<mTrainState->strGprsLac<<" strGprsCi: "<<mTrainState->strGprsCi<<std::endl;

								{
									bGprsRegisterAck = true;//m_MRMRegisterACK_Event.SetEvent();
									std::unique_lock<std::mutex> lck(mtxGprsRegisterAck);
									mwaitGprsRegisterAck.notify_all();
									mLogInstance->Log(tag+"1 mwaitGprsRegisterAck is notify");
								}
							}
							else
							{
								if(bLogPrint2)
									std::cout<<GetCurrentTime()<<tag<<"-----------6"<<std::endl;

								SetMrmGprsStatus(BIT1,true,true,0x01);//g_MRMRegisterInit_Event.SetEvent();

								bGprsStatus = 0;//未登录状态
								bGprsStatusBefore = 0xff;
								strDestinationIPAddress="";
								strDestinationPort="";
								mTrainState->strSourAddre="";
								memset(mTrainState->arrSourAddre, 0,  sizeof(mTrainState->arrSourAddre));
								//KillTimer(14);
								mTrainState->iSignalStrenghthenSgm2 = 0;		//场强强度为0
								mTrainState->iSignalStrenghthenRealSgm2 = 0;
								/***********发送GSM-R语音模块网络注册中信息*********************/
								//if((mTrainState->iWorkMode == 0x65)||((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))
								//{
								//	_SendData  pSendData;
								//	mFrameGenerate.GetFrame_SearchOperatorResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,4,"");//主机向MMI报告网络搜索结果
								//	mDataProcesser.SendInfoForCan(pSendData);
								//}
								mLogInstance->Log(tag + "1 creg not online");
							}

						}
					}
					else if(strAtCommand.size()>=6 && strAtCommand.substr(0,6).compare("+COPS:") == 0)
					{
						if(strAtCommand.find("\"")!=-1)//if(strAtCommand[strAtCommand.length()-2] == '"' && strAtCommand[strAtCommand.length()-1] == '"')
						{
							if(bInquireAvailableOperatorFlag)
							{
								bInquireAvailableOperatorFlag = false;
								int position = 0,position2 = 0;
								vAvailableOperatorNameList.clear();
								while(position != -1)
								{
									position = strAtCommand.find("\"",position);
									position2 = strAtCommand.find("\"",position+1);
									if(position!=-1 && position2!=-1 && position2>(position+1))
									{
										vAvailableOperatorNameList.push_back(strAtCommand.substr(position+1,position2-position-1));
										position = position2+1;
									}
									else
									{
										mLogInstance->Log(tag+"have query some network");//mLogInstance->Log(tag+"have query some network");

										{
											//bMrmCops = true;
											//std::unique_lock<std::mutex> lck(mtxMrmCops);
											//mwaitMrmCops.notify_all();
											//mLogInstance->Log(tag+"4 mwaitMrmCops is notify");
										}

										break;
									}
								}
							}
							else
							{
								if(strAtCommand.find("46000") != -1 || strAtCommand.find("China Mobile") != -1 )
								{
									mTrainState->arrInitStatus[0] = 0x01;
								}
								else if(strAtCommand.find("46001") != -1 || strAtCommand.find("China Unicom") != -1 )
								{
									mTrainState->arrInitStatus[0] = 0x03;
								}
								else
								{
									mTrainState->arrInitStatus[0] = 0x02;
								}
								mLogInstance->Log(tag+"have query some network");//mLogInstance->Log(tag+"have query some network");

								{
									//g_MRMCOPS_Event.SetEvent();
									//bMrmCops = true;
									//std::unique_lock<std::mutex> lck(mtxMrmCops);
									//mwaitMrmCops.notify_all();
									//mLogInstance->Log(tag+"5 mwaitMrmCops is notify");
								}

								if((mTrainState->iWorkMode == 0x65)||((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))
								{
									_SendData  pSendData;
									mFrameGenerate.GetFrame_InitNetworkInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME);//暂时注释
									mDataProcesser.SendInfoForCan(pSendData);
								}
							}
						}
						else
						{
							mLogInstance->Log(tag+"the cops command is something wrong");//mLogInstance->Log(tag+"the cops command is something wrong");
						}
						//bNeedDeleteOk = true;//此处不能删除ok,因为不再使用mwaitMrmCops改用OK作为通知事件,需要该OK来触发通知
					}
					else if(strAtCommand.find("MRM") != -1 && strAtCommand.find("BOOTING") != -1 )
					{//SAGEM模块启动

						bMrmStatus = 0;
						SetMrmGprsStatus(BIT0,true,true,0x00);//标记g_MRMBooting_Event

						bTimer17ForReset = false;
						iTime17ForReset = 1;
						//KillTimer(12);
						//SetTimer(13);
						//KillTimer(15);
					}
					else if(strAtCommand.size()>=10 && strAtCommand.substr(0,10).compare("NO CARRIER") == 0)
					{
						if(bActor == MRM_MODE )
						{
							//if((mTrainState->iWorkMode == 0x65)||((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))//出入库检测也要处理NoCarrier
							//{
								{
									bMrmNoCarrier = true;
									std::unique_lock<std::mutex> lck(mtxMrmOk);//std::unique_lock<std::mutex> lck(mtxMrmNoCarrier);
									mwaitMrmOk.notify_all();//mwaitMrmNoCarrier.notify_all();
									mLogInstance->Log(tag+"mwaitMrmNoCarrier is notify");
								}
								if(bEmergencyCallFlag)				//记录退出紧急呼叫的时间
								{
		//								bEmergencyCallFlag = false;
		//								EmergencyCallDUREndTime = CTime::GetCurrentTime();
		//								EmergencyCallDURSpanTime = EmergencyCallDUREndTime-EmergencyCallDURStartTime;
		//								EmergencyCallRELStartTime = CTime::GetCurrentTime();
		//								SetTimer(9,rand(),NULL);			//请求发送AC确认
		//								bACFailedCounter = 0;
								}
							//}
								SetMrmGprsStatus(BIT12,true,true,0x00);//
						}
						else if(bActor == GPRS_MODE )
						{
							{
								bGprsNoCarrier = true;
								std::unique_lock<std::mutex> lck(mtxGprsOk);//std::unique_lock<std::mutex> lck(mtxMrmNoCarrier);
								mwaitGprsOk.notify_all();//mwaitMrmNoCarrier.notify_all();
								mLogInstance->Log(tag+"mwaitGprsOk is notify");
							}
						}
						else if(bActor == ALL_MODE )
						{
							mLogInstance->Log(tag+"bActor is ALL_MODE???");
						}
					}
					else if(strAtCommand.size()>=4 && strAtCommand.substr(0,4).compare("BUSY") == 0)
					{//返回结果码为BUSY，呼叫失败
						if(bActor == MRM_MODE )
						{
							if((mTrainState->iWorkMode == 0x65)||((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))
							{
								if(bMrmStatus==0x31)
								{
									//CloseVoiceSwitch();//关闭音频矩阵电路
									SetMrmGprsStatus(BIT12,true,true,0x00);//g_MRMInquireCall_Event.SetEvent();
								}
								else
								{
									{
										bMrmBusy = true; //m_MRMBusy_Event.SetEvent();		//呼叫失败：占线
										std::unique_lock<std::mutex> lck(mtxMrmOk);//std::unique_lock<std::mutex> lck(mtxMrmBusy);
										mwaitMrmOk.notify_all();//mwaitMrmBusy.notify_all();
										mLogInstance->Log(tag+"mwaitMrmBusy is notify");
									}

								}
								bMrmStatus = 0x01;
							}
						}
						else if(bActor == GPRS_MODE )
						{//呼叫失败：占线
							{
								bGprsBusy = true;
								std::unique_lock<std::mutex> lck(mtxGprsOk);//std::unique_lock<std::mutex> lck(mtxMrmNoCarrier);
								mwaitGprsOk.notify_all();//mwaitMrmNoCarrier.notify_all();
								mLogInstance->Log(tag+"mwaitGprsOk is notify");
							}
						}
						else if(bActor == ALL_MODE )
						{
							mLogInstance->Log(tag+"bActor is ALL_MODE???");
						}
					}
					else if((strAtCommand.size()>=5 && strAtCommand.substr(0,5).compare("ERROR") == 0)
							|| (strAtCommand.size()>=12 && strAtCommand.substr(0,12).compare("+CME ERROR: ") == 0))
					{
						if(bActor == MRM_MODE )
						{
							{
								if(strAtCommand.size()>=13 && strAtCommand.substr(0,13).compare("+CME ERROR: 3") == 0)
								{
									bMrmError3 = true;
								}
								else if(strAtCommand.size()>=15 && strAtCommand.substr(0,15).compare("+CME ERROR: 155") == 0)
								{
									bMrmError155 = true;
								}
								else if(strAtCommand.size()>=15 && strAtCommand.substr(0,15).compare("+CME ERROR: 156") == 0)
								{
									bMrmError156 = true;
								}
								else if(strAtCommand.size()>=15 && strAtCommand.substr(0,15).compare("+CME ERROR: 162") == 0)
								{
									bMrmError162 = true;
								}
								bMrmError = true;
								std::unique_lock<std::mutex> lck(mtxMrmOk);
								mwaitMrmOk.notify_all();
								mLogInstance->Log(tag+"mwaitMrmError is notify");
							}
							//SetMrmStatus(BIT12,true,true,0x00);//
						}
						else if(bActor == GPRS_MODE )
						{
							{
								bGprsError = true;
								std::unique_lock<std::mutex> lck(mtxGprsOk);
								mwaitGprsOk.notify_all();
								mLogInstance->Log(tag+"mwaitGprsError is notify");
							}
						}
						else if(bActor == ALL_MODE )
						{
							mLogInstance->Log(tag+"bActor is ALL_MODE???");
						}
					}
					else if(strAtCommand.size()>=7 && strAtCommand.substr(0,7).compare("+CCFC: ") == 0)
					{
						strCCFC = strAtCommand;
						{
							bMrmCfcc = true; //m_MRMCCFC_Event.SetEvent();		//
							std::unique_lock<std::mutex> lck(mtxMrmCfcc );
							mwaitMrmCfcc.notify_all();
							mLogInstance->Log(tag+"mwaitMrmCfcc is notify");
						}
						bMrmNeedDeleteOk = true;
					}
					else if(strAtCommand.size()>=8 && strAtCommand.substr(0,8).compare("+CALCC: ") == 0)
					{
						if(bCurrentActiveVGCInquireFlag)
						{
							bCurrentActiveVGCInquireFlag =0;//由于AT+CALCC=1查询最多有一个返回,所以可以清除掉此标志
							mCurrentActiveVGCbuffer.strGcGID = strAtCommand.substr(8,3);
							mCurrentActiveVGCbuffer.strGcGCA = strAtCommand.substr(12,5);
							mCurrentActiveVGCbuffer.strGcIs17Or18 = strAtCommand.substr(12,2);
							mCurrentActiveVGCbuffer.strGcStat = strAtCommand.substr(21,1);
							mCurrentActiveVGCbuffer.strGcDir = strAtCommand.substr(23,1);
							mCurrentActiveVGCbuffer.strGcAckFlag = strAtCommand.substr(25,1);
							mCurrentActiveVGCbuffer.strGcPriority = strAtCommand.substr(27,1);
						}
						else
						{
							_GroupCallStruct tmpGroupCallStruct;
							tmpGroupCallStruct.strGcGID = strAtCommand.substr(8,3);
							tmpGroupCallStruct.strGcGCA = strAtCommand.substr(12,5);
							tmpGroupCallStruct.strGcIs17Or18 = strAtCommand.substr(18,2);
							tmpGroupCallStruct.strGcStat = strAtCommand.substr(21,1);
							tmpGroupCallStruct.strGcDir = strAtCommand.substr(23,1);
							tmpGroupCallStruct.strGcAckFlag = strAtCommand.substr(25,1);
							tmpGroupCallStruct.strGcPriority = strAtCommand.substr(27,1);
							vVGCTempbuffer.push_back(tmpGroupCallStruct);
						}
					}
					else if(strAtCommand.size()>=7 && strAtCommand.substr(0,7).compare("+CLCC: ") == 0)
					{

						_PToPCallStruct tmpPToPCallStruct;
						tmpPToPCallStruct.strPtpcsIdx = strAtCommand.substr(7,1);
						tmpPToPCallStruct.strPtpcsDir = strAtCommand.substr(9,1);
						tmpPToPCallStruct.strPtpcsStat = strAtCommand.substr(11,1);
						tmpPToPCallStruct.strPtpcsMode = strAtCommand.substr(13,1);
						tmpPToPCallStruct.strPtpcsMpty = strAtCommand.substr(15,1);
						int j=17;
						for(;j<strAtCommand.length();j++)
						{
							if(strAtCommand[j] == ',')
								break;
						}
						tmpPToPCallStruct.strPtpcsNumber = strAtCommand.substr(18,j-19);
						//tmpPToPCallStruct.strPtpcsType = "";
						tmpPToPCallStruct.strPtpcsAlpha = "";
						tmpPToPCallStruct.strPtpcsUUS1 = "";
						if(mTrainState->arrInitStatus[0]==0x02)//GSMR
							tmpPToPCallStruct.strPtpcsPriority = strAtCommand.substr(strAtCommand.length()-1,1);
						else
							tmpPToPCallStruct.strPtpcsPriority = "4";
						vPToPTempbuffer.push_back(tmpPToPCallStruct);

					}
					else if(strAtCommand.size()>=9 && strAtCommand.substr(0,9).compare("+CUUS1U: ") == 0)
					{
						if(bMrmStatus == 0x61)	//发送AC确认状态
						{
							if((strAtCommand.size()>=18 && strAtCommand.substr(12,6).compare("000200") == 0)//AC确认发送成功
									||(strAtCommand.size()>=18 && strAtCommand.substr(12,6).compare("000300") == 0))
							{
								bMrmAcOkFlag = true;

								bMrmAcOk = true; //m_MRMACOK_Event.SetEvent();
								std::unique_lock<std::mutex> lck(mtxMrmAcOk);
								mwaitMrmAcOk.notify_all();
								mLogInstance->Log(tag+"mwaitMrmAcOk is notify");
							}
							else if (strAtCommand.size()>=15 && strAtCommand.substr(12,4).compare("0002") == 0)
							{
								if((strAtCommand[16]>='0')&&(strAtCommand[16]<='7'))		//01~7f：AC确认失败，需要重发
									;
								else																//80~ff：AC确认失败，不需要重发
								{
									bMrmAcOkFlag = false;

									bMrmAcOk = true; //m_MRMACOK_Event.SetEvent();
									std::unique_lock<std::mutex> lck(mtxMrmAcOk);
									mwaitMrmAcOk.notify_all();
									mLogInstance->Log(tag+"mwaitMrmAcOk is notify");
								}
							}
						}
					}
					else if(strAtCommand.size()>=9 && strAtCommand.substr(0,9).compare("+CAULEV: ") == 0)
					{//PTT链路
						if(strAtCommand[strAtCommand.size()-1] == '1')
						{//忙
							_SendData  pSendData;
							mFrameGenerate.GetFrame_PttState(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x01);//上行链路忙
							mDataProcesser.SendInfoForCan(pSendData);
						}
						else
						{//闲
							_SendData  pSendData;
							mFrameGenerate.GetFrame_PttState(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x00);//上行链路空闲
							mDataProcesser.SendInfoForCan(pSendData);
						}
					}
					else if(strAtCommand.size()>=13 && strAtCommand.substr(0,13).compare("+CRING: VOICE") == 0)
					{//个别呼叫被呼
						bTimer8ForQueryCallList = false;
						iTime8ForQueryCallList = 1;

						bMrmStatus = 0x31;
						vPToPbuffer.clear();//CRING来的时候，不可能还存在其他个呼
						_PToPCallStruct newPToPCallStruct;
						newPToPCallStruct.strPtpcsDir = "1";
						newPToPCallStruct.strPtpcsStat = "4";
						newPToPCallStruct.strPtpcsMode = "0";
						if(strAtCommand.size()>=15)
						{
							newPToPCallStruct.strPtpcsPriority = "";
							newPToPCallStruct.strPtpcsPriority += strAtCommand[14];
						}
						else
							newPToPCallStruct.strPtpcsPriority = "4";
						newPToPCallStruct.strPtpcsIdx = "";
						newPToPCallStruct.strPtpcsMpty = "";
						newPToPCallStruct.strPtpcsNumber = "";
						//newPToPCallStruct.strPtpcsType = "1";
						newPToPCallStruct.strPtpcsAlpha = "";
						newPToPCallStruct.strPtpcsUUS1= "";
						if(!listAtCommand.empty())
							listAtCommand.pop_front();

						//读取CLIP信息
						mLogInstance->Log(tag+"read call number");
						int iNumberForClip =0;
						while(iNumberForClip<100)
						{
							iNumberForClip++;
							if(!listAtCommand.empty())
							{
								strAtCommand = listAtCommand.front();
								mLogInstance->Log(tag+"recv: "+strAtCommand);
								if(strAtCommand.size()>=7 && strAtCommand.substr(0,7).compare("+CLIP: ") == 0)
								{
									int iFistYinHao = strAtCommand.find_first_of('"');
									int iSecondYinHao = strAtCommand.find_last_of('"');
									if(iFistYinHao != std::string::npos && iSecondYinHao != std::string::npos)
									{
										std::string strNumber = strAtCommand.substr(iFistYinHao+1,iSecondYinHao - iFistYinHao-1);
										mLogInstance->Log(tag+"call number: "+strNumber);
										newPToPCallStruct.strPtpcsNumber = strNumber;
										if(!listAtCommand.empty())
												listAtCommand.pop_front();
										break;
									}
									else
									{
										if(!listAtCommand.empty())
											listAtCommand.pop_front();
									}
								}
								else
								{
									if(!listAtCommand.empty())
										listAtCommand.pop_front();
								}
							}
							std::this_thread::sleep_for(std::chrono::milliseconds(10));
							mLogInstance->Log(tag + "1.read again...........................");
						}

						mLogInstance->Log(tag+"");

						//读取UUS1信息
						mLogInstance->Log(tag+"read uus1");
						int iNumberForUus1u =0;
						while(iNumberForUus1u<100)
						{
							iNumberForUus1u++;
							if(!listAtCommand.empty())
							{
								strAtCommand = listAtCommand.front();
								mLogInstance->Log(tag+"recv: "+strAtCommand);
								if(strAtCommand.size()>=9 && strAtCommand.substr(0,9).compare("+CUUS1U: ") == 0)
								{
									int iFistYinHao = strAtCommand.find_first_of('"');
									int iSecondYinHao = strAtCommand.find_last_of('"');
									if(iFistYinHao != std::string::npos && iSecondYinHao != std::string::npos)
									{
										std::string strNumber = strAtCommand.substr(iFistYinHao+1,iSecondYinHao - iFistYinHao-1);
										mLogInstance->Log(tag+"1.uus1u: "+strNumber);

										for(int i=0; i<(strNumber.size()/2); i++)		//奇数字节和偶数字节互换
										{
											char tempuchar = strNumber[2*i];
											strNumber[2*i] = strNumber[2*i+1];
											strNumber[2*i+1] = tempuchar;
										}
										mLogInstance->Log(tag+"2.uus1u: "+strNumber);
										if(strNumber.compare("00") != 0 && (strNumber[4] != '0' || strNumber[5] != '0'))//长度为0
										{
											if(strNumber[strNumber.length()-1] == 'F')
												strNumber = strNumber.substr(0,strNumber.length()-1);
											mLogInstance->Log(tag+"3.uus1u: "+strNumber);
											newPToPCallStruct.strPtpcsUUS1 = strNumber.substr(6);
//											//将此UUS1信息保存到UUS1信息的列表当中
//											for(i=0;i<10;i++)
//											{
//												if(UUS1Infobuf[i].flag==0)
//												{
//													UUS1Infobuf[i].flag = 1;
//													UUS1Infobuf[i].number = PToPbuffer[counterCLCC-1].number;
//													UUS1Infobuf[i].UUS1 = PToPbuffer[counterCLCC-1].UUS1;
//													break;
//												}
//											}
										}
										if(!listAtCommand.empty())
											listAtCommand.pop_front();
										break;
									}
									else
									{
										if(!listAtCommand.empty())
											listAtCommand.pop_front();
									}
								}
								else
								{
									if(!listAtCommand.empty())
										listAtCommand.pop_front();
								}
							}
							std::this_thread::sleep_for(std::chrono::milliseconds(10));
							mLogInstance->Log(tag + "2.read again...........................");
						}

						vPToPbuffer.push_back(newPToPCallStruct);

						CallListToMMI(0);

						//GSMR网络下，向对方发送UUS1信息
						if(mTrainState->arrInitStatus[0]==0x02)
						{
							SetMrmGprsStatus(BIT4,true,true,0x00);
						}
						continue;//已删除
					}
					else if((strAtCommand.size()>=11 && strAtCommand.substr(0,11).compare("+CRING: VGC") == 0)
							||(strAtCommand.size()>=11 && strAtCommand.substr(0,11).compare("+CRING: VBC") == 0))
					{//组呼呼叫被呼
						if(!bWaitForJoinVGCS)
						{
							int iPToP = 0, iVgc = 0;
							for(iPToP =0 ;iPToP < vPToPbuffer.size();iPToP++)
								if(vPToPbuffer[iPToP].strPtpcsStat.compare("0") == 0)
									break;
							for(iVgc =0 ;iVgc < vVGCbuffer.size();iVgc++)
								if(vVGCbuffer[iVgc].strGcStat.compare("0") == 0)
									break;
							//判断当前是否有激活的通话,如果没有,则可以进行AT+CALCC=0的查询
							if(iPToP >= vPToPbuffer.size() && iVgc >= vVGCbuffer.size())
							{//无
								SetMrmGprsStatus(BIT12,true,true,0x00);
							}
							else
							{
								if(strAtCommand.size()>=24)
								{
									std::string stringGCA = strAtCommand.substr(12,5);
									std::string stringGID = strAtCommand.substr(18,3);
									std::string stringAckFlag = "";
									stringAckFlag += strAtCommand[22];
									std::string stringPriority = "";
									stringPriority += strAtCommand[24];
									for(iVgc=0;iVgc<vVGCbuffer.size();iVgc++)
									{
										if(vVGCbuffer[iVgc].strGcStat.compare(stringGID)==0)
											break;
									}
									if(iVgc < vVGCbuffer.size())//在呼叫列表中已存在此组呼
									{
										for(iPToP = 0;iPToP<vVGCbuffer.size();iPToP++)
										{
											if(vVGCbuffer[iPToP].strGcStat.compare("0")==0)
												break;
										}
										//有激活的组呼,但是激活的组呼ID和+CRING来的GID一致,说明组呼列表出错了,当然要查
										if(vVGCbuffer[iPToP].strGcGID.compare( stringGID ) == 0 )
										{
											SetMrmGprsStatus(BIT12,true,true,0x00);
										}
										vVGCbuffer[iVgc].strGcStat = "2";
									}
									else //在呼叫列表中不存在此组呼
									{
										_GroupCallStruct newVGCbuffer;
										newVGCbuffer.strGcGID = stringGID;
										newVGCbuffer.strGcGCA = stringGCA;
										if(strAtCommand.substr(0,11).compare("+CRING: VGC") == 0)
											newVGCbuffer.strGcIs17Or18 = "17";
										else
											newVGCbuffer.strGcIs17Or18 = "18";
										newVGCbuffer.strGcStat = "2";
										newVGCbuffer.strGcDir = "1";
										newVGCbuffer.strGcAckFlag = stringAckFlag;
										newVGCbuffer.strGcPriority = stringPriority;
										vVGCbuffer.push_back(newVGCbuffer);
									}
									CallListToMMI(0);
								}
								else
								{
									mLogInstance->Log(tag+"strAtCommand length should be at least 24");
								}
							}
						}
					}
					else if(strAtCommand.size()>=9 && strAtCommand.substr(0,9).compare("+CRING: ,") == 0)
					{//组呼激活状态下来个个呼
						int j=0;
						for(;j<vVGCbuffer.size();j++)
						{
							if(vVGCbuffer[j].strGcStat.compare("0")==0)
							{
								break;
							}
						}
						if(j<vVGCbuffer.size())//当前存在激活的组呼
						{
							if(strAtCommand[strAtCommand.length()-1] <= vVGCbuffer[j].strGcPriority[0])
							{
								SetMrmGprsStatus(BIT11,true,true,0x00);
							}
						}
					}
					else if(strAtCommand.size()>=6 && strAtCommand.substr(0,6).compare("+CUSD:") == 0)
					{
						if(((strAtCommand.find("23233231342A",0) != -1) && (strAtCommand.find("2A38382A",0) != -1))
								|| ((strAtCommand.find("23233231342a",0) != -1) && (strAtCommand.find("2a38382a",0) != -1)))
						{//字符串含有##214* 和 *88*认为是强制注销指令，##214*FN*88*FDMSISDN*addInfField#
							//车次功能号强制注销
							if(strAtCommand.find("30383632") != -1)
							{
								bTrainNumberFNRegisterStatus = 3;						//功能号未被任何SIM卡注册
								mParamOperation->SaveSpecialField("RegisterStatus","0");
								mTrainState->arrInitStatus[3]  &= (~BIT3);
								strTrainFunctionNumber = "";
								_SendData  pSendData;
								mFrameGenerate.GetFrame_FunctionTrainNumberResultInfo(pSendData,BOARD_BROADCAST,CAN1,PRIORITY0,ACK0,BASEFRAME,mTrainState,0x80,0x04,1);
								mDataProcesser.SendInfoForCan(pSendData);
							}

							if(strAtCommand.find("30383633") != -1)
							{
								bEngineNumberFNRegisterStatus = 3;						//功能号未被任何SIM卡注册
								mTrainState->arrInitStatus[3]  &= (~BIT4);
								strEngineFunctionNumber = "";
								_SendData  pSendData;
								mFrameGenerate.GetFrame_FunctionEngineNumberResultInfo(pSendData,BOARD_BROADCAST,CAN1,PRIORITY0,ACK0,BASEFRAME,mTrainState,0x80,0x04,1);
								mDataProcesser.SendInfoForCan(pSendData);
							}
						}
						else ////非强制注销指令，认为是注册／注销返回的结果码
						{
							strCUSD = strAtCommand;
							bMrmCUSD = true;
							std::unique_lock<std::mutex> lck(mtxMrmCUSD);
							mwaitMrmCUSD.notify_all();
						}
					}
					else if(strAtCommand.size()>=7 && strAtCommand.substr(0,7).compare("+CCWA: ") == 0)
					{//个别呼叫通话中，进入呼叫保持
						//找出CCWA中,带有的ISDN号码
						string CCWAnumber = "";
						int i;
						for(i=8;i<strAtCommand.size()&&strAtCommand[i]!='"';i++)
							CCWAnumber += strAtCommand[i];
						for(i=0;i<vPToPbuffer.size();i++)
							if(vPToPbuffer[i].strPtpcsNumber.compare(CCWAnumber) == 0)
								break;
						if(i>vPToPbuffer.size())//该通话在呼叫列表中不存在
						{
							_PToPCallStruct newPToPCallStruct;
							newPToPCallStruct.strPtpcsDir = "1";
							newPToPCallStruct.strPtpcsStat = "5";
							newPToPCallStruct.strPtpcsMode = "0";
							if(mTrainState->arrInitStatus[0] == 0x02)
								newPToPCallStruct.strPtpcsPriority = strAtCommand.substr(strAtCommand.size()-1);
							else
								newPToPCallStruct.strPtpcsPriority = "4";
							newPToPCallStruct.strPtpcsIdx = "";
							newPToPCallStruct.strPtpcsMpty = "";
							newPToPCallStruct.strPtpcsNumber = CCWAnumber;
							//newPToPCallStruct.strPtpcsType = "1";
							newPToPCallStruct.strPtpcsAlpha = "";
							newPToPCallStruct.strPtpcsUUS1 = "";
							if(!listAtCommand.empty())
								listAtCommand.pop_front();
							//读取UUS1信息
							mLogInstance->Log(tag+"read uus1");
							int iNumberForUus1u =0;
							while(iNumberForUus1u<100)
							{
								iNumberForUus1u++;
								if(!listAtCommand.empty())
								{
									strAtCommand = listAtCommand.front();
									mLogInstance->Log(tag+"recv: "+strAtCommand);
									if(strAtCommand.size()>=9 && strAtCommand.substr(0,9).compare("+CUUS1U: ") == 0)
									{
										int iFistYinHao = strAtCommand.find_first_of('"');
										int iSecondYinHao = strAtCommand.find_last_of('"');
										if(iFistYinHao != std::string::npos && iSecondYinHao != std::string::npos)
										{
											std::string strNumber = strAtCommand.substr(iFistYinHao+1,iSecondYinHao - iFistYinHao-1);
											mLogInstance->Log(tag+"1.uus1u: "+strNumber);

											for(int i=0; i<(strNumber.size()/2); i++)		//奇数字节和偶数字节互换
											{
												char tempuchar = strNumber[2*i];
												strNumber[2*i] = strNumber[2*i+1];
												strNumber[2*i+1] = tempuchar;
											}
											mLogInstance->Log(tag+"2.uus1u: "+strNumber);
											if(strNumber.compare("00") != 0 && (strNumber[4] != '0' || strNumber[5] != '0'))//长度为0
											{
												if(strNumber[strNumber.length()-1] == 'F')
													strNumber = strNumber.substr(0,strNumber.length()-1);
												mLogInstance->Log(tag+"3.uus1u: "+strNumber);
												newPToPCallStruct.strPtpcsUUS1 = strNumber.substr(6);
	//											//将此UUS1信息保存到UUS1信息的列表当中
	//											for(i=0;i<10;i++)
	//											{
	//												if(UUS1Infobuf[i].flag==0)
	//												{
	//													UUS1Infobuf[i].flag = 1;
	//													UUS1Infobuf[i].number = PToPbuffer[counterCLCC-1].number;
	//													UUS1Infobuf[i].UUS1 = PToPbuffer[counterCLCC-1].UUS1;
	//													break;
	//												}
	//											}
											}
											if(!listAtCommand.empty())
												listAtCommand.pop_front();
											break;
										}
										else
										{
											if(!listAtCommand.empty())
												listAtCommand.pop_front();
										}
									}
									else
									{
										if(!listAtCommand.empty())
											listAtCommand.pop_front();
									}
								}
								std::this_thread::sleep_for(std::chrono::milliseconds(10));
								mLogInstance->Log(tag + "3.read again...........................");
							}
							vPToPbuffer.push_back(newPToPCallStruct);

							CallListToMMI(0);
						}
					}
					else if(strAtCommand.size()>=9 && strAtCommand.substr(0,9).compare("+CUUS1I: ") == 0)
					{
						if(bMrmStatus == 0x21 || bMrmStatus == 0x31)
						{
							int iFistYinHao = strAtCommand.find_first_of('"');
							int iSecondYinHao = strAtCommand.find_last_of('"');
							if(iFistYinHao != std::string::npos && iSecondYinHao != std::string::npos)
							{
								std::string strNumber = strAtCommand.substr(iFistYinHao+1,iSecondYinHao - iFistYinHao-1);
								mLogInstance->Log(tag+"1.uus1u: "+strNumber);

								for(int i=0; i<(strNumber.size()/2); i++)		//奇数字节和偶数字节互换
								{
									char tempuchar = strNumber[2*i];
									strNumber[2*i] = strNumber[2*i+1];
									strNumber[2*i+1] = tempuchar;
								}
								mLogInstance->Log(tag+"2.uus1u: "+strNumber);
								if(strNumber.compare("00") != 0 && (strNumber[4] != '0' || strNumber[5] != '0'))//长度为0
								{
									if(strNumber[strNumber.length()-1] == 'F')
										strNumber = strNumber.substr(0,strNumber.length()-1);
									mLogInstance->Log(tag+"3.uus1u: "+strNumber);
									if(vPToPbuffer.size() > 0)
									{
										vPToPbuffer[vPToPbuffer.size() - 1].strPtpcsUUS1 = strNumber.substr(6);
									}
//											//将此UUS1信息保存到UUS1信息的列表当中
//											for(i=0;i<10;i++)
//											{
//												if(UUS1Infobuf[i].flag==0)
//												{
//													UUS1Infobuf[i].flag = 1;
//													UUS1Infobuf[i].number = PToPbuffer[counterCLCC-1].number;
//													UUS1Infobuf[i].UUS1 = PToPbuffer[counterCLCC-1].UUS1;
//													break;
//												}
//											}
									CallListToMMI(0);
								}
							}
							else
							{
								mLogInstance->Log(tag+"CUUS1I info is wrong");
							}
						}
					}
					else if((strAtCommand.size()>=3 && strAtCommand.find("RDY") != -1))
					{//SIMCOM模块启动
						bGprsStatus = 0;
						bGprsStatusBefore = 0xff;
						strDestinationIPAddress = "";
						strDestinationPort ="";
						SetMrmGprsStatus(BIT0,true,true,0x01);//标记g_GPRSBooting_Event

						bTimer17ForReset = false;
						iTime17ForReset = 1;
						//KillTimer(12);
						//SetTimer(13);
						//KillTimer(15);
					}
					else if((strAtCommand.size()>=7 && strAtCommand.substr(0,7).compare("+STC: 0") == 0))
					{//读取"+STC: 0"
						{
							bGprsStc = true;
							std::unique_lock<std::mutex> lck(mtxGprsStc);
							mwaitGprsStc.notify_all();
							mLogInstance->Log(tag + "mwaitGprsStc is notify");
						}
					}
					else if((strAtCommand.size()>=12 && strAtCommand.substr(0,12).compare("CONNECT 9600") == 0))
					{//CSD连接成功
						{
							bGprs9600 = true;
							std::unique_lock<std::mutex> lck(mtxGprsOk);
							mwaitGprsOk.notify_all();
							mLogInstance->Log(tag + "mwaitGprsOk is notify");
						}
					}
					else if((strAtCommand.size()>=7 && strAtCommand.substr(0,7).compare("SHUT OK") == 0))
					{//关闭移动场景
						{
							bGprsShut = true;
							std::unique_lock<std::mutex> lck(mtxGprsOk);
							mwaitGprsOk.notify_all();
							mLogInstance->Log(tag + "mwaitGprsOk is notify");
						}
					}
					else if((strAtCommand.size()>=8 && strAtCommand.substr(0,8).compare("CLOSE OK") == 0))
					{
						{
							bGprsCloseOk = true;
							std::unique_lock<std::mutex> lck(mtxGprsOk);
							mwaitGprsOk.notify_all();
							mLogInstance->Log(tag + "mwaitGprsOk is notify");
						}
					}
					else if((strAtCommand.size()>=15 && strAtCommand.substr(0,15).compare("ALREADY CONNECT") == 0))
					{
						{
							bGprsAlreadyConnect = true;
							std::unique_lock<std::mutex> lck(mtxGprsOk);
							mwaitGprsOk.notify_all();
							mLogInstance->Log(tag + "mwaitGprsOk is notify");
						}
					}
					else if((strAtCommand.size()>=10 && strAtCommand.substr(0,10).compare("CONNECT OK") == 0))
					{
						{
							bGprsConnectOk = true;
							std::unique_lock<std::mutex> lck(mtxGprsOk);
							mwaitGprsOk.notify_all();
							mLogInstance->Log(tag + "mwaitGprsOk is notify");
						}
					}
					else if((strAtCommand.size()>=4 && strAtCommand.substr(0,4).compare("+IPD") == 0))
					{//处于接收GPRS数据状态
						if((bGprsStatus == 0x01)||(bGprsStatus == 0x03))
						{
							if(bGprsStatus == 0x03)
								bGprsStatusBefore = 0x03;
							else if(bGprsStatus == 0x01)
								bGprsStatusBefore = 0xff;

							//SetTimer(9);
							bGprsStatus = 0x04;//设置为接收GPRS数据状态
						}
					}
					else if((strAtCommand.size()>=1 && strAtCommand.substr(0,1).compare(">") == 0))
					{
						if((bGprsStatus == 0x03))
						{
							{
								bGprsSendStart = true;
								//std::unique_lock<std::mutex> lck(mtxGprsSendStart);
								//mwaitGprsSendStart.notify_all();
								//mLogInstance->Log(tag + "mwaitGprsSendStart is notify");
								std::unique_lock<std::mutex> lck(mtxGprsOk);
								mwaitGprsOk.notify_all();
								mLogInstance->Log(tag + "mwaitGprsOk is notify");
							}
						}
						else
						{
							mLogInstance->Log("not in sending mode, please attention");
						}
					}
					else if((strAtCommand.size()>=7 && strAtCommand.substr(0,7).compare("SEND OK") == 0))
					{
						if((bGprsStatus == 0x03))
						{
							bGprsStatus = 0x01;//GPRS空闲状态
						}

						{
							bGprsSendOk = true;
							//std::unique_lock<std::mutex> lck(mtxGprsSendOk);
							//mwaitGprsSendOk.notify_all();
							//mLogInstance->Log(tag + "mwaitGprsSendOk is notify");
							std::unique_lock<std::mutex> lck(mtxGprsOk);
							mwaitGprsOk.notify_all();
							mLogInstance->Log(tag + "mwaitGprsOk is notify");
						}
					}
					else if((strAtCommand.size()>=9 && strAtCommand.substr(0,9).compare("SEND FAIL") == 0))
					{
						if((bGprsStatus == 0x03))
						{
							bGprsStatus = 0x01;//GPRS空闲状态
						}

						{
							bGprsSendFailed = true;
							//std::unique_lock<std::mutex> lck(mtxGprsSendFailed);
							//mwaitGprsSendFailed.notify_all();
							//mLogInstance->Log(tag + "mwaitGprsSendFailed is notify");
							std::unique_lock<std::mutex> lck(mtxGprsOk);
							mwaitGprsOk.notify_all();
							mLogInstance->Log(tag + "mwaitGprsOk is notify");
						}
					}
					else if((strAtCommand.size()>=11 && strAtCommand.substr(0,11).compare("+PDP: DEACT") == 0)
							|| (strAtCommand.size()>=16 && strAtCommand.substr(0,16).compare("STATE: PDP DEACT") == 0))
					{
						SetMrmGprsStatus(BIT1,true,true,0x01);//g_MRMRegisterInit_Event.SetEvent();

						bGprsStatus = 0;
						bGprsStatusBefore = 0xff;
						strDestinationIPAddress = "";
						strDestinationPort ="";
						//KillTimer(14);
						mTrainState->strSourAddre = "";
						memset(mTrainState->arrSourAddre, 0,  sizeof(mTrainState->arrSourAddre));
						//发送GPRS模块网络注册中信息
					}
					else if((bHaveGprsIPAddressFlag) && (strAtCommand.size()>=7 && strAtCommand[0]>='0' && strAtCommand[0]<='9' && strAtCommand.find(".") != -1))
					{
						mTrainState->strSourAddre = strAtCommand.substr(0,strAtCommand.size()-2);
						mLogInstance->Log(tag + "gprs ip is:" + mTrainState->strSourAddre );
						vector<string> tmpIp;
						StringSplit(mTrainState->strSourAddre, '.', tmpIp);
						if(tmpIp.size()!=4)
						{
							mLogInstance->Log("gprs ip is invalid");
						}
						else
						{
							for(int i=0; i<4 ;i++)
							{
								stringstream ss;
								ss << tmpIp[i];
								ss >> mTrainState->arrSourAddre[i];
							}
						}
						mTrainState->arrInitStatus[3] = mTrainState->arrInitStatus[3] | BIT5;

						{
							bGprsIPAdress = true;
							std::unique_lock<std::mutex> lck(mtxGprsIPAdress);
							mwaitGprsIPAdress.notify_all();
							mLogInstance->Log(tag + "mwaitGprsIPAdress is notify");
						}
					}
					else
					{
						mLogInstance->Log(tag+"something at command i don't know, please attention...");
					}




					if(!listAtCommand.empty())
						listAtCommand.pop_front();

//					{
//						bMrmResponse = true;
//						std::unique_lock<std::mutex> lck(mtxMrmResponse);
//						mwaitMrmResponse.notify_all();
//						std::cout<<GetCurrentTime()<<tag<<"mwaitMrmAck is notify"<<std::endl;
//					}

					if(bLogPrint3)
						std::cout<<GetCurrentTime()<<tag<< "listAtCommand number end :" << listAtCommand.size() <<std::endl<<std::endl;
				}
			}
		}
		catch(...)
		{
			mLogInstance->Log(tag+"something wrong  HandleAtCommandThread");
		}
	}

	mLogInstance->Log(tag+ "please attention, why this thread exit????????????????????????" );
}
void MrmGprsHandle::SendGprsDataThread()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+"start send gprs data process");

	while(bSendGprsData)
	{
		try
		{
			if (listGprsData.empty())
			{
				try
				{
					std::unique_lock <std::mutex> lck(mtx_GprsData);
					mwaitGprsData.wait(lck);
				}
				catch(...)
				{
					mLogInstance->Log(tag+"wait wrong mtx_GprsData");
				}
			}
			else
			{
				while(!listGprsData.empty())
				{
					if(bLogPrint3)
						std::cout<<std::endl<<GetCurrentTime()<<tag<< "listGprsData number begin :" << listGprsData.size() <<std::endl;

					_SendDataNet pSendDataNet = listGprsData.front();

					GprsDataSend(pSendDataNet);

					if(!listGprsData.empty())
						listGprsData.pop_front();

					if(bLogPrint3)
						std::cout<<GetCurrentTime()<<tag<< "listGprsData number end :" << listGprsData.size() <<std::endl<<std::endl;
				}
			}
		}
		catch(...)
		{
			mLogInstance->Log(tag+"something wrong  SendGprsDataThread");
		}
	}

	mLogInstance->Log(tag+ "please attention, why this thread exit????????????????????????" );
}
void MrmGprsHandle::DoWorkThread()
{
	if(bLogPrint2)
		mLogInstance->Log(tag+"start do work process");

	while (bDoWork)
	{
		try
		{
			std::unique_lock <std::mutex> lck(mtxSgmEvent);
			mLogInstance->Log(tag+"mwaitSgmEvent wait" );
			mwaitSgmEvent.wait(lck);//
		}
		catch(...)
		{

		}

		bTimer1ForQueryCsq = false;//暂停查询
		//std::cout<< "bTimer1ForQueryCsq = " <<bTimer1ForQueryCsq<<std::endl;

		while(iMrmForStep != 0x00 || iGprsForStep != 0x00)
		{
			std::cout<<GetCurrentTime()<<tag<<"please attention, iMrmForStep="<<iMrmForStep<<" iGprsForStep="<<iGprsForStep<<std::endl;

			if((iMrmForStep & BIT0) != 0 && mTrainState->bPower == 0x01)
			{//mrm初始化
				//bMRMResetForbidFlag = 1;
				if(!MrmBooting())
				{
					SetMrmGprsStatus(~BIT1,false,false,0x00);//g_MRMRegisterInit_Event.ResetEvent();
					bMrmForbidRigisterInitFlag = 1;
					//无法设置sgm,需要重启
					_SendData  pSendData;
					mFrameGenerate.GetFrame_RestBoard(pSendData,bPort,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,bPort);//复位sgm模块
					mDataProcesser.SendInfoForCan(pSendData);

					//设置计时器,30秒内收不到MRM发出的复位成功信息，直接执行booting
					mLogInstance->Log(tag+"reset sgm model because of booting faild");
					bTimer17ForReset = true;
					iTime17ForReset = 1;
				}
				//bMRMResetForbidFlag = 0;
				SetMrmGprsStatus(~BIT0,false,false,0x00);
			}

			if((iMrmForStep & BIT1) != 0 && mTrainState->bPower == 0x01)
			{//mrm登录网络后初始化
				//bMRMResetForbidFlag = 1;
				if(!MrmRegisterInit())
				{
					SetMrmGprsStatus(~BIT1,false,false,0x00);
					bMrmForbidRigisterInitFlag = 1;

					//无法初始化sgm,需要重启
					_SendData  pSendData;
					mFrameGenerate.GetFrame_RestBoard(pSendData,bPort,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,bPort);//复位sgm模块
					mDataProcesser.SendInfoForCan(pSendData);

					//设置计时器,30秒内收不到MRM发出的复位成功信息，直接执行booting
					mLogInstance->Log(tag+"reset sgm model because of MrmRegisterInit faild");
					bTimer17ForReset = true;
					iTime17ForReset = 1;
				}
				else
				{
					bMrmForbidRigisterInitFlag = 0;
					//if(bMrmStatus != 0x00)
					//{
					//	//初始状态不是空闲态
					//	_SendData  pSendData;
					//	mFrameGenerate.GetFrame_RestBoard(pSendData,bPort,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,bPort);//复位sgm模块
					//	mDataProcesser.SendInfoForCan(pSendData);
					//
					//	//设置计时器,30秒内收不到MRM发出的复位成功信息，直接执行booting
					//	mLogInstance->Log(tag+"reset sgm model because of mrm status is not 0x00");
					//	bTimer17ForReset = true;
					//	iTime17ForReset = 1;
					//}
				}
				//bMRMResetForbidFlag = 0;
				SetMrmGprsStatus(~BIT1,false,false,0x00);
			}

			if((iMrmForStep & BIT2) != 0 && mTrainState->bPower == 0x01)
			{//查询运营商并向MMI发送网络状态//不使用BIT2
				COPSInquire();
				SetMrmGprsStatus(~BIT2,false,false,0x00);
			}

			if((iMrmForStep & BIT3) != 0)
			{//退出所有通话
				if(vPToPbuffer.size()>=1)
					ExitAllPToPCall();
				if(vVGCbuffer.size()>=1)
					ExitGroupCall();

				{
					bMrmExitAllComplete = true;
					std::unique_lock<std::mutex> lck(mtxMrmExitAllComplete);
					mwaitMrmExitAllComplete.notify_all();
				}
				SetMrmGprsStatus(~BIT3,false,false,0x00);
			}

			if((iMrmForStep & BIT4) != 0 && mTrainState->bPower == 0x01)
			{//接收到个呼信息时，向对方发送UUS1信息
				UUS1InfoTx();
				SetMrmGprsStatus(~BIT4,false,false,0x00);
			}

			if((iMrmForStep & BIT5) != 0 && mTrainState->bPower == 0x01)
			{//按键呼叫操作
				for(int i=0;i<3;i++)
				{
					byte result=OriginateCall(strDialNumber,strDialType,strDialPriority);
					if((result==1)||(strDialNumber.compare("299") == 0))
						break;
				}
				strDialNumber=("");
				SetMrmGprsStatus(~BIT5,false,false,0x00);
			}

			if((iMrmForStep & BIT6) != 0 && mTrainState->bPower == 0x01)
			{//个呼被叫时,加入该个呼通话
				JoinPToPCall();
				SetMrmGprsStatus(~BIT6,false,false,0x00);
			}

			if((iMrmForStep & BIT7) != 0 && mTrainState->bPower == 0x01)
			{////挂断当前呼叫处理
				CallHangUpProcess();
				SetMrmGprsStatus(~BIT7,false,false,0x00);
			}

			if((iMrmForStep & BIT8) != 0 && mTrainState->bPower == 0x01)
			{//选择某个通话处理
				CallSelectProcess();
				SetMrmGprsStatus(~BIT8,false,false,0x00);
			}

			if((iMrmForStep & BIT9) != 0 && mTrainState->bPower == 0x01)
			{//占用上行信道
				if(GroupCallUplinkApply())
				{
					if(InquireCurrentVGC1())				//查询当前激活的组呼,并且改变呼叫列表(主要是为了保证作为主叫时挂机)
					{
						for(int i=0;i<vVGCbuffer.size();i++)
						{
							if(vVGCbuffer[i].strGcGID.compare(mCurrentActiveVGCbuffer.strGcGID)==0)
							{
								vVGCbuffer[i].strGcGID = mCurrentActiveVGCbuffer.strGcGID;
								vVGCbuffer[i].strGcGCA = mCurrentActiveVGCbuffer.strGcGCA;
								vVGCbuffer[i].strGcIs17Or18 = mCurrentActiveVGCbuffer.strGcIs17Or18;
								vVGCbuffer[i].strGcStat = mCurrentActiveVGCbuffer.strGcStat;
								vVGCbuffer[i].strGcDir = mCurrentActiveVGCbuffer.strGcDir;
								vVGCbuffer[i].strGcAckFlag = mCurrentActiveVGCbuffer.strGcAckFlag;
								vVGCbuffer[i].strGcPriority = mCurrentActiveVGCbuffer.strGcPriority;
								break;
							}
						}
					}
					CallListToMMI(0);
				}

				SetMrmGprsStatus(~BIT9,false,false,0x00);
			}

			if((iMrmForStep & BIT10) != 0 && mTrainState->bPower == 0x01)
			{//释放上行信道
				GroupCallUplinkRelease();

				SetMrmGprsStatus(~BIT10,false,false,0x00);
			}

			if((iMrmForStep & BIT11) != 0 && mTrainState->bPower == 0x01)
			{//退出当前组呼,不管主叫被叫
				HeldCurrentVGC();

				SetMrmGprsStatus(~BIT11,false,false,0x00);
			}

			if((iMrmForStep & BIT12) != 0 && mTrainState->bPower == 0x01)
			{//查询呼叫列表事件
				InquirePToPCall();
				InquireCurrentVGC();
				//调用呼叫处理程序
				CallListToMMI(0);

				//以下处理针对收到模块返回"NO CARRIER",关闭音频矩阵及恢复g_MRM.m_Status的状态？？？
				int i=0;
				for(i=0;i<vVGCbuffer.size();i++)
				{
					if(vVGCbuffer[i].strGcStat.compare("0") == 0)
						break;
				}
				if((vPToPbuffer.size()==0)&&(i>=vVGCbuffer.size()))
				{
					bMrmStatus = 0x01;
					if((mTrainState->iWorkMode == 0x65)||((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))
					{
						//CloseVoiceSwitch();
					}
				}

				SetMrmGprsStatus(~BIT12,false,false,0x00);
			}

			if((iMrmForStep & BIT13) != 0 && mTrainState->bPower == 0x01)
			{//当使用AT+CALCC=0查询失败时，使用AT+CALCC=1查询呼叫列表事件
				InquirePToPCall();
				InquireCurrentVGC2();
				CallListToMMI(0);

				SetMrmGprsStatus(~BIT13,false,false,0x00);
			}

			if((iMrmForStep & BIT14) != 0 && mTrainState->bPower == 0x01)
			{//
				if(AcknowledgeConfirmeProcess())//AC确认发送成功
				{
					//KillTimer(9);
					bACFailedCounter = 0;
				}
				else
				{
					if(bACFailedCounter<=3)//AC确认最多发送4次
						SetTimer(9);//发送不成功,启动random0x7fff(32767)重发定时器
					else
					{
						//KillTimer(9);
						bACFailedCounter = 0;
					}
				}

				InquirePToPCall();
				InquireCurrentVGC();
				CallListToMMI(0);

				SetMrmGprsStatus(~BIT14,false,false,0x00);
			}

			if((iMrmForStep & BIT15) != 0 && mTrainState->bPower == 0x01)
			{//查询网络运营商
				InquireAvailableOperator();
				SetMrmGprsStatus(~BIT15,false,false,0x00);
			}

			if((iMrmForStep & BIT16) != 0 && mTrainState->bPower == 0x01)
			{//选网操作
				SelectOperator(mTrainState->strSelectOperatorName);
				SetMrmGprsStatus(~BIT16,false,false,0x00);
			}

			if((iGprsForStep & BIT0) != 0 && mTrainState->bPower == 0x01)
			{//GPRS初始化
				if(GprsBooting())
				{
					 //"GPRS正在初始化……"
					mLogInstance->Log(tag+"gprs init successful");
				}
				else
				{
					mLogInstance->Log(tag+"gprs init failed");
					//"GPRS初始化失败……"
					SetMrmGprsStatus(~BIT1,false,false,0x01);//g_MRMRegisterInit_Event.ResetEvent();
					//无法设置gprs,需要重启
					_SendData  pSendData;
					mFrameGenerate.GetFrame_RestBoard(pSendData,bPort,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,bPort);//复位gprs模块
					mDataProcesser.SendInfoForCan(pSendData);
					//设置计时器,30秒内收不到gprs发出的复位成功信息，直接执行booting
					//mLogInstance->Log(tag+"reset sgm model because of booting faild");
					//bTimer17ForReset = true;
					//iTime17ForReset = 1;
				}
				SetMrmGprsStatus(~BIT0,false,false,0x01);
			}

			if((iGprsForStep & BIT1) != 0 && mTrainState->bPower == 0x01)
			{//GPRS登录网络后初始化
				if(GprsRegisterInit())
				{
					//"GPRS正在初始化……"
					mLogInstance->Log(tag+"gprs register init successful");
				}
				else
				{
					//"GPRS初始化失败……"
					mLogInstance->Log(tag+"gprs register init failed");

					//无法初始化sgm,需要重启
					_SendData  pSendData;
					mFrameGenerate.GetFrame_RestBoard(pSendData,bPort,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,bPort);//复位gprs模块
					mDataProcesser.SendInfoForCan(pSendData);

					//设置计时器,30秒内收不到gprs发出的复位成功信息，直接执行booting
					//mLogInstance->Log(tag+"reset sgm model because of booting faild");
					//bTimer17ForReset = true;
					//iTime17ForReset = 1;

				}
				SetMrmGprsStatus(~BIT1,false,false,0x01);
			}

			if((iGprsForStep & BIT3) != 0 && mTrainState->bPower == 0x01)
			{//进入GPRS空闲模式
				if(bGprsStatus == 0x00 || bGprsStatus == 0x01)
				{//待机模式->GPRS模式
					//"GPRS正在登录......"
					if(EnterGPRSMode())
					{
						mLogInstance->Log(tag+"gprs enter gprs successful");
						//g_GPRSData_List.RemoveAll();		//每次进入GPRS模式，将以前未发送出去的非重要GPRS数据清除掉
						//g_GPRS.SetTimer(2,1000,NULL);		//立即向GROS查询当前GRIS的IP地址
						//g_GPRS.m_InqiureGROSCounter = 6;
						//if(g_GPRS.m_Status!=0x00)
						//	 g_GPRS.SetTimer(14,180000,NULL);	//长时间收不到GPRS模块的数据,复位GPRS模块
					}
					else
					{
						mLogInstance->Log(tag+"gprs enter gprs failed");
						//SetTimer(8);//启动按固定间隔发送车次号校核信息
						//KillTimer(16);
					}
				}
				SetMrmGprsStatus(~BIT3,false,false,0x01);
			}

			if((iGprsForStep & BIT5) != 0 && mTrainState->bPower == 0x01)
			{//退出GPRS模式
				if( bGprsStatus == 0x01) //GPRS空闲状态下才退出
				{
					if(RetireGPRSMode())
					{//退出成功
						mLogInstance->Log(tag+"retire gprs successful");
						if(bNeedEnterCSDFlag)
						{
							bNeedEnterCSDFlag = false;
							SetMrmGprsStatus(BIT2,true,true,0x01);
						}
						else if(bNeedEnterGPRSFlag)
						{
							bNeedEnterGPRSFlag= false;
							SetMrmGprsStatus(BIT3,true,true,0x01);
						}
					}
					else
					{//退出失败
						mLogInstance->Log(tag+"retire gprs failed");
					}
				}
				else
				{
					if( bGprsStatus == 0x00)
					{
						if(bNeedEnterCSDFlag)
						{
							bNeedEnterCSDFlag = false;
							SetMrmGprsStatus(BIT2,true,true,0x01);
						}
					}
				}
				SetMrmGprsStatus(~BIT5,false,false,0x01);
			}

		}
		bTimer1ForQueryCsq = true;//重新开始查询
		//std::cout<<GetCurrentTime()<< "bTimer1ForQueryCsq = " <<bTimer1ForQueryCsq<<std::endl;
	}
}
void MrmGprsHandle::SetTimer(int iTimerId)
{

}
void MrmGprsHandle::KillTimer(int iTimerId)
{

}
void MrmGprsHandle::SendAT(string strAtCommand)
{
	_SendData  pSendData;
	mFrameGenerate.GetFrame_ATCommandToZhuanHuan(pSendData,bPort,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,strAtCommand);//ACK2//暂时注释
	mDataProcesser.SendInfoForCan(pSendData);

	if(bLogPrint1)
	{
		mLogInstance->Log(tag+"send: "+strAtCommand);
		//mLogInstance->Log(tag+"send: "+strAtCommand);
	}
	//std::cout<<GetCurrentTime()<<tag<<"send :"<< strAtCommand <<std::endl;
}
void MrmGprsHandle::SetMrmGprsStatus(int mask, bool bYuHuo, bool bShouldNotify, byte bWhileOne)
{//true代表与操作

	if(bWhileOne == 0x00)
	{
		if(!bYuHuo)
		{//false取消
			iMrmForStep &= mask;
		}
		else
		{//true置于
			iMrmForStep |= mask;
		}

		stringstream ss;
		if(bShouldNotify)
		{
			ss<<"set mask: ";
		}
		else
		{
			ss<<"uset mask: " ;
		}
		if(mask == BIT0 || mask == ~BIT0)
			ss<<"g_MRMBooting_Event";
		else if(mask == BIT1 || mask == ~BIT1)
			ss<<"g_MRMRegisterInit_Event";
		else if(mask == BIT2 || mask == ~BIT2)
			ss<<"g_MRMCOPS_Event";
		else if(mask == BIT3 || mask == ~BIT3)
			ss<<"g_MRMExitAllCall_Event";
		else if(mask == BIT4 || mask == ~BIT4)
			ss<<"g_MRMUUS1Tx_Event";
		else if(mask == BIT5 || mask == ~BIT5)
			ss<<"g_MRMDial_Event";
		else if(mask == BIT6 || mask == ~BIT6)
			ss<<"g_MRMJoinPtoPCall_Event";
		else if(mask == BIT7 || mask == ~BIT7)
			ss<<"g_MRMCallHangUp_Event";
		else if(mask == BIT8 || mask == ~BIT8)
			ss<<"g_MRMCallSelect_Event";
		else if(mask == BIT9 || mask == ~BIT9)
			ss<<"g_MRMPushPTT_Event";
		else if(mask == BIT10 || mask == ~BIT10)
			ss<<"g_MRMReleasePTT_Event";
		else if(mask == BIT11 || mask == ~BIT11)
			ss<<"g_MRMHeldCurruntVGC_Event";
		else if(mask == BIT12 || mask == ~BIT12)
			ss<<"g_MRMInquireCall_Event";
		else if(mask == BIT13 || mask == ~BIT13)
			ss<<"g_MRMInquireActiveCall_Event";
		else if(mask == BIT14 || mask == ~BIT14)
			ss<<"g_MRMSentAc_Event";
		else if(mask == BIT15 || mask == ~BIT15)
			ss<<"g_MRMInquireOperator_Event";
		else if(mask == BIT16 || mask == ~BIT16)
			ss<<"g_MRMSelectOperator_Event";
		else if(mask == BIT17 || mask == ~BIT17)
			ss<<"g_MRMCFInquire_Event";
		else if(mask == BIT18 || mask == ~BIT18)
			ss<<"g_MRMCFSet_Event";
		else if(mask == BIT19 || mask == ~BIT19)
			ss<<"g_MRMCFCancel_Event";
		else if(mask == BIT20 || mask == ~BIT20)
			ss<<"TrainNumberFNDeregisterEvent";
		else if(mask == BIT21 || mask == ~BIT21)
			ss<<"TrainNumberFNRegisterEvent";
		else if(mask == BIT22 || mask == ~BIT22)
			ss<<"EngineNumberFNDeregisterEvent";
		else if(mask == BIT23 || mask == ~BIT23)
			ss<<"EngineNumberFNRegisterEvent";
		else
			ss<<"1.unkown event, please attention";
		ss<<" ";
		for (int i=25;i>=0;i--)
		{
			ss<<((iMrmForStep>>i)&1);
		}
		mLogInstance->Log(tag+ss.str());
	}
	else
	{
		if(!bYuHuo)
		{//false取消
			iGprsForStep &= mask;
		}
		else
		{//true置于
			iGprsForStep |= mask;
		}

		stringstream ss;
		if(bShouldNotify)
		{
			ss<<"set mask: ";
		}
		else
		{
			ss<<"uset mask: " ;
		}
		if(mask == BIT0 || mask == ~BIT0)
			ss<<"g_GPRSBooting_Event";
		else if(mask == BIT1 || mask == ~BIT1)
			ss<<"g_GPRSRegisterInit_Event";
		else if(mask == BIT2 || mask == ~BIT2)
			ss<<"g_GPRSEnterCSDMode_Event";
		else if(mask == BIT3 || mask == ~BIT3)
			ss<<"g_GPRSEnterGPRSMode_Event";
		else if(mask == BIT4 || mask == ~BIT4)
			ss<<"g_GPRSRetireCSDMode_Event";
		else if(mask == BIT5 || mask == ~BIT5)
			ss<<"g_GPRSRetireGPRSMode_Event";
		else
			ss<<"2.unkown event, please attention";

		ss<<" ";
		for (int i=25;i>=0;i--)
		{
			ss<<((iGprsForStep>>i)&1);
		}
		mLogInstance->Log(tag+ss.str());
	}

	if(bShouldNotify)
	{
		{
			std::unique_lock<std::mutex> lck(mtxSgmEvent);
			mwaitSgmEvent.notify_all();
			mLogInstance->Log(tag+"mwaitSgmEvent is notify" );
		}
	}

}

bool MrmGprsHandle::MrmBooting()
{//函数目的：MRM模块初始化函数	//返回值: 成功为1，失败为0

	mLogInstance->Log(tag+"MrmBooting in");
	int i;

	for(i=0;i<3;i++)
	{
		bMrmOk = false;
		SendAT("AT+CMEE=1");
		{
			std::unique_lock <std::mutex> lck(mtxMrmOk);
			if(mwaitMrmOk.wait_for(lck, std::chrono::milliseconds(3000),[this]{ return bMrmOk; }))
			{
				mLogInstance->Log(tag+"AT+CMEE=1 answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+CMEE=1 answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	//设置是否回显,使用接收呼叫的扩展格式,在组呼中请求模块返回上行链路忙/闲状态,使能错误提示+CME ERROR,
	//不接收作为主呼时,被呼方的ID;接收作为被呼时,主呼方的ID号;设置主动返回位置更新及网络登录状态
	for(i=0;i<3;i++)
	{
		bMrmOk = false;
		SendAT("ATE1+CRC=1;+CAULEV=1;+CMEE=1;+COLP=0;+CLIP=1;+CREG=2");//ATE1+CRC=1;+CAULEV=1;+CMEE=1;+COLP=0;+CLIP=1;+CREG=2
		{
			std::unique_lock <std::mutex> lck(mtxMrmOk);
			if(mwaitMrmOk.wait_for(lck, std::chrono::milliseconds(3000),[this]{ return bMrmOk; }))
			{
				mLogInstance->Log(tag+"ATE1+CRC=1 answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"ATE1+CRC=1 answer none") ;//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	//查询位置更新及网络登录状态
	std::this_thread::sleep_for(std::chrono::milliseconds(500));//500
	for(i=0;i<3;i++)
	{
		bMrmRegisterAck = false;
		SendAT("AT+CREG?");  //此处的回应将影响 bMrmCregSuccess
		{
			std::unique_lock <std::mutex> lck(mtxMrmRegisterAck);
			if(mwaitMrmRegisterAck.wait_for(lck, std::chrono::milliseconds(3000),[this]{ return bMrmRegisterAck; }))
			{//
				mLogInstance->Log(tag+"AT+CREG? answer ok");//mLogInstance->Log(tag+"registerack is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+CREG? answer none");//mLogInstance->Log(tag+"can not wait registerack");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	if(i>=3)
	{
		mLogInstance->Log(tag+"MrmBooting out bad");
		return false;
	}

	SetMrmGprsStatus(BIT1,true,true,0x00);//标记g_MRMRegisterInit_Event

	mLogInstance->Log(tag+"MrmBooting out ok");
	return true;
}

bool MrmGprsHandle::MrmRegisterInit()
{//MRM模块登录后初始化 //成功为1，失败为0

	mLogInstance->Log(tag+"MrmRegisterInit in");

	int i,counter;
	{
		std::unique_lock <std::mutex> lck(mtxMrmCregSuccess);
		if(mwaitMrmCregSuccess.wait_for(lck, std::chrono::milliseconds(20000),[this]{ return bMrmCregSuccess; }))
		{
			mLogInstance->Log(tag+"creg answer ok");//mLogInstance->Log(tag+"creg is answered");
		}
		else
		{//尚未登录成功
			mLogInstance->Log(tag+"creg answer none");//mLogInstance->Log(tag+"can not wait creg");
			////////////选择GSMR网络//////////////////////////////////
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			counter = 1;
			for(i=0;i<counter;i++)
			{
				bMrmOk=false;
				SendAT("AT+COPS=1,2,\"46020\"");	//AT+COPS=1,2,"46020"
				{
					std::unique_lock <std::mutex> lck2(mtxMrmOk);
					if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(80000),[this]{ return bMrmOk; }))
					{
						mLogInstance->Log(tag+"AT+COPS=1,2,46020 answer ok");//mLogInstance->Log(tag+"mrmok is answered");
						mTrainState->arrInitStatus[0] = 0x02;		//GSM-R
						break;
					}
					else
					{
						mLogInstance->Log(tag+"AT+COPS=1,2,,46020 answer none");//mLogInstance->Log(tag+"can not wait mrmok");
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));
			}
			if(i >= counter)
			{
				mLogInstance->Log(tag+"MrmRegisterInit out bad");
				return false;//MRM模块初始化失败
			}
		}
	}
	//////////////////////设置呼叫保持CCWA的主动返回码/////////////////////////
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	counter = 3;
	for(i=0;i<counter;i++)
	{
		bMrmOk=false;
		SendAT("AT+CCWA=1,1,1");
		{
			std::unique_lock <std::mutex> lck3(mtxMrmOk);
			if(mwaitMrmOk.wait_for(lck3, std::chrono::milliseconds(5000),[this]{ return bMrmOk; }))
			{
				mLogInstance->Log(tag+"AT+CCWA=1,1,1 answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+CCWA=1,1,1 answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		//return 0;							//由于有的SIM卡在网络中没有开通呼叫等待业务，所以不能认为是MRM模块初始化失败
	}

	//设置是否回显,使用接收呼叫的扩展格式,在组呼中请求模块返回上行链路忙/闲状态,使能错误提示+CME ERROR,
	//不接收作为主呼时,被呼方的ID;接收作为被呼时,主呼方的ID号;设置主动返回位置更新及网络登录状态,接收UUS1信息
	counter = 3;
	for(i=0;i<counter;i++)
	{
		bMrmOk = false;
		SendAT("ATE1+CRC=1;+CAULEV=1;+CMEE=1;+COLP=0;+CLIP=1;+CREG=2;+CUUS1=1,1"); //ATE1+CRC=1;+CAULEV=1;+CMEE=1;+COLP=0;+CLIP=1;+CREG=2;+CUUS1=1,1
		{
			std::unique_lock <std::mutex> lck4(mtxMrmOk);
			if(mwaitMrmOk.wait_for(lck4, std::chrono::milliseconds(3000),[this]{ return bMrmOk; }))
			{
				mLogInstance->Log(tag+"ATE1+CRC=1 answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"ATE1+CRC=1 answer error");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		mLogInstance->Log(tag+"MrmRegisterInit out bad");
		return false;//MRM模块初始化失败
	}

	bMrmStatus = 0x01;  //空闲状态

	InquirePToPCall();////查询GSMR组呼和个呼,为迟后进入进行查询
	InquireCurrentVGC();//
	if((vPToPbuffer.size() !=0)||(vVGCbuffer.size()!=0))//
		CallListToMMI(0);//

	//请求注册机车号功能号
	if((mTrainState->iWorkMode == 0x65) || ((mTrainState->iWorkMode >= 0x81)&&(mTrainState->iWorkMode <= 0xe4)))
	{
		EngineNumberFNTemp.strFnNumber.clear();
		EngineNumberFNTemp.strFnNumber = mTrainState->strEngineNumber;
		EngineNumberFNTemp.strFnNumber = ("0863")+EngineNumberFNTemp.strFnNumber+("01");
		EngineNumberFNTemp.bRegisterFlag = 1;		//注册
		vEngineNumberFNList.push_back(EngineNumberFNTemp);
		if((vPToPbuffer.size() ==0)||(vVGCbuffer.size()==0))		//没有组呼和个呼,马上注册机车号功能号
			SetTimer(2);//SetTimer(2,10000,NULL);
		else
			SetTimer(2);//SetTimer(2,15000,NULL);
	}

	//已经初始化成功，不再重复进行初始化
	SetMrmGprsStatus(~BIT1,false,false,0x00);//g_MRMRegisterInit_Event.ResetEvent();

	//为避免强制选网模块误返回OK,需要再查一次网络登录状态
	bMrmRegisterAck = false;
	SendAT("AT+CREG?");
	{
		std::unique_lock <std::mutex> lck5(mtxMrmRegisterAck);
		if(mwaitMrmRegisterAck.wait_for(lck5, std::chrono::milliseconds(2000),[this]{ return bMrmRegisterAck; }))
		{//
			mLogInstance->Log(tag+"AT+CREG? answer ok");//mLogInstance->Log(tag+"registerack is answered");
		}
		else
		{
			mLogInstance->Log(tag+"AT+CREG? answer none");//mLogInstance->Log(tag+"can not wait registerack");
		}
	}
	mLogInstance->Log(tag+"MrmRegisterInit out ok");
	return true;
}
bool MrmGprsHandle::COPSInquire()
{
	mLogInstance->Log(tag+"COPSInquire in");
	bMrmOk = false;
	SendAT("AT+COPS?");
	{
		std::unique_lock <std::mutex> lck(mtxMrmOk);
		if(mwaitMrmOk.wait_for(lck, std::chrono::milliseconds(2000),[this]{ return bMrmOk; }))
		{//查询网络登录状态
			mLogInstance->Log(tag+"AT+COPS? answer ok");//mLogInstance->Log(tag+"mrmcops is answered");
			mLogInstance->Log(tag+"COPSInquire out ok");
			return true;
		}
		else
		{
			mLogInstance->Log(tag+"AT+COPS? answer none");//mLogInstance->Log(tag+"can not wait mrmcops");
		}
	}
	mLogInstance->Log(tag+"COPSInquire out bad");
	return false;
}
//bool MrmGprsHandle::COPSInquire()
//{
//	mLogInstance->Log(tag+"COPSInquire in");
//	bMrmCops = false;
//	SendAT("AT+COPS?");
//	{
//		std::unique_lock <std::mutex> lck(mtxMrmCops);
//		if(mwaitMrmCops.wait_for(lck, std::chrono::milliseconds(2000),[this]{ return bMrmCops; }))
//		{//查询网络登录状态
//			mLogInstance->Log(tag+"mrmcops answer ok");//mLogInstance->Log(tag+"mrmcops is answered");
//			mLogInstance->Log(tag+"COPSInquire out ok");
//			return true;
//		}
//		else
//		{
//			mLogInstance->Log(tag+"mrmcops answer none");//mLogInstance->Log(tag+"can not wait mrmcops");
//		}
//	}
//	mLogInstance->Log(tag+"COPSInquire out bad");
//	return false;
//}
void MrmGprsHandle::CallListToMMI(byte TxFlag)
{
	mLogInstance->Log(tag+"CallListToMMI in");

	std::cout<<GetCurrentTime()<<tag<<"vPToPbuffer.size()=" << vPToPbuffer.size() <<" vVGCbuffer.size()=" << vVGCbuffer.size() << std::endl;

	if(vPToPbuffer.size()>0)
		std::cout<<GetCurrentTime()<<tag<<"Number=" + vPToPbuffer[0].strPtpcsNumber<<" Dir=" + vPToPbuffer[0].strPtpcsDir <<" Priority=" <<vPToPbuffer[0].strPtpcsPriority<<" Stat=" << vPToPbuffer[0].strPtpcsStat<<" UUS1=" + vPToPbuffer[0].strPtpcsUUS1<<std::endl;
	if(vVGCbuffer.size()>0)
		std::cout<<GetCurrentTime()<<tag<<"Gid=" + vVGCbuffer[0].strGcGID<<" Is17Or18=" + vVGCbuffer[0].strGcIs17Or18 <<" State=" <<vVGCbuffer[0].strGcStat<<" Dir=" << vVGCbuffer[0].strGcDir<<" Gca=" + vVGCbuffer[0].strGcGCA<<std::endl;
	if(vPToPbuffer.size()==0 && vVGCbuffer.size()==0)
		std::cout<<GetCurrentTime()<<tag<<"calllsit is empty"<<std::endl;

	_SendData  pSendData;
	mFrameGenerate.GetFrame_CallListToMmi(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,vPToPbuffer,vVGCbuffer);
	mDataProcesser.SendInfoForCan(pSendData);
	mLogInstance->Log(tag+"CallListToMMI out");
}
bool MrmGprsHandle::InquirePToPCall()
{
	int	i,j;
	vPToPTempbuffer.clear();//iCounterTempCLCC = 0;
	//bMrmResponse = false;
	bMrmOk =false;
	bMrmError = false;
	mLogInstance->Log(tag+"InquirePToPCall in");
	SendAT("AT+CLCC");
	{
		//{
		//	std::unique_lock <std::mutex> lck1(mtxMrmResponse);
		//	if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(2000),[this]{ return bMrmResponse; }))
		//	{
		//		mLogInstance->Log(tag+"AT+CLCC answer none");
		//		mLogInstance->Log(tag+"InquirePToPCall out bad");
		//		return false;
		//	}
		//}
		//等到回应
		{//ok
			std::unique_lock <std::mutex> lck2(mtxMrmOk);
			if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(3000),[this]{ return (bMrmOk | bMrmError); }))
			{
				if(bMrmOk)
				{
					mLogInstance->Log(tag+"AT+CLCC answer ok");
					//iCounterCLCC = iCounterTempCLCC;
					vPToPbuffer.clear();
					for(i=0;i<vPToPTempbuffer.size();i++)
						vPToPbuffer.push_back(vPToPTempbuffer[i]);//
					if(vPToPbuffer.size()==0)
					{//当前已经没有进行的个呼通话,清除掉UUS1信息列表
						vUUS1Infobuf.clear();
					}
					else
					{
						//当前存在个呼,将UUS1信息列表添加到呼叫列表当中去
						for(i=0;i<vUUS1Infobuf.size();i++)
						{
							if(vUUS1Infobuf[i].iUiflag==1)
							{
								for(j=0;j<vPToPbuffer.size();j++)
								{
									if(vUUS1Infobuf[i].strUiNumber.compare(vPToPbuffer[j].strPtpcsNumber)==0)
									{
										vPToPbuffer[j].strPtpcsUUS1 = vUUS1Infobuf[i].strUiUUS1;
										break;
									}
								}
								if(j>=vPToPbuffer.size())
								{
									vUUS1Infobuf[i].iUiflag = 0;
								}
							}
						}
					}
					if((vPToPbuffer.size() == 0) && (bMrmStatus == 0x31))
						bMrmStatus = 0x01;
					mLogInstance->Log(tag+"InquirePToPCall out ok");
					return true;
				}
				else if(bMrmError)
				{
					mLogInstance->Log(tag+"AT+CLCC answer error");
				}
			}
			else
			{
				mLogInstance->Log(tag+"AT+CLCC answer none");
			}
		}
	}
	mLogInstance->Log(tag+"InquirePToPCall out bad");
	return false;
}
//函数目的：使用AT+CALCC=0查询当前所有的组呼
bool MrmGprsHandle::InquireCurrentVGC()
{
	int	i;
	bInquireCurrentVGCFlag = 1;
	//bMrmResponse = false;
	bMrmOk = false;
	bMrmError = false;
	bMrmError3 = false;
	//iCounterTempCALCC = 0;
	vVGCTempbuffer.clear();
	mLogInstance->Log(tag+"InquireCurrentVGC in");
	SendAT("AT+CALCC=0");
	{
		//{
		//	std::unique_lock <std::mutex> lck1(mtxMrmResponse);
		//	if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(2000),[this]{ return bMrmResponse; }))
		//	{
		//		mLogInstance->Log(tag+"InquireCurrentVGC out bad");
		//		bInquireCurrentVGCFlag = 0;
		//		return false;
		//	}
		//}
		//等到回应
		{//ok
			std::unique_lock <std::mutex> lck2(mtxMrmOk);
			if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return bMrmOk | bMrmError; }))
			{
				if(bMrmOk)
				{
					//iCounterCALCC = iCounterTempCALCC;
					mLogInstance->Log(tag+"AT+CALCC=0 answer ok");
					vVGCbuffer.clear();
					for(i=0;i<vVGCTempbuffer.size();i++)
					{
						vVGCbuffer.push_back(vVGCTempbuffer[i]);
					}
					for(i=0;i<vVGCbuffer.size();i++)
					{
						if(vVGCbuffer[i].strGcStat.compare("0")==0)//active
						{
							bMrmStatus = 0x41;
							CallListToMMI(0);
							//OpenVoiceSwitch(0x65);		//打开音频矩阵
							break;
						}
					}
					if((vVGCbuffer.size()==0) && (bMrmStatus == 0x41))
						bMrmStatus = 0x01;

					bInquireCurrentVGCFlag = 0;
					mLogInstance->Log(tag+"InquireCurrentVGC out ok");
					return true;
				}
				else if(bMrmError)
				{
					mLogInstance->Log(tag+"AT+CALCC=0 answer error");
					if(bMrmError3)
					{
						SetMrmGprsStatus(BIT13,true,true,0x00);
					}
					bInquireCurrentVGCFlag = 0;
					mLogInstance->Log(tag+"InquireCurrentVGC out bad");
					return false;
				}
			}
			else
			{
				mLogInstance->Log(tag+"AT+CALCC=0 answer none");
			}
		}
	}
	bInquireCurrentVGCFlag = 0;
	mLogInstance->Log(tag+"InquireCurrentVGC out bad");
	return false;
}


bool MrmGprsHandle::ExitAllPToPCall()
{
	mLogInstance->Log(tag+"ExitAllPToPCall in");
	while(true)
	{
		if(vPToPbuffer.size() == 0)								//当前已经没有个呼，可以退出
			break;

		if(vPToPbuffer.size() == 1)								//当前只有一个个呼
		{
			bMrmOk = false;
			bMrmError = false;
			//bMrmResponse = false;
			SendAT("ATH");					//退出当前个呼
			{
				//{
				//	std::unique_lock <std::mutex> lck1(mtxMrmResponse);
				//	if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(2000),[this]{ return bMrmResponse; }))
				//	{
				//		continue;
				//	}
				//}
				{
					std::unique_lock <std::mutex> lck(mtxMrmOk);
					if(mwaitMrmOk.wait_for(lck, std::chrono::milliseconds(1000),[this]{ return (bMrmOk | bMrmError); }))
					{
						if(bMrmOk)
						{
							mLogInstance->Log(tag+"ATH answer ok");
							break;//唯一存在的个呼已经挂断，可以退出
						}
						else if(bMrmError)
						{
							mLogInstance->Log(tag+"ATH answer error");
						}
					}
					else
					{
						mLogInstance->Log(tag+"ATH answer nocarrier");
					}
				}
			}
		}
		else if(vPToPbuffer.size()>1)								//当前有两个或两个以上的个呼同时存在
		{
			bMrmOk = false;
			//bMrmResponse = false;
			bMrmError = false;
			SendAT("AT+CHLD=1");					//退出当前个呼，但是会激活处于保持状态的呼叫
			{
				//{
				//	std::unique_lock <std::mutex> lck1(mtxMrmResponse);
				//	if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(2000),[this]{ return bMrmResponse; }))
				//	{
				//		continue;
				//	}
				//}
				{
					std::unique_lock <std::mutex> lck2(mtxMrmOk);
					if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(1000),[this]{ return (bMrmOk | bMrmError); }))
					{
						if(bMrmOk)
						{
							mLogInstance->Log(tag+"AT+CHLD=1 answer ok");
							continue;//继续退出
						}
						else if(bMrmError)
						{
							mLogInstance->Log(tag+"AT+CHLD=1 answer error");
						}
					}
					else
					{
						mLogInstance->Log(tag+"AT+CHLD=1 answer nocarrier");
					}
				}
			}
		}
		InquirePToPCall();								//当前存在激活的个呼，也进行了个呼的查询，可能需要调整!!
	}

	bMrmStatus = 0x01;
	//CloseVoiceSwitch(); //关闭音频矩阵

	InquirePToPCall();
	InquireCurrentVGC();

	CallListToMMI(0);//
	mLogInstance->Log(tag+"ExitAllPToPCall out ok");
	return true;
}
bool MrmGprsHandle::ExitGroupCall()
{
	mLogInstance->Log(tag+"ExitAllPToPCall in");

	string stringtemp;
	string stringGID,stringGCA,string17Or18,stringstat,stringdir,stringack_flag,stringpriority;
	int i,counter;

	if(InquireCurrentVGC1())				//查询当前激活的组呼,并且改变呼叫列表(主要是为了保证作为主叫时挂机)
	{
		for(i=0;i<vVGCbuffer.size();i++)
		{
			if(vVGCbuffer[i].strGcGID.compare(mCurrentActiveVGCbuffer.strGcGID) == 0)
			{
				vVGCbuffer[i].strGcGID = mCurrentActiveVGCbuffer.strGcGID;
				vVGCbuffer[i].strGcGCA = mCurrentActiveVGCbuffer.strGcGCA;
				vVGCbuffer[i].strGcIs17Or18 = mCurrentActiveVGCbuffer.strGcIs17Or18;
				vVGCbuffer[i].strGcStat = mCurrentActiveVGCbuffer.strGcStat;
				vVGCbuffer[i].strGcDir = mCurrentActiveVGCbuffer.strGcDir;
				vVGCbuffer[i].strGcAckFlag = mCurrentActiveVGCbuffer.strGcAckFlag;
				vVGCbuffer[i].strGcPriority = mCurrentActiveVGCbuffer.strGcPriority;
				break;
			}
		}
	}

	for(i=0;i<vVGCbuffer.size();i++)
	{
		if(vVGCbuffer[i].strGcStat.compare("0") == 0)			//这是当前激活的组呼
		{
			stringGID = vVGCbuffer[i].strGcGID;
			stringGCA = vVGCbuffer[i].strGcGCA;
			string17Or18 = vVGCbuffer[i].strGcIs17Or18;
			stringstat = vVGCbuffer[i].strGcStat;	//0:active,1:held,2:incoming call
			stringdir = vVGCbuffer[i].strGcDir;	//0:MO,1:MT
			stringack_flag = vVGCbuffer[i].strGcAckFlag;
			stringpriority = vVGCbuffer[i].strGcPriority;
			break;
		}
	}

	if(stringstat.compare("0") == 0)								//当前存在正在进行的组呼
	{
		if(stringdir.compare("0") == 0)							//组呼主叫
		{
			counter = 100;
			while(counter)
			{
				bMrmOk =false;
				//bMrmResponse = false;
				bMrmError = false;
				bMrmNoCarrier = false;
				bMrmError162 = false;

				SendAT("AT+CAPTT=2");		//退出组呼前必须抢占上行信道
				{
					//std::unique_lock <std::mutex> lck1(mtxMrmResponse);
					//if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(2000),[this]{ return bMrmResponse; }))
					//{
					//	mLogInstance->Log(tag+"AT+CAPTT=2 answer none");
					//	counter--;
					//	if(counter<=0)
					//	{
					//		mLogInstance->Log(tag+"ExitAllPToPCall out bad");
					//		mLogInstance->Log(tag+"ExitAllPToPCall out bad");
					//		return false;					//多次抢占信道未成功，退出组呼失败
					//	}
					//}
					//else
					{
						std::unique_lock <std::mutex> lck2(mtxMrmOk);
						if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(1000),[this]{ return (bMrmOk | bMrmError | bMrmNoCarrier); }))
						{
							if(bMrmOk)
							{
								mLogInstance->Log(tag+"AT+CAPTT=2 answer ok");
								break;//
							}
							else if(bMrmError)
							{
								mLogInstance->Log(tag+"AT+CAPTT=2 answer error");
								if(bMrmError162)
								{
									if(stringGID.compare("299") == 0)		//记录退出紧急呼叫的时间
									{
//												EmergencyCallDUREndTime = CTime::GetCurrentTime();
//												EmergencyCallDURSpanTime = EmergencyCallDUREndTime-EmergencyCallDURStartTime;
//												bEmergencyCallFlag = 0;
//												EmergencyCallRELStartTime = CTime::GetCurrentTime();
//												SetTimer(9,rand(),NULL);	//请求发送AC确认
//												ACFailedCounter = 0;
									}
									InquireCurrentVGC();
									//调用呼叫处理程序
									if(bNoListToMMIFlag!=0)
										bNoListToMMIFlag = 0;
									else
										CallListToMMI(0);
									//CloseVoiceSwitch();
									mLogInstance->Log(tag+"ExitAllPToPCall out ok");
									return true;					//这个组呼已经不存在了，退出成功
								}
							}
							else if(bMrmNoCarrier)
							{
								mLogInstance->Log(tag+"AT+CAPTT=2 answer nocarrier");
								if(stringGID.compare("299") == 0)		//记录退出紧急呼叫的时间
								{
//									EmergencyCallDUREndTime = CTime::GetCurrentTime();
//									EmergencyCallDURSpanTime = EmergencyCallDUREndTime-EmergencyCallDURStartTime;
//									bEmergencyCallFlag = 0;
//									EmergencyCallRELStartTime = CTime::GetCurrentTime();
//									SetTimer(9,rand(),NULL);	//请求发送AC确认
//									ACFailedCounter = 0;
								}
								InquireCurrentVGC();
								//调用呼叫处理程序
								if(bNoListToMMIFlag)
									bNoListToMMIFlag = 0;
								else
									CallListToMMI(0);
								//CloseVoiceSwitch();
								mLogInstance->Log(tag+"ExitAllPToPCall out ok");
								return true;					//中途掉网，退出组呼成功
							}
						}
						else
						{
							mLogInstance->Log(tag+"AT+CAPTT=2 answer nocarrier");
						}
					}
				}
			}

			counter = 3;
			while(counter)
			{
				bMrmOk =false;
				//bMrmResponse = false;
				bMrmError = false;
				bMrmNoCarrier = false;

				SendAT("ATH");		//退出组呼前必须抢占上行信道
				{
//						std::unique_lock <std::mutex> lck1(mtxMrmResponse);
//						if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(3000),[this]{ return bMrmResponse; }))
//						{
//
//						}
//						else
						{
							std::unique_lock <std::mutex> lck2(mtxMrmOk);
							if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(1000),[this]{ return (bMrmOk | bMrmError | bMrmNoCarrier); }))
							{
								if(bMrmOk)
								{
									mLogInstance->Log(tag+"ATH answer ok");
									if(stringGID.compare("299") == 0)		//记录退出紧急呼叫的时间
									{
			//									EmergencyCallDUREndTime = CTime::GetCurrentTime();
			//									EmergencyCallDURSpanTime = EmergencyCallDUREndTime-EmergencyCallDURStartTime;
			//									bEmergencyCallFlag = 0;
			//									EmergencyCallRELStartTime = CTime::GetCurrentTime();
			//									SetTimer(9,(rand()/3)+10,NULL);	//请求发送AC确认
			//									ACFailedCounter = 0;
									}
									InquireCurrentVGC();
									//调用呼叫处理程序
									if(bNoListToMMIFlag!=0)
										bNoListToMMIFlag = 0;
									else
										CallListToMMI(0);
									//CloseVoiceSwitch();
									mLogInstance->Log(tag+"ExitAllPToPCall out ok");
									return true;
								}
								else if(bMrmError)
								{
									mLogInstance->Log(tag+"ATH answer error");
								}
								else if(bMrmNoCarrier)
								{
									mLogInstance->Log(tag+"ATH answer nocarrier");
									if(stringGID.compare("299") == 0)		//记录退出紧急呼叫的时间
									{
	//									EmergencyCallDUREndTime = CTime::GetCurrentTime();
	//									EmergencyCallDURSpanTime = EmergencyCallDUREndTime-EmergencyCallDURStartTime;
	//									bEmergencyCallFlag = 0;
	//									EmergencyCallRELStartTime = CTime::GetCurrentTime();
	//									SetTimer(9,rand(),NULL);	//请求发送AC确认
	//									ACFailedCounter = 0;
									}
									InquireCurrentVGC();
									//调用呼叫处理程序
									if(bNoListToMMIFlag)
										bNoListToMMIFlag = 0;
									else
										CallListToMMI(0);
									//CloseVoiceSwitch();
									mLogInstance->Log(tag+"ExitAllPToPCall out ok");
									return true;					//中途掉网，退出组呼成功
								}
							}
							else
							{
								mLogInstance->Log(tag+"ATH answer none");
								counter--;
								if(counter<=0)
								{
									InquireCurrentVGC();//可能死锁???
									if(bNoListToMMIFlag!=0)
										bNoListToMMIFlag = 0;
									else
										CallListToMMI(0);
									mLogInstance->Log(tag+"ExitAllPToPCall out bad");
									return	false;	//多次挂机未成功，退出组呼失败
								}
							}
						}
				}

			}
		}

		if(stringdir.compare("1") == 0)							//组呼被叫
		{
			if(string17Or18.compare("17") == 0)
			{
				bMrmOk =false;
				//bMrmResponse = false;
				bMrmError = false;
				bMrmNoCarrier = false;

				SendAT("AT+CAPTT=0");		//退出之前释放信道
				{
//						std::unique_lock <std::mutex> lck1(mtxMrmResponse);
//						if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(4000),[this]{ return bMrmResponse; }))
//						{
//
//						}
//						else
						{
								std::unique_lock <std::mutex> lck2(mtxMrmOk);
								if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(1000),[this]{ return (bMrmOk | bMrmError | bMrmNoCarrier); }))
								{
									if(bMrmOk)
									{
										mLogInstance->Log(tag+"AT+CAPTT=0 answer ok");
									}
									else if(bMrmError)
									{
										mLogInstance->Log(tag+"AT+CAPTT=0 answer error");
									}
									else if(bMrmNoCarrier)
									{
										mLogInstance->Log(tag+"AT+CAPTT=0 answer nocarrier");
										if(stringGID.compare("299") == 0)		//记录退出紧急呼叫的时间
										{
			//												EmergencyCallDUREndTime = CTime::GetCurrentTime();
			//												EmergencyCallDURSpanTime = EmergencyCallDUREndTime-EmergencyCallDURStartTime;
			//												bEmergencyCallFlag = 0;
			//												EmergencyCallRELStartTime = CTime::GetCurrentTime();
			//												SetTimer(9,rand(),NULL);	//请求发送AC确认
			//												ACFailedCounter = 0;
										}
										InquireCurrentVGC();
										//调用呼叫处理程序
										if(bNoListToMMIFlag!=0)
											bNoListToMMIFlag = 0;
										else
											CallListToMMI(0);
										//CloseVoiceSwitch();
										mLogInstance->Log(tag+"ExitAllPToPCall out ok");
										return true;					//这个组呼已经不存在了，退出成功
									}
								}
								else
								{
									mLogInstance->Log(tag+"AT+CAPTT=0 answer error");
								}
						}
				}
			}
			counter = 3;
			while(counter)
			{
				bMrmOk =false;
				//bMrmResponse = false;
				bMrmError = false;
				bMrmNoCarrier = false;

				SendAT("AT+CAHLD");		//离开组呼
				{
//					{
//						std::unique_lock <std::mutex> lck1(mtxMrmResponse);
//						if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(3000),[this]{ return bMrmResponse; }))
//						{
//
//						}
//					}
					{
						std::unique_lock <std::mutex> lck2(mtxMrmOk);
						if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(1000),[this]{ return (bMrmOk | bMrmError | bMrmNoCarrier); }))
						{
							if(bMrmOk)
							{
								mLogInstance->Log(tag+"AT+CAPTT=0 answer ok");
								if(stringGID.compare("299") == 0)		//记录退出紧急呼叫的时间
								{
		//							EmergencyCallDUREndTime = CTime::GetCurrentTime();
		//							EmergencyCallDURSpanTime = EmergencyCallDUREndTime-EmergencyCallDURStartTime;
		//							bEmergencyCallFlag = 0;
		//							EmergencyCallRELStartTime = CTime::GetCurrentTime();
		//							SetTimer(9,rand()/3+5000,NULL);	//请求发送AC确认
		//							ACFailedCounter = 0;
								}
								for(i=0;i<vVGCbuffer.size();i++)
								{
									if(vVGCbuffer[i].strGcStat.compare("0") == 0)
										break;
								}
								vVGCbuffer[i].strGcStat = "1";
								//调用呼叫处理程序
								if(bNoListToMMIFlag)
									bNoListToMMIFlag = 0;
								else
									CallListToMMI(0);
								bMrmStatus = 0x01;
								//CloseVoiceSwitch();
								mLogInstance->Log(tag+"ExitAllPToPCall out ok");
								return true;					//这个组呼已经不存在了，退出成功
							}
							else if(bMrmError)
							{
								mLogInstance->Log(tag+"AT+CAPTT=0 answer error");
								InquireCurrentVGC();
								if(bNoListToMMIFlag)
									bNoListToMMIFlag = 0;
								else
									CallListToMMI(0);
								mLogInstance->Log(tag+"ExitAllPToPCall out bad");
								return false;
							}
							else if(bMrmNoCarrier)
							{
								mLogInstance->Log(tag+"AT+CAPTT=0 answer nocarrier");
								if(stringGID.compare("299") == 0)		//记录退出紧急呼叫的时间
								{
//									EmergencyCallDUREndTime = CTime::GetCurrentTime();
//									EmergencyCallDURSpanTime = EmergencyCallDUREndTime-EmergencyCallDURStartTime;
//									bEmergencyCallFlag = 0;
//									EmergencyCallRELStartTime = CTime::GetCurrentTime();
//									SetTimer(9,rand(),NULL);	//请求发送AC确认
//									ACFailedCounter = 0;
								}
								InquireCurrentVGC();
								//调用呼叫处理程序
								if(bNoListToMMIFlag)
									bNoListToMMIFlag = 0;
								else
									CallListToMMI(0);
								//CloseVoiceSwitch();
								mLogInstance->Log(tag+"ExitAllPToPCall out ok");
								return true;					//中途掉网，退出组呼成功
							}
						}
						else
						{
							mLogInstance->Log(tag+"AT+CAPTT=0 answer none");
							counter--;
							if(counter<=0)
							{
								InquireCurrentVGC();
								if(bNoListToMMIFlag)
									bNoListToMMIFlag = 0;
								else
									CallListToMMI(0);
								mLogInstance->Log(tag+"ExitAllPToPCall out bad");
								return	false;					//多次申请未成功，离开组呼失败
							}
						}
					}
				}

			}
		}
	}

	InquireCurrentVGC();
	//调用呼叫处理程序
	if(bNoListToMMIFlag!=0)
		bNoListToMMIFlag = 0;
	else
		CallListToMMI(0);
	//CloseVoiceSwitch();
	mLogInstance->Log(tag+"ExitAllPToPCall out ok");
	return true;										//没有正在进行的组呼，也算退出成功
}
bool MrmGprsHandle::InquireCurrentVGC1()//函数目的：使用AT+CALCC=1查询当前激活的组呼(不直接改变呼叫列表)
{
	mLogInstance->Log(tag+"InquireCurrentVGC1 in");
	int	i;
	bMrmOk = false;
	bMrmError = false;
	//bMrmResponse = false;
	bCurrentActiveVGCInquireFlag = true;
	mCurrentActiveVGCbuffer.strGcGID = ("");
	SendAT("AT+CALCC=1");
	{
		std::unique_lock <std::mutex> lck2(mtxMrmOk);
		if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(1000),[this]{ return (bMrmOk | bMrmError); }))
		{
			if(bMrmOk)
			{
				mLogInstance->Log(tag+"AT+CALCC=1 answer ok");
				bCurrentActiveVGCInquireFlag = false;
				mLogInstance->Log(tag+"InquireCurrentVGC1 out ok");
				return true;//退出
			}
			else if(bMrmError)
			{
				mLogInstance->Log(tag+"AT+CALCC=1 answer error");
				bCurrentActiveVGCInquireFlag = 0;
				mLogInstance->Log(tag+"InquireCurrentVGC1 out bad");
				return false;
			}
		}
		else
		{
			mLogInstance->Log(tag+"AT+CALCC=1 answer none");
			mLogInstance->Log(tag+"InquireCurrentVGC1 out bad");
			bCurrentActiveVGCInquireFlag = false;
			return false;
		}
	}
	bCurrentActiveVGCInquireFlag = false;
	mLogInstance->Log(tag+"InquireCurrentVGC1 out bad");
	return false;
}
bool MrmGprsHandle::InquireCurrentVGC2()//函数目的：//使用AT+CALCC=1查询当前所有的组呼(直接改变呼叫列表，在使用AT+CALCC=0发生+CME ERROR: 3时使用)
{
	mLogInstance->Log(tag+"InquireCurrentVGC2 in");
	int	i;
	bInquireCurrentVGCFlag = true;
	bMrmOk = false;
	bMrmError = false;
	//bMrmResponse = false;
	vVGCTempbuffer.clear();

	SendAT("AT+CALCC=1");
//	{
//		std::unique_lock <std::mutex> lck1(mtxMrmResponse);
//		if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(2000),[this]{ return bMrmResponse; }))
//		{
//			mLogInstance->Log(tag+"AT+CALCC=1 answer none");
//			std::cout<<GetCurrentTime()<<tag<<"InquireCurrentVGC2 out bad"<<std::endl;
//			bInquireCurrentVGCFlag = false;
//			return false;
//		}
//	}
	{
		std::unique_lock <std::mutex> lck2(mtxMrmOk);
		if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return (bMrmOk | bMrmError); }))
		{
			if(bMrmOk)
			{
				mLogInstance->Log(tag+"AT+CALCC=1 answer ok");
				vVGCbuffer.clear();
				for(i=0;i<vVGCTempbuffer.size();i++)
				{
					vVGCbuffer.push_back(vVGCTempbuffer[i]);
				}

				if(vVGCbuffer.size()>=1)
				{
					bMrmStatus = 0x41;
					CallListToMMI(0);
					//OpenVoiceSwitch(0x65);		//打开音频矩阵
				}
				if((vVGCbuffer.size()==0)&&(bMrmStatus == 0x41))
					bMrmStatus = 0x01;

				bInquireCurrentVGCFlag = false;
				mLogInstance->Log(tag+"InquireCurrentVGC2 out ok");
				return true;//继续退出
			}
			else if(bMrmError)
			{
				mLogInstance->Log(tag+"AT+CALCC=1 answer error");
				bInquireCurrentVGCFlag = false;
				mLogInstance->Log(tag+"InquireCurrentVGC2 out bad");
				return false;
			}
		}
		else
		{
			mLogInstance->Log(tag+"AT+CALCC=1 answer none");
			mLogInstance->Log(tag+"InquireCurrentVGC2 out bad");
			bInquireCurrentVGCFlag = false;
			return false;
		}
	}
	bInquireCurrentVGCFlag = false;
	mLogInstance->Log(tag+"InquireCurrentVGC2 out bad");
	return false;
}


bool MrmGprsHandle::UUS1InfoTx()
{
	mLogInstance->Log(tag+"UUS1InfoTx in");
	string stringtemp, stringchar;
	int counter, length, i;
	byte buffer[100];

	if(((mTrainState->strTrainNumber[0]!='X')||(mTrainState->strTrainNumber[1]!='X')
		||(mTrainState->strTrainNumber[2]!='X')||(mTrainState->strTrainNumber[3]!='X')
		||(mTrainState->strTrainNumber[4]!='X')||(mTrainState->strTrainNumber[5]!='X')
		||(mTrainState->strTrainNumber[6]!='X'))&&(bTrainNumberFNRegisterStatus==1))
	{
		//当前车次号存在，发送车次号功能号
		length = 0;
		length += MRMTxTrainNumberFNProcess(buffer);
		stringstream ss;
		for(i=0;i<length;i++)
			ss<<buffer[i];
		stringtemp = ss.str();

		counter = 3;
		for(i=0;i<counter;i++)
		{
			bMrmOk = false;
			bMrmError = false;
			//bMrmResponse = false;

			SendAT("AT+CUUS1=1,1,0,\"00"+stringtemp+"\"");					//退出当前个呼，但是会激活处于保持状态的呼叫
			{
//				{
//					std::unique_lock <std::mutex> lck1(mtxMrmResponse);
//					if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(3000),[this]{ return bMrmResponse; }))
//					{
//						mLogInstance->Log(tag+"AT+CUUS1=1,1,0 answer none");
//						continue;
//					}
//				}
				{
					std::unique_lock <std::mutex> lck2(mtxMrmOk);
					if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return (bMrmOk | bMrmError); }))
					{
						if(bMrmOk)
						{
							mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer ok");
							mLogInstance->Log(tag+"UUS1InfoTx out ok");
							return true;//
						}
						else if(bMrmError)
						{
							mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer error");
						}
					}
					else
					{
						mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer none");
					}
				}
			}
		}
	}
	else if(((mTrainState->strEngineNumber[0]!='X')||(mTrainState->strEngineNumber[1]!='X')
		||(mTrainState->strEngineNumber[2]!='X')||(mTrainState->strEngineNumber[3]!='X')
			||(mTrainState->strEngineNumber[4]!='X')||(mTrainState->strEngineNumber[5]!='X')
			||(mTrainState->strEngineNumber[6]!='X'))&&(bEngineNumberFNRegisterStatus==1))
	{
		length = 0;
		length += MRMTxEngineNumberFNProcess(buffer);
		stringstream ss;
		for(i=0;i<length;i++)
			ss<<buffer[i];
		stringtemp = ss.str();

		counter = 3;
		for(i=0;i<counter;i++)
		{
			bMrmOk = false;
			bMrmError = false;
			//bMrmResponse = false;
			SendAT("AT+CUUS1=1,1,0,\"00"+stringtemp+"\"");					//退出当前个呼，但是会激活处于保持状态的呼叫
			{
//				{
//					std::unique_lock <std::mutex> lck1(mtxMrmResponse);
//					if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(3000),[this]{ return bMrmResponse; }))
//					{
//						mLogInstance->Log(tag+"AT+CUUS1=1,1,0 answer none");
//						continue;
//					}
//				}
				{
					std::unique_lock <std::mutex> lck(mtxMrmOk);
					if(mwaitMrmOk.wait_for(lck, std::chrono::milliseconds(5000),[this]{ return (bMrmOk | bMrmError); }))
					{
						if(bMrmOk)
						{
							mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer ok");
							mLogInstance->Log(tag+"UUS1InfoTx out ok");
							return true;//
						}
						else if(bMrmError)
						{
							mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer error");
						}
					}
					else
					{
						mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer none");
					}
				}
			}
		}
	}
	mLogInstance->Log(tag+"UUS1InfoTx out bad");
	return false;
}

int MrmGprsHandle::MRMTxTrainNumberFNProcess(byte* point)
{
	mLogInstance->Log(tag+"MRMTxTrainNumberFNProcess in");
	unsigned int i,counter1,counter2;
	unsigned int temp1[20],temp2[100];

	string tempstring;
	unsigned int length;


	tempstring = TrainNumberToFunctionNumber(mTrainState->strTrainNumber,mTrainState->iRegisterStatus);
	length = tempstring.size();

	counter1 = 0;
	for(i=0;i<length;i++)
	{
		temp1[counter1++] = tempstring[i];
	}

	for(i=0;i<counter1;i++)
		temp1[i] -= 0x30;
	if(((counter1/2)*2)!=counter1)								//counter1 is odd?
		temp1[counter1++] = 0x0f;//奇数

	counter2 = 0;
	temp2[counter2++] = 0x05;
	temp2[counter2++] = counter1/2;								//length
	for(i=0;i<(counter1/2);i++)
	{
		temp2[counter2++] = (temp1[i*2]<<4)+(temp1[i*2+1]&0x0f);
	}

	for(i=2;i<counter2;i++)
		temp2[i] = ((temp2[i]>>4)&0x0f)+((temp2[i]<<4)&0xf0);

	for(i=0;i<counter2;i++)
	{
		if(((temp2[i]>>4)&0x0f)>=0x0a)
			(*point) = (((temp2[i]>>4)&0x0f)+0x37);
		else
			(*point) = (((temp2[i]>>4)&0x0f)+0x30);
		point++;
		if((temp2[i]&0x0f)>=0x0a)
			(*point) = ((temp2[i]&0x0f)+0x37);
		else
			(*point) = ((temp2[i]&0x0f)+0x30);
		point++;
	}
	mLogInstance->Log(tag+"MRMTxTrainNumberFNProcess out ok");
	return	counter2*2;
}
int MrmGprsHandle::MRMTxEngineNumberFNProcess(byte* point)
{
	mLogInstance->Log(tag+"MRMTxEngineNumberFNProcess in");
	unsigned int i,counter1,counter2;
	byte temp1[20],temp2[100];

	string stringtemp;
	unsigned int length;

	counter1 = 0;												//convert the engine number

	stringtemp = mTrainState->strEngineNumber;
	stringtemp = ("0863")+stringtemp+("01");

	length = stringtemp.size();
	for(i=0;i<length;i++)
		temp1[counter1++] = stringtemp[i];

	for(i=0;i<counter1;i++)
		temp1[i] -= 0x30;
	if(((counter1/2)*2)!=counter1)								//counter1 is odd?
		temp1[counter1++] = 0x0f;

	counter2 = 0;
	temp2[counter2++] = 0x05;
	temp2[counter2++] = counter1/2;								//length
	for(i=0;i<(counter1/2);i++)
	{
		temp2[counter2++] = (temp1[i*2]<<4)+(temp1[i*2+1]&0x0f);
	}


	for(i=2;i<counter2;i++)
		temp2[i] = ((temp2[i]>>4)&0x0f)+((temp2[i]<<4)&0xf0);

	for(i=0;i<counter2;i++)
	{
		if(((temp2[i]>>4)&0x0f)>=0x0a)
			(*point) = (((temp2[i]>>4)&0x0f)+0x37);
		else
			(*point) = (((temp2[i]>>4)&0x0f)+0x30);
		point++;
		if((temp2[i]&0x0f)>=0x0a)
			(*point) = ((temp2[i]&0x0f)+0x37);
		else
			(*point) = ((temp2[i]&0x0f)+0x30);
		point++;
	}
	mLogInstance->Log(tag+"MRMTxEngineNumberFNProcess out ok");
	return	counter2*2;
}

string MrmGprsHandle::TrainNumberToFunctionNumber(string trainnumber,byte runningnumberstatus)
{
	mLogInstance->Log(tag+"TrainNumberToFunctionNumber in");
	string tempstring;
	int tempchar,tempchar1;
	int length;

	tempchar = trainnumber[0];
	tempchar1 = trainnumber[1];

	if(((tempchar>='A')&&(tempchar<='Z'))||((tempchar>='a')&&(tempchar<='z'))||(tempchar=='0'))	//第1位是A~Z或a~z或0
	{
		if(((tempchar1>='A')&&(tempchar1<='Z'))||((tempchar1>='a')&&(tempchar1<='z')))			//第2位是A~Z或a~z
		{
			length = trainnumber.size();
			stringstream ss;
			ss<<tempchar<<tempchar1;
			tempstring = ss.str();
			trainnumber = tempstring+trainnumber.substr(2,length-2);
			switch(runningnumberstatus)
			{
			case	0x00:
				trainnumber =("0862")+trainnumber+("01");
				break;
			case	0x01:
				trainnumber = ("0862")+trainnumber+("02");
				break;
			case	0x02:
				trainnumber = ("0862")+trainnumber+("03");
				break;
			case	0x03:
				trainnumber = ("0862")+trainnumber+("04");
				break;
			case	0x04:
				trainnumber = ("0862")+trainnumber+("05");
				break;
			default:
				trainnumber = ("0862")+trainnumber+("01");
				break;
			}
		}
		else if((tempchar1=='0')&&(tempchar=='0'))		//第1位是和第2位都是0
		{
			length = trainnumber.size();
			stringstream ss;
			ss<<tempchar<<tempchar1;
			tempstring = ss.str();
			trainnumber = tempstring+trainnumber.substr(2,length-2);
			switch(runningnumberstatus)
			{
			case	0x00:
				trainnumber = ("0862")+trainnumber+("01");
				break;
			case	0x01:
				trainnumber = ("0862")+trainnumber+("02");
				break;
			case	0x02:
				trainnumber = ("0862")+trainnumber+("03");
				break;
			case	0x03:
				trainnumber = ("0862")+trainnumber+("04");
				break;
			case	0x04:
				trainnumber = ("0862")+trainnumber+("05");
				break;
			default:
				trainnumber = ("0862")+trainnumber+("01");
				break;
			}
		}
		else											//第2位是其他数字(1~9)或者 第1位为字母时第2位为0
		{
			length = trainnumber.size();
			stringstream ss;
			ss<<tempchar;
			tempstring = ss.str();
			trainnumber = tempstring+trainnumber.substr(1,length-1);
			switch(runningnumberstatus)
			{
			case	0x00:
				trainnumber = ("086200")+trainnumber+("01");
				break;
			case	0x01:
				trainnumber = ("086200")+trainnumber+("02");
				break;
			case	0x02:
				trainnumber = ("086200")+trainnumber+("03");
				break;
			case	0x03:
				trainnumber = ("086200")+trainnumber+("04");
				break;
			case	0x04:
				trainnumber = ("086200")+trainnumber+("05");
				break;
			default:
				trainnumber = ("0862")+trainnumber+("01");
				break;
			}
		}
	}
	else								//第1位为其他数字(1~9)
	{
		switch(runningnumberstatus)
		{
		case	0x00:
			trainnumber = ("08620000")+trainnumber+("01");
			break;
		case	0x01:
			trainnumber = ("08620000")+trainnumber+("02");
			break;
		case	0x02:
			trainnumber = ("08620000")+trainnumber+("03");
			break;
		case	0x03:
			trainnumber = ("08620000")+trainnumber+("04");
			break;
		case	0x04:
			trainnumber = ("08620000")+trainnumber+("05");
			break;
		default:
			trainnumber = ("0862")+trainnumber+("01");
			break;
		}
	}
	mLogInstance->Log(tag+"TrainNumberToFunctionNumber out ok");
	return trainnumber;
}

bool MrmGprsHandle::OriginateCall(string number, string CallType, string Priority)//发起呼叫//输入参数：number为号码，CallType为呼叫类型，Priority为优先级
{
	mLogInstance->Log(tag+"OriginateCall in");
	mLogInstance->Log(tag+"number="+number+" CallType="+CallType+" Priority="+Priority);
	string stringtemp,stringchar;
	int counter,length,i;
	byte buffer[200];
	byte buffertx[1000];
	int	txdatalength;

	if(bMrmStatus==0x01)
		bMrmStatus = 0x21;		//呼叫发起中

	//刷新当前的组呼列表
	for(i=0;i<vVGCbuffer.size();i++)
	{
		if(vVGCbuffer[i].strGcStat.compare("0") == 0)
			break;
	}
	if(i>=vVGCbuffer.size())
	{
		if(InquireCurrentVGC()==0)				//没有激活的组呼,可以进行组呼的查询
		{
			if(bMrmStatus==0x21)
				bMrmStatus = 0x01;
			mLogInstance->Log(tag+"OriginateCall out bad");
			return false;
		}
	}

	if(CallType.compare("1") == 0)	//个呼：发送UUS1信息
	{
		if(((mTrainState->strTrainNumber[0]!='X')||(mTrainState->strTrainNumber[1]!='X')
				||(mTrainState->strTrainNumber[2]!='X')||(mTrainState->strTrainNumber[3]!='X')
				||(mTrainState->strTrainNumber[4]!='X')||(mTrainState->strTrainNumber[5]!='X')
				||(mTrainState->strTrainNumber[6]!='X'))&&(bTrainNumberFNRegisterStatus==1))
		{
			//当前车次号存在，发送车次号功能号
			length = 0;
			length += MRMTxTrainNumberFNProcess(buffer);
			stringstream ss;
			for(i=0;i<length;i++)
				ss<<buffer[i];
			stringtemp = ss.str();
			counter = 3;
			for(i=0;i<counter;i++)
			{
				bMrmOk = false;
				bMrmError = false;
				//bMrmResponse = false;
				SendAT("AT+CUUS1=1,1,0,\"00"+stringtemp+"\"");					//发送UUS1命令
				{
//					std::unique_lock <std::mutex> lck1(mtxMrmResponse);
//					if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(3000),[this]{ return bMrmResponse; }))
//					{
//						mLogInstance->Log(tag+"AT+CUUS1=1,1,0 answer none");
//					}
//					else
//					{
						std::unique_lock <std::mutex> lck2(mtxMrmOk);
						if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return (bMrmOk | bMrmError); }))
						{
							if(bMrmOk)
							{
								mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer ok");
								break;
							}
							else if(bMrmError)
							{
								mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer error");
							}
						}
						else
						{
							mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer none");
						}
//					}
				}
			}
			if(i>=counter)
			{
				//通知MMI处于正在发起呼叫中状态(呼叫确认)
				_SendData  pSendData;
				mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,8);//呼叫失败//呼叫失败
				mDataProcesser.SendInfoForCan(pSendData);
				if(bMrmStatus==0x21)
					bMrmStatus = 0x01;		//恢复到空闲状态
				mLogInstance->Log(tag+"OriginateCall out bad");
				return false;
			}
		}
		else if(((mTrainState->strEngineNumber[0]!='X')||(mTrainState->strEngineNumber[1]!='X')
			||(mTrainState->strEngineNumber[2]!='X')||(mTrainState->strEngineNumber[3]!='X')
				||(mTrainState->strEngineNumber[4]!='X')||(mTrainState->strEngineNumber[5]!='X')
				||(mTrainState->strEngineNumber[6]!='X'))&&(bEngineNumberFNRegisterStatus==1))
		{
			length = 0;
			length += MRMTxEngineNumberFNProcess(buffer);
			stringstream ss;
			for(i=0;i<length;i++)
				ss<<buffer[i];
			stringtemp = ss.str();
			counter = 3;
			for(i=0;i<counter;i++)
			{
				bMrmOk = false;
				bMrmError = false;
				//bMrmResponse = false;
				SendAT("AT+CUUS1=1,1,0,\"00"+stringtemp+"\"");					//发送UUS1命令
				{
//					std::unique_lock <std::mutex> lck1(mtxMrmResponse);
//					if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(3000),[this]{ return bMrmResponse; }))
//					{
//						mLogInstance->Log(tag+"AT+CUUS1=1,1,0 answer none");
//					}
//					else
//					{
						std::unique_lock <std::mutex> lck2(mtxMrmOk);
						if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return (bMrmOk | bMrmError); }))
						{
							if(bMrmOk)
							{
								mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer ok");
								break;
							}
							else if(bMrmError)
							{
								mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer error");
							}
						}
						else
						{
							mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer none");
						}
//					}
				}
			}
			if(i>=counter)
			{
				//通知MMI处于正在发起呼叫中状态(呼叫确认)
				_SendData  pSendData;
				mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,8);//呼叫失败
				mDataProcesser.SendInfoForCan(pSendData);
				if(bMrmStatus==0x21)
					bMrmStatus = 0x01;		//恢复到空闲状态
				mLogInstance->Log(tag+"OriginateCall out bad");
				return false;
			}
		}
		else													//当前未注册功能号，UUS1信息为空
		{
			counter = 3;
			for(i=0;i<counter;i++)
			{
				bMrmOk = false;
				bMrmError = false;
				//bMrmResponse = false;
				SendAT("AT+CUUS1=1,1,0,\"000500\"");					//退出当前个呼，但是会激活处于保持状态的呼叫
				{
//					std::unique_lock <std::mutex> lck1(mtxMrmResponse);
//					if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(3000),[this]{ return bMrmResponse; }))
//					{
//						mLogInstance->Log(tag+"AT+CUUS1=1,1,0 answer none");
//					}
//					else
//					{
						std::unique_lock <std::mutex> lck2(mtxMrmOk);
						if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return (bMrmOk | bMrmError); }))
						{
							if(bMrmOk)
							{
								mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer ok");
								break;
							}
							else if(bMrmError)
							{
								mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer error");
							}
						}
						else
						{
							mLogInstance->Log(tag+"AT+CUUS1=1,1,0, answer none");
						}
//					}
				}
			}
			if(i>=counter)
			{
				//通知MMI处于正在发起呼叫中状态(呼叫确认)
				_SendData  pSendData;
				mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,8);//呼叫失败
				mDataProcesser.SendInfoForCan(pSendData);
				if(bMrmStatus==0x21)
					bMrmStatus = 0x01;		//恢复到空闲状态
				mLogInstance->Log(tag+"OriginateCall out bad");
				return false;
			}
		}
	}

	//发起个别呼叫
	if(CallType.compare("1")==0)
	{
		bMrmOk =false;
		//bMrmResponse = false;
		bMrmError = false;
		bMrmNoCarrier = false;
		bMrmBusy = false;
		if((mTrainState->arrInitStatus[0] == 0x01)||(mTrainState->arrInitStatus[0] == 0x03))	//公网
			SendAT(("ATD")+number+";");	//发送拨号命令
		else
			SendAT("ATD*75"+Priority+"#"+number+";");	//发送拨号命令
		{
//			std::unique_lock <std::mutex> lck1(mtxMrmResponse);
//			if(mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(3000),[this]{ return bMrmResponse; }))
//			{
				std::unique_lock <std::mutex> lck2(mtxMrmOk);
				if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return (bMrmOk | bMrmError | bMrmNoCarrier | bMrmBusy); }))
				{
					if(bMrmOk)
					{
						mLogInstance->Log(tag+"ATD*75 answer ok");
						bMrmStatus = 0x31;  //个呼通话中
						_PToPCallStruct newPToPCallStruct;
						newPToPCallStruct.strPtpcsNumber = number;
						newPToPCallStruct.strPtpcsDir = "0";
						newPToPCallStruct.strPtpcsStat = "0";
						newPToPCallStruct.strPtpcsMode = "0";
						newPToPCallStruct.strPtpcsPriority = Priority;
						newPToPCallStruct.strPtpcsUUS1 = "";
						//newPToPCallStruct.strPtpcsType = "1";
						vPToPbuffer.push_back(newPToPCallStruct);

						mLogInstance->Log(tag+"1.Number=" + vPToPbuffer[0].strPtpcsNumber + " Priority=" + vPToPbuffer[0].strPtpcsPriority+" Stat=" + vPToPbuffer[0].strPtpcsStat );

						_SendData  pSendData;
						mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0xC0);//呼叫成功
						mDataProcesser.SendInfoForCan(pSendData);

						CallListToMMI(0);
						mLogInstance->Log(tag+"OriginateCall out ok");
						return true;
					}
					else if(bMrmNoCarrier)
					{
						mLogInstance->Log(tag+"ATD*75 answer nocarrier");
						_SendData  pSendData;
						mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x02);//呼叫失败//呼叫失败
						mDataProcesser.SendInfoForCan(pSendData);
						if(bMrmStatus==0x21)
							bMrmStatus = 0x01;		//恢复到空闲状态
						mLogInstance->Log(tag+"OriginateCall out bad");
						return false;
					}
					else if(bMrmBusy)
					{
						mLogInstance->Log(tag+"ATD*75 answer busy");
						_SendData  pSendData;
						mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x04);//呼叫失败
						mDataProcesser.SendInfoForCan(pSendData);
						if(bMrmStatus==0x21)
							bMrmStatus = 0x01;		//恢复到空闲状态
						mLogInstance->Log(tag+"OriginateCall out bad");
						return false;
					}
					else if(bMrmError)
					{
						mLogInstance->Log(tag+"ATD*75 answer error");
						_SendData  pSendData;
						mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x08);//呼叫失败
						mDataProcesser.SendInfoForCan(pSendData);
						if(bMrmStatus==0x21)
							bMrmStatus = 0x01;		//恢复到空闲状态
						mLogInstance->Log(tag+"OriginateCall out bad");
						return false;
					}
				}
				else
				{
					mLogInstance->Log(tag+"ATD*75 answer none");
				}
//			}
		}
	}

	//发起组呼呼叫
	if(CallType.compare("17") ==0)
	{
		bMrmOk =false;
		//bMrmResponse = false;
		bMrmError = false;
		bMrmNoCarrier = false;
		bMrmError156 = false;
		SendAT("ATD*17*75"+Priority+"#"+number+";");	//发送拨号命令
		{
//			std::unique_lock <std::mutex> lck1(mtxMrmResponse);
//			if(mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(3000),[this]{ return bMrmResponse; }))
//			{
				std::unique_lock <std::mutex> lck2(mtxMrmOk);
				if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return (bMrmOk | bMrmError | bMrmNoCarrier ); }))
				{
					if(bMrmOk)
					{
						mLogInstance->Log(tag+"ATD*75 answer ok");
						bMrmStatus = 0x41; //组呼通话中
						SetTimer(7);		//10秒后自动释放组呼PTT
						_GroupCallStruct newVGCbuffer;
						if(InquireCurrentVGC1())
						{
							newVGCbuffer.strGcGID = mCurrentActiveVGCbuffer.strGcGID;
							newVGCbuffer.strGcGCA = mCurrentActiveVGCbuffer.strGcGCA;
							newVGCbuffer.strGcIs17Or18 = mCurrentActiveVGCbuffer.strGcIs17Or18;
							newVGCbuffer.strGcStat = mCurrentActiveVGCbuffer.strGcStat;
							newVGCbuffer.strGcDir = mCurrentActiveVGCbuffer.strGcDir;
							newVGCbuffer.strGcAckFlag = mCurrentActiveVGCbuffer.strGcAckFlag;
							newVGCbuffer.strGcPriority = mCurrentActiveVGCbuffer.strGcPriority;
						}
						else
						{
							newVGCbuffer.strGcGID = number;
							newVGCbuffer.strGcGCA = "00000";
							newVGCbuffer.strGcIs17Or18 = "17";
							newVGCbuffer.strGcStat = "0";
							newVGCbuffer.strGcDir = "0";
							newVGCbuffer.strGcAckFlag = "";
							newVGCbuffer.strGcPriority = Priority;
						}
						vVGCbuffer.push_back(newVGCbuffer);

						_SendData  pSendData;
						mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0xC0);//呼叫成功
						mDataProcesser.SendInfoForCan(pSendData);
						CallListToMMI(0);
						//OpenVoiceSwitch(0x65);		//打开音频矩阵

						if(number.compare("299")==0)
						{
		//					EmergencyCallDURStartTime = CTime::GetCurrentTime();
		//					bEmergencyCallFlag = 1;
		//					EmergencyCallMakerFlag = 1;	//主呼
		//					EmergencyCallGCA = VGCbuffer[counterCALCC-1].GCA;
						}
						else
							bEmergencyCallFlag = 0;
						mLogInstance->Log(tag+"OriginateCall out ok");
						return true;
					}
					else if(bMrmNoCarrier)
					{
						mLogInstance->Log(tag+"ATD*75 answer nocarrier");
						_SendData  pSendData;
						mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x02);//呼叫失败//呼叫失败
						mDataProcesser.SendInfoForCan(pSendData);
						if(bMrmStatus==0x21)
							bMrmStatus = 0x01;		//恢复到空闲状态
						mLogInstance->Log(tag+"OriginateCall out bad");
						return false;
					}
					else if(bMrmError)
					{
						mLogInstance->Log(tag+"ATD*75 answer error");
						if(bMrmError156)
						{
							for(i=0;i<vVGCbuffer.size();i++)
							{
								if(vVGCbuffer[i].strGcGID.compare(number) == 0)
								{
									if(JoinVGCS("17",vVGCbuffer[i].strGcGID,vVGCbuffer[i].strGcGCA,vVGCbuffer[i].strGcPriority))
									{
										//通知MMI处于正在发起呼叫中状态(呼叫确认)
										_SendData  pSendData;
										mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0xe0);//呼叫成功
										mDataProcesser.SendInfoForCan(pSendData);
										//调用呼叫处理程序
										CallListToMMI(0);
										//OpenVoiceSwitch(0x65);		//打开音频矩阵
										bMrmStatus = 0x41;			//组呼通话中
										mLogInstance->Log(tag+"OriginateCall out ok");
										return true;
									}
									break;
								}
							}
						}
						else
						{
							_SendData  pSendData;
							mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x08);//呼叫失败
							mDataProcesser.SendInfoForCan(pSendData);
							if(bMrmStatus==0x21)
								bMrmStatus = 0x01;		//恢复到空闲状态
							mLogInstance->Log(tag+"OriginateCall out bad");
							return false;
						}
					}
				}
				else
				{
					mLogInstance->Log(tag+"ATD*75 answer none");
				}
//			  }
		}
	}

	//通知MMI处于正在发起呼叫中状态(呼叫确认)
	_SendData  pSendData;
	mFrameGenerate.GetFrame_CallResponse(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x08);//呼叫失败
	mDataProcesser.SendInfoForCan(pSendData);
	if(bMrmStatus==0x21)
		bMrmStatus = 0x01;		//恢复到空闲状态
	mLogInstance->Log(tag+"OriginateCall out bad");
	return false;
}
//函数目的：个呼被叫时,加入该个呼通话
bool MrmGprsHandle::JoinPToPCall()
{
	mLogInstance->Log(tag+"JoinPToPCall in");
	string stringtemp;
	int i,counter;
	counter = 2;
	for(i=0;i<counter;i++)
	{
		bMrmOk = false;
		//bMrmResponse = false;
		SendAT(("ATA"));					//
		{
			//std::unique_lock <std::mutex> lck1(mtxMrmResponse);
			//if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(1000),[this]{ return bMrmResponse; }))
			//{
			//	mLogInstance->Log(tag+"ATA answer none");
			//}
			//else
			{
				std::unique_lock <std::mutex> lck2(mtxMrmOk);
				if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(1000),[this]{ return (bMrmOk ); }))
				{
						mLogInstance->Log(tag+"ATA answer ok");
						break;
				}
				else
				{
					mLogInstance->Log(tag+"ATA answer none");
				}
			}
		}
	}
	if(i >= counter)
	{
		InquirePToPCall();
		InquireCurrentVGC();
		//调用呼叫处理程序
		CallListToMMI(0);
		mLogInstance->Log(tag+"JoinPToPCall out bad");
		return false;		//接受通话失败
	}
	else
	{
		//调用呼叫处理程序
		for(i=0;i<vPToPbuffer.size();i++)
		{
			if(vPToPbuffer[i].strPtpcsNumber.compare(mMMISelectCallNumber.strMmiSlcNumber) == 0)
				break;
		}
		if(i<vPToPbuffer.size())
		{
			vPToPbuffer[i].strPtpcsStat = "0";
		}
		//OpenVoiceSwitch(0x65);			//打开音频矩阵
		CallListToMMI(0);
		bMrmStatus = 0x31;				//个呼通话中
		mLogInstance->Log(tag+"JoinPToPCall out ok");
		return true;
	}
}

bool MrmGprsHandle::JoinVGCS(string string17Or18,string stringGID,string stringGCA,string stringpriority)
{
	mLogInstance->Log(tag+"JoinVGCS in");
	int	i;
	bMrmOk =false;
	//bMrmResponse = false;
	bMrmError = false;
	bMrmNoCarrier = false;
	bMrmError156 = false;
	SendAT("AT+CAJOIN="+string17Or18+","+stringGID+","+stringGCA);	//发送组呼加入指令
	{
		//std::unique_lock <std::mutex> lck1(mtxMrmResponse);
		//if(!mwaitMrmResponse.wait_for(lck1, std::chrono::milliseconds(1000),[this]{ return bMrmResponse; }))
		//{
		//	mLogInstance->Log(tag+"AT+CAJOIN answer none");
		//}
		//else
		{
			std::unique_lock <std::mutex> lck2(mtxMrmOk);
			if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(1000),[this]{ return (bMrmOk | bMrmNoCarrier | bMrmError); }))
			{
				if(bMrmOk)
				{
					mLogInstance->Log(tag+"AT+CAJOIN= answer ok");
					for(i=0;i<vVGCbuffer.size();i++)
					{
						if(vVGCbuffer[i].strGcGID.compare(stringGID) == 0)
							break;
					}
					if(i<vVGCbuffer.size())
					{
						vVGCbuffer[i].strGcGID = stringGID;
						vVGCbuffer[i].strGcGCA = stringGCA;
						vVGCbuffer[i].strGcIs17Or18 = string17Or18;
						vVGCbuffer[i].strGcStat = "0";
						vVGCbuffer[i].strGcPriority = stringpriority;
					}
					else
					{
						_GroupCallStruct newVGCbuffer;
						newVGCbuffer.strGcGID = stringGID;
						newVGCbuffer.strGcGCA = stringGCA;
						newVGCbuffer.strGcIs17Or18 = string17Or18;
						newVGCbuffer.strGcStat = "0";
						newVGCbuffer.strGcPriority = stringpriority;
						vVGCbuffer.push_back(newVGCbuffer);
					}
					//调用呼叫处理程序
					CallListToMMI(0);
					//OpenVoiceSwitch(0x65);		//打开音频矩阵
					bMrmStatus = 0x41;			//组呼通话中

					if(stringGID.compare("299")==0)
					{
		//						bEmergencyCallFlag = 1;
		//						EmergencyCallDURStartTime = CTime::GetCurrentTime();
		//						EmergencyCallMakerFlag = 0;			//被呼
		//						EmergencyCallGCA = stringGCA;
					}
					else
						bEmergencyCallFlag = 0;

					return true;
				}
				else if(bMrmNoCarrier || bMrmError)
				{
					mLogInstance->Log(tag+"AT+CAJOIN= answer error");
					InquireCurrentVGC();
					CallListToMMI(0);
					mLogInstance->Log(tag+"JoinVGCS out bad");
					return false;
				}
			}
			else
			{
				mLogInstance->Log(tag+"AT+CAJOIN= answer none");
			}
		}
	}
	InquireCurrentVGC();
	CallListToMMI(0);
	mLogInstance->Log(tag+"JoinVGCS out bad");
	return false;
}

void  MrmGprsHandle::CallHangUpProcess()
{
	int	i;
	//当前激活的是个呼
	for(i=0;i<vPToPbuffer.size();i++)
	{
		if(vPToPbuffer[i].strPtpcsStat.compare("0") == 0)
		{
			break;
		}
	}

	if(i<vPToPbuffer.size())
	{
		ExitCurrentPToPCall();
		return;
	}

//当前激活的是组呼?
	for(i=0;i<vVGCbuffer.size();i++)
	{
		if(vVGCbuffer[i].strGcStat.compare("0") == 0)
		{
			break;
		}
	}

	if(i<vVGCbuffer.size())
	{
		ExitGroupCall();
		return;
	}
}


//函数目的：挂掉个呼
//输入参数：无
//输出参数：无
//返回值: 成功为1，失败为0
//挂掉当前正在进行通话的个呼
//如果存在其他处于挂起或等待状态的个呼,加入到其他个呼
bool MrmGprsHandle::ExitCurrentPToPCall()
{
	string stringtemp;
	mLogInstance->Log(tag+"ExitCurrentPToPCall in");
	if(vPToPbuffer.size()==1)								//当前只有一个个呼
	{
		bMrmOk = false;
		bMrmError = false;
		//bMrmResponse = false;
		SendAT("ATH");					//退出当前个呼，但是会激活处于保持状态的呼叫
		{
			{
				std::unique_lock <std::mutex> lck2(mtxMrmOk);
				if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(3000),[this]{ return (bMrmOk | bMrmError); }))
				{
					if(bMrmOk)
					{
						mLogInstance->Log(tag+"ATH answer ok");
						InquirePToPCall();
						//调用呼叫处理程序
						CallListToMMI(0);
						mLogInstance->Log(tag+"ExitCurrentPToPCall out ok");
						return true;
					}
					else if(bMrmError)
					{
						mLogInstance->Log(tag+"ATH answer error");
						mLogInstance->Log(tag+"ExitCurrentPToPCall out bad");
						return false;
					}
				}
				else
				{
					mLogInstance->Log(tag+"ATH answer none");
					mLogInstance->Log(tag+"ExitCurrentPToPCall out bad");
					return false;
				}
			}
		}
	}

	if(vPToPbuffer.size()>1)								//当前有两个或两个以上的个呼同时存在
	{
		bMrmOk = false;
		bMrmError = false;
		//bMrmResponse = false;
		SendAT("AT+CHLD=1");					//退出当前个呼，激活等待的个呼
		{
			{
				std::unique_lock <std::mutex> lck2(mtxMrmOk);
				if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(3000),[this]{ return (bMrmOk | bMrmError); }))
				{
					if(bMrmOk)
					{
						mLogInstance->Log(tag+"AT+CHLD=1 answer ok");
						InquirePToPCall();					//当前存在激活的个呼，也进行了个呼的查询，可能需要调整!!
						CallListToMMI(0);
						mLogInstance->Log(tag+"ExitCurrentPToPCall out ok");
						return true;
					}
					else if(bMrmError)
					{
						mLogInstance->Log(tag+"AT+CHLD=1 answer error");
						mLogInstance->Log(tag+"ExitCurrentPToPCall out bad");
						return false;
					}
				}
				else
				{
					mLogInstance->Log(tag+"AT+CHLD=1 answer none");
					mLogInstance->Log(tag+"ExitCurrentPToPCall out bad");
					return false;
				}
			}
		}
	}
	mLogInstance->Log(tag+"ExitCurrentPToPCall out bad");
	return false;
}

bool MrmGprsHandle::GroupCallUplinkApply()
{
	mLogInstance->Log(tag+"GroupCallUplinkApply in");
	bMrmOk = false;
	bMrmError = false;
	//bMrmResponse = false;
	SendAT("AT+CAPTT=2");					//发送抢占信道指令
	{
		{
			std::unique_lock <std::mutex> lck2(mtxMrmOk);
			if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(3000),[this]{ return (bMrmOk | bMrmError); }))
			{
				if(bMrmOk)
				{
					mLogInstance->Log(tag+"AT+CAPTT=2 answer ok");

					_SendData  pSendData;
					mFrameGenerate.GetFrame_PttState(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x02);//呼叫失败
					mDataProcesser.SendInfoForCan(pSendData);

					mLogInstance->Log(tag+"GroupCallUplinkApply out ok");
					return true;
				}
				else if(bMrmError)
				{
					mLogInstance->Log(tag+"AT+CAPTT=2 answer error");

					_SendData  pSendData;
					mFrameGenerate.GetFrame_PttState(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x03);//呼叫失败
					mDataProcesser.SendInfoForCan(pSendData);

					mLogInstance->Log(tag+"GroupCallUplinkApply out bad");
					return false;
				}
			}
			else
			{
				mLogInstance->Log(tag+"AT+CAPTT=2 answer none");

				_SendData  pSendData;
				mFrameGenerate.GetFrame_PttState(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x03);//呼叫失败
				mDataProcesser.SendInfoForCan(pSendData);

				mLogInstance->Log(tag+"GroupCallUplinkApply out bad");
				return false;
			}
		}
	}
}
bool MrmGprsHandle::GroupCallUplinkRelease()
{
	mLogInstance->Log(tag+"GroupCallUplinkRelease in");
	bMrmOk = false;
	bMrmError = false;
	//bMrmResponse = false;
	SendAT("AT+CAPTT=0");					//发送释放信道指令
	{
		{
			std::unique_lock <std::mutex> lck2(mtxMrmOk);
			if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(3000),[this]{ return (bMrmOk | bMrmError); }))
			{
				if(bMrmOk)
				{
					mLogInstance->Log(tag+"AT+CAPTT=0 answer ok");

					_SendData  pSendData;
					mFrameGenerate.GetFrame_PttState(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x00);//上行链路空闲
					mDataProcesser.SendInfoForCan(pSendData);

					mLogInstance->Log(tag+"GroupCallUplinkRelease out ok");
					return true;
				}
				else if(bMrmError)
				{
					mLogInstance->Log(tag+"AT+CAPTT=0 answer error");

					_SendData  pSendData;
					mFrameGenerate.GetFrame_PttState(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x00);//上行链路空闲
					mDataProcesser.SendInfoForCan(pSendData);

					mLogInstance->Log(tag+"GroupCallUplinkRelease out bad");
					return false;
				}
			}
			else
			{
				mLogInstance->Log(tag+"AT+CAPTT=0 answer none");

				_SendData  pSendData;
				mFrameGenerate.GetFrame_PttState(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,SHORTFRAME,0x00);//上行链路空闲
				mDataProcesser.SendInfoForCan(pSendData);

				mLogInstance->Log(tag+"GroupCallUplinkRelease out bad");
				return false;
			}
		}
	}
}



void MrmGprsHandle::CallSelectProcess()
{
	int	i=0;
	mLogInstance->Log(tag+"CallSelectProcess in");

	if(mMMISelectCallNumber.strMmiSlcType.compare("1") ==0 )					//需要选择的是个呼
	{
		for(i=0;i<vPToPbuffer.size();i++)
		{
			if(vPToPbuffer[i].strPtpcsStat.compare("0")==0)
				break;
		}
		if(i<vPToPbuffer.size())								//当前存在激活的个呼，进行呼叫切换
		{
			if(vPToPbuffer[i].strPtpcsNumber.compare(mMMISelectCallNumber.strMmiSlcNumber)!=0)
				HeldCurrentPToPCall();
			else
				CallListToMMI(0);
			mLogInstance->Log(tag+"CallSelectProcess out1");
			return;
		}


		for(i=0;i<vVGCbuffer.size();i++)
		{
			if(vVGCbuffer[i].strGcStat.compare("0")==0)
				break;
		}
		if(i<vVGCbuffer.size())								//当前存在激活的组呼，需要退出此组呼
		{
			if(ExitGroupCall())
				JoinPToPCall();
			mLogInstance->Log(tag+"CallSelectProcess out2");
			return;
		}

		//当前既不存在激活的个呼，也不存在激活的组呼
		for(i=0;i<vPToPbuffer.size();i++)
		{
			if(vPToPbuffer[i].strPtpcsStat.compare("4") == 0)					//incoming call
				break;
		}
		if(i<vPToPbuffer.size())								//当前存在未接通的个呼被叫
		{
			JoinPToPCall();
		}
		else
		{
			HeldCurrentPToPCall();						//当前虽然没有激活的个呼,但有个呼被保持了,重新激活
		}
	}
	else if(mMMISelectCallNumber.strMmiSlcType.compare("17")==0)				//需要选择的是组呼
	{
		for(i=0;i<vPToPbuffer.size();i++)
		{
			if((vPToPbuffer[i].strPtpcsStat.compare("0") ==0 )||(vPToPbuffer[i].strPtpcsStat.compare("4")==0))
				break;
		}
		if(i<vPToPbuffer.size())								//当前存在激活或等待状态的的个呼,需要退出个呼
		{
			if(ExitAllPToPCall())
			{
				if(!JoinVGCS(mMMISelectCallNumber.strMmiSlcIs17Or18,mMMISelectCallNumber.strMmiSlcGID,mMMISelectCallNumber.strMmiSlcGCA, mMMISelectCallNumber.strMmiSlcPriority))
					CallListToMMI(1);
				mLogInstance->Log(tag+"CallSelectProcess out3");
				return;
			}
		}

		for(i=0;i<vVGCbuffer.size();i++)
		{
			if(vVGCbuffer[i].strGcStat.compare("0")==0)
				break;
		}
		if(i<vVGCbuffer.size())								//当前存在激活的组呼，需要退出此组呼
		{
			if(vVGCbuffer[i].strGcGID.compare(mMMISelectCallNumber.strMmiSlcGID)!=0)
			{
				bWaitForJoinVGCS = 1;
				if(ExitGroupCall())
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(2000));//退出组呼后等待2秒再加入其他组呼,否则MRM容易出错
					if(!JoinVGCS(mMMISelectCallNumber.strMmiSlcIs17Or18,mMMISelectCallNumber.strMmiSlcGID,mMMISelectCallNumber.strMmiSlcGCA,mMMISelectCallNumber.strMmiSlcPriority))
						CallListToMMI(1);
				}
				bWaitForJoinVGCS = 0;
			}
			else
				CallListToMMI(0);
			mLogInstance->Log(tag+"CallSelectProcess out4");
			return;
		}

		//当前既不存在激活的个呼，也不存在激活的组呼
		for(i=0;i<vVGCbuffer.size();i++)
		{
			if(mMMISelectCallNumber.strMmiSlcGID.compare(vVGCbuffer[i].strGcGID) == 0)
			{
				if(!JoinVGCS(mMMISelectCallNumber.strMmiSlcIs17Or18,mMMISelectCallNumber.strMmiSlcGID,mMMISelectCallNumber.strMmiSlcGCA,mMMISelectCallNumber.strMmiSlcPriority))
					CallListToMMI(1);
			}
		}
	}
	mLogInstance->Log(tag+"CallSelectProcess out");
}

bool MrmGprsHandle::HeldCurrentPToPCall()
{
	mLogInstance->Log(tag+"HeldCurrentPToPCall in");
	int	i,j;

	if(vPToPbuffer.size()>=1)//当前有两个或两个以上的个呼同时存在
	{
		bMrmOk =false;
		bMrmError = false;
		SendAT("AT+CHLD=2");//挂起当前个呼,激活其他处于挂起或等待状态的个呼
		{
			{//ok
				std::unique_lock <std::mutex> lck2(mtxMrmOk);
				if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(3000),[this]{ return (bMrmOk | bMrmError); }))
				{
					if(bMrmOk)
					{
						mLogInstance->Log(tag+"AT+CHLD=2 answer ok");
						InquirePToPCall();
						CallListToMMI(0);
						mLogInstance->Log(tag+"HeldCurrentPToPCall out ok");
						return true;
					}
					else if(bMrmError)
					{
						mLogInstance->Log(tag+"AT+CHLD=2 answer error");
					}
				}
				else
				{
					mLogInstance->Log(tag+"AT+CHLD=2 answer none");
				}
			}
		}
	}
	InquirePToPCall();
	CallListToMMI(0);
	mLogInstance->Log(tag+"HeldCurrentPToPCall out bad");
	return	false;
}

bool MrmGprsHandle::HeldCurrentVGC()
{
	mLogInstance->Log(tag+"HeldCurrentVGC in");
	int i,counter;
	counter = 3;
	while(counter>0)
	{
		bMrmOk =false;
		bMrmError = false;
		bMrmNoCarrier = false;
		SendAT("AT+CAHLD");		//离开组呼
		{
			//SetTimer(8,10000,NULL);			//离开组呼10秒后,还没有个呼信息进来,则进行呼叫列表的查询.
			std::unique_lock <std::mutex> lck2(mtxMrmOk);
			if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(4000),[this]{ return (bMrmOk | bMrmError |bMrmNoCarrier); }))
			{
				if(bMrmOk)
				{
					mLogInstance->Log(tag+"AT+CAHLD answer ok");
					for(i=0;i<vVGCbuffer.size();i++)
					{
						if(vVGCbuffer[i].strGcStat.compare("0") == 0)
							break;
					}
					vVGCbuffer[i].strGcStat = "1";
					bMrmStatus = 0x01;
					//CloseVoiceSwitch();
					mLogInstance->Log(tag+"HeldCurrentVGC out ok");
					return true;
				}
				else if(bMrmError)
				{
					mLogInstance->Log(tag+"AT+CAHLD answer error");
					InquireCurrentVGC();
					CallListToMMI(0);
					mLogInstance->Log(tag+"HeldCurrentVGC out bad");
					return false;
				}
				else if(bMrmNoCarrier)
				{
					mLogInstance->Log(tag+"AT+CAHLD answer nocarrier");
					InquireCurrentVGC();
					CallListToMMI(0);
					bMrmStatus = 0x01;
					//CloseVoiceSwitch();
					mLogInstance->Log(tag+"HeldCurrentVGC out ok");
					return true;					//中途掉网，退出组呼成功
				}
			}
			else
			{
				mLogInstance->Log(tag+"AT+CAHLD answer none");
			}
		}
	}

	InquireCurrentVGC();
	CallListToMMI(0);//多次抢占信道未成功，退出组呼失败
	mLogInstance->Log(tag+"HeldCurrentVGC out bad");
	return	false;
}
//AC确认处理
bool MrmGprsHandle::AcknowledgeConfirmeProcess()
{
	int i,counter,counter1,counter2,counter3,counter4;
	long DURSpanTime;
	long RELSpanTime;

	if(bMrmStatus != 0x01)//MRM模块忙,不发送AC确认
		return false;
	bMrmStatus =  0x61;	//发送AC确认过程中

//	EmergencyCallRELEndTime = CTime::GetCurrentTime();
//	EmergencyCallRELSpanTime = EmergencyCallRELEndTime-EmergencyCallRELStartTime;
//	RELSpanTime = EmergencyCallRELSpanTime.GetTotalSeconds();//从通话结束到发送确认的时间间隔
//	RELSpanTime *= 10;							//单位为十分之一秒
//	DURSpanTime = EmergencyCallDURSpanTime.GetTotalSeconds();//从通话开始到通话结束的时间间隔
//	DURSpanTime *= 10;							//单位为十分之一秒

	bMrmStatus = 0x01;
	return false;
}
bool MrmGprsHandle::InquireAvailableOperator()
{
	mLogInstance->Log(tag+"InquireAvailableOperator in");
	int i,j;
	if((bMrmStatus!=0x01)&&(bMrmStatus!=0x00))
	{
		//向MMI发送网络搜索的结果
		_SendData  pSendData;
		mFrameGenerate.GetFrame_SearchNetResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,false,vAvailableOperatorNameList);//主机向MMI报告网络搜索结果
		mDataProcesser.SendInfoForCan(pSendData);
		mLogInstance->Log(tag+"InquireAvailableOperator out bad");
		return false;
	}

	bMrmStatus = 0x81;							//查询网络运营商
	bInquireAvailableOperatorFlag = 1;
	vAvailableOperatorNameList.clear();

	bMrmOk =false;
	bMrmError = false;
	SendAT("AT+COPS=?");		//发送"查询当前可用的网络运营商"指令
	{
		std::unique_lock <std::mutex> lck2(mtxMrmOk);
		if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(60000),[this]{ return ( bMrmError | bMrmOk); }))
		{
			if(bMrmError)
			{
				mLogInstance->Log(tag+"AT+COPS=? answer error");
				_SendData  pSendData;
				mFrameGenerate.GetFrame_SearchNetResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,false,vAvailableOperatorNameList);//主机向MMI报告网络搜索结果
				mDataProcesser.SendInfoForCan(pSendData);

				bMrmStatus = 0x01;
				bInquireAvailableOperatorFlag = 0;
				mLogInstance->Log(tag+"InquireAvailableOperator out bad");
				return false;
			}
			else if(bMrmOk)
			{
				mLogInstance->Log(tag+"AT+COPS=? answer ok");
				_SendData  pSendData;
				mFrameGenerate.GetFrame_SearchNetResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,true,vAvailableOperatorNameList);//主机向MMI报告网络搜索结果
				mDataProcesser.SendInfoForCan(pSendData);

				bMrmStatus = 0x01;
				bInquireAvailableOperatorFlag = 0;
				mLogInstance->Log(tag+"InquireAvailableOperator out ok");
				return true;
			}
		}
		else
		{
			mLogInstance->Log(tag+"AT+COPS=? answer none");
		}
	}

	_SendData  pSendData;
	mFrameGenerate.GetFrame_SearchNetResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,false,vAvailableOperatorNameList);//主机向MMI报告网络搜索结果
	mDataProcesser.SendInfoForCan(pSendData);

	bMrmStatus = 0x01;
	bInquireAvailableOperatorFlag = 0;
	mLogInstance->Log(tag+"InquireAvailableOperator out bad");
	return false;
}
//bool MrmGprsHandle::InquireAvailableOperator()
//{
//	mLogInstance->Log(tag+"InquireAvailableOperator in");
//	int i,j;
//	if((bMrmStatus!=0x01)&&(bMrmStatus!=0x00))
//	{
//		//向MMI发送网络搜索的结果
//		_SendData  pSendData;
//		mFrameGenerate.GetFrame_SearchNetResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,false,vAvailableOperatorNameList);//主机向MMI报告网络搜索结果
//		mDataProcesser.SendInfoForCan(pSendData);
//		mLogInstance->Log(tag+"InquireAvailableOperator out bad");
//		return false;
//	}
//
//	bMrmStatus = 0x81;							//查询网络运营商
//	bInquireAvailableOperatorFlag = 1;
//	vAvailableOperatorNameList.clear();
//
//	bMrmCops =false;
//	bMrmError = false;
//	SendAT("AT+COPS=?");		//发送"查询当前可用的网络运营商"指令
//	{
//		std::unique_lock <std::mutex> lck2(mtxMrmOk);
//		if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(60000),[this]{ return ( bMrmError | bMrmCops); }))
//		{
//			if(bMrmError)
//			{
//				mLogInstance->Log(tag+"AT+COPS=? answer error");
//				_SendData  pSendData;
//				mFrameGenerate.GetFrame_SearchNetResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,false,vAvailableOperatorNameList);//主机向MMI报告网络搜索结果
//				mDataProcesser.SendInfoForCan(pSendData);
//
//				bMrmStatus = 0x01;
//				bInquireAvailableOperatorFlag = 0;
//				mLogInstance->Log(tag+"InquireAvailableOperator out bad");
//				return false;
//			}
//			else if(bMrmCops)
//			{
//				mLogInstance->Log(tag+"AT+COPS=? answer ok");
//				_SendData  pSendData;
//				mFrameGenerate.GetFrame_SearchNetResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,true,vAvailableOperatorNameList);//主机向MMI报告网络搜索结果
//				mDataProcesser.SendInfoForCan(pSendData);
//
//				bMrmStatus = 0x01;
//				bInquireAvailableOperatorFlag = 0;
//				mLogInstance->Log(tag+"InquireAvailableOperator out ok");
//				return true;
//			}
//		}
//		else
//		{
//			mLogInstance->Log(tag+"AT+COPS=? answer none");
//		}
//	}
//
//	_SendData  pSendData;
//	mFrameGenerate.GetFrame_SearchNetResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,false,vAvailableOperatorNameList);//主机向MMI报告网络搜索结果
//	mDataProcesser.SendInfoForCan(pSendData);
//
//	bMrmStatus = 0x01;
//	bInquireAvailableOperatorFlag = 0;
//	mLogInstance->Log(tag+"InquireAvailableOperator out bad");
//	return false;
//}
bool MrmGprsHandle::SelectOperator(std::string operatorname)
{//MMI操作选网时调用
	mLogInstance->Log(tag+"SelectOperator in");
	byte statustemp;

	if((bMrmStatus!=0x01)&&(bMrmStatus!=0x00))
	{
		//向MMI发送网络选择的结果
		_SendData  pSendData;
		mFrameGenerate.GetFrame_SearchOperatorResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,0,operatorname);//主机向MMI报告网络搜索结果
		mDataProcesser.SendInfoForCan(pSendData);
		mLogInstance->Log(tag+"SelectOperator out bad");
		return false;
	}
	statustemp = bMrmStatus;
	bMrmStatus = 0x81;							//查询网络运营商

	bMrmOk =false;
	bMrmError = false;
	SendAT("AT+COPS=1,2,\""+operatorname+"\"");			//发送"选择指定的网络运营商"指令

	_SendData  pSendData;
	mFrameGenerate.GetFrame_SearchOperatorResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,1,operatorname);//主机向MMI报告网络搜索结
	mDataProcesser.SendInfoForCan(pSendData);

	{
		std::unique_lock <std::mutex> lck2(mtxMrmOk);
		if(mwaitMrmOk.wait_for(lck2, std::chrono::milliseconds(60000),[this]{ return ( bMrmError | bMrmOk); }))
		{
			if(bMrmError)
			{
				mLogInstance->Log(tag+"AT+COPS=1,2, answer error");
				_SendData  pSendData;
				mFrameGenerate.GetFrame_SearchOperatorResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,2,operatorname);//主机向MMI报告网络搜索结果
				mDataProcesser.SendInfoForCan(pSendData);
				bMrmStatus = statustemp;
				mLogInstance->Log(tag+"SelectOperator out bad");
				return false;
			}
			else if(bMrmOk)
			{
				mLogInstance->Log(tag+"AT+COPS=1,2, answer ok");
				_SendData  pSendData;
				mFrameGenerate.GetFrame_SearchOperatorResultInfo(pSendData,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,2,operatorname);//主机向MMI报告网络搜索结果
				mDataProcesser.SendInfoForCan(pSendData);

				SetMrmGprsStatus(BIT1,true,true,0x00);//标记g_MRMRegisterInit_Event

				bMrmCregSuccess = true;
				std::unique_lock<std::mutex> lck(mtxMrmCregSuccess);
				mwaitMrmCregSuccess.notify_all();

				bMrmStatus = 0x01;
				mLogInstance->Log(tag+"SelectOperator out ok");
				return true;
			}
		}
		else
		{
			mLogInstance->Log(tag+"AT+COPS=1,2, answer none");
		}
	}
	_SendData  pSendData2;
	mFrameGenerate.GetFrame_SearchOperatorResultInfo(pSendData2,mTrainState->bMainMMI,mTrainState->bZongXian,PRIORITY0,ACK2,BASEFRAME,2,operatorname);//主机向MMI报告网络搜索结果
	mDataProcesser.SendInfoForCan(pSendData2);
	bMrmStatus = statustemp;
	mLogInstance->Log(tag+"SelectOperator out bad");
	return false;
}

bool MrmGprsHandle::GprsBooting()
{//函数目的：GPRS模块初始化函数	//返回值: 成功为1，失败为0

	mLogInstance->Log(tag+"GprsBooting in");
	int i;

	//查询GPRS模块软件版本号
	for(i=0;i<3;i++)
	{
		bGprsOk = false;
		SendAT("AT+GMR");
		{
			std::unique_lock <std::mutex> lck(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck, std::chrono::milliseconds(10000),[this]{ return bGprsOk; }))
			{
				mLogInstance->Log(tag+"AT+GMR answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+GMR answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i>=3)
	{
		mLogInstance->Log(tag+"GprsBooting out bad");
		return false;
	}
	/////////////////设置工作频段////////////////////////
	for(i=0;i<3;i++)
	{
		bGprsOk = false;
		bGprsError = false;
		SendAT("AT+CBAND=\"EGSM_MODE\"");//
		{
			std::unique_lock <std::mutex> lck2(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return ( bGprsError | bGprsOk); }))
			{
				if(bGprsOk)
				{
					mLogInstance->Log(tag+"AT+CBAND=\"EGSM_MODE\" answer ok");
					break;
				}
				else  if(bGprsError)
				{
					mLogInstance->Log(tag+"AT+CBAND=\"EGSM_MODE\" answer error");
					break;
				}
			}
			else
			{
				mLogInstance->Log(tag+"AT+CBAND=\"EGSM_MODE\" answer none");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i>=3)
	{
		mLogInstance->Log(tag+"GprsBooting out bad");
		return false;
	}

	//设置GPRS模块开始工作,主动返回位置更新及网络登录状态
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	for(i=0;i<3;i++)
	{
		bGprsOk = false;
		SendAT("AT+CFUN=1;+CGREG=2");
		{
			std::unique_lock <std::mutex> lck(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck, std::chrono::milliseconds(10000),[this]{ return bGprsOk; }))
			{
				mLogInstance->Log(tag+"AT+CFUN=1;+CGREG=2 answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+CFUN=1;+CGREG=2 answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	if(i>=3)
	{
		mLogInstance->Log(tag+"GprsBooting out bad");
		return false;
	}

	SetMrmGprsStatus(BIT1,true,true,0x01);//标记g_MRMRegisterInit_Event

	mLogInstance->Log(tag+"GprsBooting out ok");
	return true;
}

bool MrmGprsHandle::GprsRegisterInit()
{//GPRS模块登录后初始化 //成功为1，失败为0

	mLogInstance->Log(tag+"GprsRegisterInit in");

	int i,counter;
	counter = 15;
	for(i=0;i<counter;i++)
	{
		bGprsRegisterAck = false;
		SendAT("AT+CGREG?");
		{
			std::unique_lock <std::mutex> lck5(mtxGprsRegisterAck);
			if(mwaitGprsRegisterAck.wait_for(lck5, std::chrono::milliseconds(5000),[this]{ return bGprsRegisterAck; }))
			{//
				mLogInstance->Log(tag+"AT+CGREG? answer ok");//mLogInstance->Log(tag+"registerack is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+CGREG? answer none");//mLogInstance->Log(tag+"can not wait registerack");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		mLogInstance->Log(tag+"GprsRegisterInit out bad");
		return false;
	}

	//关闭回显,设置主动返回位置更新及网络登录状态,在应用数据之前增加+IPD,设置在AT+CIPSEND后返回 ">"
	counter = 3;
	for(i=0;i<counter;i++)
	{
		bGprsOk = false;
		SendAT("ATE0+CGREG=2;+CIPHEAD=1;+CIPSPRT=1");
		{
			std::unique_lock <std::mutex> lck(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck, std::chrono::milliseconds(10000),[this]{ return bGprsOk; }))
			{
				mLogInstance->Log(tag+"ATE0+CGREG=2;+CIPHEAD=1;+CIPSPRT=1 answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"ATE0+CGREG=2;+CIPHEAD=1;+CIPSPRT=1 answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		mLogInstance->Log(tag+"GprsRegisterInit out bad");
		return false;//MRM模块初始化失败
	}

	SetMrmGprsStatus(~BIT1,false,false,0x01);//g_MRMRegisterInit_Event.ResetEvent();

	{//////////////等"+STC: 0"读出来，再执行别的操作，以免影响随后的操作//////////////////////////////////
		bGprsStc = false;
		{
			std::unique_lock <std::mutex> lck5(mtxGprsStc);
			if(mwaitGprsStc.wait_for(lck5, std::chrono::milliseconds(5000),[this]{ return bGprsStc; }))
			{//
				mLogInstance->Log(tag+"stc answer ok");//mLogInstance->Log(tag+"registerack is answered");
			}
			else
			{
				mLogInstance->Log(tag+"stc answer none");//mLogInstance->Log(tag+"can not wait registerack");
			}
		}
	}

	SetMrmGprsStatus(BIT3,true,true,0x01);//
	mLogInstance->Log(tag+"GprsRegisterInit out ok");
	return true;
}

bool MrmGprsHandle::EnterGPRSMode()
{//进入GPRS通信模式
	mLogInstance->Log(tag+"EnterGPRSMode in");
	string stringtemp;
	int i,counter;
	if((bGprsStatus == 0x01)||(bGprsStatus == 0x03)||(bGprsStatus == 0x04))		//当前状态是GPRS状态
	{
		mLogInstance->Log(tag+"EnterGPRSMode out ok");
		return true;
	}
	if(bGprsStatus == 0x02)												//当前状态是CSD状态(必须先退出CSD状态)
	{
		mLogInstance->Log(tag+"EnterGPRSMode out bad");
		return false;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	/////////////////关闭移动场景(在此之前，移动场景可能已经关闭)////////////////////////
	counter = 3;
	for(i=0;i<counter;i++)
	{
		bGprsShut = false;
		bGprsError = false;
		SendAT("AT+CIPSHUT");//AT+CIPSHUT
		{
			std::unique_lock <std::mutex> lck2(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return ( bGprsError | bGprsShut); }))
			{
				if(bGprsShut)
				{
					mLogInstance->Log(tag+"AT+CIPSHUT answer ok");
					break;
				}
				else  if(bGprsError)
				{
					mLogInstance->Log(tag+"AT+CIPSHUT answer error");
					break;
				}
			}
			else
			{
				mLogInstance->Log(tag+"AT+CIPSHUT answer none");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i>=counter)
	{
		mLogInstance->Log(tag+"EnterGPRSMode out bad");
		return false;
	}

	////////////////////////////设置UDP端口号/////////////////////////////////
	counter = 3;
	for(i=0;i<counter;i++)
	{
		bGprsOk = false;
		SendAT("AT+CLPORT=\"UDP\",\"20000\"");
		{
			std::unique_lock <std::mutex> lck(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck, std::chrono::milliseconds(2000),[this]{ return bGprsOk; }))
			{
				mLogInstance->Log(tag+"AT+CLPORT=\"UDP\",\"20000\" answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+CLPORT=\"UDP\",\"20000\" answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		mLogInstance->Log(tag+"EnterGPRSMode out bad");
		return false;//MRM模块初始化失败
	}

	////使能UDP扩展模式（便于SIM900模块接收任意IP地址发来的数据），一定要放在CSTT设置之前////
	counter = 3;
	for(i=0;i<counter;i++)
	{
		bGprsOk = false;
		SendAT("AT+CIPUDPMODE=1");
		{
			std::unique_lock <std::mutex> lck(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck, std::chrono::milliseconds(2000),[this]{ return bGprsOk; }))
			{
				mLogInstance->Log(tag+"AT+CIPUDPMODE=1 answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+CIPUDPMODE=1 answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	///////////////////////////////选择APN////////////////////////////////////////
	counter = 3;
	for(i=0;i<counter;i++)
	{
		bGprsOk = false;
		SendAT("AT+CSTT=\""+mTrainState->strApn+"\",\""+mTrainState->strEngineNumber+"\",\""+mTrainState->strEngineNumber+"\"");
		{
			std::unique_lock <std::mutex> lck(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck, std::chrono::milliseconds(5000),[this]{ return bGprsOk; }))
			{
				mLogInstance->Log(tag+"AT+CSTT= answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+CSTT= answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		mLogInstance->Log(tag+"EnterGPRSMode out bad");
		return false;//MRM模块初始化失败
	}

	/////////////////////////////////激活移动场景////////////////////////////////////////
	counter = 3;
	for(i=0;i<counter;i++)
	{
		bGprsOk = false;
		SendAT("AT+CIICR");
		{
			std::unique_lock <std::mutex> lck(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck, std::chrono::milliseconds(2000),[this]{ return bGprsOk; }))
			{
				mLogInstance->Log(tag+"AT+CIICR answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+CIICR answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		mLogInstance->Log(tag+"EnterGPRSMode out bad");
		return false;//MRM模块初始化失败
	}

	///////////////////////////////查询IP地址////////////////////////////////////////
	counter = 3;
	for(i=0;i<counter;i++)
	{
		bHaveGprsIPAddressFlag = true;
		bGprsIPAdress = false;
		SendAT("AT+CIFSR");
		{
			std::unique_lock <std::mutex> lck(mtxGprsIPAdress);
			if(mwaitGprsIPAdress.wait_for(lck, std::chrono::milliseconds(3000),[this]{ return bGprsIPAdress; }))
			{
				mLogInstance->Log(tag+"AT+CIFSR answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				bHaveGprsIPAddressFlag = false;
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+CIFSR answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		bHaveGprsIPAddressFlag = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		mLogInstance->Log(tag+"EnterGPRSMode out bad");
		return false;//MRM模块初始化失败
	}
	////////////////////////////////开始UDP////////////////////////////////////////
	counter = 5;
	for(i=0;i<counter;i++)
	{
		bGprsConnectOk = false;
		bGprsAlreadyConnect = false;
		SendAT("AT+CIPSTART=\"UDP\",\"" + mTrainState->strSourAddre + "\",\"20000\"");
		{
			std::unique_lock <std::mutex> lck2(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck2, std::chrono::milliseconds(10000),[this]{ return ( bGprsAlreadyConnect | bGprsConnectOk ); }))
			{
				if( bGprsAlreadyConnect || bGprsConnectOk)
				{
					mLogInstance->Log(tag+"AT+CIPSTART=\"UDP\", answer connect ok");
					break;
				}
			}
			else
			{
				mLogInstance->Log(tag+"AT+CIPSTART=\"UDP\" answer none");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i>=counter)
	{
		mLogInstance->Log(tag+"EnterGPRSMode out bad");
		return false;
	}

	SetMrmGprsStatus(~BIT1,false,false,0x01);
	bGprsStatus = 0x01;//GPRS状态
	mTrainState->arrInitStatus[3] = mTrainState->arrInitStatus[3] & (~BIT6);
	strDestinationIPAddress = "";
	strDestinationPort = "";

	mLogInstance->Log(tag+"EnterGPRSMode out ok");
	return true;
}

bool MrmGprsHandle::EnterCSDMode()
{//进入CSD通信模式
	mLogInstance->Log(tag+"EnterCSDMode in");
	int i,counter;
	if(bGprsStatus == 0x02)		//当前状态就是CSD状态
	{
		mLogInstance->Log(tag+"EnterCSDMode out ok");
		return true;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	///////////////////////////////设置CSD方式参数////////////////////////////////////////
	counter = 3;
	for(i=0;i<counter;i++)
	{
		bGprsOk = false;
		SendAT("AT+CBST=71,0,1");
		{
			std::unique_lock <std::mutex> lck(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck, std::chrono::milliseconds(2000),[this]{ return bGprsOk; }))
			{
				mLogInstance->Log(tag+"AT+CBST=71,0,1 answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"AT+CBST=71,0,1 answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		mLogInstance->Log(tag+"EnterCSDMode out bad");
		return false;//MRM模块初始化失败
	}

	///////////////////////////////拨号命令////////////////////////////////////////
	counter = 3;
	for(i=0;i<counter;i++)
	{
		bGprsOk = false;
		bGprs9600 = false;
		bGprsBusy = false;
		bGprsNoCarrier = false;
		SendAT("ATD13911740472");
		{
			std::unique_lock <std::mutex> lck2(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck2, std::chrono::milliseconds(50000),[this]{ return ( bGprsOk | bGprs9600 | bGprsBusy | bGprsNoCarrier); }))
			{
				if(bGprsOk || bGprs9600)
				{
					mLogInstance->Log(tag+"ATD13911740472 answer ok");
					bGprsStatus = 0x02;
					mLogInstance->Log(tag+"EnterCSDMode out ok");
					return true;//
				}
				else if(bGprsBusy )
				{
					mLogInstance->Log(tag+"ATD13911740472 answer busy");
					break;
				}
				else if(bGprsNoCarrier )
				{
					mLogInstance->Log(tag+"ATD13911740472 answer nocarrier");
					break;
				}
			}
			else
			{
				mLogInstance->Log(tag+"ATD13911740472 answer none");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		mLogInstance->Log(tag+"EnterCSDMode out bad");
		return false;//
	}
	mLogInstance->Log(tag+"EnterCSDMode out bad");
	return false;//
}

bool MrmGprsHandle::RetireCSDMode()
{//退出CSD通信模式
	mLogInstance->Log(tag+"RetireCSDMode in");
	int i,counter;
	if(bGprsStatus == 0x00)		//当前状态为停止工作状态
	{
		mLogInstance->Log(tag+"RetireCSDMode out ok");
		return true;
	}
	if((bGprsStatus == 0x01)||(bGprsStatus == 0x03)||(bGprsStatus == 0x04))//当前状态是GPRS状态
	{
		mLogInstance->Log(tag+"RetireCSDMode out bad");
		return false;
	}
	counter = 1;
	for(i=0;i<counter;i++)
	{
		bGprsOk = false;
		SendAT("+++");
		//bGprsStatus = 0x00;//必须改变工作状态值，否则收不到返回的OK
		{
			std::unique_lock <std::mutex> lck5(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck5, std::chrono::milliseconds(6000),[this]{ return bGprsOk; }))
			{//
				mLogInstance->Log(tag+"+++ answer ok");//mLogInstance->Log(tag+"registerack is answered");
				break;
			}
			else
			{
				mLogInstance->Log(tag+"+++ answer none");//mLogInstance->Log(tag+"can not wait registerack");
				//bGprsStatus = 0x02;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		mLogInstance->Log(tag+"RetireCSDMode out bad");
		return false;
	}

	///////////////////////////////发送挂机命令////////////////////////////////////////
	counter = 1;
	for(i=0;i<counter;i++)
	{
		bGprsOk = false;
		SendAT("ATH");
		{
			std::unique_lock <std::mutex> lck(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck, std::chrono::milliseconds(6000),[this]{ return bGprsOk; }))
			{
				mLogInstance->Log(tag+"ATH answer ok");//mLogInstance->Log(tag+"mrmok is answered");
				bGprsStatus = 0x00;
				mLogInstance->Log(tag+"RetireCSDMode out ok");
				return true;
			}
			else
			{
				mLogInstance->Log(tag+"ATH answer none");//mLogInstance->Log(tag+"can not wait mrmok");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		mLogInstance->Log(tag+"RetireCSDMode out bad");
		return false;//MRM模块初始化失败
	}

	mLogInstance->Log(tag+"RetireCSDMode out bad");
	return false;
}
bool MrmGprsHandle::RetireGPRSMode()
{//退出GPRS模式,函数成功执行后返回到停止工作状态
	mLogInstance->Log(tag+"RetireGPRSMode in");
	int i,counter;
	string stringtemp;
	if(bGprsStatus == 0x00)		//当前是停止工作状态
	{
		mLogInstance->Log(tag+"RetireGPRSMode out ok");
		return true;
	}
	if(bGprsStatus == 0x02)//当前是CSD状态
	{
		mLogInstance->Log(tag+"RetireGPRSMode out bad");
		return false;
	}
	/////////////////////关闭移动场景///////////////////////
	counter = 1;
	for(i=0;i<counter;i++)
	{
		bGprsShut = false;
		bGprsError = false;
		SendAT("AT+CIPSHUT");
		{
			std::unique_lock <std::mutex> lck5(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck5, std::chrono::milliseconds(1000),[this]{ return bGprsShut | bGprsError; }))
			{//
				if(bGprsShut)
				{
					mLogInstance->Log(tag+"AT+CIPSHUT answer ok");
					break;
				}
				else  if(bGprsError)
				{
					mLogInstance->Log(tag+"AT+CIPSHUT answer error");
					break;
				}
			}
			else
			{
				mLogInstance->Log(tag+"AT+CIPSHUT answer none");//mLogInstance->Log(tag+"can not wait registerack");
				//bGprsStatus = 0x02;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		bGprsStatus = 0x01;//返回GPRS空闲模式
		mLogInstance->Log(tag+"RetireGPRSMode out bad");
		return false;
	}

	//退出成功，返回停止工作模式
	bGprsStatus = 0x00;
	bGprsStatusBefore= 0xff;
	strDestinationIPAddress = ("");
	strDestinationPort = ("");
	//KillTimer(14);
	//KillTimer(17);
	mTrainState->bGprsCheckNormalFlag = 0;

	mLogInstance->Log(tag+"RetireGPRSMode out bad");
	return false;
}

bool MrmGprsHandle::GprsDataSend(_SendDataNet& pSendDataNet)
{//GPRS数据发送
	mLogInstance->Log(tag+"GprsDataSend in");

	if(bGprsStatus == 0x01)
	{//不是GPRS空闲模式，返回失败
		mLogInstance->Log(tag+"GprsDataSend out bad, not 0x01");
		return false;
	}
	bGprsStatus = 0x03;

	//////////////////关闭UDP连接///////////////////////
	bGprsOk = false;
	SendAT("AT+CIPCLOSE");
	{
		std::unique_lock <std::mutex> lck(mtxGprsOk);
		if(mwaitGprsOk.wait_for(lck, std::chrono::milliseconds(1000),[this]{ return bGprsOk; })) //不必等正确的应答，因为发送此命令之前，可能已经处于连接断开状态
		{
			mLogInstance->Log(tag+"AT+CIPCLOSE answer ok");//mLogInstance->Log(tag+"mrmok is answered");
		}
		else
		{
			mLogInstance->Log(tag+"AT+CIPCLOSE answer none");//mLogInstance->Log(tag+"can not wait mrmok");
		}
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(200));


	bGprsConnectOk = false;
	bGprsAlreadyConnect = false;

	//char s[12];	itoa(pSendDataNet.iDestinationPort,s,10); std::string strPort = s;
	std::stringstream ss;
	ss<<pSendDataNet.iDestinationPort;
	std::string strPort = ss.str();

	SendAT("AT+CIPSTART=\"UDP\",\"" + pSendDataNet.strDestinationIPAddress + "\",\"+ strPort +\"");
	{
		std::unique_lock <std::mutex> lck2(mtxGprsOk);
		if(mwaitGprsOk.wait_for(lck2, std::chrono::milliseconds(3000),[this]{ return ( bGprsAlreadyConnect | bGprsConnectOk ); }))
		{
			if( bGprsAlreadyConnect || bGprsConnectOk)
			{
				mLogInstance->Log(tag+"AT+CIPSTART=\"UDP\", answer connect ok");
				strDestinationIPAddress = pSendDataNet.strDestinationIPAddress;
				strDestinationPort = strPort;
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
		else
		{
			mLogInstance->Log(tag+"AT+CIPSTART=\"UDP\" answer none");
		}
	}

	//////////////////设置发送数据的长度///////////////////////
	int counter = 3, i=0;
	for(i=0;i<counter;i++)
	{
		bGprsSendStart = false;
		bGprsError = false;

		//char s1[12];	itoa(pSendDataNet.vSendData.size(),s1,10); std::string strLen = s1;
		ss.clear();
		ss<<pSendDataNet.vSendData.size();
		std::string strLen = ss.str();

		SendAT("AT+CIPSEND="+strLen);//
		{
			std::unique_lock <std::mutex> lck2(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return ( bGprsError | bGprsSendStart); }))
			{
				if(bGprsSendStart)
				{
					mLogInstance->Log(tag+"AT+CIPSEND= answer ok");
					break;
				}
				else if(bGprsError)
				{
					mLogInstance->Log(tag+"AT+CIPSEND= answer error");
				}
			}
			else
			{
				mLogInstance->Log(tag+"AT+CIPSEND= answer none");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		if(bGprsStatus==0x03)
			bGprsStatus = 0x01;
		bGprsStatusBefore = 0xff;
		mLogInstance->Log(tag+"GprsDataSend out bad");
		return false;
	}

	for(i=0;i<counter;i++)
	{
		bGprsSendFailed = false;
		bGprsSendOk = false;

//		char s1[12];	itoa(pSendDataNet.vSendData.size(),s1,10); std::string strLen = s1;
//		std::string strSendData="";
//		SendAT(strSendData);//
//		{
//			std::unique_lock <std::mutex> lck2(mtxGprsOk);
//			if(mwaitGprsOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return ( bGprsSendOk | bGprsSendFailed); }))
//			{
//				if(bGprsSendOk)
//				{
//					mLogInstance->Log(tag+"send data ok");
//					break;
//				}
//				else if(bGprsSendFailed)
//				{
//					mLogInstance->Log(tag+"send data failed");
//				}
//			}
//			else
//			{
//				mLogInstance->Log(tag+"send data answer none");
//			}
//		}
//		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		_SendData  pSendData;
		mFrameGenerate.GetFrame_GprsDataToZhuanHuan(pSendData,bPort,mTrainState->bZongXian,PRIORITY0,ACK0,BASEFRAME,pSendDataNet);//ACK2//暂时注释
		mDataProcesser.SendInfoForCan(pSendData);
		{
			std::unique_lock <std::mutex> lck2(mtxGprsOk);
			if(mwaitGprsOk.wait_for(lck2, std::chrono::milliseconds(5000),[this]{ return ( bGprsSendOk | bGprsSendFailed); }))
			{
				if(bGprsSendOk)
				{
					mLogInstance->Log(tag+"send data ok");
					break;
				}
				else if(bGprsSendFailed)
				{
					mLogInstance->Log(tag+"send data failed");
				}
			}
			else
			{
				mLogInstance->Log(tag+"send data answer none");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	if(i >= counter)
	{
		if(bGprsStatus==0x03)
			bGprsStatus = 0x01;
		bGprsStatusBefore = 0xff;
		mLogInstance->Log(tag+"GprsDataSend out bad");
		return false;
	}

	if(bGprsStatus==0x03)
		bGprsStatus = 0x01;
	bGprsStatusBefore = 0xff;
	mLogInstance->Log(tag+"GprsDataSend out ok");
	return true;
}
