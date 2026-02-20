#pragma once
#include <memory>
#include <stdexcept>

// 类型擦除容器类，类似于 std::any
class Any
{
public:
    Any() = default;
    ~Any() = default;

    // 禁用拷贝
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;

    // 允许移动
    Any(Any&&) = default;
    Any& operator=(Any&&) = default;

    // 模板构造函数，接受任意类型
    template<typename T>
    Any(T data) : base_(std::make_unique<Derived<T>>(data))
    {
    }

    // 类型转换方法，使用 RTTI 机制将基类指针转为派生类对象
    template<typename T>
    T cast_()
    {
        Derived<T>* pd = dynamic_cast<Derived<T>*>(base_.get());
        if (pd == nullptr)
        {
            throw std::runtime_error("type is not match");
        }
        return pd->data_;
    }

private:
    // 基类（用于类型擦除）
    class Base
    {
    public:
        virtual ~Base() = default;
    };

    // 派生类模板（持有具体类型的数据）
    template<typename T>
    class Derived : public Base
    {
    public:
        Derived(T data) : data_(data) {}
        T data_;
    };

    std::unique_ptr<Base> base_;
};
