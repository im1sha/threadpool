#include "UnitOfWork.h"

UnitOfWork::UnitOfWork(_beginthreadex_proc_type method, void* paremeters)
{
	this->method = method;
	this->paremeters = paremeters;
}

UnitOfWork::~UnitOfWork()
{
}
