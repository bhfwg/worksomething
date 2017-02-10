/*
 * ParamOperation.cpp
 *
 *  Created on: 2016-8-17
 *      Author: lenovo
 */

#include "ParamOperation.h"

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

ParamOperation* ParamOperation::mInstance = NULL;

ParamOperation* ParamOperation::GetInstance()
{
	if(mInstance == NULL)
		mInstance = new ParamOperation();
	return mInstance;
}

ParamOperation::ParamOperation() {
	mTrainState = TrainState::GetInstance();
	mLogInstance = LogInstance::GetInstance();
	mapConfig.clear();
}

ParamOperation::~ParamOperation() {
}

void ParamOperation::SaveSpecialField(std::string name, std::string value )
{
	mLogInstance->Log(name+ ": "+value);
	mapConfig[name] = value;
	WriteConfigFile(PATHPARAMETERFILE);
}
bool ParamOperation::ReadConfigFile(std::string cfgfilepath)
{
	mapConfig.clear();
    fstream cfgFile;
    cfgFile.open(cfgfilepath.c_str());//打开文件
    if( !cfgFile.is_open())
    {
		//cout<<"can not open para.cfg file"<<endl;
        mLogInstance->Log("can not open para.cfg file");
        return false;
    }
    char tmp[512];
    while(!cfgFile.eof())//循环读取每一行
    {
        cfgFile.getline(tmp,512);//每行读取前512个字符
        string line(tmp);
        size_t pos = line.find('=');//找到每行的“=”号位置，之前是key之后是value
        if(pos==string::npos)
        {
        	//mLogInstance->Log("have no '=' in: " + line);
        	continue;
        }
        string key = line.substr(0,pos);//取=号之前
        string value = line.substr(pos+1);//取=号之后
        if(key.empty() || value.empty() )
        {
        	//cout<<"key or value is null in: "<<line<<endl;
        	mLogInstance->Log("key or value is null in: " + line);
        	continue;
        }
        else
        {
        	map<string ,string >::iterator l_it;
        	l_it=mapConfig.find(key);
        	if(l_it == mapConfig.end())
        	{
        		mapConfig.insert(map<string,string>::value_type(key,value));
        	}
        	else
        	{
        		//cout<<"duplicate key in: "<<line<<endl;
        		mLogInstance->Log("duplicate key in: " + line);
        	}
        }
    }
    cfgFile.close();
    return false;
}

bool ParamOperation::WriteConfigFile(std::string cfgfilepath)
{
    fstream cfgFile;
    cfgFile.open(cfgfilepath);//打开文件
    if( !cfgFile.is_open())
    {
        //cout<<"can not open para.cfg file"<<endl;
        mLogInstance->Log("can not open para.cfg file");
        return false;
    }

	map<string ,string >::iterator l_it = mapConfig.begin();
	while(l_it != mapConfig.end())
	{
		string str = l_it->first+"="+ l_it->second;
		cfgFile.write(str.c_str(),str.length());
		cfgFile<<std::endl;
		l_it++;
	}
	cfgFile.close();
	//mLogInstance->Log("save parameter right" );
    return true;
}


bool ParamOperation::FindInConfigFile( string  key, string  value)
{
	if( mapConfig.empty() || key.empty() || value.empty() )
		return false;

	map<string ,string >::iterator l_it;
	l_it = mapConfig.find(key);
	if(l_it == mapConfig.end())
	{
		return false;
	}
	else
	{
		return true;
	}
    return false;
}

