#include "ThreadPool.h"

ThreadPool::ThreadPool(int maxThreads, int maxIdleTime)
{
	// fields initialization
	this->setMinThreads(ThreadPool::DEFAULT_MIN_THREADS);
	this->setMaxThreads(maxThreads);
	this->setMaxIdleTime(maxIdleTime);

	unitsList_ = new std::vector<UnitOfWork>();
	threadList_ = new std::vector<WorkTask>();

	// synchronizing items initialization
	::InitializeCriticalSectionAndSpinCount(&unitsSection_, DEFAULT_SPIN_COUNT);
	::InitializeCriticalSectionAndSpinCount(&threadsSection_, DEFAULT_SPIN_COUNT);
	availableEvent_ = ::CreateEvent(nullptr, true, false, nullptr);

	// start management thread
	keepManagementThreadRunning_ = true;

	managementThread_ = (HANDLE) ::_beginthreadex(nullptr, 0, (_beginthreadex_proc_type) ThreadPool::keepManagement,
		(void *) this, 0, managementThreadAddress_);
}
	

ThreadPool::~ThreadPool()
{
	this->close();	
}

void ThreadPool::close()
{
	// managementThread_ destroying
	keepManagementThreadRunning_ = false;
	if (managementThread_ != nullptr)
	{
		WorkTask::interrupt(managementThread_, INFINITE);
		managementThread_ = nullptr;
	}	

	::EnterCriticalSection(&threadsSection_);	
	//  no need in ::WaitForMultipleObjects() : task.close() kills all the threads
	for (WorkTask task : *threadList_)
	{
		task.close();		
	}	
	::LeaveCriticalSection(&threadsSection_);

	// free resources
	this->deleteFields();
	::DeleteCriticalSection(&unitsSection_);
	::DeleteCriticalSection(&threadsSection_);
	::CloseHandle(availableEvent_);
}

void ThreadPool::deleteFields()
{
	if (maxThreads_ != nullptr)
	{
		delete maxThreads_;
	}
	if (minThreads_ != nullptr)
	{
		delete minThreads_;
	}
	if (maxIdleTime_ != nullptr)
	{
		delete maxIdleTime_;
	}
	if (unitsList_ != nullptr)
	{
		delete unitsList_;
	}
	if (threadList_ != nullptr)
	{
		delete threadList_;
	}	
}


void ThreadPool::enqueue(UnitOfWork task)
{
	// add task 
	::EnterCriticalSection(&unitsSection_);
	if (unitsList_ != nullptr)
	{
		unitsList_->push_back(task);
	}	
	::LeaveCriticalSection(&unitsSection_);

	// signal to waiting for task thread if it's first task
	if (getUnitListSize() == 1)
	{
		::SetEvent(availableEvent_);
	}
		
	// check if idling thread exists
	bool idleThreadExists = false;
	::EnterCriticalSection(&threadsSection_);
	for (WorkTask t : *threadList_)
	{
		if (!t.isBusy())
		{
			t.wakeUp();
			idleThreadExists = true;
			break;
		}
	}	
	if (!idleThreadExists)
	{
		// new thread creating if conditions are correct
		if (threadList_->size() < getMaxThreads())
		{
			WorkTask t(unitsList_, availableEvent_, unitsSection_, maxIdleTime_); 
			threadList_->push_back(t);			
		}
	}	
	::LeaveCriticalSection(&threadsSection_);	
	printf("pushed\n");

}

