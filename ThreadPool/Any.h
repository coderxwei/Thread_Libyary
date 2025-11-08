#pragma once
#include <memory>

class Any
{
public:
	Any() = default;
	~Any() = default;

	Any(const Any& any) = delete;

	Any& operator=(const Any& any) = delete;
	//右值 不能为const 类型
	Any ( Any&&)=default;

	Any& operator=( Any&&) = default;

	//构造函数
	template<typename T>
	Any(T data) : base_(std::make_unique<Drive<T>>(data))
	{
	};

	//定义一个模板的方法
	template<typename T>
	T cast_()
	{
		//RTTI 机制。---父类指针执行子类的对象。
		Drive<T>* pd = dynamic_cast<Drive<T>*> (base_.get());
		//
		if (pd==nullptr)
		{
			throw("type is notmatch");
		}
		return pd->data_;

	}
private:
	class Base
	{
	public:
		virtual  ~Base() = default;
	};
	//派生类类型
	template<typename T>
	class Drive:public  Base
	{
	public:
		Drive(T data) :data_(data) {};
	
		T data_;
	};
private:
	std::unique_ptr<Base> base_;
	int  sun;
};