bool ParamOperation::HandleParamFile()
{
	// 为分类文件的记数变量赋值
	// 0:DispatchCommand //"/LocomotiveMain/DispatchCommand/DispatchCommandCount.txt"
	fstream cfgFile;
	cfgFile.open(PATHDISPATCHCOMMANDFILE.c_str());//打开文件
	if( !cfgFile.is_open())
	{
		//cout<<"can not open file: "<<PATHDISPATCHCOMMANDFILE<<endl;
		mLogInstance->Log("can not open file: "+PATHDISPATCHCOMMANDFILE);
		return false;
	}
	char content[2];
	cfgFile.read(content,2);
	if(cfgFile.gcount() < 2)
	{
		mTrainState->m_Last_File[0] = 0;
	}
	else
	{
		mTrainState->m_Last_File[0] = content[0] + content[1]*256;
	}
	cfgFile.close();

	// 1:RuningToken "/LocomotiveMain/RuningToken/RuningTokenCount.txt"
	cfgFile.open(PATHRUNINGTOKENFILE.c_str());//打开文件
	if( !cfgFile.is_open())
	{
		//cout<<"can not open file: "<<PATHRUNINGTOKENFILE<<endl;
		mLogInstance->Log("can not open file: "+PATHRUNINGTOKENFILE);
		return false;
	}
	cfgFile.read(content,2);
	if(cfgFile.gcount() < 2)
	{
		mTrainState->m_Last_File[1] = 0;
	}
	else
	{
		mTrainState->m_Last_File[1] = content[0] + content[1]*256;
	}
	cfgFile.close();

	// 2:ShuntingOperation "/LocomotiveMain/ShuntingOperation/ShuntingOperationCount.txt"
	cfgFile.open(PATHSHUNTINGOPERATIONFILE.c_str());//打开文件
	if( !cfgFile.is_open())
	{
		//cout<<"can not open file: "<<PATHSHUNTINGOPERATIONFILE<<endl;
		mLogInstance->Log("can not open file: "+PATHSHUNTINGOPERATIONFILE);
		return false;
	}
	cfgFile.read(content,2);
	if(cfgFile.gcount() < 2)
	{
		mTrainState->m_Last_File[2] = 0;
	}
	else
	{
		mTrainState->m_Last_File[2] = content[0] + content[1]*256;
	}
	cfgFile.close();

	// 3:接车AdvanceNotice "/LocomotiveMain/AdvanceNotice/AdvanceNoticeCount.txt"
	cfgFile.open(PATHADVANCENOTICEFILE.c_str());//打开文件
	if( !cfgFile.is_open())
	{
		//cout<<"can not open file: "<<PATHADVANCENOTICEFILE<<endl;
		mLogInstance->Log("can not open file: "+PATHADVANCENOTICEFILE);
		return false;
	}
	cfgFile.read(content,2);
	if(cfgFile.gcount() < 2)
	{
		mTrainState->m_Last_File[3] = 0;
	}
	else
	{
		mTrainState->m_Last_File[3] = content[0] + content[1]*256;
	}
	cfgFile.close();

	// 4:OtherCommand "/LocomotiveMain/OtherCommand/OtherCommandCount.txt"
	cfgFile.open(PATHOTHERCOMMANDFILE.c_str());//打开文件
	if( !cfgFile.is_open())
	{
		//cout<<"can not open file: "<<PATHOTHERCOMMANDFILE<<endl;
		mLogInstance->Log("can not open file: "+PATHOTHERCOMMANDFILE);
		return false;
	}
	cfgFile.read(content,2);
	if(cfgFile.gcount() < 2)
	{
		mTrainState->m_Last_File[4] = 0;
	}
	else
	{
		mTrainState->m_Last_File[4] = content[0] + content[1]*256;
	}
	cfgFile.close();


	// 5:LossPackage "/LocomotiveMain/LossPackage/LossPackageCount.txt"
	cfgFile.open(PATHLOSSPACKAGEFILE.c_str());//打开文件
	if( !cfgFile.is_open())
	{
		//cout<<"can not open file: "<<PATHLOSSPACKAGEFILE<<endl;
		mLogInstance->Log("can not open file: "+PATHLOSSPACKAGEFILE);
		return false;
	}
	cfgFile.read(content,2);
	if(cfgFile.gcount() < 2)
	{
		mTrainState->m_Last_File[5] = 0;
	}
	else
	{
		mTrainState->m_Last_File[5] = content[0] + content[1]*256;
	}
	cfgFile.close();

	//配置参数 CheckParamFile()
	bool iNeedSave = false;
	map<string ,string >::iterator l_it;
	l_it = mapConfig.find("WorkMode");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->iWorkMode = 0x65;
		stringstream ss;
		ss << mTrainState->iWorkMode;
		string tmp = ss.str();
//		mLogInstance->Log("WorkMode: "+tmp);
		mapConfig["WorkMode"] = tmp;
		iNeedSave = true;
	}
	else
	{
		stringstream ss;
		ss << l_it->second;
		ss >> mTrainState->iWorkMode;
//		mLogInstance->Log("WorkMode: "+l_it->second);
	}

	l_it = mapConfig.find("LineName");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strLineName = "京沪线";
		mapConfig["LineName"] = mTrainState->strLineName;
