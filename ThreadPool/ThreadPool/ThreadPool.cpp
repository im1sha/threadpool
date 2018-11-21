#include "ThreadPool.h"

ThreadPool::ThreadPool(int maxThreads, int timeout)
{
	unitsList_ = new std::vector<UnitOfWork *>();
	threadList_ = new std::vector<WorkTask *>();

	// synchronizing sections initialization
	unitsSection_ = new CRITICAL_SECTION();
	threadsSection_ = new CRITICAL_SECTION();
	fieldsSection_ = new CRITICAL_SECTION();
	::InitializeCriticalSectionAndSpinCount(unitsSection_, DEFAULT_SPIN_COUNT);
	::InitializeCriticalSectionAndSpinCount(threadsSection_, DEFAULT_SPIN_COUNT);
	::InitializeCriticalSectionAndSpinCount(fieldsSection_, DEFAULT_SPIN_COUNT);

	// events creation
	availableEvent_ = new HANDLE(::CreateEvent(nullptr, true, false, nullptr));
	emptyEvent_ = new HANDLE(::CreateEvent(nullptr, true, false, nullptr));
	startedEvent_ = new HANDLE(::CreateEvent(nullptr, true, false, nullptr));

	// fields initialization
	this->setMinThreads(ThreadPool::DEFAULT_MIN_THREADS);
	this->setMaxThreads(maxThreads);
	this->setTimeoutInMs(timeout);

	managementThread_ = (HANDLE) ::_beginthreadex(nullptr, 0, 
		(_beginthreadex_proc_type) ThreadPool::keepManagement,
		(void *) this, 0, managementThreadAddress_);
}

void ThreadPool::closeSafely()
{
	::EnterCriticalSection(fieldsSection_);
	if (isDestroyed() || isDestroySafe())
	{
		::LeaveCriticalSection(fieldsSection_);
		return;
	}
	isDestroySafe_ = true;
	::LeaveCriticalSection(fieldsSection_);

	this->close();	
}

void ThreadPool::interrupt()
{	
	::EnterCriticalSection(fieldsSection_);
	if (isDestroyed() || isDestroySafe())
	{
		::LeaveCriticalSection(fieldsSection_);
		return;
	}
	isDestroyed_ = true;
	::LeaveCriticalSection(fieldsSection_);

	this->close();
}

void ThreadPool::close()
{
	::WaitForSingleObject(*startedEvent_, INFINITE);

	// managementThread_ destroying; management thread  will destroy another threads
	if (managementThread_ != nullptr)
	{
		WorkTask::interrupt(managementThread_, INFINITE); 
	}
	
	// free resources
	this->releaseMemory();
}

void ThreadPool::keepManagement(ThreadPool* threadPool)
{
	if (threadPool == nullptr)
	{
		return;
	}

	printf("management thread started %d\n", (int)threadPool->managementThread_);

	bool keepTracking = true;

	while (keepTracking)
	{		
		std::exception_ptr exception;		
		try
		{																
			::EnterCriticalSection(threadPool->threadsSection_);

			std::vector<WorkTask*> * threads = threadPool->threadList_;
			int threadListSize = threadPool->getThreadListSize();

			if (threadListSize > threadPool->getMinThreads())
			{
				for (size_t i = 0; i < threadListSize; i++)
				{				
					WorkTask* workTask = (*threads)[i];

					// thread destroying if timeout is exceeded
					if (::time(nullptr) - workTask->getLastOperationTime() > workTask->getTimeoutInMs())
					{
						workTask->close(false); 
						threads->erase(threads->begin() + i); // delete item # i 
						threadListSize--; 
						i--;
					}
				}
			}			
			::LeaveCriticalSection(threadPool->threadsSection_);		

			::EnterCriticalSection(threadPool->fieldsSection_);
			::SetEvent(*(threadPool->startedEvent_));	
			
			if (threadPool->isDestroyed())
			{
				threadPool->killThreads(true, threadPool->getTimeoutInMs());
				keepTracking = false;
			}
			else if (threadPool->isDestroySafe())
			{
				threadPool->killThreads(false);			
				//keepTracking = false;
			}
			::LeaveCriticalSection(threadPool->fieldsSection_);			
		}
		catch(...)
		{		
			exception = std::current_exception();
		}
		::Sleep((DWORD) threadPool->getTrackingInterval());
	}

	printf("management thread succeeded %d\n", (int)threadPool->managementThread_);
}

bool ThreadPool::enqueue(UnitOfWork t)
{
	if (isDestroyed())
	{
		return false;
	}

	// add task 
	::EnterCriticalSection(unitsSection_);
	if (unitsList_ != nullptr)
	{
		UnitOfWork * task = new UnitOfWork(t);
		unitsList_->push_back(task);
	}

	// signal to thread waiting for task if it's first task
	if (getUnitListSize() == 1)
	{
		::SetEvent(*availableEvent_);
		::ResetEvent(*emptyEvent_);
	}
	::LeaveCriticalSection(unitsSection_);

	::EnterCriticalSection(threadsSection_);
	// new thread creating if it's not enough threads which are running now
	if (threadList_->size() < getMaxThreads())
	{
		WorkTask *t = new WorkTask(unitsList_, availableEvent_, emptyEvent_, unitsSection_);
		threadList_->push_back(t);
	}
	::LeaveCriticalSection(threadsSection_);

	return true;
}

