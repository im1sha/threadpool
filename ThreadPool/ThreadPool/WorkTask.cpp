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


void WorkTask::close(bool forced, time_t timeout)
{		
	::EnterCriticalSection(localFieldSection_);
	
	time_t prefferedTimeout = getTimeoutInMs();
	if (forced)
	{
		if (timeout >= 0 || timeout == INFINITE)
		{
			prefferedTimeout = timeout;
		}
	}

	shouldKeepRunning_ = false;
	busy_ = false;
	WorkTask::interrupt(thread_, prefferedTimeout);
	delete runningThreadAddress_;
	::CloseHandle(thread_);

	::LeaveCriticalSection(localFieldSection_);
	::DeleteCriticalSection(localFieldSection_);
	delete localFieldSection_;
}

void WorkTask::interrupt(HANDLE hThread, time_t msWaitTimeout)
{
	printf("interrupt call : %d\n", (int)hThread);

	DWORD returnValue = ::WaitForSingleObject(hThread, (DWORD) msWaitTimeout);

	if (returnValue == WAIT_OBJECT_0)
	{
		// terminated itself
		// no actions needed
	}
	else
	{
		::TerminateThread(hThread, INVALID_RESULT);
		printf("terminated  %d", (int)hThread);
	}
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

	printf("started %d\n", (int) task->thread_);

	if (exitCode != 0) 
	{ 
		return exitCode;
	}

	UnitOfWork* u = nullptr;
	while (task->shouldKeepRunning_)
	{
		std::exception_ptr exception;
		try
		{
			while (task->shouldKeepRunning_)
			{
				while ((u == nullptr) && task->shouldKeepRunning_)
				{
					::WaitForSingleObject(*(task->availableEvent_), (DWORD) task->getTimeoutInMs());
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
	printf("succeeded  %d\n", (int) task->thread_);
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

