/*
 * TrainState.h
 *
 *  Created on: 2016-8-16
 *      Author: lenovo
 */

#ifndef TRAINSTATE_H_
#define TRAINSTATE_H_
#include "HeadFileForAll.h"
#include "_CAnolog450.h"
class TrainState
{
public:
	static TrainState* GetInstance();
	virtual ~TrainState();

	byte bWorkState;//01H：正常   02H：测试   03H: 维护
	bool bMainUsed;//主备用

	byte bZhuKongFlag;//主控识别 ZHUKONG_LEFT  ZHUKONG_RIGHT
	byte bOtherBoardFlag;//主控识别 ZHUKONG_LEFT  ZHUKONG_RIGHT

	byte bZongXian;//当前总线号
	byte bZongXianState;//当前总线状态

	byte arrDeviceState[3];

	byte bOtherboardState;//0=down 1=ok
	long lOtherBoardRecvTime;

	byte bSgm1State,bSgm1Actort;
	byte bSgm2State,bSgm2Actort;

	byte arrFrameNumId0[512];//各个总线的流水
	byte arrFrameNumId1[512];
	byte arrFrameNumFrame0[512];//各个单元的帧流水
	byte arrFrameNumFrame1[512];//各个单元的帧流水

	byte arrCanState[512];//记录各单元总线状态
	long lCan0RecvTime;
	long lCan1RecvTime;

	byte arrDeviceId[15];//设备编号  15位数字

	std::string strSoftwareVersion ; // 主控软件版本//version
	std::string strSimVersion ; // 软件版本
	std::string strImsiVersion ; // 软件版本

	std::string strSoftwareVersion2 ; // 主控软件版本//version
	std::string strSimVersion2 ; // 软件版本
	std::string strImsiVersion2 ; // 软件版本

	int iWorkType;//=00双端主副控=01 单主机单MMI=02 单端双MMI
	int iWorkMode;
	std::string strLineName;
	int iModeFlag; //'0'自动，'1'手动
	int iShutDownStatus; //上次关机状态 0关机 1非关机
	int iEnPort;//0 普通机车或动车组  1:A，2:B

	std::string strEngineNumber;
	byte arrEngineNumber[10];
	int iEngineFlag;// 0:从TAX箱得到 1:从MMI人工输入

	std::string strTrainNumber;
	byte arrTrainNumber[10];
	int iRegisterStatus;//0未注册 1已注册
	int iTrainFlag;//0:从TAX箱得到 1:从MMI人工输入 //原Manual_Flag
	int iBenBuStatus;// 本务机补机01234 //原Status

	std::string strDisplayTrainNumber;
	int iDisplayTrainFlag;//0:从TAX箱得到 1:从MMI人工输入 //原Manual_Flag
	int iDisplayBenBuStatus;// 本务机补机01234 //原Status

	std::string strLastTrainNumber;
	int iLastRegisterStatus;//0未注册 1已注册
	int iLastBenBuStatus;// 本务机补机01234

	std::string strMsIsdn;

	std::string strCheckIp1;
	byte arrCheckIp1[4];
	std::string strCheckIp2;
	byte arrCheckIp2[4];
	std::string strCheckIp3;
	byte arrCheckIp3[4];//[2]%2==0 CAN0 //[2]==2 不打印can数据
						//[3]%2==0 bLogPrint2 = true //[3]%3==0 bLogPrint3 = true

	std::string strCheckPhoneNumber1;
	std::string strCheckPhoneNumber2;
	std::string strCheckPhoneNumber3;

	std::string strApn;

	std::string strDMISAddre;
	byte arrDMISAddre[4];
	std::string strHomeDMISAddre;
	byte arrHomeDMISAddre[4];
	std::string strGrosAddre;
	byte arrGrosAddre[4];
	std::string strGrosAddre1;
	byte arrGrosAddre1[4];

	std::string strSourAddre;
	byte arrSourAddre[6];
	std::string strTailAddre;
	byte arrTailAddre[4];