//		mLogInstance->Log("LineName: "+mTrainState->strLineName);
		iNeedSave = true;
	}
	else
	{
		mTrainState->strLineName = l_it->second;
//		mLogInstance->Log("LineName: "+mTrainState->strLineName);
	}

	l_it = mapConfig.find("ModeFlag");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->iModeFlag = 0;
		stringstream ss;
		ss << mTrainState->iModeFlag;
		string tmp = ss.str();
//		mLogInstance->Log("ModeFlag: "+tmp);
		mapConfig["ModeFlag"] = tmp;
		iNeedSave = true;
	}
	else
	{
		stringstream ss;
		ss << l_it->second;
		ss >> mTrainState->iModeFlag;
//		mLogInstance->Log("ModeFlag: "+l_it->second);
	}
	if(mTrainState->iModeFlag==0)
	{
		mTrainState->arrInitStatus[3] &= (~BIT0);		//自动

	}
	else
	{
		mTrainState->arrInitStatus[3] |= BIT0;			//手动
	}

	l_it = mapConfig.find("ShutDownStatus");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->iShutDownStatus = 0;
		stringstream ss;
		ss << mTrainState->iShutDownStatus;
		string tmp = ss.str();
//		mLogInstance->Log("ShutDownStatus: "+tmp);
		mapConfig["ShutDownStatus"] = tmp;
		iNeedSave = true;
	}
	else
	{
		stringstream ss;
		ss << l_it->second;
		ss >> mTrainState->iShutDownStatus;
//		mLogInstance->Log("ShutDownStatus: "+l_it->second);
	}
	mapConfig["ShutDownStatus"] = "1";//开机后设置为未关机,正常关机后置为0

	l_it = mapConfig.find("TrainNumber");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strLastTrainNumber = "XXXXXXX";
		mapConfig["TrainNumber"] = mTrainState->strLastTrainNumber;
//		mLogInstance->Log("LastTrainNumber: " + mTrainState->strLastTrainNumber);
		iNeedSave = true;
	}
	else
	{
		mTrainState->strLastTrainNumber = l_it->second;
//		mLogInstance->Log("LastTrainNumber: " + mTrainState->strLastTrainNumber);
	}

	l_it = mapConfig.find("RegisterStatus");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->iLastRegisterStatus = 0;
		stringstream ss;
		ss << mTrainState->iLastRegisterStatus;
		string tmp = ss.str();
//		mLogInstance->Log("LastRegisterStatus: " + tmp);

		mapConfig["RegisterStatus"] = tmp;
		iNeedSave = true;
	}
	else
	{
		stringstream ss;
		ss << l_it->second;
		ss >> mTrainState->iLastRegisterStatus;
//		mLogInstance->Log("LastRegisterStatus: "+ l_it->second);
	}

	l_it = mapConfig.find("BenBuStatus");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->iLastBenBuStatus = 0;
		stringstream ss;
		ss << mTrainState->iLastBenBuStatus;
		string tmp = ss.str();
