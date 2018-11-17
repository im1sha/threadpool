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
#include <synchapi.h>
#include "UnitOfWork.h"

class ThreadPool {
public:
	ThreadPool(int maxThreads);
	~ThreadPool();	
	void enqueue(UnitOfWork task);
private:
	const int DEFAULT_THREADS = 4;
	const DWORD defaultSpinCount = 4000;
	CRITICAL_SECTION taskListCriticalSection;

	int create();
	std::vector<UnitOfWork> taskList;
};



