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
	bFeature |= BIT6;//01：3.0数据帧

	bFeature &= MSK4;//0：不要求应答

	pSendData.bNeedAckFlag =bNeedAckFlag;// ACK0;
	pSendData.bPriority =bPriority;// PRIORITY3;
	pSendData.bZongXian = bZongXian;//PackageInfo.bZongXian;
	pSendData.bDestCode =bDestCode;// PackageInfo.bSourCode;
	pSendData.bType =bFrameType;// SHORTFRAME;

	bFeature |= BIT5;//1：精简帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(PackageInfo.bSourCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(PackageInfo.bServiceType);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x41);// 命令
	pSendData.iLength++;

	pSendData.vSendData.push_back(PackageInfo.bCommand);// 第一字节
	pSendData.iLength++;

	pSendData.vSendData.push_back(PackageInfo.bSequence);//第二字节
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;
}

void FrameGenerate::GetFrame_CanState(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1：精简帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x07);// 命令
	pSendData.iLength++;

	pSendData.vSendData.push_back(mTrainState->bZongXianState);//
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//未用
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;
}

void FrameGenerate::GetFrame_RestBoard(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType,byte bDeviceCode)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1：精简帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x12);// 命令
	pSendData.iLength++;

	pSendData.vSendData.push_back(bDeviceCode);//
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//未用
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;
}

void FrameGenerate::GetFrame_ChangeMainUsedState(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType ,byte bData)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1：精简帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x06);// 命令
	pSendData.iLength++;

	pSendData.vSendData.push_back(bData);
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//未用
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;
}

void FrameGenerate::GetFrame_StartReport(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
		,byte bFrameType,byte bData)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1：精简帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x02);// 命令
	pSendData.iLength++;

	pSendData.vSendData.push_back(bData);// 发送次数
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//未用
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;

}

void FrameGenerate::GetFrame_StopPermit(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;
	bFeature |= BIT5;//1：精简帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x04);// 命令
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//未用
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//未用
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;

}
void FrameGenerate::GetFrame_DeviceState(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0：基础帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //后面需要修改
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 源通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 目的通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE3);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x02);// 命令
	pSendData.iLength++;

	// 第1字节：工作状态
	pSendData.vSendData.push_back(mTrainState->bWorkState);
	pSendData.iLength++;
	//第2~7字节：系统本地时间
	time_t now = time(NULL);
	struct tm *timenow = localtime(&now);
	pSendData.vSendData.push_back((byte)(timenow->tm_year-100));//00-99，年份加上1900
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
	pSendData.iLength++;//预留


	byte bState = 0x00;
	if(mTrainState->bZhuKongFlag == (byte)BOARD_ZK_LEFT && mTrainState->bMainUsed)
	{
		bState |= BIT0;//
		bState &= (~BIT1);//
	}
	else if(mTrainState->bZhuKongFlag == (byte)BOARD_ZK_LEFT && !mTrainState->bMainUsed)
	{
		bState |= BIT1;//01H：控制单元1为主用 02H：控制单元2为主用
		bState &= (~BIT0);//
	}
	else if(mTrainState->bZhuKongFlag == (byte)BOARD_ZK_RIGHT && mTrainState->bMainUsed)
	{
		bState |= BIT1;////01H：控制单元1为主用 02H：控制单元2为主用
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
	{//sgm角色
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
	{//mmi角色
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
	pSendData.iLength++;//预留
	pSendData.vSendData.push_back(0x00);
	pSendData.iLength++;//预留

	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //后面需要修改
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}
void FrameGenerate::GetFrame_IntegeInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0：基础帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //后面需要修改
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 源通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 目的通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE3);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// 命令
	pSendData.iLength++;

	// 第1字节：工作状态
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
	pSendData.iLength++;//预留
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

	pSendData.vSendData[1] = (pSendData.iLength / 256); //后面需要修改
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}
void FrameGenerate::GetFrame_CallListToMmi(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,std::vector<_PToPCallStruct>& vPToPbuffer,std::vector<_GroupCallStruct>& vVGCbuffer)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0：基础帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //后面需要修改
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 源通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 目的通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x0E);// 命令
	pSendData.iLength++;

	for(int i=0;i<vPToPbuffer.size();i++)
	{
		byte temp = 0;
		if(vPToPbuffer[i].strPtpcsStat.compare("0") == 0)
			temp |= 0x80;	//当前电话
		else
			temp &= 0x7f;	//等待电话
		if(vPToPbuffer[i].strPtpcsDir.compare("0")==0)
			temp |= 0x40;	//主叫
		else
			temp &= 0xbf;	//被叫
		if(vPToPbuffer[i].strPtpcsStat.compare("0")==0)
			temp |= 0x20;	//通话中
		else
			temp &= 0xdf;
		temp |= 0x01;		//个呼
		pSendData.vSendData.push_back(temp);// 命令
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
			temp |= 0x80;	//当前电话
		else
			temp &= 0x7f;	//等待电话
		if(vVGCbuffer[i].strGcDir.compare("0")==0)
			temp |= 0x40;	//主叫
		else
			temp &= 0xbf;	//被叫
		if(vVGCbuffer[i].strGcStat.compare("0")==0)
			temp |= 0x20;	//通话中
		else
			temp &= 0xdf;
		if(vVGCbuffer[i].strGcStat.compare("2")==0)
			temp |= 0x10;	//未接听(incoming状态)
		else
			temp &= 0xef;
		if(vVGCbuffer[i].strGcStat.compare("1")==0)
			temp |= 0x08;	//手动退出(held状态)
		else
			temp &= 0xf7;
		if(vVGCbuffer[i].strGcIs17Or18.compare("17")==0)
			temp |= 0x02;		//组呼
		else
			temp |= 0x04;		//广播
		pSendData.vSendData.push_back(temp);// 命令
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

	pSendData.vSendData[1] = (pSendData.iLength / 256); //后面需要修改
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}
void FrameGenerate::GetFrame_SetIoForGsmrModel(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
		,byte bFrameType,byte bData)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1：精简帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xE0);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x21);// 命令
	pSendData.iLength++;

	pSendData.vSendData.push_back(bData);// 控制电平
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//未用
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;

}

