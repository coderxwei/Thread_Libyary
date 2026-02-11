#include "Task.h"
#include "Result.h"

Task::Task() : result_(nullptr)
{
}

void Task::exec()
{
    if (result_ != nullptr)
    {
        // 执行用户定义的 run() 方法，并将返回值设置到 Result 中
        result_->setValue(run());
    }
}

void Task::setResult(Result* res)
{
    result_ = res;
}
