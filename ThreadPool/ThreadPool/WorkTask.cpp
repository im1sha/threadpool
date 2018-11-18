#include "WorkTask.h"



WorkTask::WorkTask(std::vector<UnitOfWork>* workQueue, CRITICAL_SECTION* determinant)
{
	workQueue_ = workQueue;
	emptyDeterminant_ = determinant;

	lastOperation_ = ::time(nullptr);

	// thread start should be here 
	runningThread_ = new uintptr_t();
	// *runningThread_ = new Thread(ExecutePassedMethod);
	// runningThread_->Start();
}


WorkTask::~WorkTask()
{
	if (runningThread_ != nullptr)
	{
		busy_ = false;
		shouldKeepRunning_ = false;
		if (/*runningThread_->ThreadState == ThreadState.WaitSleepJoin*/true)
		{
			//runningThread_->Interrupt();
		}
		//runningThread_->Join();

		delete runningThread_;
	}	
}

void WorkTask::close() 
{
	shouldKeepRunning_ = false;
	if (runningThread_ != nullptr)
	{
		if (/*runningThread_.ThreadState == ThreadState.WaitSleepJoin*/true)
		{
			/*runningThread_.Interrupt();*/
		}
		/*runningThread_.Join();*/
		delete runningThread_;
	}
}

bool WorkTask::isBusy()
{
	return busy_;
}

time_t WorkTask::getLastOperationTime()
{
	return lastOperation_;
}

UnitOfWork WorkTask::dequeue()
{
	UnitOfWork* t = nullptr;
	if ((workQueue_ != nullptr) && (workQueue_->size() != 0))
	{
		*t = (*workQueue_)[0];
		workQueue_->erase(workQueue_->begin());
	}
	return *t;
}

void WorkTask::wakeUp()
{
//if (runningThread_.ThreadState == runningThread_.WaitSleepJoin)
//{
//	runningThread_.Interrupt();
//}
//isBusy_ = true;
}


void WorkTask::startExecutableLoop()
{	
	UnitOfWork* u = nullptr;
	while (shouldKeepRunning_)
	{
		std::exception_ptr exception;
		try
		{
			while (workQueue_->size() > 0)
			{			
				*u = dequeue();	

				if ((u != nullptr) && (u->getMethod() != nullptr))
				{
					lastOperation_ = ::time(nullptr);
					busy_ = true;
					runUnitOfWork(u);
					u->~UnitOfWork();
				}
			}

			busy_ = false;
			//Thread.Sleep(ManagementInterval);
		}
		catch (...)
		{
			exception = std::current_exception();
		}
		// exception will be destroyed here
	}
}

void WorkTask::runUnitOfWork(UnitOfWork* u)
{


}

