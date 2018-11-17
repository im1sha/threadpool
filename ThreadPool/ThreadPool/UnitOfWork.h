#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <process.h>

class UnitOfWork
{
public:
	UnitOfWork(_beginthreadex_proc_type method, void* paremeters);
	~UnitOfWork();
private:
	_beginthreadex_proc_type method; 
	void* paremeters;
};