//		mLogInstance->Log("BenBuStatus: " + tmp);

		mapConfig["BenBuStatus"] = tmp;
		iNeedSave = true;
	}
	else
	{
		stringstream ss;
		ss << l_it->second;
		ss >> mTrainState->iLastBenBuStatus;
//		mLogInstance->Log("BenBuStatus: "+ l_it->second);
	}

	if(mTrainState->iShutDownStatus == 0)
	{//上次非正常关机
		mTrainState->iTrainFlag = 0;
		mTrainState->iBenBuStatus = mTrainState->iLastBenBuStatus;
		if(mTrainState->iBenBuStatus !=0 )
			mTrainState->arrInitStatus[3] |= BIT2;
		else
			mTrainState->arrInitStatus[3] &= (~BIT2);

		mTrainState->strTrainNumber = mTrainState->strLastTrainNumber;
		mTrainState->iDisplayTrainFlag = 0;
		mTrainState->iDisplayBenBuStatus = mTrainState->iLastBenBuStatus;
		mTrainState->strDisplayTrainNumber =mTrainState->strLastTrainNumber;


		if((mTrainState->iLastRegisterStatus==1)&&(mTrainState->strTrainNumber.compare("XXXXXXX")!= 0))	//上次注册过车次号功能号，需要恢复到注册状态
		{//unfinished
//			g_MRM.TrainNumberFNTemp.number.Empty();
//			for(i=0;(i<7)&&(g_TrainNumber.Number[i]!= ' ');i++)
//			{
//				g_MRM.TrainNumberFNTemp.number.Insert(i,g_TrainNumber.Number[i]);
//			}
//			g_MRM.TrainNumberFNTemp.Status = g_TrainNumber.Status;
//			g_MRM.TrainNumberFNTemp.number = TrainNumberToFunctionNumber(g_MRM.TrainNumberFNTemp.number,g_MRM.TrainNumberFNTemp.Status);
//			g_MRM.TrainNumberFNTemp.flag = 1;		//注册
//			g_MRM.TrainNumberFNTemp.Manual_Flag = g_TrainNumber.Manual_Flag;
//			for(i=0;i<10;i++)
//				g_MRM.TrainNumberFNTemp.Number[i] = g_TrainNumber.Number[i];
//			g_MRM.m_TrainNumberFNList.AddTail(g_MRM.TrainNumberFNTemp);
//			g_MRM.SetTimer(2,1000,NULL);
//			g_NeedLastRegisterFlag = 1;		//重新完成车次号注册后，才允许向MMI发送车次号选择指令
		}
	}
	else
	{//正常关机
		mTrainState->iTrainFlag = 0;
		mTrainState->iBenBuStatus =0;
		mTrainState->arrInitStatus[3] &= (~BIT2);

		mTrainState->strTrainNumber = "XXXXXXX";

		mTrainState->iDisplayTrainFlag = 0;
		mTrainState->iDisplayBenBuStatus =0;
		mTrainState->strDisplayTrainNumber = "XXXXXXX";

		mapConfig["RegisterStatus"] = "0";
		mapConfig["TrainNumber"] = "XXXXXXX";
		mapConfig["BenBuStatus"] = "0";
		iNeedSave = true;
	}
	for(int i = 0 ; i< mTrainState->strTrainNumber.size() || i<7; i++)
	{
		if(i< mTrainState->strTrainNumber.size())
			mTrainState->arrTrainNumber[i] = mTrainState->strTrainNumber[i];
		else
			mTrainState->arrTrainNumber[i] = ' ';
	}


	l_it = mapConfig.find("EngineFlag");//Manual_Flag
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->iEngineFlag = 1;
		stringstream ss;
		ss << mTrainState->iEngineFlag;
		string tmp = ss.str();
