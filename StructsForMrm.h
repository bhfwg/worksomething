#ifndef STRUCTSFORMRM_H_
#define STRUCTSFORMRM_H_
#include <string>
//+COPS: (2,,"460 09","46009")(3,"China Unicom",,"46001")(3,"China Mobile",,"46000"),,(0,1,3,4),(0,1,2)
struct MRMCOPSStruct
{
	std::string  strMrmCopsStat;							//0	unknown,1 available,2 current,3	forbidden
	std::string  strMrmCopsLongAlphanumericOperator;		//运营商长名
	std::string  strMrmCopsShortAlphanumericOperator;		//运营商短名
	std::string  strMrmCopsNumericOperator;				//运营商数字名
	std::string  strMrmCopsWholeString;					//包括这一运营商全部信息,如2,,"460 09","46009"
};
struct DataHandle
{
	std::string   strData;
	byte	bHandledFlag;/*0:未处理；1:已处理*/
};
struct _PToPCallStruct
{
	std::string   strPtpcsIdx;    //call identification number
	std::string   strPtpcsDir;    //0:MO,1:MT
	std::string   strPtpcsStat;   //0:active,1:held,2:dialing (MO call),3:alerting (MO call),4:incoming (MT call),5:waiting (MT call)
	std::string   strPtpcsMode;   //0:voice,1:data,2:fax,9:unknown
	std::string   strPtpcsMpty;
	std::string   strPtpcsNumber;	//string type phone number in format specified by <type>
	//std::string   strPtpcsType; //"1"个呼"17/18"组呼
	std::string   strPtpcsAlpha;
	std::string   strPtpcsPriority;
	std::string   strPtpcsUUS1;
};
struct _GroupCallStruct
{
	std::string  strGcGID;    //group ID
	std::string  strGcGCA;    //group area
	std::string  strGcIs17Or18; //17:VGC,18:VBC
	std::string  strGcStat;   //0:active,1:held,2:incoming call
	std::string  strGcDir;    //0:MO,1:MT
	std::string  strGcAckFlag;
	std::string  strGcPriority;
};
struct MMISelectCallNum
{
	std::string strMmiSlcType;				//1:个呼,17:组呼
	std::string	strMmiSlcNumber;				//MMI选择通话的号码03,15
	std::string strMmiSlcGID;				//group ID
	std::string strMmiSlcGCA;				//group area
	std::string strMmiSlcIs17Or18;			//17:VGC,18:VBC
	std::string strMmiSlcPriority;
};
struct _UUS1Info
{
	std::string strUiUUS1;
	std::string strUiNumber;
	int iUiflag;					//此信息是否有用
};						//在个呼时存储UUS1信息所用

#endif /* FRAMEGENERATE_H_ */