void FrameGenerate::GetFrame_FunctionEngineNumberResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,TrainState* mTrainState,byte bResult, byte bReason,byte bNumber)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0：基础帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //后面需要修改
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 源通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 目的通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x59);// 命令
	pSendData.iLength++;

	pSendData.vSendData.push_back(bResult); //车次号、机车号，注册、注销成功
	pSendData.iLength++;
	pSendData.vSendData.push_back(bResult); //原因
	pSendData.iLength++;
	pSendData.vSendData.push_back(4-bNumber); //注册注销次数
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

	pSendData.vSendData[1] = (pSendData.iLength / 256); //后面需要修改
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}

void FrameGenerate::GetFrame_FunctionTrainNumberResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,TrainState* mTrainState,byte bResult, byte bReason,byte bNumber)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0：基础帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //后面需要修改
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 源通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 目的通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x58);// 命令
	pSendData.iLength++;

	pSendData.vSendData.push_back(bResult); //车次号、机车号，注册、注销成功
	pSendData.iLength++;
	pSendData.vSendData.push_back(bResult); //原因
	pSendData.iLength++;
	pSendData.vSendData.push_back(4-bNumber); //注册注销次数
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

	pSendData.vSendData[1] = (pSendData.iLength / 256); //后面需要修改
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}

void FrameGenerate::GetFrame_CallResponse(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
		,byte bFrameType,byte bData)
{
	std::cout<<GetCurrentTime()<<"GetFrame_CallResponse in"<<std::endl;

	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1：精简帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x0D);// 命令
	pSendData.iLength++;

	pSendData.vSendData.push_back(bData);// 呼叫成功或失败
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//未用
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;

	std::cout<<GetCurrentTime()<<"GetFrame_CallResponse out"<<std::endl;
}

void FrameGenerate::GetFrame_PttState(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
		,byte bFrameType,byte bData)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature |= BIT5;//1：精简帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x52);// 命令
	pSendData.iLength++;

	pSendData.vSendData.push_back(bData);// 呼叫成功或失败
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//未用
	pSendData.iLength++;

	pSendData.vSendData.push_back(0xFF);//CRC8
	pSendData.iLength++;

}

void FrameGenerate::GetFrame_SearchNetResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,bool bHaveFound,std::vector<std::string> & mAvailableOperatorNameList)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0：基础帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //后面需要修改
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 源通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 目的通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x4F);// 命令
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

	pSendData.vSendData[1] = (pSendData.iLength / 256); //后面需要修改
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}

