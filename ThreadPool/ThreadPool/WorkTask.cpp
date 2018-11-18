#include "WorkTask.h"

WorkTask::WorkTask(std::vector<UnitOfWork>* workQueue, HANDLE queueCountSemaphore, CRITICAL_SECTION queueSection, time_t timeout)
{
	workQueue_ = workQueue;
	queueCountSemaphore_ = queueCountSemaphore;
	queueSection_ = queueSection;
	waitTimeout_ = timeout;
	lastOperationTime_ = ::time(nullptr);

	hThread_ = (HANDLE) ::_beginthreadex(
		nullptr,		
		0,				
		(_beginthreadex_proc_type) this->startExecutableLoop,
		this,
		0,				
		runningThread_);	
}

WorkTask::~WorkTask()
{
	this->close();
}

void WorkTask::close() 
{
	busy_ = false;
	shouldKeepRunning_ = false;	
	this->interrupt();	
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
	}

	// TO-DO : add __finally to CriticalSections
	::LeaveCriticalSection(&queueSection_);

	return *t;
}

size_t WorkTask::getQueueSize() 
{
	size_t length = 0;

	::EnterCriticalSection(&queueSection_);
	if (workQueue_ != nullptr)
	{
		length = workQueue_->size();
	}

	// TO-DO : add __finally to CriticalSections
	::LeaveCriticalSection(&queueSection_);

	return length;
}


void WorkTask::interrupt() 
{
	if (runningThread_ != nullptr)
	{
		DWORD returnValue = ::WaitForSingleObject(hThread_, (DWORD) waitTimeout_);
		switch (returnValue)
		{
		case WAIT_OBJECT_0:
			// terminated itself
			// no action needed
			break;
		default:
			::TerminateThread(hThread_, -1);
			break;
		}
		delete runningThread_;
		if (hThread_ != nullptr)
		{
			::CloseHandle(hThread_);
		}
	}
}

void WorkTask::wakeUp()
{
	this->interrupt();
	busy_ = true;
}

unsigned WorkTask::startExecutableLoop(WorkTask task)
{
	unsigned exitCode = (&task != nullptr) ? 0 : -1;

	if (exitCode != 0) 
	{ 
		::_endthreadex(exitCode); 
	}
	
	UnitOfWork* u = nullptr;

	while (task.shouldKeepRunning_)
	{
		std::exception_ptr exception;
		try
		{
			while (task.getQueueSize() > 0)
			{
				*u = task.dequeue();

				if ((u != nullptr) && (u->getMethod() != nullptr))
				{
					task.lastOperationTime_ = ::time(nullptr);
					task.busy_ = true;

					std::function<void(void*)> functionToExecute = u->getMethod();
					void* functionParameters = u->getParemeters();

					functionToExecute(functionParameters);

					u->~UnitOfWork();
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
	
	::_endthreadex(exitCode);
}