void ThreadPool::keepManagement(ThreadPool* t)
{
	if (t == nullptr)
	{
		return;
	}
	while (t->keepManagementThreadRunning_)
	{
		std::exception_ptr exception;		
		try
		{										
			std::vector<WorkTask> * threads = t->threadList_;
			
			::EnterCriticalSection(&t->threadsSection_);

			int threadListSize = (int) threads->size();

			printf("1.>>%i\n", (int)threads->size());

			if (threadListSize > t->getMinThreads())
			{
				for (size_t i = 0; i < threadListSize; i++)
				{				
					printf("2.>>%i\n", (int)threads->size());
					WorkTask w = ((*threads)[i]);

					// thread destroying if timeout is exceeded
					if (::time(nullptr) - w.getLastOperationTime() > t->getMaxIdleTime())
					{
						w.close(); 
						threads->erase(threads->begin() + i); // delete item # i 
					}
				}
			}					
			::LeaveCriticalSection(&t->threadsSection_);
			
		}
		catch(...)
		{
			exception = std::current_exception();
			exception.~exception_ptr();
		}
		::Sleep((DWORD) 1000 * t->getMaxIdleTime());
	}
}


int ThreadPool::getTotalPendingTasks()
{
	return this->getUnitListSize();
}

int ThreadPool::getAvailableThreads()
{
	int maxThreads = this->getMaxThreads();
	int threadListSize = this->getThreadListSize();
	if (INVALID_RESULT == maxThreads || INVALID_RESULT == threadListSize)
	{
		return INVALID_RESULT;
	}
	return maxThreads - threadListSize;
}


int ThreadPool::getThreadListSize() 
{
	int result = INVALID_RESULT;
	::EnterCriticalSection(&threadsSection_);
	if (threadList_ != nullptr)
	{
		result = (int) threadList_->size();
	}	
	::LeaveCriticalSection(&threadsSection_);
	return result;
}

int ThreadPool::getUnitListSize()
{
	int length = INVALID_RESULT;

	::EnterCriticalSection(&unitsSection_);
	if (unitsList_ != nullptr)
	{
		length = (int)unitsList_->size();
	}
	::LeaveCriticalSection(&unitsSection_);

	return length;
}


bool ThreadPool::setMaxThreads(int workerThreads)
{
	if (minThreads_ == nullptr)
	{
		minThreads_ = new int(ThreadPool::DEFAULT_MIN_THREADS);
	}
	if (maxThreads_ == nullptr)
	{
		maxThreads_ = new int(ThreadPool::DEFAULT_MAX_THREADS);
	}
	bool result = false;
	if (*minThreads_ <= workerThreads)
	{
		*maxThreads_ = workerThreads;
		result = true;
	}
	return result;
}

int ThreadPool::getMaxThreads()
{
	if (maxThreads_ == nullptr)
	{
		return INVALID_RESULT;
	}
	return *maxThreads_;
}


bool ThreadPool::setMinThreads(int workerThreads)
{
	if (minThreads_ == nullptr)
	{
		minThreads_ = new int(ThreadPool::DEFAULT_MIN_THREADS);
	}
	if (maxThreads_ == nullptr)
	{
		maxThreads_ = new int(ThreadPool::DEFAULT_MAX_THREADS);
	}
	bool result = false;
	if ((1 <= workerThreads) && (workerThreads <= *maxThreads_))
	{
		*minThreads_ = workerThreads;
		result = true;
	}
	return result;
}

int ThreadPool::getMinThreads()
{
	if (minThreads_ == nullptr)
	{
		return INVALID_RESULT;
	}
	return *minThreads_;
}

//bool ThreadPool::setManagementInterval(int millisecondsTimeout)
//{
//	bool result = false;
//	if (millisecondsTimeout > 0)
//	{
//		managementInterval_ = millisecondsTimeout;
//		result = true;
//	};
//	return result;
//}
//
//int ThreadPool::getManagementInterval()
//{
//	return managementInterval_;
//}

bool ThreadPool::setMaxIdleTime(int seconds)
{
	if (maxIdleTime_ == nullptr)
	{
		maxIdleTime_ = new int(ThreadPool::DEFAULT_IDLE_TIME);
	}
	bool result = false;
	if (seconds > 0)
	{
		*maxIdleTime_ = seconds;
		result = true;
	};
	return result;
}

int ThreadPool::getMaxIdleTime()
{
	if (maxIdleTime_ == nullptr)
	{
		return INVALID_RESULT;
	}
	return *maxIdleTime_;
}



