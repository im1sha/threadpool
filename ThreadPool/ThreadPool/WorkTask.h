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

	WorkTask(std::vector<UnitOfWork*> * unitList, volatile HANDLE* availableEvent, volatile HANDLE* emptyEvent, CRITICAL_SECTION* queueSection);


	// Destroys executed task
	bool tryClose(bool forced = false, time_t timeout = INFINITE);

	// Determines whether the thread executes the task
	bool isBusy();

	// Gets running task max time (in s)
	time_t getTimeoutInSeconds();

	// Gets running task max time (in ms)
	time_t getTimeoutInMs();

	// Gets time of last operation starting thread
	time_t getLastOperationTimeInSeconds();

	// Sets managment interval
	bool setManagementInterval(time_t milliseconds);

	// Gets managment timeout
	time_t getManagementInterval();

	// Interupts thread
	static bool interrupt(HANDLE hThread, time_t msWaitTimeout, bool forced);

private:
	
	static const time_t DEFAULT_MANAGEMENT_INTERVAL_IN_MS = 100;

	// Reference to Threadpool's queue
	std::vector<UnitOfWork*> * unitsQueue_ = nullptr;

	// Determines whether queue contains any element 
	volatile HANDLE* availableEvent_ = nullptr;
	
	// Determines whether queue contains no elements
	volatile HANDLE* emptyEvent_ = nullptr;

	// Critical section providing atomic dequeue operations
	CRITICAL_SECTION* unitsSection_ = nullptr;

	// Critical section providing atomic operations with getters
	CRITICAL_SECTION * localFieldSection_ = nullptr;

	// Current thread 
	unsigned* runningThreadAddress_ = nullptr;

	// Time before running thread closing on close request (ms)
	time_t waitTimeoutInMs_ = INFINITE;

	// Time between next check
	time_t managementIntervalInMs_ = DEFAULT_MANAGEMENT_INTERVAL_IN_MS;

	// Thread's handle
	HANDLE thread_ = nullptr;

	// Determines whether the thread preffered to continue running
	bool shouldKeepRunning_ = true;

	// Determines whether the thread executes the task
	bool busy_ = false;

	// Time of last operation starting thread
	time_t lastOperationTimeInSeconds_ = 0;

	// Retrieves first item from workQueue_
	UnitOfWork * dequeue();

	// Executes task belongs to queue of all the tasks passed to ThreadPool  
	static unsigned startExecuting(WorkTask * args);

	// Starts executing Thread safely
	void beginExecutingThread();

};


