/*
 * FrameGenerate.cpp
 *
 *  Created on: 2016-8-31
 *      Author: lenovo
 */

#include "FrameGenerate.h"

#include <vector>
#include <iostream>
using namespace std;

FrameGenerate::FrameGenerate()
{
	mTrainState = TrainState::GetInstance();
}

FrameGenerate::~FrameGenerate()
{

}

void FrameGenerate::GetFrame_AckInfo(_SendData& pSendData,byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType, _RawInfo& PackageInfo)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡

	bFeature &= MSK4;//0����Ҫ��Ӧ��

	pSendData.bNeedAckFlag =bNeedAckFlag;// ACK0;
	pSendData.bPriority =bPriority;// PRIORITY3;
	pSendData.bZongXian = bZongXian;//PackageInfo.bZongXian;
	pSendData.bDestCode =bDestCode;// PackageInfo.bSourCode;
	pSendData.bType =bFrameType;// SHORTFRAME;

	bFeature |= BIT5;//1������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(PackageInfo.bSourCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(PackageInfo.bServiceType);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x41);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(PackageInfo.bCommand);// ��һ�ֽ�
	pSendData.iLength++;

	pSendData.vSendData.push_back(PackageInfo.bSequence);//�ڶ��ֽ�
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;
}

void FrameGenerate::GetFrame_CanState(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x07);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(mTrainState->bZongXianState);//
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//δ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;
}

void FrameGenerate::GetFrame_RestBoard(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType,byte bDeviceCode)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x12);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(bDeviceCode);//
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//δ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;
}

void FrameGenerate::GetFrame_ChangeMainUsedState(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType ,byte bData)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x06);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(bData);
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//δ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;
}

void FrameGenerate::GetFrame_StartReport(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
		,byte bFrameType,byte bData)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x02);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(bData);// ���ʹ���
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//δ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;

}

void FrameGenerate::GetFrame_StopPermit(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;
	bFeature |= BIT5;//1������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x04);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//δ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//δ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;

}
void FrameGenerate::GetFrame_DeviceState(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Դͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Ŀ��ͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE3);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x02);// ����
	pSendData.iLength++;

	// ��1�ֽڣ�����״̬
	pSendData.vSendData.push_back(mTrainState->bWorkState);
	pSendData.iLength++;
	//��2~7�ֽڣ�ϵͳ����ʱ��
	time_t now = time(NULL);
	struct tm *timenow = localtime(&now);
	pSendData.vSendData.push_back((byte)(timenow->tm_year-100));//00-99����ݼ���1900
	pSendData.iLength++;
	pSendData.vSendData.push_back((byte)timenow->tm_mon+1);  //1-12
	pSendData.iLength++;
	pSendData.vSendData.push_back((byte)timenow->tm_mday);   //1-31
	pSendData.iLength++;
	pSendData.vSendData.push_back((byte)timenow->tm_hour);   //0-23
	pSendData.iLength++;
	pSendData.vSendData.push_back((byte)timenow->tm_min);    //0-59
	pSendData.iLength++;
	pSendData.vSendData.push_back((byte)timenow->tm_sec);    //0-59
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrDeviceState[0]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrDeviceState[1]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrDeviceState[2]);
	pSendData.iLength++;

	pSendData.vSendData.push_back(0x00);
	pSendData.iLength++;//Ԥ��


	byte bState = 0x00;
	if(mTrainState->bZhuKongFlag == (byte)BOARD_ZK_LEFT && mTrainState->bMainUsed)
	{
		bState |= BIT0;//
		bState &= (~BIT1);//
	}
	else if(mTrainState->bZhuKongFlag == (byte)BOARD_ZK_LEFT && !mTrainState->bMainUsed)
	{
		bState |= BIT1;//01H�����Ƶ�Ԫ1Ϊ���� 02H�����Ƶ�Ԫ2Ϊ����
		bState &= (~BIT0);//
	}
	else if(mTrainState->bZhuKongFlag == (byte)BOARD_ZK_RIGHT && mTrainState->bMainUsed)
	{
		bState |= BIT1;////01H�����Ƶ�Ԫ1Ϊ���� 02H�����Ƶ�Ԫ2Ϊ����
		bState &= (~BIT0);//
	}
	else if(mTrainState->bZhuKongFlag == (byte)BOARD_ZK_RIGHT && !mTrainState->bMainUsed)
	{
		bState |= BIT0;//
		bState &= (~BIT1);//
	}
	if(mTrainState->bZongXian == CAN0)
	{
		bState |= BIT2;//
		bState &=(~BIT3);//
	}
	else
	{
		bState &=(~BIT2);//
		bState |= BIT3;//
	}
	if((mTrainState->bSgm1Actort == MRM_MODE && mTrainState->bSgm2Actort == GPRS_MODE) || (mTrainState->bSgm1Actort == GPRS_MODE && mTrainState->bSgm2Actort ==  MRM_MODE) )
	{//sgm��ɫ
		bState |= BIT4;//
		bState &=(~BIT5);//
	}
	else if(mTrainState->bSgm1Actort == ALL_MODE && mTrainState->bSgm2Actort == BROKEN_MODE )
	{
		bState &=(~BIT4);//
		bState |= BIT5;//
	}
	else if(mTrainState->bSgm1Actort == BROKEN_MODE  && mTrainState->bSgm2Actort ==   ALL_MODE)
	{
		bState |= BIT4;//
		bState |= BIT5;//
	}
	else
	{
		std::cout << "please attention, sgm mode is wrong!!!"<<std::endl;
	}
	if(mTrainState->bMainMMI == BOARD_MMI_LEFT)
	{//mmi��ɫ
		bState |= BIT6;//
		bState &=(~BIT7);//
	}
	else if(mTrainState->bMainMMI == BOARD_MMI_RIGHT)
	{
		bState &=(~BIT6);//
		bState |= BIT7;//
	}
	else
	{//02
		bState &=(~BIT6);
		bState &=(~BIT7);
	}

	pSendData.vSendData.push_back(bState);
	pSendData.iLength++;

	pSendData.vSendData.push_back(0x00);
	pSendData.iLength++;//Ԥ��
	pSendData.vSendData.push_back(0x00);
	pSendData.iLength++;//Ԥ��

	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}
