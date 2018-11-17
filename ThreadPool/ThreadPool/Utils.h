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

class Utils
{
public:
	static std::wstring selectOpenedFile(HWND hWnd);
	static std::wstring selectSavedFile(HWND hWnd);
	static std::vector<std::wstring> loadStringsFromFile(std::wstring fileName);
	static void sortStrings(std::vector<std::wstring>* strings);
	static bool writeToFile(std::wstring path, std::vector<std::wstring> strings);
};

