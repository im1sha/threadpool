#pragma once

#ifndef UNICODE
#define UNICODE
#endif

//#include <queue>
//#include <memory>
#include <thread>
//#include <condition_variable>
//#include <future>
//#include <functional>

#include <vector>
#include <windows.h>
#include <synchapi.h>
#include "UnitOfWork.h"
#include "WorkTask.h"

class ThreadPool
{
public:
	ThreadPool(int maxThreads, int msTimeout);
	~ThreadPool();	

	// Queues a method for execution. 
	// The method executes when a TaskQueue 
	// thread becomes available 
	void enqueue(UnitOfWork task);

	// Destroys TaskQueue instance, 
	// running tasks and management thread
	void close();

	// Gets total tasks number waiting for executing at the moment
	int getLength();

	// Retrieves the difference between the maximum number of TaskQueue threads
	// and the number currently active
	int getAvailableThreads();

	// Sets the number of requests to the TaskQueue that can be active concurrently
	bool setMaxThreads(int workerThreads);

	int getMaxThreads();

	// Sets the minimum number of threads the TaskQueue creates on demand, as new requests are made, 
    // before switching to an algorithm for managing thread creation and destruction
	bool setMinTreads(int workerThreads);

	int getMinThreads();

	// Sets interval between calls of management thread
	//bool setManagementInterval(int millisecondsTimeout);

	// Sets max time thread's allowed to be in state of Wait, Sleep or Join
	bool setMaxIdleTime(int seconds);

	int getMaxIdleTime();

private:
	ThreadPool();
	const int DEFAULT_THREADS = 4;
	const DWORD DEFAULT_SPIN_COUNT = 4000;
	const int DEFAULT_IDLE_TIME = 4000;

	// Reference to queue
	std::vector<UnitOfWork>* unitsQueue_ = nullptr;

	// Running tasks that execute methods passed by ThreadPool.enqueue()  
	std::vector<WorkTask>* threadList_ = nullptr;

	// Thread that controls executing of other running threads
	HANDLE managementThread_ = nullptr;

	unsigned* managementThreadAddress_ = nullptr;

	// Detemines whether management Thread should run
	bool keepManagementThreadRunning_ = true;

	// Event determining if queue contains 1 item at least
	HANDLE availableEvent_ = nullptr;

	// Critical section providing atomic enque/dequeue operations with queue of UnitIfWork
	CRITICAL_SECTION unitsSection_;

	// Critical section providing atomic enque/dequeue operations with queue of threads
	CRITICAL_SECTION threadsSection_;

	// Min threads running at the moment
	int minThreads_ = 0;

	// Max threads running at the moment
	int maxThreads_ = 0;

	// Max idle thread time 
	int maxIdleTime_ = 100;

	//int managementInterval_ = 100;

	// Keeps tracking of threads 
	// that are running more than MaxIdleTime
	static void keepManagement(ThreadPool* t);
};



