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
//	4. »зучение создани€ и использовани€ потоков и механизмов синхронизации.
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

//	UnitOfWork	- задание 
//	WorkTask	- thread
//	ThreadPool	- control class

void foo1(void *params) 
{
	::Sleep(100);
	printf("1xx");

}
void foo2(void *params) 
{
	::Sleep(1000);

	printf("2xx");
}
void foo3(void *params) 
{
	::Sleep(6000);

	printf("3xx");
}
void foo4(void *params) 
{
	printf("4xx");
}
void foo5(void *params) 
{
	printf("5xx");
}
void foo6(void *params) 
{
	printf("6xx");
}
void foo7(void *params) 
{
	printf("7xx");
}

//// Tasks to execute
//std::vector<UnitOfWork> * unitsList_ = nullptr;
//
//// Event determining if queue of tasks contains 1 item at least
//HANDLE availableEvent_ = nullptr;
//
//// Critical section providing atomic enque/dequeue operations with queue of UnitIfWork
//CRITICAL_SECTION unitsSection_;
//
//
//int * timeout = nullptr;
int wmain() 
{
	/*unitsList_ = new std::vector<UnitOfWork>();
	::InitializeCriticalSectionAndSpinCount(&unitsSection_, 4000);
	availableEvent_ = ::CreateEvent(nullptr, true, false, nullptr);*/
	/*
	printf("HANDLE: %p\n", GetCurrentProcess());
	printf("HANDLE: %p\n", GetCurrentThread());
	*/	

	/*
	std::wstring source = Utils::selectOpenedFile(nullptr);
	std::vector<std::wstring> strings = Utils::loadStringsFromFile(source);
	Utils::sortStrings(&strings);
	std::wstring destination = Utils::selectSavedFile(nullptr);
	bool result = Utils::writeToFile(destination, strings);
	*/

	//std::function<void(void*)> f1 = foo1;


	UnitOfWork x1(foo1, nullptr);
	UnitOfWork x2(foo2, nullptr);
	UnitOfWork x3(foo3, nullptr);

	std::vector <UnitOfWork> uList_ ;

	uList_.push_back(x1);
	uList_.push_back(x2);
	uList_.push_back(x3);

	//WorkTask w(unitsList_, availableEvent_,
	//	unitsSection_, timeout);


	
	
	
	ThreadPool tpool(2, 1);
	tpool.enqueue(uList_[0]);
	tpool.enqueue(uList_[1]);
	tpool.enqueue(uList_[2]);

	printf_s("\n\t===OK===\n\n");
	getchar();	

	return 0;
}


