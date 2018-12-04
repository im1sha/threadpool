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




