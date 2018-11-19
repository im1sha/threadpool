#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#define INVALID_RESULT -1

//#include <queue>
//#include <memory>
//#include <condition_variable>
//#include <future>
//#include <functional>
#include <windows.h>
#include <thread>
#include <vector>
#include <synchapi.h>
#include "UnitOfWork.h"
#include "WorkTask.h"

class ThreadPool
{
public:
	ThreadPool(int maxThreads = DEFAULT_MAX_THREADS, int maxIdleTime = DEFAULT_IDLE_TIME);

	~ThreadPool();	

	// Queues a function for execution. 
	// The method executes when one of the ThreadPool's 
	// thread becomes available 
	void enqueue(UnitOfWork task);

	// Destroys ThreadPool instance, 
	// running threads and management thread
	void close();

	// Gets total tasks number waiting for executing at the moment
	int getTotalPendingTasks();

	// Retrieves the difference between 
	// the maximum number of Threadpool's threads
	// and the currently active ones
	int getAvailableThreads();

	// Sets the number of threads
	// that can be active concurrently
	bool setMaxThreads(int workerThreads);

	// Gets the number of threads
	// that can be active concurrently
	int getMaxThreads();

	// Sets the minimum number of threads on which 
	// ThreadPool will not force destruction
	bool setMinThreads(int workerThreads);

	// Gets the minimum number of threads on which 
	// ThreadPool will not force destruction
	int getMinThreads();

	// Sets max time thread's allowed to end execution
	bool setMaxIdleTime(int seconds);

	// Gets max time thread's allowed to end execution
	int getMaxIdleTime();

private:
	
	static const int DEFAULT_MAX_THREADS = 4;

	static const int DEFAULT_MIN_THREADS = 1;

	static const DWORD DEFAULT_SPIN_COUNT = 4000;

	static const int DEFAULT_IDLE_TIME = 4; // (in seconds)

	// Tasks to execute
	std::vector<UnitOfWork> * unitsList_ = nullptr;

	// Running tasks that execute methods passed by ThreadPool.enqueue()  
	std::vector<WorkTask> * threadList_ = nullptr;

	// Thread that controls executing of other running threads
	HANDLE managementThread_ = nullptr;

	// ManagementThread_'s address
	unsigned* managementThreadAddress_ = nullptr;

	// Detemines whether management Thread should continue running
	bool keepManagementThreadRunning_ = true;

	// Event determining if queue of tasks contains 1 item at least
	HANDLE availableEvent_ = nullptr;

	// Critical section providing atomic enque/dequeue operations with queue of UnitIfWork
	CRITICAL_SECTION unitsSection_;

	// Critical section providing atomic enque/dequeue operations with queue of threads
	CRITICAL_SECTION threadsSection_;

	// Min threads running at the moment
	int* minThreads_ = nullptr;

	// Max threads running at the moment
	int* maxThreads_ = nullptr;

	// Max idle thread time 
	int* maxIdleTime_ = nullptr;
	
	// Keeps tracking of threads 
	// that are running more than MaxIdleTime
	static void keepManagement(ThreadPool* t);

	// Gets size() on unitsList_
	int getThreadListSize();

	// Gets size() on threadList_
	int getUnitListSize();

	// Releases all allocated fields 
	void deleteFields();

	//// Sets interval between calls of management thread
	//bool setManagementInterval(int millisecondsTimeout);

	//int getManagementInterval();
	
	//int managementInterval_ = 100;
};



