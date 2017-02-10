#ifndef STRUCTSFORMRM_H_
#define STRUCTSFORMRM_H_
#include <string>
//+COPS: (2,,"460 09","46009")(3,"China Unicom",,"46001")(3,"China Mobile",,"46000"),,(0,1,3,4),(0,1,2)
struct MRMCOPSStruct
{
	std::string  strMrmCopsStat;							//0	unknown,1 available,2 current,3	forbidden
	std::string  strMrmCopsLongAlphanumericOperator;		//��Ӫ�̳���
	std::string  strMrmCopsShortAlphanumericOperator;		//��Ӫ�̶���
	std::string  strMrmCopsNumericOperator;				//��Ӫ��������
	std::string  strMrmCopsWholeString;					//������һ��Ӫ��ȫ����Ϣ,��2,,"460 09","46009"
};
struct DataHandle
{
	std::string   strData;
	byte	bHandledFlag;/*0:δ����1:�Ѵ���*/
};
struct _PToPCallStruct
{
	std::string   strPtpcsIdx;    //call identification number
	std::string   strPtpcsDir;    //0:MO,1:MT
	std::string   strPtpcsStat;   //0:active,1:held,2:dialing (MO call),3:alerting (MO call),4:incoming (MT call),5:waiting (MT call)
	std::string   strPtpcsMode;   //0:voice,1:data,2:fax,9:unknown
	std::string   strPtpcsMpty;
	std::string   strPtpcsNumber;	//string type phone number in format specified by <type>
	//std::string   strPtpcsType; //"1"����"17/18"���
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
	std::string strMmiSlcType;				//1:����,17:���
	std::string	strMmiSlcNumber;				//MMIѡ��ͨ���ĺ���03,15
	std::string strMmiSlcGID;				//group ID
	std::string strMmiSlcGCA;				//group area
	std::string strMmiSlcIs17Or18;			//17:VGC,18:VBC
	std::string strMmiSlcPriority;
};
struct _UUS1Info
{
	std::string strUiUUS1;
	std::string strUiNumber;
	int iUiflag;					//����Ϣ�Ƿ�����
};						//�ڸ���ʱ�洢UUS1��Ϣ����

#endif /* FRAMEGENERATE_H_ */
