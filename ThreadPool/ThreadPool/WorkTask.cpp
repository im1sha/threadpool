#include "WorkTask.h"

WorkTask::WorkTask(std::vector<UnitOfWork*> * unitList, HANDLE* availableEvent, HANDLE* emptyEvent, CRITICAL_SECTION * unitsSection)
{
	unitsQueue_ = unitList;
	availableEvent_ = availableEvent;
	emptyEvent_ = emptyEvent;
	unitsSection_ = unitsSection;
	lastOperationTimeInSeconds_ = ::time(nullptr);

	localFieldSection_ = new CRITICAL_SECTION();
	::InitializeCriticalSection(localFieldSection_);

	busy_ = true;
	shouldKeepRunning_ = true;

	thread_ = (HANDLE) ::_beginthreadex(nullptr, 0, 
		(_beginthreadex_proc_type) WorkTask::startExecuting, 
		(void *) this, 0, runningThreadAddress_);	
}

bool WorkTask::tryClose(bool forced, time_t timeout)
{	
	bool isDestroyed = true;

	//if (forced)
	//{
	::EnterCriticalSection(localFieldSection_);
	shouldKeepRunning_ = false;
	::LeaveCriticalSection(localFieldSection_);
	//{

	isDestroyed = WorkTask::interrupt(thread_, timeout, forced);

	if (isDestroyed)
	{
		delete runningThreadAddress_;
		::CloseHandle(thread_);
		::DeleteCriticalSection(localFieldSection_);
		delete localFieldSection_;
	}
	else
	{
		
	}
	
	return isDestroyed;
}

bool WorkTask::interrupt(HANDLE hThread, time_t msWaitTimeout, bool forced)
{
	DWORD returnValue = ::WaitForSingleObject(hThread, (DWORD) msWaitTimeout);

	if ((returnValue != WAIT_OBJECT_0) && forced)
	{
		BOOL terminated = ::TerminateThread(hThread, INVALID_RESULT);
		if (terminated != 0)
		{
			printf("terminated  %lld\n", (long long)hThread);
			return true;
		}
	}

	if (returnValue == WAIT_OBJECT_0)
	{
		return true;
	}

	return false;
}

UnitOfWork* WorkTask::dequeue()
{
	UnitOfWork* result = nullptr;

	::EnterCriticalSection(unitsSection_);

	if ((unitsQueue_ != nullptr) && (unitsQueue_->size() != 0))
	{
		result = new UnitOfWork(*((*unitsQueue_)[0]));
		unitsQueue_->erase(unitsQueue_->begin());
		if (unitsQueue_->size() == 0)
		{
			::ResetEvent(*availableEvent_);
			::SetEvent(*emptyEvent_);
		}
	}

	::LeaveCriticalSection(unitsSection_);

	return result;
}

unsigned WorkTask::startExecuting(WorkTask* task) 
{
	unsigned exitCode = (task != nullptr) ? 0 : -1;

	printf("$ started %lld\n", (long long)task->thread_);

	if (exitCode != 0) 
	{ 
		return exitCode;
	}

	bool entryIteration = true; // instance created now and it needs reach while loop

	UnitOfWork* u = nullptr;
	while (task->shouldKeepRunning_ || entryIteration)
	{
		printf("inside");
		std::exception_ptr exception;
		try
		{
			while (task->shouldKeepRunning_ || entryIteration)
			{
				while (((u == nullptr) && task->shouldKeepRunning_) || entryIteration)
				{
					entryIteration = false;
					::WaitForSingleObject(*(task->availableEvent_), (DWORD) task->getManagementInterval());
					u = task->dequeue();
				}		

				if ((u != nullptr) && (u->getMethod() != nullptr))
				{			
					std::function<void(void **)> functionToExecute = u->getMethod();
					void ** functionParameters = u->getParameters();

					::EnterCriticalSection(task->localFieldSection_);

					task->busy_ = true;
					task->lastOperationTimeInSeconds_ = ::time(nullptr);
					task->waitTimeoutInMs_ = u->getTimeoutInMs();

					::LeaveCriticalSection(task->localFieldSection_);

					functionToExecute(functionParameters);

					delete u;
					u = nullptr;
				}

				::EnterCriticalSection(task->localFieldSection_);
				task->busy_ = false;
				::LeaveCriticalSection(task->localFieldSection_);
			}
		}
		catch (...)
		{
			exception = std::current_exception();
		}
	}
	printf("# succeeded %lld\n", (long long)task->thread_);
	return 0;
}


bool WorkTask::isBusy()
{
	bool result = false;
	::EnterCriticalSection(localFieldSection_);
	result = busy_;
	::LeaveCriticalSection(localFieldSection_);
	return result;
}

time_t WorkTask::getTimeoutInSeconds()
{
	time_t result = 0;
	::EnterCriticalSection(localFieldSection_);
	if (waitTimeoutInMs_ < 0)
	{
		result = INFINITE;
	}
	else
	{
		result = waitTimeoutInMs_ / 1000;
	}
	::LeaveCriticalSection(localFieldSection_);
	return result;
}

time_t WorkTask::getTimeoutInMs()
{
	time_t result = 0;
	::EnterCriticalSection(localFieldSection_);
	result = waitTimeoutInMs_;	
	::LeaveCriticalSection(localFieldSection_);
	return result;
}

time_t WorkTask::getLastOperationTimeInSeconds()
{
	time_t result = 0;
	::EnterCriticalSection(localFieldSection_);
	result = lastOperationTimeInSeconds_;
	::LeaveCriticalSection(localFieldSection_);
	return result;
}

bool WorkTask::setManagementInterval(time_t milliseconds)
{
	bool result = false;
	::EnterCriticalSection(localFieldSection_);
	if (milliseconds > 0)
	{
		managementIntervalInMs_ = milliseconds;
		result = true;
	}
	::LeaveCriticalSection(localFieldSection_);
	return result;
}

time_t WorkTask::getManagementInterval()
{
	time_t result = 0;
	::EnterCriticalSection(localFieldSection_);
	result = managementIntervalInMs_;
	::LeaveCriticalSection(localFieldSection_);
	return result;
}

