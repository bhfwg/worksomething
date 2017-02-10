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

	byte bWorkState;//01H������   02H������   03H: ά��
	bool bMainUsed;//������

	byte bZhuKongFlag;//����ʶ�� ZHUKONG_LEFT  ZHUKONG_RIGHT
	byte bOtherBoardFlag;//����ʶ�� ZHUKONG_LEFT  ZHUKONG_RIGHT

	byte bZongXian;//��ǰ���ߺ�
	byte bZongXianState;//��ǰ����״̬

	byte arrDeviceState[3];

	byte bOtherboardState;//0=down 1=ok
	long lOtherBoardRecvTime;

	byte bSgm1State,bSgm1Actort;
	byte bSgm2State,bSgm2Actort;

	byte arrFrameNumId0[512];//�������ߵ���ˮ
	byte arrFrameNumId1[512];
	byte arrFrameNumFrame0[512];//������Ԫ��֡��ˮ
	byte arrFrameNumFrame1[512];//������Ԫ��֡��ˮ

	byte arrCanState[512];//��¼����Ԫ����״̬
	long lCan0RecvTime;
	long lCan1RecvTime;

	byte arrDeviceId[15];//�豸���  15λ����

	std::string strSoftwareVersion ; // ��������汾//version
	std::string strSimVersion ; // ����汾
	std::string strImsiVersion ; // ����汾

	std::string strSoftwareVersion2 ; // ��������汾//version
	std::string strSimVersion2 ; // ����汾
	std::string strImsiVersion2 ; // ����汾

	int iWorkType;//=00˫��������=01 ��������MMI=02 ����˫MMI
	int iWorkMode;
	std::string strLineName;
	int iModeFlag; //'0'�Զ���'1'�ֶ�
	int iShutDownStatus; //�ϴιػ�״̬ 0�ػ� 1�ǹػ�
	int iEnPort;//0 ��ͨ�����򶯳���  1:A��2:B

	std::string strEngineNumber;
	byte arrEngineNumber[10];
	int iEngineFlag;// 0:��TAX��õ� 1:��MMI�˹�����

	std::string strTrainNumber;
	byte arrTrainNumber[10];
	int iRegisterStatus;//0δע�� 1��ע��
	int iTrainFlag;//0:��TAX��õ� 1:��MMI�˹����� //ԭManual_Flag
	int iBenBuStatus;// ���������01234 //ԭStatus

	std::string strDisplayTrainNumber;
	int iDisplayTrainFlag;//0:��TAX��õ� 1:��MMI�˹����� //ԭManual_Flag
	int iDisplayBenBuStatus;// ���������01234 //ԭStatus

	std::string strLastTrainNumber;
	int iLastRegisterStatus;//0δע�� 1��ע��
	int iLastBenBuStatus;// ���������01234

	std::string strMsIsdn;

	std::string strCheckIp1;
	byte arrCheckIp1[4];
	std::string strCheckIp2;
	byte arrCheckIp2[4];
	std::string strCheckIp3;
	byte arrCheckIp3[4];//[2]%2==0 CAN0 //[2]==2 ����ӡcan����
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
	byte bMainMMI; //����MMI��Ϊ����
	byte bGPSStatus ;
	_CAnolog450 clsAnolog450;
	bool bNeedEnterCSDFlag,bNeedEnterGPRSFlag;
	byte arrBatteryVoltage[4];
	byte arrTemperature[2];
	byte bBatteryCheckStatus;
	byte arrInitStatus[5];
		// [0] :�й���·GSM-R-02H���й���ͨ-03H���й�����-04H������·LTE-05H
		// [1] :��ʼ��״̬�ֽ�1;
		// [2] :��ʼ��״̬�ֽ�2;
		// [3] :�ۺ���Ϣ״̬;(����״̬,���һλBIT0��ʶ�ֶ����Զ���BIT1Ϊ�������쳣�澯��BIT2Ϊ����״̬��BIT3Ϊ���κ�ע��ɹ���
		// 		BIT4������ע��ɹ�,BIT5��δ��ȡIP��ַ��BIT6��δ��ȡIP��ַ��BIT7��β����)
		// [4] :�ۺ���ϢԤ��״̬��������://BIT7:��������Դ1:TAX��(�Զ�)0:�ֶ���BIT6:��β�����жϱ�־1:��β�����ж�0:��β��������
		// 		BIT5:Ԥ�� ,BIT4:POC����״̬,BIT3:Ԥ�� BIT2-BIT0:����ǣ��״̬��00H: �����  ,01H: ����1, 02H: ����2, 03H: ���� 3, 04H: ����4
	byte arrCTCMaintenanceInfo[32];

	byte bOutputSignalStatus  ;
	int  iCIRSelfCheckStatus;
	byte bCIRSelfCheckWaitCounter,bCIRSelfCheckResultTxCounter;
	byte b450MStationSelfCheckResult,bLBJStationSelfCheckResult[2],bRecordUnitSelfCheckResult;
	byte bCIRSelfCheckFlag;		//1:450MHzͨ������ң�⣻2:GPRSͨ������ң�⣻3:MMI1�����Լ죻4:MMI2�����Լ죻
	bool bCIRSelfChecking ;
	int iGPRSCheckFlag;		//GPRS��Լ���־��0�رգ�1��
	byte bGPRSCheckNormalFlag ;
	int iGPRSCheckCounter ;
	byte bGPRSCheckRepeat ;

	byte bLastSignalMachineType ;			//Ĭ��Ϊ����վ
	byte bTrainNumberForceFlag ;
	byte bModeChangeFileProtectFlag ;
	byte arrLineCode[2] ;
	int iTrainNumberInfoTxSum ;
	int iTrainNumberInfoToNowGRISTxCounter ;
	int iNowTrainNumberInfoTxCounter ;
	int iTrainNumberInfoStartOrStopTxSum ;
	int iTrainNumberInfoStartOrStopToNowGRISTxCounter ;
	int iNowTrainNumberInfoStartOrStopTxCounter ;

	std::string TrainNumberTemp;//lkj�������ֻ���ڵ�����ʾ,����ע��
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
	byte bLastSignalType ;				//��Ч
	byte bSignalTypeTemp ;				//��Ч
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
	byte bHookFlag ; //0�һ�
	byte bPttFlag ; //0����

	std::string strSelectOperatorName ;

	byte bGprsCheckNormalFlag;//1:GPRS��Լ��������2:GPRS��Լ�����
	//��ǰ�����BCD��
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
