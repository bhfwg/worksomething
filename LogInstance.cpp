/*
 * LogInstance.cpp
 *
 *  Created on: 2016-5-26
 *      Author: lenovo
 */

#include "LogInstance.h"

#include "z_zlib.h"
using namespace std;

LogInstance* LogInstance::mInstance = NULL;
LogInstance* LogInstance::GetInstance(){
	if(mInstance == NULL)
		mInstance = new LogInstance();
	return mInstance;
}

LogInstance::LogInstance():strTag("LogInstance"),strLogName("cir.log"),strBakLogName(".bak")
{
	bRunningLog = true;
}


LogInstance::~LogInstance() {
}

unsigned long LogInstance::GetFileSize(string path){
	unsigned long filesize = -1;
	FILE *fp;
	fp = fopen(path.c_str(), "r");
	if(fp == NULL)
		return filesize;
	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	fclose(fp);
	return filesize;
}

void LogInstance::LogInit() {
	cout<<"1"<<endl;

	if(IsFolderExist(PATHFILEDIR.c_str()) < 0)
	{
		if (mkdir(PATHFILEDIR.c_str(), 0755) < 0)
		{
			if(IsFolderExist(PATHFILEDIR.c_str()) < 0)
			{
				cout<<"create folder: "<< PATHFILEDIR <<" failed?"<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	cout<<"2"<<endl;

	ofstream outfile;
	outfile.open(PATHLOGFILE.c_str(),ios::app);
	if(!outfile || !outfile.is_open())
	{
		cout<<"can not open: "<<PATHLOGFILE<<endl;
		exit(EXIT_FAILURE);
	}
	outfile.close();
	cout<<"3"<<endl;

	string strLogFilePath = PATHFILEDIR + strLogName;
	int lFileSize = (int)GetFileSize(strLogFilePath.c_str());
	if(lFileSize > LOGMAXSIZEFORCIR * 1024 * 1024){
		//超出大小
		char bArrNow[6];
		GetCurrentTime2(bArrNow);
		if(bArrNow[0] >= 16)
		{
			//复制文件
			string strBakFile = PATHFILEDIR + strLogName + strBakLogName;
			FILE *fp1;
			fp1 = fopen(strLogFilePath.c_str(), "r");
			FILE *fp2;
			fp2 = fopen(strBakFile.c_str(), "w");
			char buff[1024] = {'\0'};
			int count = 0;
			while((count = fread(buff, 1, 1024, fp1)) != 0)
			{
				fwrite(buff, 1, count, fp2);
			}
			fclose(fp1);
			fclose(fp2);
			//压缩文件
			DoZip(strBakFile);
		}
		cout<<"3-1"<<endl;
	}
	else{
		//未超出检测是否有未转储压缩的文件
		string strBakFile = PATHFILEDIR + strLogName + strBakLogName;
		FILE *fp;
		fp = fopen(strBakFile.c_str(), "r");
		if(fp != NULL){//存在,继续压缩
			fclose(fp);
			DoZip(strBakFile);
		}
		cout<<"3-2"<<endl;
	}
	cout<<"4"<<endl;

	if(!ofLogFile.is_open()){
		ofLogFile.open(strLogFilePath,ios::out | ios::app);
	}
	cout<<"5"<<endl;
}

void LogInstance::LogRunning()
{
//	cout<<GetCurrentTime()<<"startlogfile start"<<endl;
	string strLogFilePath = PATHLOGFILE;
	ofstream ofile(PATHLOGFILE.c_str(),ofstream::app);
	while (bRunningLog)
	{
		try
		{
			string logStr = logQueue.Take();
			std::cout<< logStr <<std::endl;
			if(LOG_OUTFILE)
			{
				ofile<<logStr<<endl;//暂时不写文件日志
			}
		}
		catch (...)
		{
		}
	}
	ofile.close();
	cout<<"startlogfile end"<<endl;
}

void LogInstance::StartLogFile()
{
	LogInit();
	thdLog = std::thread(std::mem_fn(&LogInstance::LogRunning),this);
	//logThread.join();
	//logThread.detach();
}


void LogInstance::DoZip(string strBakFile)
{
	std::cout<<"dozip start"<<std::endl;

	char rec_head[4]={0xAA,0xBB,0xDD,0xEE};

	unsigned char sourcebuf[MAX_LOG_BUF_BYTE];
	unsigned char destbuf[MAX_LOG_BUF_BYTE];

	char slen[2];  //源数据长度
	char dlen[2];  //压缩后长度

	int result;
	unsigned long num;
	unsigned long  destlen = 65535;

	time_t now;                              //日志存储的当前时间
	struct tm *timenow;
	char now_timeval[6];                    //存储年月日时分秒
	now = time(NULL);
	timenow = localtime(&now);
	char ss[25];
	sprintf(ss, "%2.2d_%2.2d_%2.2d_%2.2d_%2.2d_%2.2d.dat", (timenow->tm_year-100),(1+timenow->tm_mon), timenow->tm_mday,
			timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
	std::string strNewBakName(ss);

	ifstream in(strBakFile.c_str());
	if (!in.is_open())
	{
		std::cout<<"opening log bak file error"<<std::endl;
		return;
	}
	ofstream out(strNewBakName.c_str());
	if (out.is_open())
	{
		std::cout<<"opening new log dat file error"<<std::endl;
		return;
	}
	char recordFileHead[100]={0xDD,0xCC,0xBB,0xAA,0x88,};
	out.write(recordFileHead,100);

	while (!in.eof())
	{
		in.read((char*)sourcebuf,MAX_LOG_BUF_BYTE);
		num = in.gcount();
		result=compress(destbuf, &destlen, sourcebuf, num);
		if(Z_OK == result)
		{

			std::cout<<"compressing, sourlen="<< num <<" destlen="<<destlen<<std::endl;
			//将压缩后的数组写入文件中
			slen[0] = num / 256;  //源数据长度高字节
			slen[1] = num % 256;  //源数据长度低字节
			dlen[0] = destlen / 256;  //压缩后长度高字节
			dlen[1] = destlen % 256;  //压缩后长度低字节
			//2015-06-10 增加时间信息，便于解析时先拆分日志文件
			now = time(NULL);
			timenow = localtime(&now);
			now_timeval[0] = (char)(timenow->tm_year-100);//00-99，年份加上1900
			now_timeval[1] = (char)(timenow->tm_mon+1);  //1-12
			now_timeval[2] = (char)timenow->tm_mday;   //1-31
			now_timeval[3] = (char)timenow->tm_hour;   //0-23
			now_timeval[4] = (char)timenow->tm_min;    //0-59
			now_timeval[5] = (char)timenow->tm_sec;    //0-59

			out.write(rec_head,4);
			out.write(slen,2);
			out.write(dlen,2);
			out.write(now_timeval,6);
			out.write((char*)destbuf,destlen);
		}
		else if(Z_MEM_ERROR == result)
		{
			std::cout<<"compress error is Z_MEM_ERROR"<<std::endl;
		}
		else if(Z_BUF_ERROR == result)
		{
			std::cout<<"compress error is Z_BUF_ERROR"<<std::endl;
		}
	}
	in.close();
	out.close();
	std::cout<<"dozip end"<<std::endl;

	remove(strBakFile.c_str());//删除文件
}

void LogInstance::Log(string strLog)
{
	logQueue.Put(GetCurrentTime() + strLog);
}
