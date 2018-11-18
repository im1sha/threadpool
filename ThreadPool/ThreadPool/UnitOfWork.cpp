#include "UnitOfWork.h"

UnitOfWork::UnitOfWork(_beginthreadex_proc_type method, void* parameters)
{
	this->method_ = method;
	this->parameters_ = parameters;
}

UnitOfWork::~UnitOfWork()
{
}

_beginthreadex_proc_type UnitOfWork::getMethod()
{
	return method_;
}

void * UnitOfWork::getParemeters()
{
	return parameters_;
}
