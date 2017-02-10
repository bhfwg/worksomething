/*
 * TrainState.cpp
 *
 *  Created on: 2016-8-16
 *      Author: lenovo
 */

#include "TrainState.h"
#include <time.h>

TrainState* TrainState::mInstance = NULL;

TrainState* TrainState::GetInstance()
{
	if(mInstance == NULL)
		mInstance = new TrainState();
	return mInstance;
}

TrainState::TrainState()
{
	bWorkState = WORKSTATE_NORMAL;
	bMainUsed = false;
	bZongXian = CAN0;//默认CAN0,可修改
	bZongXianState = 0x00;

	bOtherboardState = 0x01; lOtherBoardRecvTime = time((time_t*)NULL);
	arrDeviceState[0] = 0x00;
	arrDeviceState[1] = 0x00;
	arrDeviceState[2] = 0x00;

	arrDeviceState[0] |= BIT0;//主控单元初始为正常
	arrDeviceState[0] |= BIT1;

	bSgm1State = 0;
	bSgm2State = 0;
	arrDeviceState[0] &= (~BIT4);//模块初始为不正常
	arrDeviceState[0] &= (~BIT5);
	bSgm1Actort  = (byte)MRM_MODE;
	bSgm2Actort  = (byte)GPRS_MODE;

	memset(arrFrameNumId0, 0, 512*sizeof(byte));
	memset(arrFrameNumId1, 0, 512*sizeof(byte));
	memset(arrFrameNumFrame0, 0, 512*sizeof(byte));
	memset(arrFrameNumFrame1, 0, 512*sizeof(byte));
	memset(arrCanState, 0, 512*sizeof(byte));
	lCan0RecvTime = time((time_t*)NULL);
	lCan1RecvTime = time((time_t*)NULL);

	//arrDeviceId[0]


	strSoftwareVersion = "V1.0.00 16/10/01"; // 主控软件版本//version
	strSimVersion = "00000000000"; // 软件版本
	strImsiVersion = "000000000000000"; // 软件版本

	strSoftwareVersion2 = "V0.0.00 00/00/00"; // 对板控软件版本//version
	strSimVersion2 = "00000000000"; // 软件版本
	strImsiVersion2 = "000000000000000"; // 软件版本

	bZhuKongFlag = 0;
	bOtherBoardFlag = 0;

	iWorkMode = 0x65;
	strLineName = "京沪线";
	iModeFlag = 0;
	iEnPort = 1;

	strEngineNumber = "XXXXXXXX";
	iEngineFlag = 1;

	strTrainNumber = "XXXXXXX";
	iLastRegisterStatus = 0;
	iTrainFlag = 1;
	iBenBuStatus = 0;

	strLastTrainNumber = "XXXXXXX";
	iLastRegisterStatus = 0;
	iLastBenBuStatus = 0;

	strCheckIp1="0.0.0.0";
	memset(arrCheckIp1, 0, 4*sizeof(byte));
	strCheckIp2="0.0.0.0";
	memset(arrCheckIp2,0,4*sizeof(byte));
	strCheckIp3="0.0.0.0";
	memset(arrCheckIp3,0,4*sizeof(byte));

	strCheckPhoneNumber1 = "00000000";
	strCheckPhoneNumber2 = "00000000";
	strCheckPhoneNumber3 = "00000000";

	strApn = "cir.apn";

	strDMISAddre="0.0.0.0";
	memset(arrDMISAddre, 0, 4*sizeof(byte));
	strGrosAddre="0.0.0.0";
	memset(arrGrosAddre, 0, 4*sizeof(byte));
	strGrosAddre1="0.0.0.0";
	memset(arrGrosAddre1, 0, 4*sizeof(byte));
	strHomeDMISAddre = strDMISAddre;
	memset(arrHomeDMISAddre, 0, 4*sizeof(byte));

	bPower = 0x01;
	bMainMMI = 2;
	bGPSStatus = 0;
	clsAnolog450.bCallStatusIn400 = 0;
	clsAnolog450.bCallStatusIn450 = 0;
	bNeedEnterCSDFlag = false;
	bNeedEnterGPRSFlag = false;
	for(int i=0;i<4;i++)
		arrBatteryVoltage[i] = 0x00;
	for(int i=0;i<2;i++)
		arrTemperature[i] = 0x00;
	bBatteryCheckStatus = 0x00;
	for(int i=0;i<5;i++)
		arrInitStatus[i] = 0;
	for(int i=0;i<32;i++)
		arrCTCMaintenanceInfo[i] = 0x00;
	bOutputSignalStatus = 0xffff;
	bLastSignalMachineType = 0x20;			//默认为编组站
	bTrainNumberForceFlag = 0;
	bModeChangeFileProtectFlag = 0;
	for(int i=0;i<2;i++)
		arrLineCode[i] = 0x00;
	iTrainNumberInfoTxSum = 0;
	iTrainNumberInfoToNowGRISTxCounter = 0;
	iNowTrainNumberInfoTxCounter = 0;
	iTrainNumberInfoStartOrStopTxSum = 0;
	iTrainNumberInfoStartOrStopToNowGRISTxCounter = 0;
	iNowTrainNumberInfoStartOrStopTxCounter = 0;
	bCIRSelfChecking = 0;
	bGPRSCheckNormalFlag = 0;
	iGPRSCheckCounter = 0;
	bGPRSCheckRepeat = 0;
	TrainNumberTemp="XXXXXXX";
	TrainNumberCmp="XXXXXXX";
	bTAXTNChange = 0xff;
	bTrainNumberBenBuTemp = 0xff;
	bTrainNumberBenBuCmp = 0xff;
	bTrainNumberBenBuT4 = 0xff;
	bTrainNumberBenBuT3 = 0xff;
	bTrainNumberBenBuT2 = 0xff;
	bTrainNumberBenBuT1 = 0xff;
	TrainNumberT1="XXXXXXX";
	TrainNumberT2="XXXXXXX";
	TrainNumberT3="XXXXXXX";
	TrainNumberT4="XXXXXXX";
	EngineNumberTemp="XXXXXXXX";
	EngineNumberT1="XXXXXXXX";
	EngineNumberT2="XXXXXXXX";
	EngineNumberT3="XXXXXXXX";
	EngineNumberT4="XXXXXXXX";
	bLastSignalType = 0xff;				//无效
	bSignalTypeTemp = 0xff;				//无效
	bTNFromMMI = 0xff;
	bNeedLastRegisterFlag = 0;
	bTrainNumberStatusRxReportFlag = 0;
	bGPSInfoToLBJFlag = 0;
	bCMRepeatRxAllowProcessFlag = 0;
	for(int i=0;i<32;i++)
			arrSelfCheckResultBuf[i] = 0x00;
	bSelfCheckResultCounter = 0x00;

	strMrmLac ="";
	strMrmCi = "";
	strGprsLac="";
	strGprsCi="";
	iSignalStrenghthenSgm1 = 0;
	iSignalStrenghthenRealSgm1 =0 ;
	iSignalStrenghthenSgm2 = 0;
	iSignalStrenghthenRealSgm2 =0 ;

	bHookFlag = 0;
	bPttFlag = 0;

	strSelectOperatorName = "";

	for(int i=0; i<6; i++)
			m_Last_File[i] = 0;
}

TrainState::~TrainState()
{
}


