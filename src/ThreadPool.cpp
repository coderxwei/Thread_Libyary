#include "ThreadPool.h"
#include <functional>
#include <iostream>
#include <chrono>

const int TASK_MAX = 1024;

// 构造函数
ThreadPool::ThreadPool()
    : initThreadNum_(4)
    , taskNum_(0)
    , poolMode_(PoolMode::FIXED)
    , taskQueueMaxSize_(TASK_MAX)
    , isRunning_(false)
    , idleThreadNum_(0)
    , threadSizeThreshold_(30)
    , totalThreadNum_(0)
{
}

// 析构函数 - 回收所有线程资源
ThreadPool::~ThreadPool()
{
    isRunning_ = false;

    // 先获得锁，然后通知其他正在 wait 的线程
    std::unique_lock<std::mutex> lock(mutex_);
    condNotEmpty_.notify_all();

    // 等待所有线程退出
    exitCond_.wait(lock, [&]() -> bool
    {
        return threads_.size() == 0;
    });
}

// 启动线程池
void ThreadPool::start(int initThreadNum)
{
    isRunning_ = true;
    initThreadNum_ = initThreadNum;

    // 创建线程对象
    for (int i = 0; i < initThreadNum; i++)
    {
        // Thread(param: myboundfunc)
        //myboundfunc=std::bind(&ThreadPool::threadHandler, this, std::placeholders::_1)
        auto threadPtr = std::make_unique<Thread>(
            std::bind(&ThreadPool::threadHandler, this, std::placeholders::_1));
        //获取当前线程的id
        int threadId = threadPtr->getId();
        //把当前的线程id 和对象的线程指针加入到线程池中。 线程池是一个hashtable(unordered_map)
        threads_.emplace(threadId, std::move(threadPtr));
        totalThreadNum_++;
    }

    // 启动所有线程
    for (auto& [id, thread] : threads_)
    {
        thread->start();
        idleThreadNum_++;
    }
}

// 设置线程池工作模式
void ThreadPool::setMode(PoolMode mode)
{
    if (isRunning_) return;
    poolMode_ = mode;
}

// 设置任务队列上限
void ThreadPool::setTaskQueueMaxSize(int maxSize)
{
    if (isRunning_) return;
    taskQueueMaxSize_ = maxSize;
}

// 设置线程数量上限（仅 CACHED 模式有效）
void ThreadPool::setThreadSizeThreshold(int size)
{
    if (isRunning_) return;
    if (poolMode_ == PoolMode::CACHED)
    {
        threadSizeThreshold_ = size;
    }
}

// 提交任务
Result ThreadPool::submitTask(std::shared_ptr<Task> task)
{
    // 获取锁
    std::unique_lock<std::mutex> lock(mutex_);

    // 等待任务队列有空位（最多等待1秒）
    if (!condNotFull_.wait_for(lock, std::chrono::seconds(1),
        [&]() -> bool { return taskQueue_.size() < (size_t)taskQueueMaxSize_; }))
    {
        std::cerr << "任务提交失败：任务队列已满" << std::endl;
        return Result(task, false);
    }

    // 将任务放入队列
    taskQueue_.emplace(task);
    taskNum_++;
    condNotEmpty_.notify_all();

    //消费任务
    // CACHED 模式下：当任务数量 > 空闲线程数量，且未超过线程上限时，动态创建新线程
    if (poolMode_ == PoolMode::CACHED
        && taskNum_ > idleThreadNum_
        && totalThreadNum_ < threadSizeThreshold_)
    {
        auto threadPtr = std::make_unique<Thread>(
            std::bind(&ThreadPool::threadHandler, this, std::placeholders::_1));
        int threadId = threadPtr->getId();
        threads_.emplace(threadId, std::move(threadPtr));
        threads_[threadId]->start();
        totalThreadNum_++;
        idleThreadNum_++;
    }

    return Result(task);
}

// 线程池工作线程的任务处理函数
void ThreadPool::threadHandler(int threadID)
{
    //这个精度很高 用于记录线程上一次处理最后的执行时间。
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (isRunning_)
    {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);

            while (taskQueue_.size() == 0)
            {
                // 线程池已停止，退出线程
                if (!isRunning_)
                {
                    threads_.erase(threadID);
                    exitCond_.notify_all();
                    return;
                }

                if (poolMode_ == PoolMode::CACHED)
                {
                    // CACHED 模式：每秒检查一次，超过60秒空闲则回收线程
                    if (std::cv_status::timeout ==
                        condNotEmpty_.wait_for(lock, std::chrono::seconds(1)))
                    {
                        auto now = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);

                        if (duration.count() >= 60 && totalThreadNum_ > (int)initThreadNum_)
                        {
                            // 回收当前空闲线程
                            threads_.erase(threadID);
                            totalThreadNum_--;
                            idleThreadNum_--;
                            return;
                        }
                    }
                }
                else
                {
                    // FIXED 模式：阻塞等待任务
                    condNotEmpty_.wait(lock);
                }
            }

            // 从任务队列中取出任务
            idleThreadNum_--;
            task = taskQueue_.front();
            taskQueue_.pop();
            taskNum_--;

            // 如果队列中仍有任务，继续通知其他线程消费
            if (taskQueue_.size() > 0)
            {
                condNotEmpty_.notify_all();
            }

            // 通知提交方可以继续提交任务
            condNotFull_.notify_all();
        }

        // 释放锁后执行任务
        if (task != nullptr)
        {
            task->exec();
        }

        // 更新线程最后执行时间
        lastTime = std::chrono::high_resolution_clock::now();
        idleThreadNum_++;
    }
}
