

#ifndef MRMGPRS_H_
#define MRMGPRS_H_

#include "HeadFileForAll.h"
#include "TrainState.h"
#include "LogInstance.h"

#include "DataProcesser.h"
#include "FrameGenerate.h"
#include "_RawInfo.h"
#include "_SendData.h"
#include "_SendDataNet.h"
#include "_AtCommand.h"

#include <iostream>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <map>
#include <chrono>
#include <sstream>

#include "StructsForMrm.h"

class MrmGprsHandle {
public:
	MrmGprsHandle();
	virtual ~MrmGprsHandle();

	bool bMrmNeedDeleteOk;
	std::mutex mtx_AtCommand ; // 对AT命令列表加锁
	std::condition_variable mwaitAtCommand;
	std::list<std::string> listAtCommand;

	std::mutex mtx_GprsData ;
	std::condition_variable mwaitGprsData;
	std::list<_SendDataNet> listGprsData;

	byte bPort;//端口
	byte bActor;//角色 mrm gprs
	long bLastLiveTime;
	bool bIsLive;
	std::string tag ;

	TrainState *mTrainState;
	ParamOperation *mParamOperation;
	LogInstance* mLogInstance;

	DataProcesser mDataProcesser ;
	FrameGenerate mFrameGenerate;

	void Init(byte port,byte actor);
	void StartService();

	bool bHandleAtCommand;
	std::thread thdHandleAtCommand;
	void HandleAtCommandThread();

	bool bSendGprsData;
	std::thread thdHandleSendGprsData;
	void SendGprsDataThread();

	bool bDoWork;
	std::thread thdDoWork;
	void DoWorkThread();

	bool bTimer;
	std::thread thdTimer;
	void TimerThread();

	int iMrmForStep;//g_MRMBooting_Event=BIT0  g_MRMRegisterInit_Event=BIT1  g_MRMCOPS_Event=BIT2 g_MRMExitAllCall_Event=BIT3 g_MRMUUS1Tx_Event=BIT4
	//g_MRMDial_Event=BIT5  g_MRMJoinPtoPCall_Event=BIT6 g_MRMCallHangUp_Event=BIT7  g_MRMCallSelect_Event=BIT8 g_MRMPushPTT_Event=BIT9 g_MRMReleasePTT_Event=BIT10
	//g_MRMHeldCurruntVGC_Event=BIT11 g_MRMInquireCall_Event=BIT12  g_MRMInquireActiveCall_Event=BIT13  g_MRMSentAc_Event=BIT14 g_MRMInquireOperator_Event=BIT15
	//g_MRMSelectOperator_Event=BIT16 g_MRMCFInquire_Event=BIT17  g_MRMCFSet_Event=BIT18  g_MRMCFCancel_Event=BIT19 TrainNumberFNDeregisterEvent=BIT20
	//TrainNumberFNRegisterEvent=BIT21  EngineNumberFNDeregisterEvent=BIT22  EngineNumberFNRegisterEvent=BIT23
	byte bMrmStatus; // 0x00:未登录状态;  0x01:空闲模式;  0x21:发起呼叫中;  0x31:个呼通话中; 0x41: 组呼通话中;0x51:功能号查询/注册/注销;0x61:发送AC确认;0x71:呼叫转移查询/设置/取消状态;0x81:查询/选择网络运营商

	std::mutex mtxSgmEvent;//mtxMrmStartEvent
	std::condition_variable mwaitSgmEvent;//mwaitMrmStartEvent

	std::mutex mtxMrmOk;
	std::condition_variable mwaitMrmOk;
	bool bMrmOk;

//	std::mutex mtxMrmResponse;
//	std::condition_variable mwaitMrmResponse;
//	bool bMrmResponse;

	//std::mutex mtxMrmNoCarrier;
	//std::condition_variable mwaitMrmNoCarrier;
	bool bMrmNoCarrier;

	std::mutex mtxMrmCfcc;
	std::condition_variable mwaitMrmCfcc;
	bool bMrmCfcc;

	std::mutex mtxMrmCUSD;
	std::condition_variable mwaitMrmCUSD;
	bool bMrmCUSD;

	//std::mutex mtxMrmBusy;
	//std::condition_variable mwaitMrmBusy;
	bool bMrmBusy;

	//std::mutex mtxMrmError;
	//std::condition_variable mwaitMrmError;
	bool bMrmError,bMrmError3,bMrmError155,bMrmError156,bMrmError162;

	bool	bMrmAcOkFlag;//ac确认成功
	std::mutex mtxMrmAcOk;
	std::condition_variable mwaitMrmAcOk;
	bool bMrmAcOk;//收到ac确认

