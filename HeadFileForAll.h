/*
 * _HeadFileForAll.h
 *
 *  Created on: 2016-5-26
 *      Author: lenovo
 */

#ifndef _HEADFILEFORALL_H_
#define _HEADFILEFORALL_H_

#include "libs_emfuture_odm.h"

#include <string>
#include <vector>
typedef char byte; //typedef unsigned char byte;

#define GPIO_ZHUKONG1    110  //gpio3_14
#define GPIO_ZHUKONG2    111 //gpio3_15
#define GPIO_ZHUKONG3    113 //gpio3_17
#define GPIO_ZHUANHUAN1    70
#define GPIO_ZHUANHUAN2    76

//testcode
//#define GPIO_ZHUKONG1    115
//#define GPIO_ZHUKONG2    116
//#define GPIO_ZHUKONG3    117
//

#define BUF_SIZ 1024

#define CRC_HIGH   0x10
#define CRC_LOW    0x21

#define WORKSTATE_NORMAL    1 //01H：正常   02H：测试   03H: 维护
#define WORKSTATE_TEST    2
#define WORKSTATE_REPAIR    3

#define MRM_MODE    0
#define GPRS_MODE   1
#define ALL_MODE    2
#define BROKEN_MODE    3

#define CAN0    0
#define CAN1    1

#define ETH0    0
#define ETH1    1

#define ACK0    0
#define ACK1    1
#define ACK2    2

#define BASEFRAME    0
#define SHORTFRAME   1

#define PRIORITY0    0
#define PRIORITY1    1
#define PRIORITY2    2
#define PRIORITY3    3

#define BOARD_BROADCAST   	0xFF

#define BOARD_ZK_ALL    	0x12
#define BOARD_ZK_LEFT   	0x13
#define BOARD_ZK_RIGHT    	0x14

#define BOARD_MMI_ALL    	0x02
#define BOARD_MMI_LEFT   	0x03
#define BOARD_MMI_RIGHT    	0x04

#define BOARD_GPS    		0x83
#define BOARD_JL    		0x84
#define BOARD_WX    		0x85
#define BOARD_JH    		0x86
#define BOARD_400M    		0x87
#define BOARD_LBJ    		0x88
#define BOARD_JK    		0x89
#define BOARD_450M    		0x8A
#define BOARD_DL    		0x8B

#define BOARD_ZH_ALL    	0x22
#define BOARD_ZH_LEFT   	0x23
#define BOARD_ZH_RIGHT    	0x24

#define BOARD_LTE_ALL    	0x32
#define BOARD_LTE_LEFT   	0x33
#define BOARD_LTE_RIGHT    	0x34

#define WATCHDOGTIME    3 //3秒必须喂狗
#define WATCHDOGINTERVAL    500 //喂狗间隔500毫秒

#define MAX_LOG_BUF_BYTE 60000

#define CAN_TIMEOUT    10
#define OTHERBOARD_TIMEOUT    10

extern const int BIT0 ;
extern const int BIT1 ;
extern const int BIT2 ;
extern const int BIT3 ;
extern const int BIT4 ;
extern const int BIT5 ;
extern const int BIT6 ;
extern const int BIT7 ;
extern const int BIT8 ;
extern const int BIT9 ;
extern const int BIT10 ;
extern const int BIT11 ;
extern const int BIT12 ;
extern const int BIT13 ;
extern const int BIT14 ;
extern const int BIT15 ;
extern const int BIT16 ;
extern const int BIT17 ;
extern const int BIT18 ;
extern const int BIT19 ;
extern const int BIT20 ;
extern const int BIT21 ;
extern const int BIT22 ;
extern const int BIT23 ;

extern const int MSK0 ;
extern const int MSK1 ;
extern const int MSK2 ;
extern const int MSK3 ;
extern const int MSK4 ;
extern const int MSK5 ;
extern const int MSK6 ;
extern const int MSK7 ;
extern const int MSK8 ;
extern const int MSK9 ;
extern const int MSK10 ;
extern const int MSK11 ;
extern const int MSK12 ;
extern const int MSK13 ;
extern const int MSK14 ;
extern const int MSK15 ;

//本机IP地址相关
extern const u_short LOCALPORT ;
extern const u_short REMOTEPORT ;

extern const int CANBAUDRATE ;//can 波特率
extern const int UARTBAUDRATE ;//串口 波特率

extern const int LEVELERROR ; // 错误级别
extern const int LEVELEXCEPTION ; // 异常级别
extern const int LEVELDEBUG ; // 调试级别

