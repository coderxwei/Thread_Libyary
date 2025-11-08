#include "Task.h"
 Task::Task():result_(nullptr)
{

}

 void Task::exec()
{
	if (result_!=nullptr)
	{
		result_->setValue(run());
	}
}
 void Task::setResult(Result* res)
{
	result_ = res;
}