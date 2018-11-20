#include "WorkTask.h"

WorkTask::WorkTask(std::vector<UnitOfWork*> * workQueue, HANDLE availableEvent, CRITICAL_SECTION queueSection, int* timeout)
{
	unitsQueue_ = workQueue;
	availableEvent_ = availableEvent;
	unitsSection_ = queueSection;
	waitTimeout_ = timeout;
	lastOperationTime_ = ::time(nullptr);

	busy_ = true;
	shouldKeepRunning_ = true;

	thread_ = (HANDLE) ::_beginthreadex(nullptr, 0, 
		(_beginthreadex_proc_type) WorkTask::startExecutableLoop, 
		(void *) this, 0, runningThread_);	
}

WorkTask::~WorkTask()
{
	if (!isDestroyed_)
	{
		isDestroyed_ = true;
		this->close();
	}
}

void WorkTask::close() 
{	
	isDestroyed_ = true;
	shouldKeepRunning_ = false;
	busy_ = false;
	this->interrupt(thread_, (time_t) waitTimeout_);
}

UnitOfWork* WorkTask::dequeue()
{
	UnitOfWork* result = nullptr;

	::EnterCriticalSection(&unitsSection_);
	if ((unitsQueue_ != nullptr) && (unitsQueue_->size() != 0))
	{
		result = new UnitOfWork(*((*unitsQueue_)[0]));
		unitsQueue_->erase(unitsQueue_->begin());
		if (unitsQueue_->size() == 0)
		{
			::ResetEvent(availableEvent_);
		}
	}
	::LeaveCriticalSection(&unitsSection_);

	return result;
}

void WorkTask::interrupt(HANDLE hThread, time_t secondsWaitTimeout)
{	
	printf("interrupt call  %d\n", (int) hThread);
	DWORD returnValue = ::WaitForSingleObject(hThread, (DWORD) (1000 * secondsWaitTimeout));
	
	if (returnValue == WAIT_OBJECT_0) 
	{
		// terminated itself
		// no actions needed
	}
	else
	{
		::TerminateThread(hThread, -1);
		printf("terminated  %d", (int) hThread);
	}
}

void WorkTask::wakeUp()
{
	this->interrupt(thread_, (time_t) waitTimeout_);
	busy_ = true;

	// _beginthreadex here
}

unsigned WorkTask::startExecutableLoop(WorkTask* task) 
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
					WaitForSingleObject(task->availableEvent_, (DWORD) *(task->waitTimeout_) * 1000 /*INFINITY*/);
					u = task->dequeue();
				}		

				if ((u != nullptr) && (u->getMethod() != nullptr))
				{			
					task->busy_ = true;

					std::function<void(void *)> functionToExecute = u->getMethod();
					void * functionParameters = u->getParameters();

					task->lastOperationTime_ = ::time(nullptr);

					functionToExecute(functionParameters);

					delete u;
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
	printf("succeeded  %d\n", (int) task->thread_);
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

