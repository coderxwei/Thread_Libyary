#pragma once
#include <mutex>
#include  <condition_variable>
class Semaphore
{
public:
	Semaphore(int limit = 0) :resLimit_(0) {};
	~Semaphore();
	//获取一个信号量资源（p）

	void wait_()
	{

		std::unique_lock<std::mutex>lock(mutex_);
		// 当lamda 表达式为真的时候线程继续往下执行，
		condition_.wait(lock, [&]()->bool
			{
				return  resLimit_ > 0;

			});
		resLimit_--;
	}

	void post_()
	{
		{
			std::unique_lock<std::mutex>lock(mutex_);
			resLimit_++; //表示资源的数量
			condition_.notify_all();
		}
	};
private:
	int resLimit_;
	std::mutex mutex_;
	std::condition_variable condition_;

};
