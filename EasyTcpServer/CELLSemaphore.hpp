#ifndef _CEELSemaphore_HPP_
#define _CEELSemaphore_HPP_
#include<chrono>
#include<thread>
#include<mutex>
#include<condition_variable>
//信号量
class CELLSemaphore
{
public:
	//阻塞当前线程
	void wait()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0)
		{
			//阻塞等待OnRun退出,lambda表达式判断虚假唤醒的情况（就是在没有调用阻塞函数的
			//情况下，先调用了唤醒函数，导致后面调用阻塞的时候没人唤醒，一直阻塞）
			//只有当lambda表达式返回的值为false时才会调用阻塞函数
			_cv.wait(lock, [this]()->bool {return _wakeup > 0;});
			--_wakeup;
		}
	}
	//唤醒当前线程
	void wakeup()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (++_wait <= 0)
		{
			++_wakeup;
			_cv.notify_one();
		}
	}
private:
	//互斥锁
	std::mutex _mutex;
	//阻塞等待-条件变量
	std::condition_variable _cv;
	//wait等待计数
	int _wait = 0;
	//wakeup唤醒计数
	int _wakeup = 0;
};
#endif // !_CEELSemaphore_HPP_
