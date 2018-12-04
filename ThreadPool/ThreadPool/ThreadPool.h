#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#define INVALID_RESULT -1

#include <windows.h>
#include <thread>
#include <vector>
#include <synchapi.h>
#include "UnitOfWork.h"
#include "WorkTask.h"

class ThreadPool
{
public:

	ThreadPool(int maxThreads = DEFAULT_MAX_THREADS);


	// Queues a function for execution. 
	// The method executes when one of the ThreadPool's 
	// thread becomes available 
	bool enqueue(UnitOfWork task);

	// Destroys ThreadPool instance, 
	// running threads and management thread
	// All queued tasks will be waited for terminations
	void closeSafely();

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
	bool setTimeoutInMs(time_t milliseconds);

	// Gets max time thread's allowed to end execution
	time_t getTimeoutInMs();
	
	// Gets max time thread's allowed to end execution
	time_t getTimeoutInSeconds();

private:
	
	static const int DEFAULT_MAX_THREADS = 4;

	static const int DEFAULT_MIN_THREADS = 0;

	static const DWORD DEFAULT_SPIN_COUNT = 4000;

	static const time_t DEFAULT_TIMEOUT_IN_MS = 100; // (in ms)

	static const int DEFAULT_MANAGE_INTERVAL_IN_MS = 100; // (in ms)

	// Tasks to execute
	std::vector<UnitOfWork*> * unitsQueue_ = nullptr;

	// Running tasks that execute methods passed by ThreadPool.enqueue()  
	std::vector<WorkTask*> * threadList_ = nullptr;

	// Thread that controls executing of other running threads
	HANDLE managementThread_ = nullptr;

	// ManagementThread_'s address
	unsigned* managementThreadAddress_ = nullptr;

	// Instance is about destroying.
	// Caused by interrupt() function 
	bool isDestroyed_ = false;

	// Instance is about destroying.
	// Caused by closeSafely() function call 
	bool isDestroySafe_ = false;

	// Event determining if queue of units contains 1 item at least
	volatile HANDLE* availableEvent_ = nullptr;

	// Event determining if queue of tasks contains 0 items
	volatile HANDLE* emptyEvent_ = nullptr;

	// Event determining if managementThread started
	volatile HANDLE* startedEvent_ = nullptr;

	// Critical section providing atomic enque/dequeue operations with queue of UnitIfWork
	CRITICAL_SECTION* unitsSection_ = nullptr;

	// Critical section providing atomic enque/dequeue operations with queue of threads
	CRITICAL_SECTION* threadsSection_ = nullptr;

	// Guards fields
	CRITICAL_SECTION* fieldsSection_ = nullptr;

	// Min threads running at the moment
	int minThreads_ = 1;

	// Max threads running at the moment
	int maxThreads_ = DEFAULT_MAX_THREADS;

	// Max idle thread time 
	time_t timeout_ = DEFAULT_TIMEOUT_IN_MS;

	// interval between management activities
	time_t trackingInterval_ = DEFAULT_MANAGE_INTERVAL_IN_MS;

	// Keeps tracking of threads 
	// that are running more than MaxIdleTime
	static void keepManagement(ThreadPool* t);

	// Gets size() on unitsList_ as atomic operation
	int getThreadListSize();

	// Gets size() on threadList_ as atomic operation
	int getUnitListSize();

	// Releases all allocated fields 
	void releaseMemory();

	// Kills all running tasks
	bool tryKillThreads(bool forced = false);

	// Atomic getter under isDestroyed_
	bool isDestroyed();
	
	// Atomic getter under isDestroySafe_
	bool isDestroySafe();

	// Destructs current instance
	void close();	

	// Returns trackingInterval_
	time_t getTrackingInterval();

	// Destroys ThreadPool instance, 
	// running threads and management thread
	// All not started tasks will be ignored
	void interrupt();

	// Initializes event safely
	void initializeEvent(volatile HANDLE * eventName);

	// Starts management thread safely
	void beginManagementThread();
};