void FrameGenerate::GetFrame_IntegeInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Դͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Ŀ��ͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE3);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// ����
	pSendData.iLength++;

	// ��1�ֽڣ�����״̬
	pSendData.vSendData.push_back(mTrainState->arrInitStatus[0]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrInitStatus[3]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->iSignalStrenghthenSgm1);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->iSignalStrenghthenSgm2);
	pSendData.iLength++;
	pSendData.vSendData.push_back((byte)(mTrainState->iWorkMode));
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrInitStatus[4]);
	pSendData.iLength++;
	pSendData.vSendData.push_back((byte)(mTrainState->iWorkType));
	pSendData.iLength++;

	pSendData.vSendData.push_back(mTrainState->arrSourAddre[0]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrSourAddre[1]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrSourAddre[2]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrSourAddre[3]);
	pSendData.iLength++;

	pSendData.vSendData.push_back(mTrainState->arrDMISAddre[0]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrDMISAddre[1]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrDMISAddre[2]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrDMISAddre[3]);
	pSendData.iLength++;

	pSendData.vSendData.push_back(mTrainState->arrTailAddre[0]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrTailAddre[1]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrTailAddre[2]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrTailAddre[3]);
	pSendData.iLength++;

	pSendData.vSendData.push_back(mTrainState->arrHomeDMISAddre[0]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrHomeDMISAddre[1]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrHomeDMISAddre[2]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrHomeDMISAddre[3]);
	pSendData.iLength++;

	pSendData.vSendData.push_back(mTrainState->arrGrosAddre[0]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrGrosAddre[1]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrGrosAddre[2]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrGrosAddre[3]);
	pSendData.iLength++;

	pSendData.vSendData.push_back(mTrainState->arrGrosAddre1[0]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrGrosAddre1[1]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrGrosAddre1[2]);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrGrosAddre1[3]);
	pSendData.iLength++;


	for(int i=0; i<mTrainState->strDisplayTrainNumber.length(); i++)
	{
		pSendData.vSendData.push_back(mTrainState->strDisplayTrainNumber[i]);
		pSendData.iLength++;
	}
	pSendData.vSendData.push_back(';');
	pSendData.iLength++;

	for(int i=0; i<mTrainState->strEngineNumber.length(); i++)
	{
		pSendData.vSendData.push_back(mTrainState->strEngineNumber[i]);
		pSendData.iLength++;
	}
	pSendData.vSendData.push_back(';');
	pSendData.iLength++;

	for(int i=0; i<mTrainState->strLineName.length(); i++)
	{
		pSendData.vSendData.push_back(mTrainState->strLineName[i]);
		pSendData.iLength++;
	}
	pSendData.vSendData.push_back(';');
	pSendData.iLength++;

	for(int i=0; i<mTrainState->strApn.length(); i++)
	{
		pSendData.vSendData.push_back(mTrainState->strApn[i]);
		pSendData.iLength++;
	}
	pSendData.vSendData.push_back(';');
	pSendData.iLength++;

	for(int i=0; i<mTrainState->TrainNumberTemp.length(); i++)
	{
		pSendData.vSendData.push_back(mTrainState->TrainNumberTemp[i]);
		pSendData.iLength++;
	}
	pSendData.vSendData.push_back(';');
	pSendData.iLength++;

	pSendData.vSendData.push_back(0x00);
	pSendData.iLength++;//Ԥ��
	pSendData.vSendData.push_back(0x00);
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x00);
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x00);
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x00);
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x00);
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}
void FrameGenerate::GetFrame_CallListToMmi(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,std::vector<_PToPCallStruct>& vPToPbuffer,std::vector<_GroupCallStruct>& vVGCbuffer)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Դͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Ŀ��ͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x0E);// ����
	pSendData.iLength++;

	for(int i=0;i<vPToPbuffer.size();i++)
	{
		byte temp = 0;
		if(vPToPbuffer[i].strPtpcsStat.compare("0") == 0)
			temp |= 0x80;	//��ǰ�绰
		else
			temp &= 0x7f;	//�ȴ��绰
		if(vPToPbuffer[i].strPtpcsDir.compare("0")==0)
			temp |= 0x40;	//����
		else
			temp &= 0xbf;	//����
		if(vPToPbuffer[i].strPtpcsStat.compare("0")==0)
			temp |= 0x20;	//ͨ����
		else
			temp &= 0xdf;
		temp |= 0x01;		//����
		pSendData.vSendData.push_back(temp);// ����
		pSendData.iLength++;

		if(!vPToPbuffer[i].strPtpcsPriority.empty())
		{
			pSendData.vSendData.push_back(vPToPbuffer[i].strPtpcsPriority[0]);
			pSendData.iLength++;
		}
		pSendData.vSendData.push_back(',');//
		pSendData.iLength++;

		if(!vPToPbuffer[i].strPtpcsUUS1.empty())
		{
			for(int j=0;j<vPToPbuffer[i].strPtpcsUUS1.size();j++)
			{
				pSendData.vSendData.push_back(vPToPbuffer[i].strPtpcsUUS1[j]);
				pSendData.iLength++;
			}
		}
		pSendData.vSendData.push_back(',');//
		pSendData.iLength++;

		if(!vPToPbuffer[i].strPtpcsNumber.empty())
		{
			for(int j=0;j<vPToPbuffer[i].strPtpcsNumber.size();j++)
			{
				pSendData.vSendData.push_back(vPToPbuffer[i].strPtpcsNumber[j]);
				pSendData.iLength++;
			}
		}
		pSendData.vSendData.push_back(',');//
		pSendData.iLength++;
	}
	for(int i=0;i<vVGCbuffer.size();i++)
	{
		byte temp = 0;
		if(vVGCbuffer[i].strGcStat.compare("0")==0)
			temp |= 0x80;	//��ǰ�绰
		else
			temp &= 0x7f;	//�ȴ��绰
		if(vVGCbuffer[i].strGcDir.compare("0")==0)
			temp |= 0x40;	//����
		else
			temp &= 0xbf;	//����
		if(vVGCbuffer[i].strGcStat.compare("0")==0)
			temp |= 0x20;	//ͨ����
		else
			temp &= 0xdf;
		if(vVGCbuffer[i].strGcStat.compare("2")==0)
			temp |= 0x10;	//δ����(incoming״̬)
		else
			temp &= 0xef;
		if(vVGCbuffer[i].strGcStat.compare("1")==0)
			temp |= 0x08;	//�ֶ��˳�(held״̬)
		else
			temp &= 0xf7;
		if(vVGCbuffer[i].strGcIs17Or18.compare("17")==0)
			temp |= 0x02;		//���
		else
			temp |= 0x04;		//�㲥
		pSendData.vSendData.push_back(temp);// ����
		pSendData.iLength++;
		if(!vVGCbuffer[i].strGcPriority.empty())
		{
			pSendData.vSendData.push_back(vVGCbuffer[i].strGcPriority[0]);
			pSendData.iLength++;
		}
		pSendData.vSendData.push_back(',');//
		pSendData.iLength++;

		if(!vVGCbuffer[i].strGcGCA.empty())
		{
			for(int j=0;j<vVGCbuffer[i].strGcGCA.size();j++)
			{
				pSendData.vSendData.push_back(vVGCbuffer[i].strGcGCA[j]);
				pSendData.iLength++;
			}
		}
		pSendData.vSendData.push_back(',');//
		pSendData.iLength++;

		if(vVGCbuffer[i].strGcGID.empty()!=0)
		{
			for(int j=0;j<vVGCbuffer[i].strGcGID.size();j++)
			{
				pSendData.vSendData.push_back(vVGCbuffer[i].strGcGID[j]);
				pSendData.iLength++;
			}
		}
		pSendData.vSendData.push_back(',');//
		pSendData.iLength++;
	}

	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}
