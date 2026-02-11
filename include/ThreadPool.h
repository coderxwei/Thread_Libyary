#pragma once
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <memory>
#include <queue>
#include <atomic>
#include <functional>
#include <unordered_map>

#include "Any.h"
#include "PoolMode.h"
#include "Result.h"
#include "Task.h"
#include "Thread.h"

// 线程池类
class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    // 禁用拷贝和赋值
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 启动线程池，指定初始线程数量
    void start(int initThreadNum);

    // 设置线程池工作模式（FIXED / CACHED）
    void setMode(PoolMode mode);

    // 设置任务队列上限阈值
    void setTaskQueueMaxSize(int maxSize);

    // 设置线程数量上限（仅 CACHED 模式有效）
    void setThreadSizeThreshold(int size);

    // 提交任务到线程池
    Result submitTask(std::shared_ptr<Task> task);

private:
    // 线程池工作线程的任务处理函数
    void threadHandler(int threadID);

private:
    size_t initThreadNum_;              // 初始线程数量
    int threadSizeThreshold_;           // 线程数量上限阈值（CACHED 模式），默认30
    std::atomic_int totalThreadNum_;    // 当前线程池中线程的总数量
    std::atomic_int idleThreadNum_;     // 空闲线程数量

    // 线程列表（通过线程ID关联线程对象）
    std::unordered_map<int, std::unique_ptr<Thread>> threads_;

    // 任务队列相关
    std::queue<std::shared_ptr<Task>> taskQueue_;   // 任务队列
    std::atomic_uint taskNum_;                       // 当前任务数量
    int taskQueueMaxSize_;                           // 任务队列上限

    // 线程同步
    std::mutex mutex_;                              // 保证任务队列的线程安全
    std::condition_variable condNotFull_;            // 任务队列未满条件
    std::condition_variable condNotEmpty_;           // 任务队列非空条件
    std::condition_variable exitCond_;              // 等待所有线程退出条件

    // 线程池状态
    PoolMode poolMode_;                 // 线程池模式
    std::atomic_bool isRunning_;        // 线程池是否正在运行
};
