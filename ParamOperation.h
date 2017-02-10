/*
 * ParamOperation.h
 *
 *  Created on: 2016-8-17
 *      Author: lenovo
 */

#ifndef PARAMOPERATION_H_
#define PARAMOPERATION_H_
#include <map>
#include "HeadFileForAll.h"
#include "TrainState.h"
//#include "DispatchCommandFileHandle.h"
#include "LogInstance.h"
class ParamOperation {
public:
	static ParamOperation* GetInstance();
	virtual ~ParamOperation();
	TrainState* mTrainState;
	LogInstance* mLogInstance;
	void SaveSpecialField(std::string name, std::string value );
	bool ReadConfigFile(std::string cfgfilepath);
	bool WriteConfigFile(std::string cfgfilepath);
	bool FindInConfigFile( std::string  key, std::string  value);
	bool HandleParamFile();
	std::map<std::string,std::string> mapConfig;

private:
	static ParamOperation* mInstance;
	ParamOperation();
};

#endif /* PARAMOPERATION_H_ */