void FrameGenerate::GetFrame_SearchOperatorResultInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,int iResult, std::string operatorname)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0：基础帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //后面需要修改
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 源通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 目的通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x54);// 命令//主机向MMI报告网络选择结果
	pSendData.iLength++;

	if(iResult == 0)
	{
		byte temp[3];
		if(operatorname.compare("46000") == 0)//中国移动
			temp[0] = 0x01;
		else if(operatorname.compare("46001") == 0)//中国联通
			temp[0] = 0x03;
		else
			temp[0] = 0x02;//中国铁路GSMR

		temp[1] = temp[1] & (~BIT7);
		temp[1] = temp[1] | BIT6;
		temp[1] = temp[1] & (~BIT1);
		temp[1] = temp[1] | BIT0;
		temp[2] = 0x02;
		pSendData.vSendData.push_back(temp[0] );//CRC16
		pSendData.iLength++;
		pSendData.vSendData.push_back(temp[1] );//CRC16//GSM语音模块注册完成，异常
		pSendData.iLength++;
		pSendData.vSendData.push_back(temp[2] );//CRC16//选网失败
		pSendData.iLength++;
	}
	else if (iResult == 1)
	{
		if(operatorname.compare("46000") == 0)//中国移动
			mTrainState->arrInitStatus[0] = 0x01;
		else if(operatorname.compare("46001") == 0)//中国联通
			mTrainState->arrInitStatus[0] = 0x03;
		else
			mTrainState->arrInitStatus[0] = 0x02;//中国铁路GSMR

		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | (BIT7);//GSM语音模块正在注册
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
		if(operatorname.compare("46000") == 0)//中国移动
			mTrainState->arrInitStatus[0] = 0x01;
		else if(operatorname.compare("46001") == 0)//中国联通
			mTrainState->arrInitStatus[0] = 0x03;
		else
			mTrainState->arrInitStatus[0] = 0x02;//中国铁路GSMR

		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT7);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | (BIT6);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT1);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | BIT0;
		mTrainState->arrInitStatus[2] = 0x02;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[0] );//
		pSendData.iLength++;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[1] );////GSM语音模块注册完成，异常
		pSendData.iLength++;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[2] );////选网失败
		pSendData.iLength++;
	}
	else if (iResult == 3)
	{
		if(operatorname.compare("46000") == 0)//中国移动
			mTrainState->arrInitStatus[0] = 0x01;
		else if(operatorname.compare("46001") == 0)//中国联通
			mTrainState->arrInitStatus[0] = 0x03;
		else
			mTrainState->arrInitStatus[0] = 0x02;//中国铁路GSMR

		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT7);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT6);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] & (~BIT1);
		mTrainState->arrInitStatus[1] = mTrainState->arrInitStatus[1] | BIT0;

		pSendData.vSendData.push_back(mTrainState->arrInitStatus[0] );//
		pSendData.iLength++;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[1] );////GSM语音模块注册完成，成功
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
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[1] );////GSM语音模块注册完成，异常
		pSendData.iLength++;
		pSendData.vSendData.push_back(mTrainState->arrInitStatus[2] );////选网失败
		pSendData.iLength++;
	}
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;
	pSendData.vSendData.push_back(0xFF);//CRC16
	pSendData.iLength++;

	pSendData.vSendData[1] = (pSendData.iLength / 256); //后面需要修改
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}
void FrameGenerate::GetFrame_ATCommandToZhuanHuan(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType
		,string strCommand)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0：基础帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //后面需要修改
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 源通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0);// 目的通信地址长度
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x01);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x01);// 命令
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

	pSendData.vSendData[1] = (pSendData.iLength / 256); //后面需要修改
	pSendData.vSendData[2] = (pSendData.iLength % 256);

}
void FrameGenerate::GetFrame_InitNetworkInfo(_SendData& pSendData, byte bDestCode,byte bZongXian, byte bPriority,byte bNeedAckFlag
		,byte bFrameType)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0：基础帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;
	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(bDestCode);// 目的端口
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x03);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(0x54);// 命令
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

	pSendData.vSendData[1] = (pSendData.iLength / 256); //后面需要修改
	pSendData.vSendData[2] = (pSendData.iLength % 256);
}