//		mLogInstance->Log("EngineNumber: " + tmp);
		mapConfig["EngineFlag"] = tmp;
		iNeedSave = true;
	}
	else
	{
		stringstream ss;
		ss << l_it->second;
		ss >> mTrainState->iEngineFlag;
//		mLogInstance->Log("EngineFlag: "+l_it->second);
	}

	l_it = mapConfig.find("EngineNumber");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strEngineNumber = "30100020";//"XXXXXXXX";
//		mLogInstance->Log("EngineNumber: " + mTrainState->strEngineNumber);
		mapConfig["EngineNumber"] = mTrainState->strEngineNumber;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strEngineNumber = l_it->second;
//		mLogInstance->Log("EngineNumber: " + mTrainState->strEngineNumber);
	}
	for(int i = 0 ; i< mTrainState->strEngineNumber.size() || i<8; i++)
	{
		if(i< mTrainState->strEngineNumber.size())
			mTrainState->arrEngineNumber[i] = mTrainState->strEngineNumber[i];
		else
			mTrainState->arrEngineNumber[i] = ' ';
	}


	l_it = mapConfig.find("ENPort");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->iEnPort = 1;
		stringstream ss;
		ss << mTrainState->iEnPort;
		string tmp = ss.str();
//		mLogInstance->Log("AbJie: "+tmp);
		mapConfig["AbJie"] = tmp;
		iNeedSave = true;
	}
	else
	{
		stringstream ss;
		ss << l_it->second;
		ss >> mTrainState->iEnPort;
//		mLogInstance->Log("ENPort: "+l_it->second);
	}

	l_it = mapConfig.find("MsIsdn");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strMsIsdn = "13800138000";
//		mLogInstance->Log("MsIsdn: " + mTrainState->strMsIsdn);
		mapConfig["MsIsdn"] = mTrainState->strMsIsdn;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strMsIsdn = l_it->second;
//		mLogInstance->Log("MsIsdn: " + mTrainState->strMsIsdn);
	}



	l_it = mapConfig.find("CHECKIP1");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strCheckIp1 = "0.0.0.0";
//		mLogInstance->Log("CHECKIP1: " + mTrainState->strCheckIp1);
		mapConfig["CHECKIP1"] = mTrainState->strCheckIp1;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strCheckIp1 = l_it->second;
//		mLogInstance->Log("CHECKIP1: " + mTrainState->strCheckIp1);
	}
	vector<string> tmpIp;
	StringSplit(mTrainState->strCheckIp1, '.', tmpIp);
	if(tmpIp.size()!=4)
	{
		mLogInstance->Log("CHECKIP1 is invalid");
	}
	else
	{
		for(int i=0; i<4 ;i++)
		{
			stringstream ss;
			ss << tmpIp[i];
			ss >> mTrainState->arrCheckIp1[i];
		}
	}

	l_it = mapConfig.find("CHECKIP2");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strCheckIp2 = "0.0.0.0";
//		mLogInstance->Log("CHECKIP2: " + mTrainState->strCheckIp2);
		mapConfig["CHECKIP2"] = mTrainState->strCheckIp2;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strCheckIp2 = l_it->second;
//		mLogInstance->Log("CHECKIP2: " + mTrainState->strCheckIp2);
	}
	StringSplit(mTrainState->strCheckIp2, '.', tmpIp);
	if(tmpIp.size()!=4)
	{
		mLogInstance->Log("CHECKIP2 is invalid");
	}
	else
	{
		for(int i=0; i<4 ;i++)
		{
			stringstream ss;
			ss << tmpIp[i];
			ss >> mTrainState->arrCheckIp2[i];
		}
	}

	l_it = mapConfig.find("CHECKIP3");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strCheckIp3 = "0.0.0.1";
//		mLogInstance->Log("CHECKIP3: " + mTrainState->strCheckIp3);
		mapConfig["CHECKIP3"] = mTrainState->strCheckIp3;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strCheckIp3 = l_it->second;