void FrameGenerate::GetFrame_SetIoForGsmrModel(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
		,byte bFrameType,byte bData)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x21);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(bData);// ���Ƶ�ƽ
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//δ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;

}

void FrameGenerate::GetFrame_FunctionEngineNumberResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,TrainState* mTrainState,byte bResult, byte bReason,byte bNumber)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Դͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Ŀ��ͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x59);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(bResult); //���κš������ţ�ע�ᡢע���ɹ�
	pSendData.iLength++;
	pSendData.vSendData.push_back(bResult); //ԭ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(4-bNumber); //ע��ע������
	pSendData.iLength++;

	for(int i=0; i<mTrainState->strEngineNumber.length(); i++)
	{
		pSendData.vSendData.push_back(mTrainState->strEngineNumber[i]);
		pSendData.iLength++;
	}
	pSendData.vSendData.push_back(';');
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}

void FrameGenerate::GetFrame_FunctionTrainNumberResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,TrainState* mTrainState,byte bResult, byte bReason,byte bNumber)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Դͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Ŀ��ͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x58);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(bResult); //���κš������ţ�ע�ᡢע���ɹ�
	pSendData.iLength++;
	pSendData.vSendData.push_back(bResult); //ԭ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(4-bNumber); //ע��ע������
	pSendData.iLength++;

	for(int i=0; i<mTrainState->strTrainNumber.length(); i++)
	{
		pSendData.vSendData.push_back(mTrainState->strTrainNumber[i]);
		pSendData.iLength++;
	}
	pSendData.vSendData.push_back(';');
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}

