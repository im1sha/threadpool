#include "WorkTask.h"

WorkTask::WorkTask(std::vector<UnitOfWork>* workQueue, HANDLE availableEvent, CRITICAL_SECTION queueSection, time_t timeout)
{
	workQueue_ = workQueue;
	availableEvent_ = availableEvent;
	queueSection_ = queueSection;
	waitTimeout_ = timeout;
	lastOperationTime_ = ::time(nullptr);

	thread_ = (HANDLE) ::_beginthreadex(nullptr, 0, (_beginthreadex_proc_type) WorkTask::startExecutableLoop, 
		this, 0, runningThread_);	
}

WorkTask::~WorkTask()
{
	this->close();
}

void WorkTask::close() 
{
	busy_ = false;
	shouldKeepRunning_ = false;	
	this->interrupt(thread_, waitTimeout_);
}

bool WorkTask::isBusy()
{
	return busy_;
}

time_t WorkTask::getLastOperationTime()
{
	return lastOperationTime_;
}

UnitOfWork WorkTask::dequeue()
{
	UnitOfWork* t = nullptr;

	::EnterCriticalSection(&queueSection_);
	if ((workQueue_ != nullptr) && (workQueue_->size() != 0))
	{
		*t = (*workQueue_)[0];
		workQueue_->erase(workQueue_->begin());
		if (workQueue_->size() == 0)
		{
			::ResetEvent(availableEvent_);
		}
	}

	// TO-DO : add __finally to CriticalSections
	::LeaveCriticalSection(&queueSection_);

	return *t;
}

void WorkTask::interrupt(HANDLE hThread, time_t waitTimeout)
{	
	DWORD returnValue = ::WaitForSingleObject(hThread, (DWORD) waitTimeout);
	
	switch (returnValue)
	{
	case WAIT_OBJECT_0:
		// terminated itself
		// no action needed
		break;
	default:
		// ::_endthreadex(-1);
		::TerminateThread(hThread, -1);
		break;
	}

	/*

	if (runningThread_ != nullptr)
	{
		delete runningThread_;
	}
	if (hThread_ != nullptr)
	{
		::CloseHandle(hThread_);
	}

	*/
	
}

void WorkTask::wakeUp()
{
	this->interrupt(thread_, waitTimeout_);
	busy_ = true;
}

unsigned WorkTask::startExecutableLoop(WorkTask task)
{
	unsigned exitCode = (&task != nullptr) ? 0 : -1;

	if (exitCode != 0) 
	{ 
		return exitCode;
	}
	
	UnitOfWork* u = nullptr;

	while (task.shouldKeepRunning_)
	{
		std::exception_ptr exception;
		try
		{
			while (task.shouldKeepRunning_)
			{
				while ((u == nullptr) && task.shouldKeepRunning_)
				{
					WaitForSingleObject(task.availableEvent_, (DWORD) task.waitTimeout_);
					*u = task.dequeue();
				}

				if ((u != nullptr) && (u->getMethod() != nullptr))
				{			
					task.busy_ = true;

					std::function<void(void *)> functionToExecute = u->getMethod();

					void * functionParameters = u->getParemeters();

					task.lastOperationTime_ = ::time(nullptr);

					functionToExecute(functionParameters);

					u->~UnitOfWork();
					u = nullptr;
				}
			}
			task.busy_ = false;
		}
		catch (...)
		{
			exception = std::current_exception();
			exception.~exception_ptr();
		}
	}
	
	return 0;
}

