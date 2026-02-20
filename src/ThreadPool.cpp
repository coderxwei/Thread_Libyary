#include "ThreadPool.h"
#include <functional>
#include <iostream>
#include <chrono>

const int TASK_MAX = 1024;

// 构造函数
ThreadPool::ThreadPool()
    : initThreadNum_(4)
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
    condNotEmpty_.notify_all();

    // CACHED 模式下：当任务数量 > 空闲线程数量，且未超过线程上限时，动态创建新线程
    if (poolMode_ == PoolMode::CACHED
        && (int)taskQueue_.size() > idleThreadNum_
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
    auto lastTime = std::chrono::high_resolution_clock::now();

    for (;;)
    {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);

            while (taskQueue_.size() == 0)
            {
                if (!isRunning_)
                {
                    threads_.erase(threadID);
                    totalThreadNum_--;
                    exitCond_.notify_all();
                    return;
                }

                if (poolMode_ == PoolMode::CACHED)
                {
                    if (std::cv_status::timeout ==
                        condNotEmpty_.wait_for(lock, std::chrono::seconds(1)))
                    {
                        auto now = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);

                        if (duration.count() >= 60 && totalThreadNum_ > (int)initThreadNum_)
                        {
                            threads_.erase(threadID);
                            totalThreadNum_--;
                            idleThreadNum_--;
                            return;
                        }
                    }
                }
                else
                {
                    condNotEmpty_.wait(lock);
                }
            }

            idleThreadNum_--;
            task = taskQueue_.front();
            taskQueue_.pop();

            if (taskQueue_.size() > 0)
            {
                condNotEmpty_.notify_all();
            }

            condNotFull_.notify_all();
        }

        if (task != nullptr)
        {
            task->exec();
        }

        lastTime = std::chrono::high_resolution_clock::now();
        idleThreadNum_++;
    }
}