	byte bPower ;
	byte bMainMMI; //两个MMI均为备用
	byte bGPSStatus ;
	_CAnolog450 clsAnolog450;
	bool bNeedEnterCSDFlag,bNeedEnterGPRSFlag;
	byte arrBatteryVoltage[4];
	byte arrTemperature[2];
	byte bBatteryCheckStatus;
	byte arrInitStatus[5];
		// [0] :中国铁路GSM-R-02H，中国联通-03H，中国电信-04H，神华铁路LTE-05H
		// [1] :初始化状态字节1;
		// [2] :初始化状态字节2;
		// [3] :综合信息状态;(各种状态,最后一位BIT0标识手动和自动，BIT1为机车号异常告警，BIT2为补机状态，BIT3为车次号注册成功，
		// 		BIT4机车号注册成功,BIT5已未获取IP地址，BIT6已未获取IP地址，BIT7列尾连接)
		// [4] :综合信息预留状态（本补）://BIT7:机车号来源1:TAX箱(自动)0:手动；BIT6:列尾连接中断标志1:列尾连接中断0:列尾连接正常
		// 		BIT5:预留 ,BIT4:POC在线状态,BIT3:预留 BIT2-BIT0:机车牵引状态。00H: 本务机  ,01H: 补机1, 02H: 补机2, 03H: 补机 3, 04H: 补机4
	byte arrCTCMaintenanceInfo[32];

	byte bOutputSignalStatus  ;
	int  iCIRSelfCheckStatus;
	byte bCIRSelfCheckWaitCounter,bCIRSelfCheckResultTxCounter;
	byte b450MStationSelfCheckResult,bLBJStationSelfCheckResult[2],bRecordUnitSelfCheckResult;
	byte bCIRSelfCheckFlag;		//1:450MHz通道启动遥测；2:GPRS通道启动遥测；3:MMI1启动自检；4:MMI2启动自检；
	bool bCIRSelfChecking ;
	int iGPRSCheckFlag;		//GPRS活动性检测标志，0关闭，1打开
	byte bGPRSCheckNormalFlag ;
	int iGPRSCheckCounter ;
	byte bGPRSCheckRepeat ;

	byte bLastSignalMachineType ;			//默认为编组站
	byte bTrainNumberForceFlag ;
	byte bModeChangeFileProtectFlag ;
	byte arrLineCode[2] ;
	int iTrainNumberInfoTxSum ;
	int iTrainNumberInfoToNowGRISTxCounter ;
	int iNowTrainNumberInfoTxCounter ;
	int iTrainNumberInfoStartOrStopTxSum ;
	int iTrainNumberInfoStartOrStopToNowGRISTxCounter ;
	int iNowTrainNumberInfoStartOrStopTxCounter ;

	std::string TrainNumberTemp;//lkj中输入的只用于弹框显示,用于注册
	std::string TrainNumberCmp;
	byte bTAXTNChange;
	byte bTrainNumberBenBuTemp;
	byte bTrainNumberBenBuCmp;
	byte bTrainNumberBenBuT4;
	byte bTrainNumberBenBuT3;
	byte bTrainNumberBenBuT2;
	byte bTrainNumberBenBuT1;
	std::string TrainNumberT1;
	std::string TrainNumberT2;
	std::string TrainNumberT3;
	std::string TrainNumberT4;
	std::string EngineNumberTemp;
	std::string EngineNumberT1;
	std::string EngineNumberT2;
	std::string EngineNumberT3;
	std::string EngineNumberT4;
	byte bLastSignalType ;				//无效
	byte bSignalTypeTemp ;				//无效
	byte bTNFromMMI;
	byte bNeedLastRegisterFlag ;
	byte bTrainNumberStatusRxReportFlag ;
	byte bGPSInfoToLBJFlag;
	byte bCMRepeatRxAllowProcessFlag ;
	byte arrSelfCheckResultBuf[32];
	byte bSelfCheckResultCounter;

	int iSignalStrenghthenSgm1;
	int iSignalStrenghthenRealSgm1;
	int iSignalStrenghthenSgm2;
	int iSignalStrenghthenRealSgm2;

	std::string strMrmLac;
	std::string strMrmCi;
	std::string strGprsLac;
	std::string strGprsCi;
	byte bHookFlag ; //0挂机
	byte bPttFlag ; //0按下

	std::string strSelectOperatorName ;

	byte bGprsCheckNormalFlag;//1:GPRS活动性检测正常，2:GPRS活动性检测故障
	//当前公里标BCD码
	byte arrCurrentKilometer[4];
	byte arrHexKilometer[3];
	byte bKilometerFlag;

	byte arrCurrentLongitude[5];
	byte arrCurrentLatitude[4];

	byte bDisptchCommandStatus;

	byte arrCurrentDispatchCommandTrainNumber[10];
	byte arrCurrentDispatchCommandEngineNumber[10];

	short m_Last_File[6];

private:
	static TrainState* mInstance;
	TrainState();
};

#endif /* TRAINSTATE_H_ */
