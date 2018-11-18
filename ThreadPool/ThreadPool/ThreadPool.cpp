#include "ThreadPool.h"

ThreadPool::ThreadPool(int maxThreads, int msTimeout) : ThreadPool::ThreadPool()
{
	setMaxThreads(maxThreads);
}

ThreadPool::ThreadPool()
{
	minThreads_ = 1;
	if (maxThreads_ == 0)
	{
		maxThreads_ = minThreads_;
	}

	unitsQueue_ = new std::vector<UnitOfWork>();
	::InitializeCriticalSectionAndSpinCount(&unitsSection_, DEFAULT_SPIN_COUNT);
	::InitializeCriticalSectionAndSpinCount(&threadsSection_, DEFAULT_SPIN_COUNT);
	availableEvent_ = ::CreateEvent(nullptr, true, false, nullptr);

	managementThread_ = (HANDLE) ::_beginthreadex(nullptr, 0, (_beginthreadex_proc_type) ThreadPool::keepManagement,
		this, 0, managementThreadAddress_);
}
	
ThreadPool::~ThreadPool()
{
	this->close();	
}

void ThreadPool::enqueue(UnitOfWork task)
{
	::EnterCriticalSection(&unitsSection_);
	unitsQueue_->push_back(task);
	if (unitsQueue_->size() == 1)
	{
		::SetEvent(availableEvent_);
	}
	::LeaveCriticalSection(&unitsSection_);
	
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
		if (threadList_->size() < maxThreads_)
		{
			WorkTask* t = new WorkTask(unitsQueue_, availableEvent_, unitsSection_, getMaxIdleTime()); 

			// should be deleted later
			// with delete call

			threadList_->push_back(*t);
		}
	}	
	::LeaveCriticalSection(&threadsSection_);
}

void ThreadPool::close()
{	
	keepManagementThreadRunning_ = false;

	if (managementThread_ != nullptr)
	{
		WorkTask::interrupt(managementThread_, 2 * getMaxIdleTime());
	}

	managementThread_ = nullptr;
	
	::EnterCriticalSection(&threadsSection_);
	for (WorkTask task : *threadList_)
	{
		task.close();
	}

	// wait 'till all threads destroyed
	// ! replace with wait function
	::Sleep(getMaxIdleTime());

	::LeaveCriticalSection(&threadsSection_);

	delete unitsQueue_;
	delete threadList_;
	::DeleteCriticalSection(&unitsSection_);
	::DeleteCriticalSection(&threadsSection_);
	::CloseHandle(availableEvent_);
}

void ThreadPool::keepManagement(ThreadPool* t)
{
	while (t->keepManagementThreadRunning_)
	{
		std::exception_ptr exception;
		try
		{							
			::EnterCriticalSection(&t->threadsSection_);

			std::vector<WorkTask> * list = t->threadList_;

			if(list->size() > t->minThreads_)
			{
				for (size_t i = 0; i < list->size(); i++)
				{				
					WorkTask w = (*list)[i];
					if (::time(nullptr) - w.getLastOperationTime() > t->maxIdleTime_)
					{
						w.close();
						list->erase(list->begin() + i); // delete item # i
					}
				}
			}							
		}
		catch(...)
		{
			exception = std::current_exception();
			exception.~exception_ptr();
		}
		::LeaveCriticalSection(&t->threadsSection_);
		::Sleep((DWORD) t->getMaxIdleTime());
	}

}

int ThreadPool::getLength()
{
	int length = 0;

	::EnterCriticalSection(&unitsSection_);

	length = (int)unitsQueue_->size();

	::LeaveCriticalSection(&unitsSection_);

	return length;
}

int ThreadPool::getAvailableThreads()
{
	return maxThreads_ - (int) threadList_->size();
}

bool ThreadPool::setMaxThreads(int workerThreads)
{
	bool result = false;
	if ((1 <= workerThreads) && (minThreads_ <= workerThreads))
	{
		maxThreads_ = workerThreads;
		result = true;
	}
	return result;
}

int ThreadPool::getMaxThreads()
{
	return maxThreads_;
}

bool ThreadPool::setMinTreads(int workerThreads)
{
	bool result = false;
	if ((1 <= workerThreads) && (workerThreads <= maxThreads_))
	{
		minThreads_ = workerThreads;
		result = true;
	}
	return result;
}

int ThreadPool::getMinThreads()
{
	return minThreads_;
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

bool ThreadPool::setMaxIdleTime(int seconds)
{
	bool result = false;
	if (seconds > 0)
	{
		maxIdleTime_ = seconds;
		result = true;
	};
	return result;
}

int ThreadPool::getMaxIdleTime()
{
	return maxIdleTime_;
}
