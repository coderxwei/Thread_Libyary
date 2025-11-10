#include "Thread.h"
#include <thread>
void Thread::start()
{
	std::thread t(func_, thread_ID_);
	t.detach();
}

Thread::Thread(Threadfunc func):func_(func),thread_ID_(Generated_++)
{
	
}

Thread::~Thread()
{

}

int Thread::getId() const
{
	return thread_ID_;
}

int Thread::Generated_ = 0;
