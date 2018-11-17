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


	std::wstring source = Utils::selectOpenedFile(nullptr);
	std::vector<std::wstring> strings = Utils::loadStringsFromFile(source);

	Utils::sortStrings(&strings);

	std::wstring destination = Utils::selectSavedFile(nullptr);
	bool result = Utils::writeToFile(destination, strings);

	printf_s("OK");

	getchar();

	return 0;
}


