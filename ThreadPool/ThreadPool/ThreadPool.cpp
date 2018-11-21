#include "ThreadPool.h"

ThreadPool::ThreadPool(int maxThreads, int timeout)
{
	// fields initialization
	this->setMinThreads(ThreadPool::DEFAULT_MIN_THREADS);
	this->setMaxThreads(maxThreads);
	this->setTimeoutInMs(timeout);

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

	// start management thread
	keepManagementThreadRunning_ = true;

	managementThread_ = (HANDLE) ::_beginthreadex(nullptr, 0, 
		(_beginthreadex_proc_type) ThreadPool::keepManagement,
		(void *) this, 0, managementThreadAddress_);
}


// =========================================


void ThreadPool::closeSafely()
{
	if (isDestroyed())
	{
		return;
	}
	else
	{
		::WaitForSingleObject(*startedEvent_, INFINITE);
		printf("@ this->closeNow\n");

		// add safeClose field

		this->interrupt();
	}
}

void ThreadPool::interrupt()
{
	if (isDestroyed())
	{
		return;
	}

	::EnterCriticalSection(destoyedSection_);
	isDestroyed_ = true;
	::LeaveCriticalSection(destoyedSection_);

	// managementThread_ destroying
	keepManagementThreadRunning_ = false;

	if (managementThread_ != nullptr)
	{
		WorkTask::interrupt(managementThread_, INFINITE);
		managementThread_ = nullptr;
	}	

	killThreads();

	// free resources
	this->releaseMemory();	
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

	// signal to waiting for task thread if it's first task
	if (getUnitListSize() == 1)
	{
		::SetEvent(*availableEvent_);
	}
	::LeaveCriticalSection(unitsSection_);

	
	::EnterCriticalSection(threadsSection_);

	::ResetEvent(*emptyEvent_);	
	// new thread creating if conditions are correct
	if (threadList_->size() < getMaxThreads())
	{
		WorkTask *t = new WorkTask(unitsList_, availableEvent_, emptyEvent_, unitsSection_); 
		threadList_->push_back(t);			
	}	

	::LeaveCriticalSection(threadsSection_);	
	return true;
}

void ThreadPool::keepManagement(ThreadPool* t)
{
	if (t == nullptr)
	{
		return;
	}

	printf("management thread started %d\n", (int)t->managementThread_);

	while (t->keepManagementThreadRunning_)
	{		
		std::exception_ptr exception;		
		try
		{																
			::EnterCriticalSection(t->threadsSection_);

			std::vector<WorkTask*> * threads = t->threadList_;
			int threadListSize = (int) t->getThreadListSize();

			if (threadListSize > t->getMinThreads())
			{
				for (size_t i = 0; i < threadListSize; i++)
				{				
					WorkTask* w = (*threads)[i];

					// thread destroying if timeout is exceeded
					if (::time(nullptr) - w->getLastOperationTime() > t->getTimeoutInMs())
					{
						w->close(); 
						threads->erase(threads->begin() + i); // delete item # i 
						threadListSize--; 
						i--;
					}
				}
			}				
			::EnterCriticalSection(t->fieldsSection_);
			::SetEvent(*(t->startedEvent_));
			::LeaveCriticalSection(t->fieldsSection_);

			::LeaveCriticalSection(t->threadsSection_);			
		}
		catch(...)
		{		
			exception = std::current_exception();
		}
		::Sleep((DWORD) 1000 * t->getTimeoutInMs());

	}

	printf("management thread succeeded %d\n", (int)t->managementThread_);
}

void ThreadPool::close() {}


// =========================================


void ThreadPool::killThreads(bool force, time_t timeout)
{
	::EnterCriticalSection(threadsSection_);

	for (WorkTask* task : *threadList_)
	{
		if (task != nullptr)
		{
			task->close(force, timeout);
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

	::DeleteCriticalSection(unitsSection_);
	::DeleteCriticalSection(threadsSection_);
	::DeleteCriticalSection(fieldsSection_);

	::CloseHandle(*availableEvent_);
	::CloseHandle(*startedEvent_);
	::CloseHandle(*emptyEvent_);

	::CloseHandle(managementThread_);
	delete managementThreadAddress_;

	delete availableEvent_;
	delete startedEvent_;
	delete emptyEvent_;

	delete fieldsSection_;
	delete threadsSection_;
	delete unitsSection_;
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



