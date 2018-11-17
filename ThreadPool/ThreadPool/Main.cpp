//  TO-DO LIST
//
//	4. �������� �������� � ������������� ������� � ���������� �������������.
//
//	- ����������� ������� �������, 
//		� ������� ��������� ������� ����� ��������� �������� ��������.
//
//	- ����������� ���������� ���� �������, 
//		������� ��������� �� ��� ������� � ������� ��������� ���������� �������.
//
//	- ����������� ���������, ������� ���������� ������� ������� 
//		� ���������� ������� ��� ���������� ����� � ��������� �����:
//		������� ����� ������ ���� � ������, �������� ��� �� ����� 
//		� ������� ��������� ������� �� ����������(�� ����� ����������� �������),
//		������� �������� � ������� �������. ����������� ������ ��������� �������, 
//		��������� ���� ����� �����, ������ ��� ����������
//		��������� ������.�������� ����� ���������� ���� ������������� ������ 
//		� ���������� �� ������� ������������ �������.

//	UnitOfWork	- ������� 
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


