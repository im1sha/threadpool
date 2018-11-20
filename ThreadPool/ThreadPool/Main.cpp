#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <iostream>
#include <wchar.h>
#include "ThreadPool.h"
#include "Utils.h"

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
//	ThreadPool	- class that's enqueues de 

void foo1(void *params) 
{
	printf("1\n");

}
void foo2(void *params) 
{
	printf("2\n");
}
void foo3(void *params) 
{
	printf("3\n");
}
void foo4(void *params) 
{
	printf("4\n");
}
void foo5(void *params) 
{
	printf("5\n");
}
void foo6(void *params) 
{
	printf("6\n");
}
void foo7(void *params) 
{
	printf("7\n");
}

int wmain() 
{
	printf_s("\n\t===Started===\n\n");
	
	std::wstring source = Utils::selectOpeningFile(nullptr);
	std::vector<std::wstring> *strings = new std::vector<std::wstring>(Utils::loadStringsFromFile(source));
	//Utils::sortStrings(strings);
	Utils::margeSort(strings, (int) strings->size());
	std::wstring destination = Utils::selectSavingFile(nullptr);
	bool result = Utils::writeToFile(destination, *strings);	
	


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

	printf_s("\n\t===Finished===\n\n");
	getchar();	

	return 0;
}


