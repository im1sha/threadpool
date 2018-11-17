#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include "Definitions.h"

class UnitOfWork
{
public:
	UnitOfWork(_beginthreadex_proc_type method, void* paremeters);
	~UnitOfWork();
private:
	_beginthreadex_proc_type method; 
	void* paremeters;
};