extern bool LOG_OUTFILE;
extern bool WATCHDOG;

extern int CAN_DEFAULT;

extern const int LOGMAXSIZEFORCIR ; // CIR日志最大容量，单位:m*(1024 * 1024)//压缩后大约8M
extern const int LOGMAXNUM ; //150
extern const int LOGDELNUM ; //75

extern const std::string PATHFILEDIR ; // "/LocomotiveMain/"
extern const std::string PATHLOGFILE;// "/LocomotiveMain/cir.log"
extern const std::string PATHPARAMETERFILE ; // "/LocomotiveMain/para.bak"
extern const std::string PATHALLCOMMANDFILE ; // "/LocomotiveMain/allcommand.txt"
extern const std::string PATHDISPATCHCOMMAND ; // "/LocomotiveMain/DispatchCommand/"
extern const std::string PATHDISPATCHCOMMANDFILE ; // "/locomotivemain/dispatchcommand/dispatchcommandcount.txt"
extern const std::string PATHRUNINGTOKEN ; // "/LocomotiveMain/RuningToken/"
extern const std::string PATHRUNINGTOKENFILE ; // "/locomotivemain/runingtoken/runingtokencount.txt"
extern const std::string PATHSHUNTINGOPERATION ; // "/LocomotiveMain/ShuntingOperation/"
extern const std::string PATHSHUNTINGOPERATIONFILE ; // "/locomotivemain/shuntingoperation/shuntingoperationcount.txt"
extern const std::string PATHADVANCENOTICE ; // "/LocomotiveMain/AdvanceNotice/"
extern const std::string PATHADVANCENOTICEFILE ; // "/locomotivemain/advancenotice/advancenoticecount.txt"
extern const std::string PATHOTHERCOMMAND ; // "/LocomotiveMain/OtherCommand/"
extern const std::string PATHOTHERCOMMANDFILE ; // "/locomotivemain/othercommand/othercommandcount.txt";
extern const std::string PATHLOSSPACKAGE ; // "/LocomotiveMain/LossPackage/"
extern const std::string PATHLOSSPACKAGEFILE ; // "/locomotivemain/losspackage/losspackagecount.txt"
extern const std::string PATHOTHERRECORD ; // "/LocomotiveMain/OtherRecord/"
extern const std::string PATHOTHERRECORDTRAIN ; // "/locomotivemain/otherrecord/trainnumberchangerecord.txt";
extern const std::string PATHOTHERRECORDENGINE ; // "/locomotivemain/otherrecord/enginenumberchangeRecord.txt";
extern const std::string PATHOTHERRECORDWORKMODE ; // "/locomotivemain/otherrecord/workmodechangerecord.txt";

extern const std::string SERIALPROT_NAME;

extern bool bLogPrint3 ;
extern bool bLogPrint2 ;
extern bool bLogPrint1 ;

extern byte CRC8_TAB[256];

extern std::string GetCurrentTime();
extern int GetCurrentTime2(char now_timeval[6]);

extern int MySetRtcTime (unsigned short* configs, unsigned char syn);
extern int MySetRtcTime2 (int year, int month, int day, int hour,int minute,int second);

extern int MySetGpioDirection(int gpio, int direction);
extern int MySetOutIOState(int gpio, int state);
extern int MyGetOutIOState(int gpio,int *curstate);
extern int MySetIoState(int gpio, int state);
extern int MyReadIoState(int gpio,int *curstate);

extern int MyEnableWtd(int TimeOut);
extern int MyFeedWtd();
extern int MyDisableWtd();

extern int MySetSensorCfg(int cmd, unsigned short* arg);
extern int MyReadSensorVal(unsigned int* strValue);



extern int IsFolderExist(const char* path);
extern int IsFileExist(const char* path);

extern void StringSplit(std::string s,char splitchar,std::vector<std::string>& vec);

extern void CrcFunc16(byte *ptr,int size,byte *crc_result);
extern void CrcFunc8(byte *ptr,int size,byte *crc_result);

extern void MySetLocalIp(int port,byte leftorright);
extern bool CheckLocalIp(byte leftorright);

struct _FNStruct
{
	std::string strFnNumber;	//车次功能号//number
	bool bRegisterFlag;		//0:注销，1:注册//flag
	std::string strOriginalNumber;//原始车次号（转换成功能号格式之前）//Number
	byte bManualOrTaxFlag;  //0:从TAX箱得到  1：从MMI人工输入Manual_Flag
	byte bBenBuStatus;	//本补状态//Status
};
#endif /* _HEADFILEFORALL_H_ */
