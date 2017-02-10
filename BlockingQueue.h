/*
 * BlockingQueue.h
 *
 *  Created on: 2016-5-27
 *      Author: lenovo
 */

#ifndef BLOCKINGQUEUE_H_
#define BLOCKINGQUEUE_H_

#include "HeadFileForAll.h"

#include <mutex>
#include <condition_variable>
#include <list>


class BlockingQueue {
public:
	BlockingQueue();
	void Put(std::string task);
	std::string Take();
	int Size();
	bool IsEmpty();

	virtual ~BlockingQueue();
private:
	mutable std::mutex _mutex;
	std::condition_variable _condvar;
	std::list<std::string> _queue;
};

#endif
