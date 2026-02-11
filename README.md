# ThreadPool - C++ 线程池库

一个基于 C++17 的线程池实现，支持固定线程数（FIXED）和动态线程数（CACHED）两种工作模式。

## 功能特性

- **FIXED 模式**：固定数量的工作线程，适用于稳定负载场景
- **CACHED 模式**：根据任务量动态创建/回收线程，适用于小而快的任务场景
- 支持任务提交与结果异步获取
- 任务队列容量可配置
- 线程安全的生产者-消费者模型

## 项目结构

```
ThreadPool/
├── CMakeLists.txt          # CMake 构建配置
├── main.cpp                # 示例程序
├── include/                # 头文件
│   ├── Any.h               # 类型擦除容器
│   ├── PoolMode.h          # 线程池模式枚举
│   ├── Result.h            # 任务结果类
│   ├── Semaphore.h         # 信号量
│   ├── Task.h              # 抽象任务基类
│   ├── Thread.h            # 线程封装类
│   └── ThreadPool.h        # 线程池类
└── src/                    # 源文件
    ├── Result.cpp
    ├── Task.cpp
    ├── Thread.cpp
    └── ThreadPool.cpp
```

## 构建方式（CLion / CMake）

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

或在 CLion 中直接打开项目根目录即可自动识别 CMakeLists.txt。

## 使用示例

```cpp
#include "Task.h"
#include "ThreadPool.h"

class MyTask : public Task
{
public:
    Any run() override
    {
        // 自定义任务逻辑
        return 42;
    }
};

int main()
{
    ThreadPool pool;
    pool.start(4);  // 启动4个工作线程

    Result res = pool.submitTask(std::make_shared<MyTask>());
    int value = res.getValue().cast_<int>();

    return 0;
}
```
