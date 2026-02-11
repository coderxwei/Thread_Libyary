#include "Thread.h"
#include <thread>

int Thread::generatedId_ = 0;

Thread::Thread(ThreadFunc func)
    : func_(func)
    , threadId_(generatedId_++)
{
}

Thread::~Thread()
{
}

void Thread::start()
{
    // 创建一个新线程并分离执行
    std::thread t(func_, threadId_);
    t.detach();
}

int Thread::getId() const
{
    return threadId_;
}
