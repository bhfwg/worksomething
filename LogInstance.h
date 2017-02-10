/*
 * LogInstance.h
 *
 *  Created on: 2016-5-26
 *      Author: lenovo
 */

#ifndef LOGINSTANCE_H_
#define LOGINSTANCE_H_



#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "BlockingQueue.h"
#include "HeadFileForAll.h"


class LogInstance {
public:
	static LogInstance* GetInstance();
	virtual ~LogInstance();
	std::thread thdLog;

	static LogInstance* mInstance;
	const std::string strTag ;
	const std::string strLogName; // 日志输出路径
	const std::string strBakLogName;

	bool bRunningLog;

	std::ofstream ofLogFile;
	BlockingQueue logQueue;

	void StartLogFile();
	void Log(std::string strLog);
private:
	LogInstance();
	void LogInit();
	void LogRunning();
	unsigned long GetFileSize(std::string path);
	void DoZip(std::string strBakFile);
};

#endif /* LOGINSTANCE_H_ */
