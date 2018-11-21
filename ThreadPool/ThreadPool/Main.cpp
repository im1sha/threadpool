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

int wmain() 
{
	printf_s("\n\t===Started===\n\n");

	const int TIMEOUT = 99999;
	const int TOTAL_THREADS = 5;

	printf_s("\nParts to sort\n\t");
	int parts = -1;
	/*scanf_s("%i", &parts);*/
	if (parts <= 0)
	{
		parts = TOTAL_THREADS;
	}

	ThreadPool * threadpool = new ThreadPool(parts, TIMEOUT);

	void ** args = new void*[1];
	args[0] = threadpool;

	UnitOfWork loadAndSortTask(Sorter::loadAndSort, args);

	threadpool->enqueue(loadAndSortTask);

	threadpool->closeSafely();

	delete threadpool;
	delete[] args;

	printf_s("\n\t===Finished===\n\n");
	getchar();	
	getchar();
	getchar();

	return 0;
}


