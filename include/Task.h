#pragma once
#include "Any.h"

class Result;

// 抽象任务基类，用户需要继承并实现 run() 方法
class Task
{
public:
    Task();
    ~Task() = default;

    // 执行任务（内部调用 run() 并设置返回值）
    void exec();

    // 纯虚函数，用户自定义任务逻辑
    virtual Any run() = 0;

    // 设置 Result 对象指针（由线程池内部调用）
    void setResult(Result* res);

private:
    Result* result_;
};
