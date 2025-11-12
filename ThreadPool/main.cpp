#include <iostream>
#include "Task.h"
#include "const.h"
#include  "Thread.h"
#include  "ThreadPool.h"
#include "Any.h"



//示例--example:
class mytaks :public Task
{
public:
Any run()
{
	int snm = 0;
	std::cout << "tid" << std::this_thread::get_id() << begin_ << std::endl;
	int sum = 0;
	for (int i=begin_;i<end_;i++)
	{
		sum += i;
		std::cout << "tid" << std::this_thread::get_id() << "end" << std::endl;
	}
	return  sum;
}
    mytaks(int b, int e) ;
	~mytaks();
private:
	int  begin_;
	int  end_;

};
mytaks::mytaks(int b, int e) :begin_(b), end_(e) 
{
	
}
mytaks::~mytaks()
{
	
}





int main()
{
	{
		ThreadPool pool;
		pool.start(4);
	 //	std::shared_ptr<Task>mytask_ = std::make_shared<mytaks>(1, 100);
	 //	pool.submitTask(mytask_);
	 //	pool.submitTask(mytask_);
	 //	pool.submitTask(mytask_);
		Result res = pool.submitTask(std::make_shared<mytaks>(1, 300));

		int sum = res.getValue().cast_<int>();
		std::cout << "sum=" << sum << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(5));
		///当线程池释放的时候线程的资源怎么回收。
	}

	return 0;
}

