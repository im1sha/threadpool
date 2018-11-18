#pragma once

#include <windows.h>
#include <vector>
#include <process.h>
#include <time.h>
#include <exception>
#include <synchapi.h>

#include "UnitOfWork.h"

class WorkTask
{
public:
	WorkTask(std::vector<UnitOfWork>* workQueue, CRITICAL_SECTION* determinant);

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

	// Reference to empty queue ThreadPool determinant 
	CRITICAL_SECTION* emptyDeterminant_ = nullptr;

	// Current thread 
	uintptr_t* runningThread_ = nullptr;

	// Determines whether the thread preffered to continue running
	bool shouldKeepRunning_ = true;

	// Determines whether the thread executes the task
	bool busy_ = false;

	// Time of last operation starting thread
	time_t lastOperation_ = 0;

	// Retrieves first item from workQueue_
	UnitOfWork dequeue();

	// Interupts idling thread
	void wakeUp();

	// Executes task belongs to queue of all the tasks passed to ThreadPool  
	void startExecutableLoop();

	// Runs single task
	void runUnitOfWork(UnitOfWork* u);
};


