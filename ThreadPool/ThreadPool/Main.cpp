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
//	ThreadPool	- control class

int wmain() 
{
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

	printf_s("OK");

	getchar();

	return 0;
}


