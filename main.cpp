#include <iostream>
#include <chrono>
#include <thread>
#include "Task.h"
#include "ThreadPool.h"

// 示例：自定义任务类，计算 [begin, end) 范围内整数之和
class SumTask : public Task
{
public:
    SumTask(int begin, int end)
        : begin_(begin), end_(end)
    {
    }

    Any run() override
    {
        int sum = 0;
        for (int i = begin_; i < end_; i++)
        {
            sum += i;
        }
        std::cout << "线程 " << std::this_thread::get_id()
                  << " 完成计算 [" << begin_ << ", " << end_ << ")"
                  << " 结果: " << sum << std::endl;
        return sum;
    }

private:
    int begin_;
    int end_;
};

int main()
{
    {
        ThreadPool pool;
        pool.start(4);

        // 提交任务并获取结果
        Result res1 = pool.submitTask(std::make_shared<SumTask>(1, 100));
        Result res2 = pool.submitTask(std::make_shared<SumTask>(100, 200));
        Result res3 = pool.submitTask(std::make_shared<SumTask>(200, 300));

        int sum1 = res1.getValue().cast_<int>();
        int sum2 = res2.getValue().cast_<int>();
        int sum3 = res3.getValue().cast_<int>();

        std::cout << "总和 = " << (sum1 + sum2 + sum3) << std::endl;
    }
    // 线程池析构时会自动回收所有线程资源

    return 0;
}
