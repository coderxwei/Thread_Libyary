#pragma once
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <ostream>
#include <memory>
#include <queue>
#include  <atomic>
#include <functional>

#include "Any.h"
#include  "ThreadPool.h"
#include  "const.h"
#include "Result.h"
#include  "Task.h"
#include  "Thread.h"
//abstract task   client can self_define task type

//线程类型-----------------------------------------------

class ThreadPool
{
public:
	void start(int initTthread_num);
	      
	ThreadPool();                //构造函数

	void setMode(enum  Pool);
	                            //设置任务上限阈值
	void setTaskinit(int sum);  //任务数量的初始化
	
	
	void setTheadsizeHold(int size);   //用户自定义设置线程上线的数量

	
    Result submitTask(std::shared_ptr<Task> tp);  //提交任务
	~ThreadPool();
	
	void ThreadHandler_();             //线程池的任务处理函数-从任务队列中取出任务并执行
	ThreadPool(const ThreadPool& thread) = delete;
	ThreadPool& operator=(const ThreadPool& tp) = delete;
private:
	size_t thread_sum_;            //define thread num
	size_t initThreadnum_;         //初始的线程的数量。
	int  threadSizeHold_;          //线程数量的上线阈值。初值值定位30
	std::atomic_int total_thread_; //记录当前线程池中线程的总数量.
	std::atomic_int  idleTheadsum_;//记录空闲线程的数量


	///线程通讯
	std::mutex mutex_;             //保证任务队列的线程安全。
	std::condition_variable cond_not_Full;  //表示任务队列池没有满。
	std::condition_variable cond_not_empty; //表示任务队列不空。
	

	///线程任务相关
	std::vector<std::unique_ptr<Thread>>  threads_;	 //线程列表
	std::queue<std::shared_ptr<Task>>  task_queue_;  //任务队列
	std::atomic_uint task_sum_;                      //任务的数量
	int taskSetTheadHold;                            //线程中任务队列的阈值。

	///线程的状态信息
	enum Pool  poolMode_;           //记录当前线程的模式。
	std::atomic_bool bootRuning_;   //线程池是否正在运行。
	
	//拷贝构造
	
};

