/*
 * _HeadFileForAll.c
 *
 *  Created on: 2016-5-26
 *      Author: lenovo
 */

#include "HeadFileForAll.h"
#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <linux/rtc.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <sys/socket.h>


//置1  |
const int BIT0 = 0x01;
const int BIT1 = 0x02;
const int BIT2 = 0x04;
const int BIT3 = 0x08;
const int BIT4 = 0x10;
const int BIT5 = 0x20;
const int BIT6 = 0x40;
const int BIT7 = 0x80;
const int BIT8 = 0x100;
const int BIT9 = 0x200;
const int BIT10 = 0x400;
const int BIT11 = 0x800;
const int BIT12 = 0x1000;
const int BIT13 = 0x2000;
const int BIT14 = 0x4000;
const int BIT15 = 0x8000;
const int BIT16 = 0x010000;
const int BIT17 = 0x020000;
const int BIT18 = 0x040000;
const int BIT19 = 0x080000;
const int BIT20 = 0x100000;
const int BIT21 = 0x200000;
const int BIT22 = 0x400000;
const int BIT23 = 0x800000;
//置0  &
const int MSK0 = 0xFFFFFFFE;
const int MSK1 = 0xFFFFFFFD;
const int MSK2 = 0xFFFFFFFB;
const int MSK3 = 0xFFFFFFF7;
const int MSK4 = 0xFFFFFFEF;
const int MSK5 = 0xFFFFFFDF;
const int MSK6 = 0xFFFFFFBF;
const int MSK7 = 0xFFFFFF7F;
const int MSK8 = 0xFFFFFEFF;
const int MSK9 = 0xFFFFFDFF;
const int MSK10 = 0xFFFFFBFF;
const int MSK11 = 0xFFFFF7FF;
const int MSK12 = 0xFFFFEFFF;
const int MSK13 = 0xFFFFDFFF;
const int MSK14 = 0xFFFFBFFF;
const int MSK15 = 0xFFFF7FFF;

//本机IP地址相关
const u_short  LOCALPORT = 20000;
const u_short  REMOTEPORT = 20001;

const int CANBAUDRATE = 500000;//can 波特率
const int UARTBAUDRATE = 115200;//can 波特率

const int LEVELERROR = 1; // 错误级别
const int LEVELEXCEPTION = 3; // 异常级别
const int LEVELDEBUG = 6; // 调试级别

bool LOG_OUTFILE = true;
bool WATCHDOG = true;

int CAN_DEFAULT = CAN0;//默认CAN选择,读取到配置文件后修改它为CAN0或者CAN1

const int LOGMAXSIZEFORCIR = 100; // CIR日志最大容量，单位:m*(1024 * 1024)//压缩后大约8M
const int LOGMAXNUM = 360; //
const int LOGDELNUM = 200; //

const std::string PATHFILEDIR = "/locomotivemain/"; // 日志输出路径//"/LocomotiveMain/"
const std::string PATHLOGFILE =  "/locomotivemain/cir.log"; // 日志记录
const std::string PATHPARAMETERFILE =  "/locomotivemain/para.cfg"; // 参数记录
const std::string PATHALLCOMMANDFILE =  "/locomotivemain/allcommand.txt";
const std::string PATHDISPATCHCOMMAND = "/locomotivemain/dispatchcommand/";
const std::string PATHDISPATCHCOMMANDFILE =  "/locomotivemain/dispatchcommand/dispatchcommandcount.txt";
const std::string PATHRUNINGTOKEN =  "/locomotivemain/runingtoken/";
const std::string PATHRUNINGTOKENFILE =  "/locomotivemain/runingtoken/runingtokencount.txt";
const std::string PATHSHUNTINGOPERATION =  "/locomotivemain/shuntingoperation/";
const std::string PATHSHUNTINGOPERATIONFILE =  "/locomotivemain/shuntingoperation/shuntingoperationcount.txt";
const std::string PATHADVANCENOTICE =  "/locomotivemain/advancenotice/";
const std::string PATHADVANCENOTICEFILE =  "/locomotivemain/advancenotice/advancenoticecount.txt";
const std::string PATHOTHERCOMMAND =  "/locomotivemain/othercommand/";
const std::string PATHOTHERCOMMANDFILE =  "/locomotivemain/othercommand/othercommandcount.txt";
const std::string PATHLOSSPACKAGE =  "/locomotivemain/losspackage/";
const std::string PATHLOSSPACKAGEFILE =  "/locomotivemain/losspackage/losspackagecount.txt";
const std::string PATHOTHERRECORD =  "/locomotivemain/otherrecord/";
const std::string PATHOTHERRECORDTRAIN =  "/locomotivemain/otherrecord/trainnumberchangerecord.txt";
const std::string PATHOTHERRECORDENGINE =  "/locomotivemain/otherrecord/enginenumberchangeRecord.txt";
const std::string PATHOTHERRECORDWORKMODE =  "/locomotivemain/otherrecord/workmodechangerecord.txt";

