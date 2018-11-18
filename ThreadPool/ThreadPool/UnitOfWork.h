#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <process.h>
#include <functional>

class UnitOfWork
{
public:
	UnitOfWork(std::function<void(void *)> method, void * paremeters);
	~UnitOfWork();

	std::function<void(void *)> getMethod();
	void* getParemeters();

private:
	std::function<void(void *)> method_;
	void* parameters_;
};


