#include "UnitOfWork.h"

UnitOfWork::UnitOfWork(std::function<void(void **)> method,void ** parameters, time_t timeoutInMs)
{
	if (timeoutInMs < 0)
	{
		this->timeoutInMs_ = INFINITE;
	}
	else
	{
		this->timeoutInMs_ = timeoutInMs;
	}
	this->method_ = method;

	this->parameters_ = parameters;
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

time_t UnitOfWork::getTimeoutInMs()
{
	return timeoutInMs_;
}

