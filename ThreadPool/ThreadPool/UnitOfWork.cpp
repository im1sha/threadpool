#include "UnitOfWork.h"

UnitOfWork::UnitOfWork(std::function<void(void **)> method, void ** parameters, time_t timeoutInMs)
{
	this->method_ = method;
	this->parameters_ = parameters;

	if (timeoutInMs < 0)
	{
		this->timeoutInMs_ = INFINITE;
	}
	else
	{
		this->timeoutInMs_ = timeoutInMs;
	}
}

UnitOfWork::~UnitOfWork()
{
}

std::function<void(void **)> UnitOfWork::getMethod()
{
	return method_;
}

void ** UnitOfWork::getParameters()
{
	return parameters_;
}

int UnitOfWork::getTimeoutInMs()
{
	return timeoutInMs_;
}


