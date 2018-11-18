#pragma once

#ifndef UNICODE
#define UNICODE
#endif

//#include <vector>
//#include <queue>
//#include <memory>
//#include <thread>
//#include <condition_variable>
//#include <future>
//#include <functional>

#include <vector>
#include <windows.h>
#include <synchapi.h>
#include "UnitOfWork.h"

class ThreadPool {
public:
	ThreadPool(int maxThreads);
	~ThreadPool();	
	void enqueue(UnitOfWork task);

private:
	const int DEFAULT_THREADS = 4;
	const DWORD DEFAULT_SPIN_COUNT = 4000;

	// Reference to queue
	std::vector<UnitOfWork>* workQueue_ = nullptr;

	// Event determining if queue contains 1 item at least
	HANDLE availableEvent_ = nullptr;

	// Critical section providing atomic enque/dequeue operations
	CRITICAL_SECTION queueSection_;
};