	std::mutex mtxMrmRegisterAck;//等到+creg返回(正常或者不正常)
	std::condition_variable mwaitMrmRegisterAck;
	bool bMrmRegisterAck;

	std::mutex mtxMrmCregSuccess;//等到+creg且选网正常
	std::condition_variable mwaitMrmCregSuccess;
	bool bMrmCregSuccess;

//	std::mutex mtxMrmCops;//
//	std::condition_variable mwaitMrmCops;
//	bool bMrmCops;

	std::mutex mtxMrmExitAllComplete;//
	std::condition_variable mwaitMrmExitAllComplete;
	bool bMrmExitAllComplete;

	std::mutex mtxMrmTrainNumberFNDeregisterComplete;//
	std::condition_variable mwaitTrainNumberFNDeregisterComplete;
	bool bMrmTrainNumberFNDeregisterComplete;

	std::mutex mtxMrmEngineNumberFNDeregisterComplete;//
	std::condition_variable mwaitEngineNumberFNDeregisterComplete;
	bool bMrmEngineNumberFNDeregisterComplete;

	bool bTimer1ForQueryCsq ;int iTime1ForQueryCsq ;//
	bool bTimer17ForReset ;int iTime17ForReset; //
	bool bTimer8ForQueryCallList ;int iTime8ForQueryCallList; //


	bool bTimer2 ;int iTime2; //
	bool bTimer10 ;int iTime10; //
	bool bTimer11 ;int iTime11; //
	bool bTimer12 ;int iTime12; //
	bool bTimer13 ;int iTime13; //
	bool bTimer15 ;int iTime15; //

	void SetTimer(int iTimerId);
	void KillTimer(int iTimerId);


	std::string strOriginateNumber;
	std::string strDialType; //01:ISDN/MSISDN个呼；02:组呼；03:功能号呼叫
	std::string strDialPriority;
	std::string strDialNumber;
	std::string  strCCFC;
	std::string  strCUSD;
	DataHandle sctDataHandle;
	std::vector<_PToPCallStruct> vPToPbuffer;
	std::vector<_PToPCallStruct> vPToPTempbuffer;
	_PToPCallStruct* pPToPpointer;
	std::vector<_GroupCallStruct> vVGCbuffer;
	std::vector<_GroupCallStruct> vVGCTempbuffer;
	_GroupCallStruct* pVGCpointer;
	_GroupCallStruct mCurrentActiveVGCbuffer;
	bool bCurrentActiveVGCInquireFlag;
	std::vector<byte> vLastCallListBuf;
	_FNStruct	TrainNumberFNRegister,TrainNumberFNDeregister,EngineNumberFNRegister,EngineNumberFNDeregister;
	_FNStruct TrainNumberFNProcess,TrainNumberFNTemp,TrainNumberFNCompare;
	_FNStruct EngineNumberFNProcess,EngineNumberFNTemp,EngineNumberFNCompare;
	std::vector<_FNStruct> vTrainNumberFNList;
	std::vector<_FNStruct> vEngineNumberFNList;
	int	iTrainNumberFNCounter,iEngineNumberFNCounter;
	byte bTrainNumberFNRegisterStatus;	//	1:功能号已注册到本机SIM卡;2:功能号已注册到其他设备的SIM卡
	byte bEngineNumberFNRegisterStatus;	//	 //	3:功能号未被任何SIM卡注册//4:功能号为非法号码 //5:未开通Follow Me 业务
	long EmergencyCallDUREndTime;
	long EmergencyCallDURStartTime;
	long EmergencyCallDURSpanTime;
	long EmergencyCallRELStartTime;
	long EmergencyCallRELEndTime;
	long EmergencyCallRELSpanTime;
	bool bEmergencyCallFlag;
	bool bEmergencyCallMakerFlag;
	byte bACFailedCounter;
	std::string	strEmergencyCallGCA;
	std::string	strTrainFunctionNumber;
	std::string	strEngineFunctionNumber;
	bool	bTxCallListToMMIFlag;
	std::string	strCFSetNumber,strCFInquireNumber,strCFSetTime,strCFInquireTime,strCFSetType,strCFInquireType,strCFCancelType;
	bool	bInquireAvailableOperatorFlag;
	//int		iAvailableOperatorCounter;
	std::vector<std::string>	vAvailableOperatorNameList;		//用于存储运营商数字编码,For Example:"46000"
	MMISelectCallNum mMMISelectCallNumber;
	std::vector<_UUS1Info> vUUS1Infobuf;
	bool bInquireCurrentVGCFlag;
	bool bNoListToMMIFlag;

