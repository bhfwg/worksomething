/*
 * BlockingQueue.cpp
 *
 *  Created on: 2016-5-27
 *      Author: lenovo
 */

#include "BlockingQueue.h"

#include <iostream>
#include <iomanip>
#include <thread>
using namespace std;

BlockingQueue::BlockingQueue() : _mutex (), _condvar (), _queue () {

}

BlockingQueue::~BlockingQueue() {
}

void BlockingQueue::Put(string task)
{
	lock_guard<mutex> lock(_mutex);
	_queue.push_back(task);
	_condvar.notify_all();
}

string BlockingQueue::Take()
{
	unique_lock<mutex> lock (_mutex);
	if(_queue.empty ())
		_condvar.wait(lock);
	string front= _queue.front ();
	_queue.pop_front ();
	return front;
}

bool BlockingQueue::IsEmpty()
{
	if(_queue.size()==0)
		return true;
	else
		return false;
}

int BlockingQueue::Size()
{
	lock_guard<mutex> lock (_mutex);
	return _queue.size();
}
