# 线程池库问题记录

## 🔴 严重问题

### 1. 析构函数死锁

**文件**：`src/ThreadPool.cpp`

**问题描述**：

`threadHandler` 中，外层循环 `while (isRunning_)` 在 `isRunning_` 变为 `false` 后直接退出函数，**没有**从 `threads_` map 中移除自己，也**没有**通知 `exitCond_`。

```cpp
// threadHandler 中：
while (isRunning_)  // ← 执行完任务后回到这里，发现 isRunning_ == false，直接退出
{
    // ... 获取任务并执行 ...
    lastTime = std::chrono::high_resolution_clock::now();
    idleThreadNum_++;
}
// ← 直接 return，没有 erase(threadID)，没有 exitCond_.notify_all()
```

而析构函数等待所有线程从 `threads_` 中移除：

```cpp
ThreadPool::~ThreadPool()
{
    isRunning_ = false;
    std::unique_lock<std::mutex> lock(mutex_);
    condNotEmpty_.notify_all();
    exitCond_.wait(lock, [&]() -> bool {
        return threads_.size() == 0;  // ← 永远等不到 0，死锁！
    });
}
```

**后果**：如果某个线程正在执行任务（不在 `wait` 状态），它执行完后通过外层 `while(isRunning_)` 退出，不会清理自身，导致析构函数**永远阻塞（死锁）**。

**修复建议**：将 `threadHandler` 改为无限循环，在获取锁后统一检查退出条件，确保所有退出路径都执行 `threads_.erase(threadID)` 和 `exitCond_.notify_all()`。

---

### 2. `Result` 依赖裸指针，生命周期极其脆弱

**文件**：`src/Result.cpp`、`include/Result.h`、`include/Task.h`

**问题描述**：

`Result` 构造函数通过 `task_->setResult(this)` 将自身裸指针存入 `Task`：

```cpp
Result::Result(std::shared_ptr<Task> task, bool isValid)
    : is_valid_(isValid), task_(task)
{
    task_->setResult(this);  // ← 裸指针存入 Task
}
```

此设计**完全依赖 C++17 的保证拷贝消除（guaranteed copy elision）**才能正确工作。`Result` 内部的 `Semaphore` 包含 `std::mutex` 和 `std::condition_variable`，二者**不可移动也不可拷贝**，意味着 `Result` 本身也无法移动/拷贝。

**后果**：
- 如果编译器未进行拷贝消除，代码会**编译失败**。
- 如果 `Result` 对象被移动（例如放入容器），`Task` 中的裸指针将变成**悬空指针（dangling pointer）**，导致未定义行为。

**修复建议**：使用 `std::shared_ptr<Result>` 管理 Result 对象生命周期，或改用 `std::future` / `std::promise` 机制替代自定义的 Result + Semaphore 方案。

---

## 🟡 中等问题

### 3. `Any::cast_()` 抛出的异常类型不规范

**文件**：`include/Any.h`

**问题描述**：

```cpp
template<typename T>
T cast_()
{
    Derived<T>* pd = dynamic_cast<Derived<T>*>(base_.get());
    if (pd == nullptr)
    {
        throw "type is not match";  // ← 抛出 const char*
    }
    return pd->data_;
}
```

抛出 `const char*` 而非 `std::exception` 子类，无法通过 `catch (const std::exception& e)` 捕获。

**修复建议**：改为 `throw std::runtime_error("type is not match");`。

---

### 4. `Thread::generatedId_` 自增非线程安全

**文件**：`src/Thread.cpp`、`include/Thread.h`

**问题描述**：

```cpp
int Thread::generatedId_ = 0;

Thread::Thread(ThreadFunc func)
    : func_(func)
    , threadId_(generatedId_++)  // ← 非原子操作
{}
```

`generatedId_++` 不是原子操作。虽然当前代码在持有互斥锁时创建线程（在 `submitTask` 中），但在 `start()` 中创建线程时并没有加锁保护。如果在无锁环境下创建 `Thread` 对象，会产生数据竞争。

**修复建议**：将 `generatedId_` 类型改为 `std::atomic<int>`。

---

### 5. 提交失败时仍然设置 `Result`

**文件**：`src/Result.cpp`、`src/ThreadPool.cpp`

**问题描述**：

```cpp
// submitTask 中：
std::cerr << "任务提交失败：任务队列已满" << std::endl;
return Result(task, false);
```

即使 `isValid = false`（任务提交失败），`Result` 构造函数仍然调用 `task_->setResult(this)`，给一个已知无效的 Result 设置了指针，语义上不合理。

**修复建议**：在 `Result` 构造函数中判断 `isValid`，仅在有效时才调用 `task_->setResult(this)`。

---

## 🟢 轻微问题

### 6. `taskNum_` 与 `taskQueue_.size()` 冗余

**文件**：`include/ThreadPool.h`、`src/ThreadPool.cpp`

**问题描述**：

`taskNum_`（`atomic_uint`）的值始终等于 `taskQueue_.size()`，两者在锁内同步修改。维护两个表示同一含义的状态变量，增加了代码不一致的风险。

**修复建议**：删除 `taskNum_`，统一使用 `taskQueue_.size()`。

---

### 7. `Semaphore::post_()` 使用 `notify_all` 效率低

**文件**：`include/Semaphore.h`

**问题描述**：

```cpp
void post_()
{
    std::unique_lock<std::mutex> lock(mutex_);
    resLimit_++;
    condition_.notify_all();  // ← 每次只释放一个资源，唤醒全部线程
}
```

每次 `post` 只增加一个信号量资源，用 `notify_one` 即可唤醒一个等待者，`notify_all` 会导致不必要的惊群效应（thundering herd）。

**修复建议**：将 `notify_all()` 改为 `notify_one()`。

---

## 问题总结

| 严重程度 | 编号 | 问题 | 后果 |
|---------|------|------|------|
| 🔴 严重 | #1 | `threadHandler` 退出时不清理 | 析构函数**死锁** |
| 🔴 严重 | #2 | `Result` 裸指针 + 不可移动 | 悬空指针 / 编译失败 |
| 🟡 中等 | #3 | `Any::cast_()` 抛 `const char*` | 异常无法被标准方式捕获 |
| 🟡 中等 | #4 | `generatedId_++` 非原子 | 潜在数据竞争 |
| 🟡 中等 | #5 | 失败时仍设置 Result | 语义不一致 |
| 🟢 轻微 | #6 | `taskNum_` 冗余 | 增加维护成本和不一致风险 |
| 🟢 轻微 | #7 | `notify_all` 替代 `notify_one` | 不必要的性能开销 |