//		mLogInstance->Log("CHECKIP3: " + mTrainState->strCheckIp3);
	}
	StringSplit(mTrainState->strCheckIp3, '.', tmpIp);
	if(tmpIp.size()!=4)
	{
		mLogInstance->Log("CHECKIP3 is invalid");
	}
	else
	{
		for(int i=0; i<4 ;i++)
		{
			stringstream ss;
			ss << tmpIp[i];
			ss >> mTrainState->arrCheckIp3[i];
		}
	}

//	if(mTrainState->arrCheckIp3[2] == (byte)0)
//	{
//		bLogPrint1 = false;
//	}
//	else if(mTrainState->arrCheckIp3[2] == (byte)1)
//	{
//		bLogPrint1 = true;
//	}

	if(mTrainState->arrCheckIp3[3] % 2 == 0)
	{
		bLogPrint2 = true;
	}
	if(mTrainState->arrCheckIp3[3] % 3 == 0)
	{
		bLogPrint3 = true;
	}

	if(mTrainState->arrCheckIp3[2] % 2 == 0)
	{
		CAN_DEFAULT = CAN0;
	}
	else
	{
		CAN_DEFAULT = CAN1;
	}
	if(bLogPrint2)
	{
		stringstream ss;
		ss<<"CAN_DEFAULT: "<<(int)CAN_DEFAULT;
		mLogInstance->Log(ss.str());
	}

	l_it = mapConfig.find("CHECKPHONENUMBER1");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strCheckPhoneNumber1 = "00000000";
//		mLogInstance->Log("CHECKPHONENUMBER1: " + mTrainState->strCheckPhoneNumber1);
		mapConfig["CHECKPHONENUMBER1"] = mTrainState->strCheckPhoneNumber1;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strCheckPhoneNumber1 = l_it->second;
//		mLogInstance->Log("CHECKPHONENUMBER1: " + mTrainState->strCheckPhoneNumber1);
	}

	l_it = mapConfig.find("CHECKPHONENUMBER2");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strCheckPhoneNumber2 = "00000000";
//		mLogInstance->Log("CHECKPHONENUMBER2: " + mTrainState->strCheckPhoneNumber2);
		mapConfig["CHECKPHONENUMBER2"] = mTrainState->strCheckPhoneNumber2;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strCheckPhoneNumber2 = l_it->second;
//		mLogInstance->Log("CHECKPHONENUMBER2: " + mTrainState->strCheckPhoneNumber2);
	}

	l_it = mapConfig.find("CHECKPHONENUMBER3");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strCheckPhoneNumber3 = "00000000";
//		mLogInstance->Log("CHECKPHONENUMBER3: " + mTrainState->strCheckPhoneNumber3);
		mapConfig["CHECKPHONENUMBER3"] = mTrainState->strCheckPhoneNumber3;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strCheckPhoneNumber3 = l_it->second;
//		mLogInstance->Log("CHECKPHONENUMBER3: " + mTrainState->strCheckPhoneNumber3);
	}

	l_it = mapConfig.find("APN");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strApn = "gris.nelhsr";
//		mLogInstance->Log("APN: " + mTrainState->strApn);
		mapConfig["APN"] = mTrainState->strApn;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strApn = l_it->second;
//		mLogInstance->Log("APN: " + mTrainState->strApn);
	}

	l_it = mapConfig.find("DMISIPAddress");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strDMISAddre = "0.0.0.0";
//		mLogInstance->Log("DMISIPAddress: " + mTrainState->strDMISAddre);
		mapConfig["DMISIPAddress"] = mTrainState->strDMISAddre;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strDMISAddre = l_it->second;
