#include "UnitOfWork.h"

UnitOfWork::UnitOfWork(std::function<void(void **)> method, void ** parameters)
{
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


