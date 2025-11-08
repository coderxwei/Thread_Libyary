#pragma once
#include "Any.h"
#include "Semaphore.h"
#include "Task.h"
class Task;
//---------------------------------这是一个返回结果类
class Result
{
public:
	Result(std::shared_ptr<Task> task, bool isValid = true);
	~Result() = default;
	//SetValue获得任务执行完毕的返回的值
	void setValue(Any any)
	{
		//获得any的返回值。
		this->any_ = std::move(any);
		semaphore_.post_();  //表示已经已经获得任务的返回值，增加信号量的资源。
	};
	Any getValue()
	{
		if (!is_valid_)
		{
			return  NULL;
		}
		//task：任务如果没有执行完毕就会阻塞。
		semaphore_.wait_();
		return  std::move(any_);
	}
private:
	Any any_;
	Semaphore semaphore_;
	std::shared_ptr<Task> task_;
	std::atomic_bool is_valid_; //返回值是否有效
};
//---------------------------------------------------------这个表示Any类型



 