//		mLogInstance->Log("DMISIPAddress: " + mTrainState->strDMISAddre);
	}
	StringSplit(mTrainState->strDMISAddre, '.', tmpIp);
	if(tmpIp.size()!=4)
	{
		mLogInstance->Log("DMISIPAddress is invalid" );
	}
	else
	{
		for(int i=0; i<4 ;i++)
		{
			stringstream ss;
			ss << tmpIp[i];
			ss >> mTrainState->arrDMISAddre[i];
		}
	}
	mTrainState->strHomeDMISAddre  = mTrainState->strDMISAddre ;
	mTrainState->arrHomeDMISAddre[0] = mTrainState->arrDMISAddre[0];
	mTrainState->arrHomeDMISAddre[1] = mTrainState->arrDMISAddre[1];
	mTrainState->arrHomeDMISAddre[2] = mTrainState->arrDMISAddre[2];
	mTrainState->arrHomeDMISAddre[3] = mTrainState->arrDMISAddre[3];


	l_it = mapConfig.find("GROSIPAddress");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strGrosAddre = "0.0.0.0";
//		mLogInstance->Log("GROSIPAddress: " + mTrainState->strGrosAddre);
		mapConfig["GROSIPAddress"] = mTrainState->strGrosAddre;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strGrosAddre = l_it->second;
//		mLogInstance->Log("GROSIPAddress: " + mTrainState->strGrosAddre);
	}
	StringSplit(mTrainState->strGrosAddre, '.', tmpIp);
	if(tmpIp.size()!=4)
	{
		mLogInstance->Log("GROSIPAddress is invalid" );
	}
	else
	{
		for(int i=0; i<4 ;i++)
		{
			stringstream ss;
			ss << tmpIp[i];
			ss >> mTrainState->arrGrosAddre[i];
		}
	}

	l_it = mapConfig.find("GROSIPAddress1");
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->strGrosAddre1 = "0.0.0.0";
//		mLogInstance->Log("GROSIPAddress1: " + mTrainState->strGrosAddre1);
		mapConfig["GROSIPAddress1"] = mTrainState->strGrosAddre1;
		iNeedSave = true;
	}
	else
	{
		mTrainState->strGrosAddre1 = l_it->second;
//		mLogInstance->Log("GROSIPAddress1: " + mTrainState->strGrosAddre1);
	}
	StringSplit(mTrainState->strGrosAddre1, '.', tmpIp);
	if(tmpIp.size()!=4)
	{
		mLogInstance->Log("GROSIPAddress1 is invalid" );
	}
	else
	{
		for(int i=0; i<4 ;i++)
		{
			stringstream ss;
			ss << tmpIp[i];
			ss >> mTrainState->arrGrosAddre1[i];
		}
	}

	l_it = mapConfig.find("TrainFlag");//Manual_Flag
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->iTrainFlag = 1;
		stringstream ss;
		ss << mTrainState->iTrainFlag;
		string tmp = ss.str();
//		mLogInstance->Log("TrainFlag: " + tmp);
		mapConfig["TrainFlag"] = tmp;
		iNeedSave = true;
	}
	else
	{
		stringstream ss;
		ss << l_it->second;
		ss >> mTrainState->iTrainFlag;//不使用存储的
//		mLogInstance->Log("TrainFlag: " + l_it->second);
	}

	l_it = mapConfig.find("GprsCheckFlag");//GprsCheckFlag
	if(l_it == mapConfig.end())
	{//no found
		mTrainState->iGPRSCheckFlag = 1;
		stringstream ss;
		ss << mTrainState->iGPRSCheckFlag;
		string tmp = ss.str();
//		mLogInstance->Log("GprsCheckFlag: " + tmp);
		mapConfig["GprsCheckFlag"] = tmp;
		iNeedSave = true;
	}
	else
	{
		stringstream ss;
		ss << l_it->second;
		ss >> mTrainState->iGPRSCheckFlag;//不使用存储的
//		mLogInstance->Log("GprsCheckFlag: " + l_it->second);
	}


	if(iNeedSave)
	{//save param
		WriteConfigFile(PATHPARAMETERFILE);
	}

	return true;
}
