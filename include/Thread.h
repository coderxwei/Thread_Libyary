#pragma once
#include <functional>
#include <cstdint>

// 线程类，封装了线程的创建和启动
class Thread
{
public:
    using ThreadFunc = std::function<void(int threadID)>;

    Thread(ThreadFunc func);
    ~Thread();

    void start();           // 启动线程
    int getId() const;      // 获取线程ID

private:
    ThreadFunc func_;           // 线程执行函数（绑定到线程池的任务处理函数）
    static int generatedId_;    // 静态ID生成器
    int threadId_;              // 当前线程ID
};
