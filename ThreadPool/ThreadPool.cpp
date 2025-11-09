#pragma once
#include "ThreadPool.h"
#include <functional>
#include <iostream>
#include  "Result.h"

const int TASK_MAX = 1024;
class Task;
//开启线程池
void ThreadPool::start(int initTthread_num)
{
	bootRuning_ = true; //启动线程池
	for(int i=0 ;i< initTthread_num;i++)
	{
		auto thread_ptr = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadHandler_, this));
		threads_.emplace_back(std::move(thread_ptr));
		total_thread_++;//线程的总数量++;
	}
	for (int i=0; i< initTthread_num;i++)
	{
		threads_[i]->start();
		idleTheadsum_++;  //记录空闲线程任务的适量
	}
}
//有参构造函数
ThreadPool::ThreadPool():
thread_sum_(4),
initThreadnum_(4),
task_sum_(0),
poolMode_(Pool::fixed),
taskSetTheadHold(TASK_MAX),
bootRuning_(false),
idleTheadsum_(0),
threadSizeHold_(30),
total_thread_(0)
{
	//taskSetTheadHold = 1024;
}

void ThreadPool::setMode(Pool  p_mode)
{
	if (p_mode==Pool::fixed)
	{
		poolMode_ = Pool::fixed;
	}
	else
	{
		poolMode_ = Pool::cached;
	}
}

void ThreadPool::setTaskinit(int sum)
{
	taskSetTheadHold = sum;
}


void ThreadPool::setTheadsizeHold(int size)
{
	if (bootRuning_)
	{
		return;
	}
	//必须在cached 模式下才能设置线程的数量.
	if (poolMode_==Pool::cached)
	{
		threadSizeHold_ = size;
	}
	
}

//提交任务
Result ThreadPool::submitTask(std::shared_ptr<Task> tp)
{
	//生产社提交任务--首先获得锁
	std::unique_lock<std::mutex>lock (mutex_);
	//不能长时间阻塞有时间的情况下应该怎么做wait_for 等待的持续时间
  if (!cond_not_Full.wait_for(lock, std::chrono::seconds(1), [&]()->bool
		{
			return task_queue_.size() < taskSetTheadHold;
		})
	  )
  {
	  std::cout << "task queue is error";
	  return  Result(tp, false);
  }

		task_queue_.emplace(tp);
		task_sum_++;
		cond_not_empty.notify_all();
		//切换到Cahed 模式--任务处理比较紧急，场景:小而块的任务。 需要genju 任务的数量和空闲线程的数量，判断是否需要切换到cached模型
		/*  线程池的模式必须是cached
		 *	任务的数量必须大于空闲线程的数量
		 *	当前线程的总数量必须小于线程数量的上线
		 */
		if (poolMode_==Pool::cached &&task_sum_>idleTheadsum_&&total_thread_<threadSizeHold_)
		{
			auto thread_ptr = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadHandler_, this));
			threads_.emplace_back(std::move(thread_ptr));
			total_thread_++;//线程的总数量++;



		}

	  return Result(tp,true);;
}


//析构函数 
ThreadPool::~ThreadPool()
{}
//消费任务
void ThreadPool::ThreadHandler_()
{
	for (;;)
	{
		std::shared_ptr<Task> task;
		{
			//先获取锁
			std::unique_lock<std::mutex>lock(mutex_);
			cond_not_empty.wait(lock, [&]()->bool
				{
					return task_queue_.size() > 0;
				});
			idleTheadsum_--; //执行任务的时候空闲线程的数量--；
			task = task_queue_.front();
			task_queue_.pop();
			task_sum_--;

			//线程的队列不是空的
			if (task_queue_.size()>0)
			{
				cond_not_empty.notify_all();
			}
			//消费一个任务后，任务队列不满 通知添加任务。
			cond_not_Full.notify_all();
		}
			//释放锁后再次执行
		if (task != nullptr)
			{
			std::shared_ptr<Task> t = task;

			t->exec();
			
			//t->run();

			}
		idleTheadsum_++; //线程的任务处理完毕，空闲线程的数量要进行++
		total_thread_--;//任务执行完毕之后—该任务线程被释放，所以线程池的任务总数量要--
	}
	//std::cout << "线程：" << std::this_thread::get_id() << "现在正在运行";
}


