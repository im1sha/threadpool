#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <iostream>
#include <wchar.h>
#include "ThreadPool.h"
#include "Utils.h"
#include "Sorter.h"

//  TO-DO LIST
//
//	- –азработать очередь заданий, 
//		в которую несколько потоков могут вставл€ть элементы атомарно.
//
//	- –азработать обработчик этой очереди, 
//		который извлекает из нее задани€ и раздает заданному количеству потоков.
//
//	- –азработать программу, котора€ использует очередь заданий 
//		и обработчик очереди дл€ сортировки строк в текстовом файле:
//		¬ходной поток читает файл в пам€ть, нарезает его на части 
//		и создает несколько заданий на сортировку(по числу сортирующих потоков),
//		которые помещает в очередь заданий. —ортирующие потоки извлекают задани€, 
//		сортируют свои части файла, отдают все результаты
//		выходному потоку.¬ыходной поток дожидаетс€ всех сортированных частей 
//		и объедин€ет их методом сортирующего сли€ни€.

//	UnitOfWork	- delegate to execute 
//	WorkTask	- thread wrapper
//	ThreadPool	- class that's enqueues delegate 

//void foo1(void *params) 
//{
//	printf("1\n");
//
//}
//void foo2(void *params) 
//{
//	printf("2\n");
//}
//void foo3(void *params) 
//{
//	printf("3\n");
//}
//void foo4(void *params) 
//{
//	printf("4\n");
//}
//void foo5(void *params) 
//{
//	printf("5\n");
//}
//void foo6(void *params) 
//{
//	printf("6\n");
//}
//void foo7(void *params) 
//{
//	printf("7\n");
//}

int wmain() 
{
	printf_s("\n\t===Started===\n\n");

	const int TIMEOUT = 999;
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


// testing 
//UnitOfWork x1(foo1, nullptr);
//UnitOfWork x2(foo2, nullptr);
//UnitOfWork x3(foo3, nullptr);
//UnitOfWork x4(foo4, nullptr);
//UnitOfWork x5(foo5, nullptr);
//UnitOfWork x6(foo6, nullptr);
//UnitOfWork x7(foo7, nullptr);
//std::vector <UnitOfWork> uList_ ;
//uList_.push_back(x1);
//uList_.push_back(x2);
//uList_.push_back(x3);
//uList_.push_back(x4);
//uList_.push_back(x5);
//uList_.push_back(x6);
//uList_.push_back(x7);
//ThreadPool tpool(3, 1);
//tpool.enqueue(uList_[0]);
//tpool.enqueue(uList_[1]);
//tpool.enqueue(uList_[2]);
//tpool.enqueue(uList_[3]);
//tpool.enqueue(uList_[4]);
//tpool.enqueue(uList_[5]);
//tpool.enqueue(uList_[6]);
//tpool.enqueue(uList_[0]);
//tpool.enqueue(uList_[1]);
//tpool.enqueue(uList_[2]);
//tpool.enqueue(uList_[3]);
//tpool.enqueue(uList_[4]);
//tpool.enqueue(uList_[5]);
//tpool.enqueue(uList_[6]);
//tpool.closeSafely();
//