void ThreadPool::killThreads(bool forced, time_t timeout)
{
	::EnterCriticalSection(threadsSection_);
	int threadListSize = getThreadListSize();
	for (size_t i = 0; i < threadListSize; i++)
	{
		WorkTask* workTask = (*threadList_)[i];
		if (workTask != nullptr)
		{
			workTask->close(forced, timeout);
			threadList_->erase(threadList_->begin() + i); // delete item # i 
			threadListSize--;
			i--;
		}				
	}
	::LeaveCriticalSection(threadsSection_);
}

void ThreadPool::releaseMemory()
{
	if (unitsList_ != nullptr)
	{
		delete unitsList_;
	}

	if (threadList_ != nullptr)
	{
		delete threadList_;
	}

	if (unitsSection_ != nullptr)
	{
		::DeleteCriticalSection(unitsSection_);
		delete unitsSection_;
	}

	if (threadsSection_ != nullptr)
	{
		::DeleteCriticalSection(threadsSection_);
		delete threadsSection_;
	}

	if (fieldsSection_ != nullptr)
	{
		::DeleteCriticalSection(fieldsSection_);
		delete fieldsSection_;
	}

	if (availableEvent_ != nullptr)
	{
		::CloseHandle(*availableEvent_);
		delete availableEvent_;
	}

	if (startedEvent_ != nullptr)
	{
		::CloseHandle(*startedEvent_);
		delete startedEvent_;
	}

	if (emptyEvent_ != nullptr)
	{
		::CloseHandle(*emptyEvent_);
		delete emptyEvent_;
	}

	if (managementThreadAddress_ != nullptr)
	{
		delete managementThreadAddress_;
	}

	::CloseHandle(managementThread_);
}

bool ThreadPool::isDestroyed()
{
	bool result = true;
	::EnterCriticalSection(fieldsSection_);
	result = isDestroyed_;
	::LeaveCriticalSection(fieldsSection_);
	return result;
}

int ThreadPool::getTotalPendingTasks()
{
	return this->getUnitListSize();
}

int ThreadPool::getAvailableThreads()
{
	int result = 0;

	::EnterCriticalSection(fieldsSection_);

	int maxThreads = this->getMaxThreads();
	int threadListSize = this->getThreadListSize();
	
	if (INVALID_RESULT == maxThreads || INVALID_RESULT == threadListSize)
	{
		result = INVALID_RESULT;
	}
	else
	{
		result = maxThreads - threadListSize;
	}

	::LeaveCriticalSection(fieldsSection_);

	return result;
}

int ThreadPool::getThreadListSize() 
{
	int result = INVALID_RESULT;
	::EnterCriticalSection(threadsSection_);
	if (threadList_ != nullptr)
	{
		result = (int) threadList_->size();
	}	
	::LeaveCriticalSection(threadsSection_);
	return result;
}

int ThreadPool::getUnitListSize()
{
	int length = INVALID_RESULT;

	::EnterCriticalSection(unitsSection_);
	if (unitsList_ != nullptr)
	{
		length = (int)unitsList_->size();
	}
	::LeaveCriticalSection(unitsSection_);

	return length;
}

bool ThreadPool::setMaxThreads(int workerThreads)
{
	bool result = false;
	EnterCriticalSection(fieldsSection_);
	if (getMinThreads() <= workerThreads)
	{
		maxThreads_ = workerThreads;
		result = true;
	}
	LeaveCriticalSection(fieldsSection_);
	return result;
}

int ThreadPool::getMaxThreads()
{
	int result = 0;
	EnterCriticalSection(fieldsSection_);
	result = maxThreads_;
	LeaveCriticalSection(fieldsSection_);
	return result;
}

bool ThreadPool::setMinThreads(int workerThreads)
{
	bool result = true;
	EnterCriticalSection(fieldsSection_);

	if (workerThreads >= 0)
	{
		minThreads_ = workerThreads;
	}
	else
	{
		result = false;
	}

	LeaveCriticalSection(fieldsSection_);
	return result;
}

int ThreadPool::getMinThreads()
{
	int result = 0;
	EnterCriticalSection(fieldsSection_);
	result = minThreads_;
	LeaveCriticalSection(fieldsSection_);
	return result;
}

bool ThreadPool::setTimeoutInMs(time_t milliseconds)
{
	bool result = true;
	EnterCriticalSection(fieldsSection_);
	if (milliseconds == INFINITE || milliseconds >= 0)
	{
		timeout_ = milliseconds;
	}
	else
	{
		result = false;
	}
	LeaveCriticalSection(fieldsSection_);
	return result;
}

time_t ThreadPool::getTimeoutInMs()
{
	time_t result = 0;
	EnterCriticalSection(fieldsSection_);
	result = timeout_;
	LeaveCriticalSection(fieldsSection_);	
	return result;
}

time_t ThreadPool::getTimeoutInSeconds()
{
	time_t result = 0;
	EnterCriticalSection(fieldsSection_);
	if (timeout_ = INFINITE)
	{
		result = INFINITE;
	}
	else
	{
		result = timeout_ / 1000;
	}
	LeaveCriticalSection(fieldsSection_);
	return result;
}

bool ThreadPool::isDestroySafe()
{
	bool result = false;
	::EnterCriticalSection(fieldsSection_);
	result = isDestroySafe_;
	::LeaveCriticalSection(fieldsSection_);
	return result;
}

time_t ThreadPool::getTrackingInterval()
{
	time_t result = 0;
	EnterCriticalSection(fieldsSection_);
	result = trackingInterval_;
	LeaveCriticalSection(fieldsSection_);
	return result;
}



