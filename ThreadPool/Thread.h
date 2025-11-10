#pragma once
#include <functional>
class Thread
{
public:
	
	void start();			//开启线程
	using Threadfunc = std::function<void(int ThreadID)>;  
//	void thead_start();
	Thread(Threadfunc func);//线程的构建函数
	~Thread();
	int getId()const;		//获取线程ID
private:
	Threadfunc func_;       //这个就是线程池中的func函数，用于执行具体的任务.
	static int Generated_;
	uint16_t   thread_ID_;  //保存线程的iD
};

