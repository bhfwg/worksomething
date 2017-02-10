/*
 * main.cpp
 * gpio110 gpio111 gpio112 无法读取
 * 配置网络需要等30秒才起作用
 * 如何调试
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "MainClass.h"


#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <thread>
using namespace std;



int main()
{
	std::cout<<"enter main()"<<std::endl;
	MainClass mMainClass;
	mMainClass.StartService();


	std::cout<<"quit main()"<<std::endl;
}

