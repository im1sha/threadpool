#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <process.h>
#include <functional>

class UnitOfWork
{
public:
	UnitOfWork(std::function<void(void **)> method, void ** paremeters, int timeout = INFINITE);
	~UnitOfWork();

	std::function<void(void **)> getMethod();
	void ** getParameters();
	int getTimeout();

private:
	std::function<void(void **)> method_;
	void ** parameters_;
	int timeout_;
};