void FrameGenerate::GetFrame_CallResponse(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
		,byte bFrameType,byte bData)
{
	std::cout<<GetCurrentTime()<<"GetFrame_CallResponse in"<<std::endl;

	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x0D);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(bData);// ���гɹ���ʧ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//δ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;

	std::cout<<GetCurrentTime()<<"GetFrame_CallResponse out"<<std::endl;
}

void FrameGenerate::GetFrame_PttState(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
		,byte bFrameType,byte bData)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x52);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(bData);// ���гɹ���ʧ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//δ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;

}

void FrameGenerate::GetFrame_SearchNetResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,bool bHaveFound,std::vector<std::string> & mAvailableOperatorNameList)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Դͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Ŀ��ͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x4F);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(';');
	pSendData.iLength++;

	for(int i=0; i<mAvailableOperatorNameList.size(); i++)
	{
		for(int j=0;j<mAvailableOperatorNameList[i].size();j++)
		{
			pSendData.vSendData.push_back(mAvailableOperatorNameList[i][j]);
			pSendData.iLength++;
		}
		pSendData.vSendData.push_back(';');
		pSendData.iLength++;
	}

	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}

void FrameGenerate::GetFrame_SearchOperatorResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,int iResult, std::string operatorname)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Դͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Ŀ��ͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x54);// ����//������MMI��������ѡ����
	pSendData.iLength++;

	if(iResult == 0)
	{
		byte temp[3];
		if(operatorname.compare("46000") == 0)//�й��ƶ�
			temp[0] = 0x01;
		else if(operatorname.compare("46001") == 0)//�й���ͨ
			temp[0] = 0x03;
		else
			temp[0] = 0x02;//�й���·GSMR

		temp[1] = temp[1] & (~BIT7);
		temp[1] = temp[1] | BIT6;
		temp[1] = temp[1] & (~BIT1);
		temp[1] = temp[1] | BIT0;
		temp[2] = 0x02;
		pSendData.vSendData.push_back(temp[0] );//CRC16
		pSendData.iLength++;
		pSendData.vSendData.push_back(temp[1] );//CRC16//GSM����ģ��ע����ɣ��쳣
		pSendData.iLength++;
		pSendData.vSendData.push_back(temp[2] );//CRC16//ѡ��ʧ��
		pSendData.iLength++;
	}
	else if (iResult == 1)
	{
		if(operatorname.compare("46000") == 0)//�й��ƶ�
			mTrainState->arrInitStatus[0] = 0x01;
		else if(operatorname.compare("46001") == 0)//�й���ͨ
			mTrainState->arrInitStatus[0] = 0x03;
		else
			mTrainState->arrInitStatus[0] = 0x02;//�й���·GSMR

		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | (BIT7);//GSM����ģ������ע��
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT6);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT1);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | BIT0;

		pSendData.vSendData.push_back(mTrainState->arrInitStatus[0] );//CRC16
		pSendData.iLength++;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[1] );//CRC16
		pSendData.iLength++;
	}
	else if (iResult == 2)
	{
		if(operatorname.compare("46000") == 0)//�й��ƶ�
			mTrainState->arrInitStatus[0] = 0x01;
		else if(operatorname.compare("46001") == 0)//�й���ͨ
			mTrainState->arrInitStatus[0] = 0x03;
		else
			mTrainState->arrInitStatus[0] = 0x02;//�й���·GSMR

		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT7);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | (BIT6);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT1);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | BIT0;
		mTrainState->arrInitStatus[2] = 0x02;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[0] );//
		pSendData.iLength++;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[1] );////GSM����ģ��ע����ɣ��쳣
		pSendData.iLength++;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[2] );////ѡ��ʧ��
		pSendData.iLength++;
	}
	else if (iResult == 3)
	{
		if(operatorname.compare("46000") == 0)//�й��ƶ�
			mTrainState->arrInitStatus[0] = 0x01;
		else if(operatorname.compare("46001") == 0)//�й���ͨ
			mTrainState->arrInitStatus[0] = 0x03;
		else
			mTrainState->arrInitStatus[0] = 0x02;//�й���·GSMR

		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT7);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT6);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT1);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | BIT0;

		pSendData.vSendData.push_back(mTrainState->arrInitStatus[0] );//
		pSendData.iLength++;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[1] );////GSM����ģ��ע����ɣ��ɹ�
		pSendData.iLength++;
	}
	else if (iResult == 4)
	{
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | (BIT7);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | (BIT6);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT1);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | BIT0;
		mTrainState->arrInitStatus[2] = 0x02;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[0] );//
		pSendData.iLength++;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[1] );////GSM����ģ��ע����ɣ��쳣
		pSendData.iLength++;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[2] );////ѡ��ʧ��
		pSendData.iLength++;
	}
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}
void FrameGenerate::GetFrame_ATCommandToZhuanHuan(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,string strCommand)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Դͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// Ŀ��ͨ�ŵ�ַ����
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x01);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x01);// ����
	pSendData.iLength++;
	int i=0;
	for( i=0; i<strCommand.size(); i++)
	{
		pSendData.vSendData.push_back(strCommand[i]);
		pSendData.iLength++;
	}

	pSendData.vSendData.push_back(0x0d);
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x0a);
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}
void FrameGenerate::GetFrame_InitNetworkInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
		,byte bFrameType)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x54);// ����
	pSendData.iLength++;

	pSendData.vSendData.push_back(mTrainState->arrInitStatus[0]);//
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrInitStatus[1]);//
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->arrInitStatus[2]);//
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.vSendData[2] = (pSendData.iLength % 256);
}

