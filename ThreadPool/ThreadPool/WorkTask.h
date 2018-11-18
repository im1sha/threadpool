#pragma once

#include <windows.h>
#include <vector>
#include <time.h>
#include <exception>
#include <synchapi.h>
#include <process.h>
#include "UnitOfWork.h"

class WorkTask
{
public:
	WorkTask(std::vector<UnitOfWork>* workQueue, HANDLE availableEvent, CRITICAL_SECTION queueSection, time_t timeout);

	~WorkTask();

	// Destroys executed task
	void close();

	// Determines whether the thread executes the task
	bool isBusy();

	// Gets time of last operation starting thread
	time_t getLastOperationTime();
private:
	// Reference to Threadpool's queue
	std::vector<UnitOfWork>* workQueue_ = nullptr;

	// Determines whether queue contains any element 
	HANDLE availableEvent_ = nullptr;

	// Critical section providing atomic dequeue operations
	CRITICAL_SECTION queueSection_;

	// Current thread 
	unsigned* runningThread_ = nullptr;

	// Time before running thread closing on close request (ms)
	time_t waitTimeout_ = 0;

	// Thread's handle
	HANDLE hThread_ = nullptr;

	// Determines whether the thread preffered to continue running
	bool shouldKeepRunning_ = true;

	// Determines whether the thread executes the task
	bool busy_ = false;

	// Time of last operation starting thread
	time_t lastOperationTime_ = 0;

	// Retrieves first item from workQueue_
	UnitOfWork dequeue();

	//size_t getQueueSize();

	// Interupts thread
	void interrupt();

	// Interupts idling thread
	void wakeUp();

	// Executes task belongs to queue of all the tasks passed to ThreadPool  
	static unsigned startExecutableLoop(WorkTask args);

};


