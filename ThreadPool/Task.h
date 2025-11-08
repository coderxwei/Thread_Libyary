#pragma once
#include "Any.h"


class Result;
class Task
{
public:
	
	Task();
	~Task() = default;
	virtual Any run() = 0;
private:
	Result* result_;
};
 inline Task::Task()
{
	
}

