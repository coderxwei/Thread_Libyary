#pragma once
#include <functional>
class Thread
{
public:
	//开启线程
	void start();
	using Threadfunc = std::function<void()>;
//	void thead_start();
	//线程的构建函数
	Thread(Threadfunc func);
	~Thread();
private:
	//成员变量？
	Threadfunc func_;  //这个就是线程池中的func函数，用于执行具体的任务.

};