const std::string SERIALPROT_NAME = "/dev/ttyS1";

bool bLogPrint3 = false;//三级打印,数据收发细颗粒日志,调试打印代码
bool bLogPrint2 = false;//二级打印,打印一些数量非常多,大部分情况不看,有时候需要看的日志比如心跳、can检测帧、GPS数据、综合信息等待
bool bLogPrint1 = true;//一级打印,基本运行记录日志

byte CRC8_TAB[256] =  {   0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,   0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,   0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,   0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,   0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,   0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,   0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,   0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,   0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,   0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,   0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,   0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,   0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,   0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,   0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,   0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,   0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,   0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,   0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,   0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,   0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,   0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,   0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,   0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,   0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,   0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,   0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,   0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,   0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,   0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,   0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,   0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35  };

std::string GetCurrentTime()
{
//	time_t now;                              //日志存储的当前时间
//	struct tm *timenow;
//	char now_timeval[7];                    //存储年月日时分秒
//	now = time(NULL);
//	timenow = localtime(&now);
//	now_timeval[0] = (byte)(timenow->tm_year-100);//00-99，年份加上1900
//	now_timeval[1] = (byte)timenow->tm_mon+1;  //1-12
//	now_timeval[2] = (byte)timenow->tm_mday;   //1-31
//	now_timeval[3] = (byte)timenow->tm_hour;   //0-23
//	now_timeval[4] = (byte)timenow->tm_min;    //0-59
//	now_timeval[5] = (byte)timenow->tm_sec;    //0-59
//	now_timeval[6] = ':';
//	std::string sCurrentTime(now_timeval);
//	std::cout<<sCurrentTime<<std::endl;
//	return sCurrentTime ;

	char ss[25];
	time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep); /*取得当地时间*/
	sprintf(ss, "%2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d: ", (p->tm_year-100),(1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	//printf("%s\n", ss);

	std::string sCurrentTime(ss);
	//std::cout<<sCurrentTime<<std::endl;
	return sCurrentTime ;
}
int GetCurrentTime2(char now_timeval[6])
{
	time_t now;                              //日志存储的当前时间
	struct tm *timenow;
	now = time(NULL);
	timenow = localtime(&now);
	now_timeval[0] = (byte)(timenow->tm_year-100);//00-99，年份加上1900
	now_timeval[1] = (byte)timenow->tm_mon+1;  //1-12
	now_timeval[2] = (byte)timenow->tm_mday;   //1-31
	now_timeval[3] = (byte)timenow->tm_hour;   //0-23
	now_timeval[4] = (byte)timenow->tm_min;    //0-59
	now_timeval[5] = (byte)timenow->tm_sec;    //0-59
	return 0;
}

int MySetRtcTime (unsigned short* configs, unsigned char syn)
{//貌似用不了
	int ret = SetRtcTime (configs, syn);
	if(ret != ERROR_OK)
		std::cout<<GetCurrentTime()<<"set rtc time failed, ret="<<ret<<std::endl;
//	else
//		std::cout<<GetCurrentTime()<<"set rtc time successful"<<std::endl;
	return ret;
}
int MySetRtcTime2 (int year, int month, int day, int hour,int minute,int second)
{//貌似用不了
	char	l_c8Command[128] = {0};
	sprintf(l_c8Command, "date %4.2d.%2.2d.%2.2d-%2.2d:%2.2d:%2.2d",year,month,day,hour,minute,second);
	int ret = system(l_c8Command);
	if(ret == -1 || ret == 127)
	{
		std::cout<<"date command error"<<std::endl;
	}
	memset(l_c8Command,0, sizeof(l_c8Command));
	sprintf(l_c8Command, "hwclock -w");
	ret = system(l_c8Command);
	if(ret == -1 || ret == 127)
	{
		std::cout<<"hwclock command error"<<std::endl;
	}
	return ret;
}

int MySetGpioDirection(int gpio, int direction)
{//1:输出   0:输入
	int ret = -1;
	char	l_c8Command[128] = {0};


	sprintf(l_c8Command, "echo %d > /sys/class/gpio/export",  gpio);
	ret = system(l_c8Command);

	if(ret != 0)
		std::cout<<GetCurrentTime()<<"set gpio"<<gpio<<" export error, ret="<<ret<<std::endl;

	memset(l_c8Command,0, sizeof(l_c8Command));

	if(direction == 0)
	{
		sprintf(l_c8Command, "echo out > /sys/class/gpio/gpio%d/direction",  gpio);//配套MySetOutIOState
		//std::cout<<GetCurrentTime()<<"set gpio"<<gpio<<" out"<<std::endl;
	}
	else
	{
		sprintf(l_c8Command, "echo in > /sys/class/gpio/gpio%d/direction",  gpio);//配套MyGetOutIOState
		//std::cout<<GetCurrentTime()<<"set gpio"<<gpio<<" in"<<std::endl;
	}

	ret = system(l_c8Command);

	if(ret != 0)
		std::cout<<GetCurrentTime()<<"set gpio"<<gpio<<" direct error, ret="<<ret<<std::endl;

	return ret;
}

int MySetOutIOState(int gpio, int state)
{
	int ret = SetEnIOOutState(gpio, state);
	if(ret != ERROR_OK)
		std::cout<<GetCurrentTime()<<"set gpio"<<gpio<<" state failed, ret="<<ret<<std::endl;
//	else
//		std::cout<<GetCurrentTime()<<"set gpio"<<gpio<<" state successful"<<std::endl;
	return ret;
}
int MyGetOutIOState(int gpio,int *curstate)
{
	int ret = GetEnIOOutState(gpio,curstate);
	if(ret != ERROR_OK)
		std::cout<<GetCurrentTime()<<"get gpio"<<gpio<<" state failed, ret="<<ret<<std::endl;
	else
		std::cout<<GetCurrentTime()<<"get gpio"<<gpio<<" state successful"<<std::endl;
	return ret;
}
int MySetIoState(int gpio, int state)
{
	FILE *p = NULL;
	char	l_c8Command[128] = {0};
	sprintf(l_c8Command, "/sys/class/gpio/gpio%d/value", gpio);//配套MySetOutIOState
	p = fopen(l_c8Command,"w");
	if(p == NULL)
	{
		std::cout<<GetCurrentTime()<<"can't open gpio"<<gpio<<" value file"<<std::endl;
		return -1;
	}
	fprintf(p,"%d",state);
	fclose(p);
	return ERROR_OK;
}
int MyReadIoState(int gpio,int *curstate)
{
	FILE *p = NULL;
	char	l_c8Command[128] = {0};
	sprintf(l_c8Command, "/sys/class/gpio/gpio%d/value", gpio);//配套MySetOutIOState
	char arr[10]={0};
	p = fopen(l_c8Command,"r");
	if(p == NULL)
	{
		std::cout<<GetCurrentTime()<<"can't open gpio"<<gpio<<" value file"<<std::endl;
		return -1;
	}
	fseek(p,0,0);
	fread(arr,1,1,p);
	fclose(p);
	*curstate =  arr[0] - '0';
	return ERROR_OK;
}

int MyEnableWtd(int TimeOut)
{
	int ret = EnableWtd(TimeOut);
	if(ret != ERROR_OK)
		std::cout<<GetCurrentTime()<<"open watch dog failed, ret="<<ret<<std::endl;
//	else
//		std::cout<<GetCurrentTime()<<"open watch dog successful"<<std::endl;
	return ret;
}
int MyFeedWtd()
{
	int ret = FeedWtd();
	if(ret != ERROR_OK)
		std::cout<<GetCurrentTime()<<"feed watch dog failed, ret="<<ret<<std::endl;
	//else
	//	std::cout<<GetCurrentTime()<<"feed watch dog successful"<<std::endl;
	return ret;
}
int MyDisableWtd()
{
	int ret = DisableWtd();
	if(ret != ERROR_OK)
		std::cout<<GetCurrentTime()<<"disable watch dog failed, ret="<<ret<<std::endl;
//	else
//		std::cout<<GetCurrentTime()<<"disable watch dog successful"<<std::endl;
	return ret;
}

int MySetSensorCfg(int cmd, unsigned short* arg)
{
	int ret = SetSensorCfg(cmd, arg);
	if(ret != ERROR_OK)
		std::cout<<GetCurrentTime()<<"set sensor failed, ret="<<ret<<std::endl;
//	else
//		std::cout<<GetCurrentTime()<<"set sensor successful"<<std::endl;
	return ret;
}
int MyReadSensorVal(unsigned int* strValue)
{
	int ret = ReadSensorVal(strValue);
	if(ret != ERROR_OK)
		std::cout<<GetCurrentTime()<<"read sensor failed, ret="<<ret<<std::endl;
//	else
//		std::cout<<GetCurrentTime()<<"read sensor successful"<<std::endl;
	return ret;
}

int IsFolderExist(const char* path)
{//检查目录是否存在0:存在, -1:不存在
    DIR *dp;
    if ((dp = opendir(path)) == NULL)
    {//no
        return -1;
    }
    //yes
    closedir(dp);
    return 0;
}
int IsFileExist(const char* path)
{//检查文件是否存在0:存在, -1:不存在
    return access(path, F_OK);
}

void StringSplit(std::string s,char splitchar,std::vector<std::string>& vec)
{
	if(vec.size()>0)//保证vec是空的
	   vec.clear();
	int length = s.length();
	int start=0;
	for(int i=0; i<length; i++)
	{
	   if(s[i] == splitchar && i == 0)//第一个就遇到分割符
	   {
		   start += 1;
	   }
	   else if(s[i] == splitchar)
	   {
			vec.push_back(s.substr(start,i - start));
			start = i+1;
	   }
	   else if(i == length-1)//到达尾部
	   {
		   vec.push_back(s.substr(start,i+1 - start));
	   }
	}
}

void CrcFunc16(byte *ptr,int size,byte *crc_result)//fwg
{
	int  i;
	byte j;
	byte bit_temp;
	byte temp_data;

	crc_result[0] = 0;
	crc_result[1] = 0;

	for(i=0; i<size; i++)
	{
		temp_data = ptr[i];

		for(j=0; j<8; j++)
		{
			bit_temp = crc_result[0]^temp_data;
			bit_temp = bit_temp & 0x80;

			temp_data = temp_data << 1;
			crc_result[0] = crc_result[0] << 1;

			if(crc_result[1] & 0x80)
			{
				crc_result[0] = crc_result[0] | 0x01;
			}

			crc_result[1] = crc_result[1] << 1;

			if(bit_temp == 0x80)
			{
				crc_result[0] = crc_result[0]^CRC_HIGH;
				crc_result[1] = crc_result[1]^CRC_LOW;
			}
		}
	}

//	if(bLogPrint)
//	{
//		for (int i =0 ;i<size; i++)
//			printf("%x ",*(ptr+i));
//		printf("crc0=%x crc1=%x\n",crc_result[0] ,crc_result[1]);
//	}
}



void CrcFunc8(byte *ptr,int size,byte *crc_result)
{
	byte   ucIndex;//CRC8校验表格索引
	byte   ucCRC8   =   0;//CRC8字节初始化
	while (size--)
	{
		ucIndex   =   ucCRC8^(*ptr);
		ptr++;
		ucCRC8   =   CRC8_TAB[ucIndex];
	}
	*crc_result = ucCRC8;

//	if(bLogPrint)
//	{
//		for (int i =0 ;i<size; i++)
//			printf("%x ",*(ptr+i));
//		printf("crc=%x\n",*crc_result );
//	}
}

void MySetLocalIp(int port,byte leftorright)
{
	char	l_c8Command[128] = {0};
	if(leftorright == (byte)BOARD_ZK_LEFT)
	{
		sprintf(l_c8Command, "ip link set dev eth%d address 00:34:12:78:63:6B", port);
	}
	else
	{
		//不变
	}
	system(l_c8Command);
	std::cout<<GetCurrentTime()<<"set mac address"<<std::endl;
	memset(l_c8Command,0,sizeof(l_c8Command));
	if(leftorright == (byte)BOARD_ZK_LEFT)
	{
		sprintf(l_c8Command, "ip addr add 192.168.0.10/24 dev eth%d", port);
	}
	else
	{
		sprintf(l_c8Command, "ip addr add 192.168.0.11/24 dev eth%d", port);
	}
	system(l_c8Command);
	std::cout<<GetCurrentTime()<<"set ip address"<<std::endl;
	memset(l_c8Command,0,sizeof(l_c8Command));
	sprintf(l_c8Command, "ifconfig eth%d down", port);
	system(l_c8Command);
	std::cout<<GetCurrentTime()<<"set ip down"<<std::endl;
	memset(l_c8Command,0,sizeof(l_c8Command));
	sprintf(l_c8Command, "ifconfig eth%d up", port);
	system(l_c8Command);
	std::cout<<GetCurrentTime()<<"set ip up"<<std::endl;
}

bool CheckLocalIp(byte leftorright)
{

	struct ifaddrs * ifAddrStruct = NULL;
	void *tmpAddrPtr = NULL;

	int times = 0;
	std::string ip_tag="";
	if(leftorright == (byte)BOARD_ZK_LEFT)
		ip_tag = "10";
	else
		ip_tag = "11";

	std::cout<<"IP:1"<<std::endl;
	std::vector<std::string> tmpIp;
	while(times < 120)
	{
		std::cout<<"IP:2"<<std::endl;
		getifaddrs(&ifAddrStruct);
		while (ifAddrStruct != NULL)
		{
			std::cout<<"IP:3"<<std::endl;
			if (ifAddrStruct->ifa_addr->sa_family==AF_INET)
			{ // check it is IP4
				std::cout<<"IP:4"<<std::endl;
				tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
				char addressBuffer[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
				std::cout<<"IP:7"<<std::endl;
				printf("%s IP Address %s/n", ifAddrStruct->ifa_name, addressBuffer);
				std::cout<<"IP:8"<<std::endl;
				std::string strIP(addressBuffer);
				std::cout<<strIP<<std::endl;
				std::cout<<"IP:9"<<std::endl;
				StringSplit(strIP, '.', tmpIp);
				if(tmpIp.size()!=4)
				{
					std::cout<<"IP:12"<<std::endl;
				}
				else
				{
					//if(!tmpIp[4].compare(ip_tag))
					if(tmpIp[4] == ip_tag)
					{
						std::cout<<"IP:10"<<std::endl;
						std::cout<<GetCurrentTime()<<"IP is ok"<<std::endl;
						return true;
					}
					std::cout<<"IP:11"<<std::endl;
				}
			}
			else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6)
			{ // check it is IP6
				std::cout<<"IP:6"<<std::endl;
			}
			ifAddrStruct=ifAddrStruct->ifa_next;
			std::cout<<"IP:5"<<std::endl;
		}
		times++;
		sleep(1);
	}
	std::cout<<GetCurrentTime()<<"IP is invalid"<<std::endl;
	return false;

	/*
	char hname[128];
	struct hostent *hent;
	int i;
	gethostname(hname, sizeof(hname));
	//hent = gethostent();
	hent = gethostbyname(hname);
	printf("hostname: %s/naddress list: ", hent->h_name);
	for(i = 0; hent->h_addr_list[i]; i++) {
		printf("%s/t", inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])));
	}
	return true;
	*/
}

