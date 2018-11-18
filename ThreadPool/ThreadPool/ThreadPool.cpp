#include "ThreadPool.h"

ThreadPool::ThreadPool(int maxThreads)
{
	workQueue_ = new std::vector<UnitOfWork>();
	::InitializeCriticalSectionAndSpinCount(&queueSection_, DEFAULT_SPIN_COUNT);
}

ThreadPool::~ThreadPool()
{
	::DeleteCriticalSection(&queueSection_);
	delete workQueue_;
}

void ThreadPool::enqueue(UnitOfWork task)
{
	::EnterCriticalSection(&queueSection_);
	workQueue_->push_back(task);
	::LeaveCriticalSection(&queueSection_);
}

//int ThreadPool::create()
//{
//	/*
//	_ACRTIMP uintptr_t __cdecl _beginthreadex(
//		_In_opt_  void*                    _Security,
//		_In_      unsigned                 _StackSize,
//		_In_      _beginthreadex_proc_type _StartAddress,
//		_In_opt_  void*                    _ArgList,
//		_In_      unsigned                 _InitFlag,
//		_Out_opt_ unsigned*                _ThrdAddr
//	);
//	*/
//
//
//	//_beginthreadex(
//	//	(void *)(psa),
//	//	(unsigned)(cbStack),
//	//	(_beginthreadex_proc_type)(pfnStartAddr),
//	//	(void *)(pvParam),
//	//	(unsigned)(fdwCreate),
//	//	(unsigned *)(pdwThreadID)
//	//);
//	//
//
//	/*uintptr_t pointerToThread =
//		_beginthreadex(nullptr, 0, );*/
//
//
//		/*
//		InitializeCriticalSection
//		EnterCriticalSection(&g_cs);
//		LeaveCriticalSection(&g_cs);*/
//		//CRITICAL_SECTION g_cs
//	return 0;
//}

