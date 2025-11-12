#pragma once
#include "ThreadPool.h"
#include <functional>
#include <iostream>
#include <chrono>
#include  "Result.h"

const int TASK_MAX = 1024;
class Task;
//开启线程池
void ThreadPool::start(int initTthread_num)
{
	bootRuning_ = true; //启动线程池
	for(int i=0 ;i< initTthread_num;i++)
	{
		auto thread_ptr = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadHandler_, this,std::placeholders::_1));
		//threads_.emplace_back(std::move(thread_ptr));
		int threadID = thread_ptr->getId();
		threads_.emplace(threadID, std::move(thread_ptr));

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
poolMode_(Pool::FIXED),
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
	if (p_mode==Pool::FIXED)
	{
		poolMode_ = Pool::FIXED;
	}
	else
	{
		poolMode_ = Pool::CACHED;
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
	if (poolMode_==Pool::CACHED)
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
		//  切换到cached 模式--任务处理比较紧急，场景:小而块的任务。 需要根据任务的数量和空闲线程的数量，判断是否需要切换到cached模型
		/*  线程池的模式必须是cached
		 *	任务的数量必须大于空闲线程的数量
		 *	当前线程的总数量必须小于线程数量的上线
		 */
		if (poolMode_==Pool::CACHED &&
			task_sum_>idleTheadsum_&&
			total_thread_<threadSizeHold_)
		{
			auto thread_ptr = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadHandler_, this,std::placeholders::_1));
			//threads_.emplace_back(std::move(thread_ptr));
			int threadId = thread_ptr->getId();
			threads_.emplace(threadId, std::move(thread_ptr));
			threads_[threadId]->start();
			total_thread_++;	//线程的总数量++;
			idleTheadsum_++;	//刚创建的线程都是空闲线程。
		}
		return Result(tp);
}
///析构函数 ---线程池对象析构之后，回收所有的线程资源
ThreadPool::~ThreadPool()
{

	std::cout<<"析构函数执行了"<<std::endl;
	bootRuning_ = false;
	/*
	先获得锁	，然后通知其他正在wait的线程
	*/
	std::unique_lock<std::mutex>lock(mutex_);
	std::cout<<"获得了锁用于通知其他线程"<<std::this_thread::get_id<<std::endl;
    cond_not_empty.notify_all();   //ntify_all的位置必须在获得锁之后才能执行，要不然执行notify后其他线程没有获得到做，相当于白做，而wait的线程就形成死锁。。
	///这里如果有线程没有释放就会一直阻塞。
	exit_cond_.wait(lock, [&]()->bool
		{
			std::cout << "还剩下多少个线程" << threads_.size();
			return  threads_.size() ==0;  //
		});
	std::cout<<"还剩下多少个线程" << threads_.size();
}
///-----------------------消费任务
void ThreadPool::ThreadHandler_(int threadID)
{
	
	auto lastTime = std::chrono::high_resolution_clock::now();
	while (bootRuning_) //当boolRuning 为true 时表示线程处于正在运行的状态。
	{
		std::shared_ptr<Task> task;
		{
			/*
			cached 模型下线程数量是动态创建的，当空闲线程的时间了超过60s后，要对该空闲的线程进行回收。
			*/
			//先获取锁
			//cached 模式下的处理
			std::unique_lock<std::mutex>lock(mutex_);
		    std::cout << "线程：" << std::this_thread::get_id()<<"加锁了" << std::endl;

			while (task_queue_.size() == 0)
			{
				if (poolMode_ == Pool::CACHED)
				{
					//没一秒钟返回 返回的过程如何区分超时的返回？
					//当前线程的等待时间是否超时了
					if (std::cv_status::timeout == cond_not_empty.wait_for(lock, std::chrono::seconds(1)))
					{
						//超时返回了，就需要计算一下当前的时间
						auto now = std::chrono::high_resolution_clock::now();
						auto timer_ = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);// lastTime表示上一次最后的执行时间。
						if (timer_.count() >= 60 && total_thread_ > initThreadnum_)
						{
							//回收当前的线程？
							//把线程对象从线程列表的容器中删除 pair----->(threadID，Thread)
							threads_.erase(threadID);
							//记录当前线程相关数据的值需要进行修改
							thread_sum_--;
							idleTheadsum_--;
							return;
						}
					}
				}
				//表示为FIXED 模式
				else
				{
					cond_not_empty.wait(lock);
				}
				//如果线程不在运行，那么从线程池删去该线程
			}
			if (!bootRuning_)
			{
				break;
			}
			
			idleTheadsum_--; //执行任务的时候空闲线程的数量--；
			task = task_queue_.front();
			task_queue_.pop();
			task_sum_--;
			//线程的队列不是空的
			if (task_queue_.size()>0)
			{
				cond_not_empty.notify_all();     //继续通知其他wait 的线程继续消费任务。
			}             
			    cond_not_Full.notify_all();      //消费任务，通知提交任务的现成继续提交任务。
		}
		std::cout << "线程解锁了" << std::this_thread::get_id() <<std::endl;
			//释放锁后再次执行
		if (task != nullptr){
			std::shared_ptr<Task> t = task;
			t->exec();
			}
		lastTime=std::chrono::high_resolution_clock::now(); //更新线程执行完任务的时间。
		idleTheadsum_++;     
		threads_.erase(threadID);
		std::cout << "回收线程threadId" << std::this_thread::get_id() << std::endl;
		exit_cond_.notify_all();//要通知主线程子线程已经释放了，不然主线程会一直处于阻塞中//线程的任务处理完毕，空闲线程的数量要进行++
	                                                        //任务执行完毕之后—该任务线程被释放，所以线程池的任务总数量要--
	}
	threads_.erase(threadID);
	std::cout << "回收线程threadId" << std::this_thread::get_id() << std::endl;
	exit_cond_.notify_all();//要通知主线程子线程已经释放了，不然主线程会一直处于阻塞中
}

///线程资源的回收
void ThreadPool::retThread(std::shared_ptr<Thread> thread)
{


}