	bool bWaitForJoinVGCS;
	bool bMrmForbidRigisterInitFlag;//在初始化未成功时,禁止去注册//=1为禁止
	//bool bMRMResetForbidFlag;//初始化控制,开机和初始化过程中禁止reset//=1为禁止 主要为了延时reset
	//bool bSetMRMBaudRateFlag;//波特率设置标志//=0为未设置//由转换单元去设置
	bool bMRMACOK_Flag;

	void SendAT(std::string strAtCommand);
	void SetMrmGprsStatus(int mask, bool bYuHuo, bool bShouldNotify, byte bWhileOne);

	bool MrmBooting();
	bool MrmRegisterInit();
	bool COPSInquire();
	bool InquirePToPCall();
	bool InquireCurrentVGC();
	void CallListToMMI(byte TxFlag);

	bool ExitAllPToPCall();
	bool ExitGroupCall();
	bool InquireCurrentVGC1();
	bool InquireCurrentVGC2();
	bool UUS1InfoTx();
	int MRMTxTrainNumberFNProcess(byte* buffer);
	int MRMTxEngineNumberFNProcess(byte* point);
	std::string TrainNumberToFunctionNumber(std::string trainnumber,byte runningnumberstatus);
	bool OriginateCall(std::string str1,std::string str2,std::string str3);
	bool JoinPToPCall();
	bool JoinVGCS(std::string string17Or18,std::string stringGID,std::string stringGCA,std::string stringpriority);
	void  CallHangUpProcess();
	bool ExitCurrentPToPCall();
	bool GroupCallUplinkRelease();
	bool GroupCallUplinkApply();
	void CallSelectProcess();
	bool HeldCurrentPToPCall();
	bool HeldCurrentVGC();
	bool AcknowledgeConfirmeProcess();
	bool InquireAvailableOperator();
	bool SelectOperator(std::string operatorname);
	/*
	byte DeregisterFN(std::string str);
	byte RegisterFN(std::string str);
	*/



	int iGprsForStep; //g_GPRSBooting_Event=BIT0, g_GPRSRegisterInit_Event=BIT1,
					//g_GPRSEnterCSDMode_Event=BIT2, g_GPRSEnterGPRSMode_Event=BIT3,
					//g_GPRSRetireCSDMode_Event=BIT4, g_GPRSRetireGPRSMode_Event=BIT5

	byte bGprsStatus;  //0:停止工作模式,1:GPRS空闲模式,2:CSD模式,3:GPRS发送模式,4:GPRS数据接收状态
	byte bGprsStatusBefore;//在进入GPRS数据接收状态之前的状态
	bool bHaveGprsIPAddressFlag; //0:收到的数字不是IP地址;1:收到的数字为IP地址
	int iGPRSSendErrorCounter;
	std::string	strDestinationIPAddress,strDestinationPort;	//用于记录上次发送GPRS数据的IP地址和端口号

	bool bNeedEnterCSDFlag;
	bool bNeedEnterGPRSFlag;

	std::mutex mtxGprsIPAdress;
	std::condition_variable mwaitGprsIPAdress;
	bool bGprsIPAdress;

	std::mutex mtxGprsOk;
	std::condition_variable mwaitGprsOk;
	bool bGprsOk;

	std::mutex mtxGprsStc;
	std::condition_variable mwaitGprsStc;
	bool bGprsStc;

	bool bGprsError;
	bool bGprsConnectOk;
	bool bGprsAlreadyConnect;
	bool bGprs9600;
	bool bGprsBusy;
	bool bGprsNoCarrier;
	bool bGprsShut;

	std::mutex mtxGprsRegisterAck;//
	std::condition_variable mwaitGprsRegisterAck;
	bool bGprsRegisterAck;

	std::mutex mtxGprsCloseOk;//
	std::condition_variable mwaitGprsCloseOk;
	bool bGprsCloseOk;

	//std::mutex mtxGprsSendStart;//
	//std::condition_variable mwaitGprsSendStart;
	bool bGprsSendStart;

	//std::mutex mtxGprsSendOk;//
	//std::condition_variable mwaitGprsSendOk;
	bool bGprsSendOk;

	//std::mutex mtxGprsSendFailed;//
	//std::condition_variable mwaitGprsSendFailed;
	bool bGprsSendFailed;

	bool GprsBooting();
	bool GprsRegisterInit();
	bool EnterGPRSMode();
	bool EnterCSDMode();
	bool RetireCSDMode();
	bool RetireGPRSMode();

	bool GprsDataSend(_SendDataNet& pSendDataNet);
};

#endif
