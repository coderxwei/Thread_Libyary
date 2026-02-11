#pragma once
#include <mutex>
#include <condition_variable>

// 计数信号量类，用于线程间同步
class Semaphore
{
public:
    Semaphore(int limit = 0) : resLimit_(limit) {}
    ~Semaphore() = default;

    // P 操作：获取一个信号量资源（阻塞等待）
    void wait_()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [&]() -> bool
        {
            return resLimit_ > 0;
        });
        resLimit_--;
    }

    // V 操作：释放一个信号量资源
    void post_()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        resLimit_++;
        condition_.notify_all();
    }

private:
    int resLimit_;
    std::mutex mutex_;
    std::condition_variable condition_;
};
