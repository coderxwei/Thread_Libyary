#pragma once
#include "Any.h"
#include "Result.h"
class Result;
class Task
{
public:
	Task();
	~Task() = default;
	void exec();
	virtual Any run() = 0;
	void setResult(Result* res);
private:
	Result* result_;
};

