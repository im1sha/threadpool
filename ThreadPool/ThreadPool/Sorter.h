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
	void loadAndSort();
	void sort(void * params);
	void mergeAndOutput(void * params);
};

