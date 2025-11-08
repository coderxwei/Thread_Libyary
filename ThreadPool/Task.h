#pragma once
#include "Any.h"
#include "Result.h"

class Task
{
public:
	
	Task();
	~Task() = default;
	virtual Any run() = 0;
private:
	Result* result_;
};
