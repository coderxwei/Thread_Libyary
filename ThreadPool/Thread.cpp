#include "Thread.h"
#include <thread>
void Thread::start()
{
	std::thread t(func_);
	t.detach();
}

Thread::Thread(Threadfunc func):func_(func)
{
	
}

Thread::~Thread()
{

}
