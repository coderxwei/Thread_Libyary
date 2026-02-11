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
        //等价于
        Any result=run();
        result_->setValue(std::move(result));
    }
}

void Task::setResult(Result* res)
{
    result_ = res;
}
