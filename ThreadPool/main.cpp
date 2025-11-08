#include <iostream>
#include "Task.h"
#include "const.h"
#include  "Thread.h"
#include  "ThreadPool.h"
#include "Any.h"



//Ê¾Àý--example:
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
	mytaks();
	~mytaks();
private:
	int  begin_;
	int  end_;

};
mytaks::mytaks()
{
	
}
mytaks::~mytaks()
{
	
}





int main()
{
	ThreadPool pool;
	pool.start(4);
	std::shared_ptr<Task>mytask_ = std::make_shared<mytaks>();
	pool.submitTask(mytask_);
	pool.submitTask(mytask_);
	pool.submitTask(mytask_);
	Result res= pool.submitTask(std::make_shared<mytaks>());
	std::this_thread::sleep_for(std::chrono::seconds(5));
	return 0;
}

