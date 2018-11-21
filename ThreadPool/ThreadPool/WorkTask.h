#pragma once

#define INVALID_RESULT -1

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

	WorkTask(std::vector<UnitOfWork*> * workQueue, HANDLE* availableEvent, HANDLE* emptyEvent, CRITICAL_SECTION* queueSection);

	// Destroys executed task
	void close();

	// Determines whether the thread executes the task
	bool isBusy();

	// Gets running task max time (in s)
	int getTimeoutInSeconds();

	// Gets running task max time (in ms)
	int getTimeoutInMs();

	// Gets time of last operation starting thread
	time_t getLastOperationTime();

	// Interupts thread
	static void interrupt(HANDLE hThread, time_t secondsWaitTimeout);

private:
	
	// Reference to Threadpool's queue
	std::vector<UnitOfWork*> * unitsQueue_ = nullptr;

	// Determines whether queue contains any element 
	HANDLE* availableEvent_ = nullptr;
	
	// Determines whether queue contains no elements
	HANDLE* emptyEvent_ = nullptr;

	// Critical section providing atomic dequeue operations
	CRITICAL_SECTION* unitsSection_ = nullptr;

	// Critical section providing atomic operations with getters
	CRITICAL_SECTION * localFieldSection_ = nullptr;

	// Current thread 
	unsigned* runningThreadAddress_ = nullptr;

	// Time before running thread closing on close request (seconds)
	int waitTimeoutInMs_ = INFINITE;

	// Thread's handle
	HANDLE thread_ = nullptr;

	// Determines whether the thread preffered to continue running
	bool shouldKeepRunning_ = true;

	// Determines whether the thread executes the task
	bool busy_ = false;

	// Instance is about destroying.
	// State caused by close() function or destructor call
	bool isDestroyed_ = false;

	// Time of last operation starting thread
	time_t lastOperationTimeInSeconds_ = 0;

	// Retrieves first item from workQueue_
	UnitOfWork * dequeue();

	// Executes task belongs to queue of all the tasks passed to ThreadPool  
	static unsigned startExecuting(WorkTask * args);

};


