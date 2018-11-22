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
	UnitOfWork(std::function<void(void **)> method, void ** paremeters = nullptr, time_t timeoutInMs = INFINITE);
	~UnitOfWork();

	std::function<void(void **)> getMethod();
	void ** getParameters();
	time_t getTimeoutInMs();

private:
	std::function<void(void **)> method_;
	void ** parameters_;
	time_t timeoutInMs_;
};


