#pragma once
#include <memory>
#include <atomic>
#include "Any.h"
#include "Semaphore.h"
#include "Task.h"

class Task;

// 任务执行结果类，支持阻塞等待获取返回值
class Result
{
public:
    Result(std::shared_ptr<Task> task, bool isValid = true);
    ~Result() = default;

    // 设置任务执行完毕后的返回值
    void setValue(Any any)
    {
        this->any_ = std::move(any);
        semaphore_.post_();  // 增加信号量，通知结果已就绪
    }

    // 阻塞等待并获取任务返回值
    Any getValue()
    {
        if (!is_valid_)
        {
            return {};
        }
        semaphore_.wait_();  // 如果任务未完成，阻塞等待
        return std::move(any_);
    }

private:
    Any any_;
    Semaphore semaphore_;
    std::shared_ptr<Task> task_;
    std::atomic_bool is_valid_;  // 返回值是否有效
};
