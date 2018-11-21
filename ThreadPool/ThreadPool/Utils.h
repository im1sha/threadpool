#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <vector>
#include <fstream>
#include <codecvt>
#include <algorithm>
#include <io.h> 
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

class Utils
{
public:
	static std::wstring selectOpenFile(HWND hWnd);
	static std::wstring selectSaveFile(HWND hWnd);
	static std::vector<std::wstring> loadStringsFromFile(std::wstring fileName);
	static void sortStrings(std::vector<std::wstring>* strings);
	static bool writeToFile(std::wstring path, std::vector<std::wstring> strings);
	static void mergeSort(std::wstring a[], int n);
	static std::wstring* vectorToArray(std::vector<std::wstring> v);
	static std::vector<std::wstring> arrayToVector(std::wstring a[], int length);
private:
	static void merge(std::wstring a[], int n, int m);
};

