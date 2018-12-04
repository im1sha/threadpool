#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <iostream>
#include <wchar.h>
#include "ThreadPool.h"
#include "Utils.h"
#include "Sorter.h"

//	UnitOfWork	- delegate to execute 
//	WorkTask	- thread wrapper
//	ThreadPool	- class that's enqueues delegate 

// ====================<DEBUG>========================
//size_t TOTAL = 55;
//ThreadPool txxx(13);
//
//void procedure2(void** a) {
//	::Sleep(1);
//	int i = *((int *)(a[0]));
//	printf("#%i  %li\n", i, GetCurrentThreadId());
//}
//
//void procedure1(void** a)
//{
//	std::function<void(void**)> func = procedure2;
//	int ** arr = new int*[TOTAL];
//	for (int i = 0; i < TOTAL; i++)
//	{
//		arr[i] = new int(i);
//	}
//
//	void *** args = new void**[TOTAL];
//	for (size_t i = 0; i < TOTAL; i++)
//	{
//		args[i] = new void*[1];
//	}
//
//	for (size_t i = 0; i < TOTAL; i++)
//	{	
//		args[i][0] = arr[i];
//		//int * ii = (int *)(args[0]);
//		//printf("#%i  %li\n", *ii, GetCurrentThreadId());
//		UnitOfWork unitofwork (func,  args[i]);
//		txxx.enqueue(unitofwork);
//		//printf("=== %i\n", i);
//	}
//
//}
//
//int wmain() 
//{
//	std::function<void(void**)> func = procedure1;
//	void ** xargs = new void*[1]{ (void*) new int(-1) };
//	UnitOfWork unitofwork(func, xargs);
//	txxx.enqueue(unitofwork);
//	txxx.closeSafely();
//
//	printf("\n\n\t\t\t+++++OK %i+++++\n", GetCurrentThreadId());
//	getchar();
//	getchar();
//}
// =======================================================


//
// Starts application
//
int wmain(wchar_t ** cliargs) 
{
	printf_s("\n\t===Started===\n\n");

	const int TOTAL_THREADS = 20;

	printf_s("\nParts to sort # ");
	int parts = -1;
	scanf_s("%i", &parts);
	if (parts <= 0)
	{
		parts = TOTAL_THREADS;
	}
	printf("\n\n");
	ThreadPool * threadpool = new ThreadPool(parts);

	void ** args = new void*[1] { threadpool };

	UnitOfWork loadAndSortTask(Sorter::loadAndSort, args);

	threadpool->enqueue(loadAndSortTask);

	threadpool->closeSafely();

	delete threadpool;
	delete[] args;

	printf_s("\n\t===Finished===\n\n");
	getchar();	
	getchar();

	return 0;
}




