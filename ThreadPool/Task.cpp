#include "Task.h"
 Task::Task():result_(nullptr)
{

}

 void Task::exec()
{
	if (result_!=nullptr)
	{	
		//Ö´ÐÐrun·½·¨¡£
		result_->setValue(run());
	}
}
 void Task::setResult(Result* res)
{
	 
	result_ = res; 
}