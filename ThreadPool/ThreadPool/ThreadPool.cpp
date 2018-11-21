#include "ThreadPool.h"

ThreadPool::ThreadPool(int maxThreads, int maxIdleTime)
{
	// fields initialization
	this->setMinThreads(ThreadPool::DEFAULT_MIN_THREADS);
	this->setMaxThreads(maxThreads);
	this->setMaxTimeout(maxIdleTime);

	unitsList_ = new std::vector<UnitOfWork *>();
	threadList_ = new std::vector<WorkTask *>();

	// synchronizing sections initialization
	unitsSection_ = new CRITICAL_SECTION();
	destoyedSection_ = new CRITICAL_SECTION();
	threadsSection_ = new CRITICAL_SECTION();
	managementSection_ = new CRITICAL_SECTION();
	::InitializeCriticalSectionAndSpinCount(unitsSection_, DEFAULT_SPIN_COUNT);
	::InitializeCriticalSectionAndSpinCount(destoyedSection_, DEFAULT_SPIN_COUNT);
	::InitializeCriticalSectionAndSpinCount(threadsSection_, DEFAULT_SPIN_COUNT);
	::InitializeCriticalSectionAndSpinCount(managementSection_, DEFAULT_SPIN_COUNT);

	// events creation
	availableEvent_ = new HANDLE(::CreateEvent(nullptr, true, false, nullptr));
	emptyEvent_ = new HANDLE(::CreateEvent(nullptr, true, false, nullptr));
	finishedEvent_ = new HANDLE(::CreateEvent(nullptr, true, false, nullptr));
	startedEvent_ = new HANDLE(::CreateEvent(nullptr, true, false, nullptr));

	// start management thread
	keepManagementThreadRunning_ = true;

	managementThread_ = (HANDLE) ::_beginthreadex(nullptr, 0, 
		(_beginthreadex_proc_type) ThreadPool::keepManagement,
		(void *) this, 0, managementThreadAddress_);
}
	
void ThreadPool::closeNow()
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

	::EnterCriticalSection(threadsSection_);	
	//  no need in ::WaitForMultipleObjects() : task.close() kills all the threads
	for (WorkTask* task : *threadList_)
	{
		if (task != nullptr)
		{
			task->close();
		}
	}	
	::LeaveCriticalSection(threadsSection_);

	// free resources
	this->releaseMemory();	
}

void ThreadPool::closeSafely()
{
	if (isDestroyed())
	{
		return;
	}
	else 
	{
		::WaitForSingleObject(*startedEvent_, INFINITE);
		::WaitForSingleObject(*finishedEvent_, INFINITE);
		printf(" # WILL DESTROYED\n");
		this->closeNow();
	}
}

void ThreadPool::releaseMemory()
{
	if (maxThreads_ != nullptr)
	{
		delete maxThreads_;
	}
	if (minThreads_ != nullptr)
	{
		delete minThreads_;
	}
	if (maxTimeout_ != nullptr)
	{
		delete maxTimeout_;
	}
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
	::DeleteCriticalSection(destoyedSection_);
	::DeleteCriticalSection(managementSection_);

	::CloseHandle(*availableEvent_);
	::CloseHandle(*finishedEvent_);
	::CloseHandle(*startedEvent_);
	::CloseHandle(*emptyEvent_);

	delete availableEvent_;
	delete finishedEvent_;
	delete startedEvent_;
	delete emptyEvent_;

	delete managementSection_;
	delete destoyedSection_;
	delete threadsSection_;
	delete unitsSection_;
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
		//::ResetEvent(emptyEvent_);
	}
	::LeaveCriticalSection(unitsSection_);

	// check if idling thread exists
	
	::EnterCriticalSection(threadsSection_);
	::ResetEvent(*emptyEvent_);
	/*
	bool idleThreadExists = false;	
	for (WorkTask *t : *threadList_)
	{
		if (!t->isBusy())
		{
			t->wakeUp();
			idleThreadExists = true;
			break;
		}
	}		
	*/	
	// new thread creating if conditions are correct
	//if (!idleThreadExists && threadList_->size() < getMaxThreads())
	if (threadList_->size() < getMaxThreads())
	{
		WorkTask *t = new WorkTask(unitsList_, availableEvent_, /*emptyEvent_,*/ unitsSection_, maxTimeout_); 
		threadList_->push_back(t);			
	}	

	::LeaveCriticalSection(threadsSection_);	

	return true;
}

bool ThreadPool::isDestroyed()
{
	bool result;
	::EnterCriticalSection(destoyedSection_);
	result = isDestroyed_;
	::LeaveCriticalSection(destoyedSection_);
	return result;
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
					if (::time(nullptr) - w->getLastOperationTime() > t->getMaxTimeout())
					{
						w->close(); 
						threads->erase(threads->begin() + i); // delete item # i 
						threadListSize--; 
						i--;
					}
				}
			}				
			printf(" # SetEvent\n");
			::EnterCriticalSection(t->managementSection_);
			::SetEvent(*(t->startedEvent_));
			::LeaveCriticalSection(t->managementSection_);

			::LeaveCriticalSection(t->threadsSection_);			
		}
		catch(...)
		{		
			exception = std::current_exception();
		}
		::Sleep((DWORD) 1000 * t->getMaxTimeout());

		printf("# keep running : %i\n\n", t->keepManagementThreadRunning_ ? 1 : 0);
	}

	::EnterCriticalSection(t->managementSection_);	
	printf("# &t->finishedEvent_\n");
	::SetEvent(*(t->finishedEvent_));
	::LeaveCriticalSection(t->managementSection_);

	/*::EnterCriticalSection(&t->threadsSection_);
	if (t->getThreadListSize() == 0)
	{
		::SetEvent(t->emptyEvent_);
	}
	::LeaveCriticalSection(&t->threadsSection_);*/

	printf("management thread succeeded %d\n", (int)t->managementThread_);
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

bool ThreadPool::setMaxTimeout(int seconds)
{
	if (maxTimeout_ == nullptr)
	{
		maxTimeout_ = new int(ThreadPool::DEFAULT_TIMEOUT);
	}
	bool result = false;
	if (seconds > 0)
	{
		*maxTimeout_ = seconds;
		result = true;
	};
	return result;
}

int ThreadPool::getMaxTimeout()
{
	if (maxTimeout_ == nullptr)
	{
		return INVALID_RESULT;
	}
	return *maxTimeout_;
}



