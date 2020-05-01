#ifndef _CELL_Thread_HPP_
#define _CELL_Thread_HPP_

#include<thread>
#include<functional>
#include"CELLSemaphore.hpp"
class CELLThread
{
private:
	//定义一个事件类型 EventCall,它是一个无参数、无返回值的数据类型 
	typedef std::function<void(CELLThread*)> EventCall;
public:
	//启动线程
	void Start(EventCall onCreate = nullptr, EventCall onRun = nullptr, EventCall onDestory = nullptr)
	{
		//自解锁
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;
			if (onCreate)
				_onCreate = onCreate;
			if (onRun)
				_onRun = onRun;
			if (onDestory)
				_onDestory = onDestory;

			//mem_fn把成员函数转为函数对象，使用对象指针或对象引用进行绑定
			std::thread t(std::mem_fn(&CELLThread::OnWork), this);
			t.detach();
		}
	}
	//关闭线程
	void Close()
	{
		//自解锁
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
			_sem.wait();
		}
	}

	//OnRun()函数在调用处理线程之前退出主动退出，不需要调用阻塞函数，否则会一直阻塞
	void Exit()
	{
		//自解锁
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
		}
	}
	//线程是否运行中
	bool isRun()
	{
		return _isRun;
	}
protected:
	//线程运行时的工作函数
	void OnWork()
	{
		if (_onCreate)
			_onCreate(this);
		if (_onRun)
			_onRun(this);
		if (_onDestory)
			_onDestory(this);
		//工作函数结束的时候唤醒关闭线程
		_sem.wakeup();
	}

private:
	//线程启动事件
	EventCall _onCreate;
	//线程运行事件
	EventCall _onRun;
	//线程关闭事件
	EventCall _onDestory;
	//不同线程改变数据的时候需要加锁
	std::mutex _mutex;
	//信号量控制线程的启动、退出
	CELLSemaphore _sem;
	//线程是否启动运行中
	bool _isRun = false;
};
#endif // !_CELL_Thread_HPP_
