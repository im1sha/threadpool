#include "WorkTask.h"

WorkTask::WorkTask(std::vector<UnitOfWork>* workQueue, HANDLE availableEvent, CRITICAL_SECTION queueSection, int* timeout)
{
	workQueue_ = workQueue;
	availableEvent_ = availableEvent;
	queueSection_ = queueSection;
	waitTimeout_ = timeout;
	lastOperationTime_ = ::time(nullptr);

	busy_ = true;
	shouldKeepRunning_ = true;

	thread_ = (HANDLE) ::_beginthreadex(nullptr, 0, (_beginthreadex_proc_type) WorkTask::startExecutableLoop, 
		(void *) this, 0, runningThread_);	
}

WorkTask::~WorkTask()
{
	this->close();
}

void WorkTask::close() 
{	
	shouldKeepRunning_ = false;
	busy_ = false;
	this->interrupt(thread_, (time_t) waitTimeout_);
}

UnitOfWork* WorkTask::dequeue()
{
	UnitOfWork* result = nullptr;

	::EnterCriticalSection(&queueSection_);
	if ((workQueue_ != nullptr) && (workQueue_->size() != 0))
	{
		result = new UnitOfWork((*workQueue_)[0]);
		workQueue_->erase(workQueue_->begin());
		if (workQueue_->size() == 0)
		{
			::ResetEvent(availableEvent_);
		}
	}

	// TO-DO : add __finally to CriticalSections
	::LeaveCriticalSection(&queueSection_);

	return result;
}

void WorkTask::interrupt(HANDLE hThread, time_t secondsWaitTimeout)
{	
	DWORD returnValue = ::WaitForSingleObject(hThread, (DWORD) (1000 * secondsWaitTimeout));
	
	if (returnValue == WAIT_OBJECT_0) 
	{
		// terminated itself
		// no actions needed
	}
	else
	{
		::TerminateThread(hThread, -1);
	}
}

void WorkTask::wakeUp()
{
	this->interrupt(thread_, (time_t) waitTimeout_);
	busy_ = true;

	// _beginthreadex here
}

unsigned WorkTask::startExecutableLoop(WorkTask* task) // ? use WorkTask instead
{
	unsigned exitCode = (task != nullptr) ? 0 : -1;

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
					WaitForSingleObject(task->availableEvent_, (DWORD) 1000 * (*(task->waitTimeout_)));
					u = task->dequeue();
				}		

				if ((u != nullptr) && (u->getMethod() != nullptr))
				{			
					task->busy_ = true;

					std::function<void(void *)> functionToExecute = u->getMethod();
					void * functionParameters = u->getParameters();

					task->lastOperationTime_ = ::time(nullptr);

					functionToExecute(functionParameters);

					u->~UnitOfWork();
					u = nullptr;
				}
				task->busy_ = false;
			}
		}
		catch (...)
		{
			exception = std::current_exception();
			exception.~exception_ptr();
		}
	}
	
	return 0;
}

bool WorkTask::isBusy()
{
	return busy_;
}

time_t WorkTask::getLastOperationTime()
{
	return lastOperationTime_;
}