void FrameGenerate::GetFrame_DispatchCommand(_SendData& pSendData , byte bDestCode, byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType, _RawInfo& PackageInfo)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01��3.0����֡
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0����Ҫ��Ӧ��
	}
	else
	{
		bFeature |= BIT4;//1��Ҫ��Ӧ��
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0������֡
	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;

	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// Դ�˿�
	pSendData.iLength++;

	if(PackageInfo.bSourAddreLenth == 0)
	{
		pSendData.vSendData.push_back(0);//
		pSendData.iLength++;
	}
	else
	{
		pSendData.vSendData.push_back(PackageInfo.bSourAddreLenth);//
		pSendData.iLength++;
		for(int i=0; i < PackageInfo.bSourAddreLenth; i++)
		{
			pSendData.vSendData.push_back(PackageInfo.arrSourAddre[i]);//
			pSendData.iLength++;
		}
	}

	pSendData.vSendData.push_back(bDestCode);// Ŀ�Ķ˿�
	pSendData.iLength++;

	if(PackageInfo.bDectAddreLenth == 0)
	{
		pSendData.vSendData.push_back(0);//
		pSendData.iLength++;
	}
	else
	{
		pSendData.vSendData.push_back(PackageInfo.bDectAddreLenth);//
		pSendData.iLength++;
		for(int i=0; i < PackageInfo.bDectAddreLenth; i++)
		{
			pSendData.vSendData.push_back(PackageInfo.arrDectAddre[i]);//
			pSendData.iLength++;
		}
	}

	pSendData.vSendData.push_back(PackageInfo.bServiceType);// ҵ������
	pSendData.iLength++;
	pSendData.vSendData.push_back(PackageInfo.bCommand);// ����
	pSendData.iLength++;

	for(int i = 0; i < PackageInfo.vRawInfoData.size(); i++)
	{
		pSendData.vSendData.push_back(PackageInfo.vRawInfoData[i]);
		pSendData.iLength++;
	}

	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //������Ҫ�޸�
	pSendData.vSendData[2] = (pSendData.iLength % 256);
}
//void FrameGenerate::GetFrame_DispatchCommandAutoAck(_SendDataNet& pSendDataNet , byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType, _RawInfo& PackageInfo)
//{
//	byte bFeature = 0x00;
//	bFeature |= BIT6;//01��3.0����֡
//	if(bNeedAckFlag ==0 )
//	{
//		bFeature &= MSK4;//0����Ҫ��Ӧ��
//	}
//	else
//	{
//		bFeature |= BIT4;//1��Ҫ��Ӧ��
//	}
//	pSendData.bNeedAckFlag = bNeedAckFlag;
//	pSendData.bPriority = bPriority;
//	pSendData.bZongXian = bZongXian;
//	pSendData.bDestCode = PackageInfo.bSourCode;
//	pSendData.bType = bFrameType;
//
//	bFeature &= MSK5;//0������֡
//
//	pSendData.vSendData.push_back(bFeature);//������Ҫ�޸�:�ط���ʶ+��ˮ��(gprs������ʱ���Ӹ��ֽ�)
//	pSendData.iLength++;
//
//	pSendData.vSendData.push_back(pSendData.iLength / 256); //������Ҫ�޸�
//	pSendData.iLength++;
//	pSendData.vSendData.push_back(pSendData.iLength % 256);
//	pSendData.iLength++;
//
//	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);//Դ�˿�
//	pSendData.iLength++;
//
//	if(PackageInfo.bDectAddreLenth == 0)
//	{//Ŀ�ı�Դ
//		pSendData.vSendData.push_back(0);//
//		pSendData.iLength++;
//	}
//	else
//	{
//		pSendData.vSendData.push_back(PackageInfo.bDectAddreLenth);//
//		pSendData.iLength++;
//		for(int i=0; i < PackageInfo.bDectAddreLenth; i++)
//		{
//			pSendData.vSendData.push_back(PackageInfo.arrDectAddre[i]);//
//			pSendData.iLength++;
//		}
//	}
//
//	pSendData.vSendData.push_back(PackageInfo.bSourCode);// Դ��Ŀ��
//	pSendData.iLength++;
//
//	if(PackageInfo.bSourAddreLenth == 0)
//	{//Դ��Ŀ��
//		pSendData.vSendData.push_back(0);//
//		pSendData.iLength++;
//	}
//	else
//	{
//		pSendData.vSendData.push_back(PackageInfo.bSourAddreLenth);//
//		pSendData.iLength++;
//		for(int i=0; i < PackageInfo.bSourAddreLenth; i++)
//		{
//			pSendData.vSendData.push_back(PackageInfo.arrSourAddre[i]);//
//			pSendData.iLength++;
//		}
//	}
//
//	pSendData.vSendData.push_back(0x06);// ҵ������
//	pSendData.iLength++;
//	pSendData.vSendData.push_back(0x51);// ����
//	pSendData.iLength++;
//
//	//��Ϣ����:���������Զ�Ӧ��
//	pSendData.vSendData.push_back(0x81);
//	pSendData.iLength++;
//	//������/������/ʱ����
//	for(int i = 0; i < 7; i++)
//	{
//		pSendData.vSendData.push_back(PackageInfo.vRawInfoData[i]);
//		pSendData.iLength++;
//	}
//	//���κ�
//	if(mTrainState->strTrainNumber.compare("XXXXXXX") != 0)
//	{
//		for(int i = 0; i < 7; i++)
//		{
//			pSendData.vSendData.push_back(mTrainState->strTrainNumber[i]);
//			pSendData.iLength++;
//		}
//	}
//	else
//	{
//		for(int j =10, i = 0; i < 7; i++)
//		{
//			pSendData.vSendData.push_back(PackageInfo.vRawInfoData[j+i]);
//			pSendData.iLength++;
//		}
//	}
//	//������
//	for(int i = 0; i < 8; i++)
//	{
//		pSendData.vSendData.push_back(mTrainState->strEngineNumber[i]);
//		pSendData.iLength++;
//	}
//	//�������/������
//	for(int j =25, i = 0; i < 7; i++)
//	{
//		pSendData.vSendData.push_back(PackageInfo.vRawInfoData[i]);
//		pSendData.iLength++;
//	}
//	//�����
//	for(int i = 0; i < 3; i++)
//	{
//		pSendData.vSendData.push_back(mTrainState->arrHexKilometer[i]);
//		pSendData.iLength++;
//	}
//	//����
//	for(int i = 0; i < 5; i++)
//	{
//		pSendData.vSendData.push_back(mTrainState->arrCurrentLongitude[i]);
//		pSendData.iLength++;
//	}
//	//γ��
//	for(int i = 0; i < 4; i++)
//	{
//		pSendData.vSendData.push_back(mTrainState->arrCurrentLatitude[i]);
//		pSendData.iLength++;
//	}
//	//������Ÿ��ֽڵ�5��Ԥ���ֽ�
//	for(int j =41, i = 0; i < 5; i++)
//	{
//		pSendData.vSendData.push_back(PackageInfo.vRawInfoData[i]);
//		pSendData.iLength++;
//	}
//	//������  ע�⣺֮ǰ��3��Ԥ���ֽ�δ��ֵ
//	pSendData.vSendData.push_back(PackageInfo.vRawInfoData[47]);
//	pSendData.iLength++;
//
//	pSendData.vSendData.push_back(0xFF);//CRC16
//	pSendData.iLength++;
//	pSendData.vSendData.push_back(0xFF);//CRC16
//	pSendData.iLength++;
//
//	pSendData.vSendData[1] = (pSendData.iLength / 256); //������Ҫ�޸�
//	pSendData.vSendData[2] = (pSendData.iLength % 256);
//}
void FrameGenerate::GetFrame_DispatchCommandAutoAck(_SendDataNet& pSendDataNet , _RawInfo& PackageInfo , std::string strDestinationIp,  int iPort)
{
	pSendDataNet.strDestinationIPAddress = strDestinationIp;
	pSendDataNet.iDestinationPort = iPort;

	//pSendDataNet.vSendData.push_back(0);//������Ҫ�޸�:�ط���ʶ+��ˮ��(gprs������ʱ���Ӹ��ֽ�)
	pSendDataNet.vSendData.push_back(0); //������Ҫ�޸�
	pSendDataNet.vSendData.push_back(0);

	pSendDataNet.vSendData.push_back(mTrainState->bZhuKongFlag);//Դ�˿�

	if(PackageInfo.bDectAddreLenth == 0)
	{//Ŀ�ı�Դ
		pSendDataNet.vSendData.push_back(0);//
	}
	else
	{
		pSendDataNet.vSendData.push_back(PackageInfo.bDectAddreLenth);//
		for(int i=0; i < PackageInfo.bDectAddreLenth; i++)
		{
			pSendDataNet.vSendData.push_back(PackageInfo.arrDectAddre[i]);//
		}
	}

	pSendDataNet.vSendData.push_back(PackageInfo.bSourCode);// Դ��Ŀ��

	if(PackageInfo.bSourAddreLenth == 0)
	{//Դ��Ŀ��
		pSendDataNet.vSendData.push_back(0);//
	}
	else
	{
		pSendDataNet.vSendData.push_back(PackageInfo.bSourAddreLenth);//
		for(int i=0; i < PackageInfo.bSourAddreLenth; i++)
		{
			pSendDataNet.vSendData.push_back(PackageInfo.arrSourAddre[i]);//
		}
	}

	pSendDataNet.vSendData.push_back(0x06);// ҵ������
	pSendDataNet.vSendData.push_back(0x51);// ����

	//��Ϣ����:���������Զ�Ӧ��
	pSendDataNet.vSendData.push_back(0x81);
	//������/������/ʱ����
	for(int i = 0; i < 7; i++)
	{
		pSendDataNet.vSendData.push_back(PackageInfo.vRawInfoData[i]);
	}
	//���κ�
	if(mTrainState->strTrainNumber.compare("XXXXXXX") != 0)
	{
		for(int i = 0; i < 7; i++)
		{
			pSendDataNet.vSendData.push_back(mTrainState->strTrainNumber[i]);
		}
	}
	else
	{
		for(int j =10, i = 0; i < 7; i++)
		{
			pSendDataNet.vSendData.push_back(PackageInfo.vRawInfoData[j+i]);
		}
	}
	//������
	for(int i = 0; i < 8; i++)
	{
		pSendDataNet.vSendData.push_back(mTrainState->strEngineNumber[i]);
	}
	//�������/������
	for(int j =25, i = 0; i < 7; i++)
	{
		pSendDataNet.vSendData.push_back(PackageInfo.vRawInfoData[i]);
	}
	//�����
	for(int i = 0; i < 3; i++)
	{
		pSendDataNet.vSendData.push_back(mTrainState->arrHexKilometer[i]);
	}
	//����
	for(int i = 0; i < 5; i++)
	{
		pSendDataNet.vSendData.push_back(mTrainState->arrCurrentLongitude[i]);
	}
	//γ��
	for(int i = 0; i < 4; i++)
	{
		pSendDataNet.vSendData.push_back(mTrainState->arrCurrentLatitude[i]);
	}
	//������Ÿ��ֽڵ�5��Ԥ���ֽ�
	for(int j =41, i = 0; i < 5; i++)
	{
		pSendDataNet.vSendData.push_back(PackageInfo.vRawInfoData[i]);
	}
	//������  ע�⣺֮ǰ��3��Ԥ���ֽ�δ��ֵ
	pSendDataNet.vSendData.push_back(PackageInfo.vRawInfoData[47]);

	pSendDataNet.vSendData.push_back(0xFF);//CRC16
	pSendDataNet.vSendData.push_back(0xFF);//CRC16

	pSendDataNet.vSendData[1] = (pSendDataNet.vSendData.size() / 256); //������Ҫ�޸�
	pSendDataNet.vSendData[2] = (pSendDataNet.vSendData.size() % 256);
}
