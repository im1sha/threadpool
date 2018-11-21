#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <string>
#include <wchar.h>
#include "Utils.h"
#include "ThreadPool.h"

class Sorter
{
public:
	static void loadAndSort(void ** params);
	static void sort(void ** params);
	static void mergeAndOutput(void ** params);
};