void FrameGenerate::GetFrame_DispatchCommand(_SendData& pSendData , byte bDestCode, byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType, _RawInfo& PackageInfo)
{
	byte bFeature = 0x00;
	bFeature |= BIT6;//01：3.0数据帧
	if(bNeedAckFlag ==0 )
	{
		bFeature &= MSK4;//0：不要求应答
	}
	else
	{
		bFeature |= BIT4;//1：要求应答
	}
	pSendData.bNeedAckFlag = bNeedAckFlag;
	pSendData.bPriority = bPriority;
	pSendData.bZongXian = bZongXian;
	pSendData.bDestCode = bDestCode;
	pSendData.bType = bFrameType;

	bFeature &= MSK5;//0：基础帧
	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号
	pSendData.iLength++;

	pSendData.vSendData.push_back(pSendData.iLength / 256); //后面需要修改
	pSendData.iLength++;
	pSendData.vSendData.push_back(pSendData.iLength % 256);
	pSendData.iLength++;

	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);// 源端口
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

	pSendData.vSendData.push_back(bDestCode);// 目的端口
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

	pSendData.vSendData.push_back(PackageInfo.bServiceType);// 业务类型
	pSendData.iLength++;
	pSendData.vSendData.push_back(PackageInfo.bCommand);// 命令
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

	pSendData.vSendData[1] = (pSendData.iLength / 256); //后面需要修改
	pSendData.vSendData[2] = (pSendData.iLength % 256);
}
//void FrameGenerate::GetFrame_DispatchCommandAutoAck(_SendDataNet& pSendDataNet , byte bZongXian, byte bPriority,byte bNeedAckFlag,byte bFrameType, _RawInfo& PackageInfo)
//{
//	byte bFeature = 0x00;
//	bFeature |= BIT6;//01：3.0数据帧
//	if(bNeedAckFlag ==0 )
//	{
//		bFeature &= MSK4;//0：不要求应答
//	}
//	else
//	{
//		bFeature |= BIT4;//1：要求应答
//	}
//	pSendData.bNeedAckFlag = bNeedAckFlag;
//	pSendData.bPriority = bPriority;
//	pSendData.bZongXian = bZongXian;
//	pSendData.bDestCode = PackageInfo.bSourCode;
//	pSendData.bType = bFrameType;
//
//	bFeature &= MSK5;//0：基础帧
//
//	pSendData.vSendData.push_back(bFeature);//后面需要修改:重发标识+流水号(gprs数据暂时不加该字节)
//	pSendData.iLength++;
//
//	pSendData.vSendData.push_back(pSendData.iLength / 256); //后面需要修改
//	pSendData.iLength++;
//	pSendData.vSendData.push_back(pSendData.iLength % 256);
//	pSendData.iLength++;
//
//	pSendData.vSendData.push_back(mTrainState->bZhuKongFlag);//源端口
//	pSendData.iLength++;
//
//	if(PackageInfo.bDectAddreLenth == 0)
//	{//目的变源
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
//	pSendData.vSendData.push_back(PackageInfo.bSourCode);// 源变目的
//	pSendData.iLength++;
//
//	if(PackageInfo.bSourAddreLenth == 0)
//	{//源变目的
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
//	pSendData.vSendData.push_back(0x06);// 业务类型
//	pSendData.iLength++;
//	pSendData.vSendData.push_back(0x51);// 命令
//	pSendData.iLength++;
//
//	//信息名称:调度命令自动应答
//	pSendData.vSendData.push_back(0x81);
//	pSendData.iLength++;
//	//功能码/年月日/时分秒
//	for(int i = 0; i < 7; i++)
//	{
//		pSendData.vSendData.push_back(PackageInfo.vRawInfoData[i]);
//		pSendData.iLength++;
//	}
//	//车次号
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
//	//机车号
//	for(int i = 0; i < 8; i++)
//	{
//		pSendData.vSendData.push_back(mTrainState->strEngineNumber[i]);
//		pSendData.iLength++;
//	}
//	//处所编号/命令编号
//	for(int j =25, i = 0; i < 7; i++)
//	{
//		pSendData.vSendData.push_back(PackageInfo.vRawInfoData[i]);
//		pSendData.iLength++;
//	}
//	//公里标
//	for(int i = 0; i < 3; i++)
//	{
//		pSendData.vSendData.push_back(mTrainState->arrHexKilometer[i]);
//		pSendData.iLength++;
//	}
//	//经度
//	for(int i = 0; i < 5; i++)
//	{
//		pSendData.vSendData.push_back(mTrainState->arrCurrentLongitude[i]);
//		pSendData.iLength++;
//	}
//	//纬度
//	for(int i = 0; i < 4; i++)
//	{
//		pSendData.vSendData.push_back(mTrainState->arrCurrentLatitude[i]);
//		pSendData.iLength++;
//	}
//	//处所编号高字节等5个预留字节
//	for(int j =41, i = 0; i < 5; i++)
//	{
//		pSendData.vSendData.push_back(PackageInfo.vRawInfoData[i]);
//		pSendData.iLength++;
//	}
//	//本包号  注意：之前的3个预留字节未赋值
//	pSendData.vSendData.push_back(PackageInfo.vRawInfoData[47]);
//	pSendData.iLength++;
//
//	pSendData.vSendData.push_back(0xFF);//CRC16
//	pSendData.iLength++;
//	pSendData.vSendData.push_back(0xFF);//CRC16
//	pSendData.iLength++;
//
//	pSendData.vSendData[1] = (pSendData.iLength / 256); //后面需要修改
//	pSendData.vSendData[2] = (pSendData.iLength % 256);
//}
void FrameGenerate::GetFrame_DispatchCommandAutoAck(_SendDataNet& pSendDataNet , _RawInfo& PackageInfo , std::string strDestinationIp,  int iPort)
{
	pSendDataNet.strDestinationIPAddress = strDestinationIp;
	pSendDataNet.iDestinationPort = iPort;

	//pSendDataNet.vSendData.push_back(0);//后面需要修改:重发标识+流水号(gprs数据暂时不加该字节)
	pSendDataNet.vSendData.push_back(0); //后面需要修改
	pSendDataNet.vSendData.push_back(0);

	pSendDataNet.vSendData.push_back(mTrainState->bZhuKongFlag);//源端口

	if(PackageInfo.bDectAddreLenth == 0)
	{//目的变源
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

	pSendDataNet.vSendData.push_back(PackageInfo.bSourCode);// 源变目的

	if(PackageInfo.bSourAddreLenth == 0)
	{//源变目的
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

	pSendDataNet.vSendData.push_back(0x06);// 业务类型
	pSendDataNet.vSendData.push_back(0x51);// 命令

	//信息名称:调度命令自动应答
	pSendDataNet.vSendData.push_back(0x81);
	//功能码/年月日/时分秒
	for(int i = 0; i < 7; i++)
	{
		pSendDataNet.vSendData.push_back(PackageInfo.vRawInfoData[i]);
	}
	//车次号
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
	//机车号
	for(int i = 0; i < 8; i++)
	{
		pSendDataNet.vSendData.push_back(mTrainState->strEngineNumber[i]);
	}
	//处所编号/命令编号
	for(int j =25, i = 0; i < 7; i++)
	{
		pSendDataNet.vSendData.push_back(PackageInfo.vRawInfoData[i]);
	}
	//公里标
	for(int i = 0; i < 3; i++)
	{
		pSendDataNet.vSendData.push_back(mTrainState->arrHexKilometer[i]);
	}
	//经度
	for(int i = 0; i < 5; i++)
	{
		pSendDataNet.vSendData.push_back(mTrainState->arrCurrentLongitude[i]);
	}
	//纬度
	for(int i = 0; i < 4; i++)
	{
		pSendDataNet.vSendData.push_back(mTrainState->arrCurrentLatitude[i]);
	}
	//处所编号高字节等5个预留字节
	for(int j =41, i = 0; i < 5; i++)
	{
		pSendDataNet.vSendData.push_back(PackageInfo.vRawInfoData[i]);
	}
	//本包号  注意：之前的3个预留字节未赋值
	pSendDataNet.vSendData.push_back(PackageInfo.vRawInfoData[47]);

	pSendDataNet.vSendData.push_back(0xFF);//CRC16
	pSendDataNet.vSendData.push_back(0xFF);//CRC16

	pSendDataNet.vSendData[1] = (pSendDataNet.vSendData.size() / 256); //后面需要修改
	pSendDataNet.vSendData[2] = (pSendDataNet.vSendData.size() % 256);
}
