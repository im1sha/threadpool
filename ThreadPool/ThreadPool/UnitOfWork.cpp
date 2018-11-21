#include "UnitOfWork.h"

UnitOfWork::UnitOfWork(std::function<void(void **)> method, void ** parameters, int timeout)
{
	this->method_ = method;
	this->parameters_ = parameters;
	this->timeout_ = timeout;
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

int UnitOfWork::getTimeout()
{
	return timeout_;
}


