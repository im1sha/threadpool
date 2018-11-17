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
//	TaskQueue	- threadPool

#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <iostream>
#include <wchar.h>
#include "ThreadPool.h"
#include "Utils.h"

int wmain() 
{
	/*
	printf("PROCESS\n");

	printf("HANDLE: %p\n", GetCurrentProcess());
	printf("to long long: %lld\n", (long long) GetCurrentProcess());

	printf("THREAD\n");

	printf("HANDLE: %p\n", GetCurrentThread());
	printf("to long long: %lld\n", (long long) GetCurrentThread());
	*/

	std::vector<std::wstring> strings = Utils::loadStringsFromFile(nullptr);

	Utils::sortStrings(&strings);

	bool result = Utils::writeToFile(L"C:\\Users\\Foxx\\Desktop\\1.txt", strings);

	printf_s("OK");
	getchar();

	return 0;
}


