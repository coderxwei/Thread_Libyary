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
		condition_.wait(lock, [&]()->bool
			{
				return  resLimit_ >= 0;

			});
		resLimit_--;
	}

	void post_()
	{
		{
			std::unique_lock<std::mutex>lock(mutex_);
			resLimit_++;
			condition_.notify_all();
		}
	};
private:
	int resLimit_;
	std::mutex mutex_;
	std::condition_variable condition_;

};
