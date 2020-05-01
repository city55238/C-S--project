#ifndef _CELL_Task_H_
#include<thread>
#include<mutex>
#include<list>
#include<functional>

#include "CELLThread.hpp"
//#include "CELLLog.hpp"
//执行任务的服务类型
class CellTaskServer
{
public:
	//该处理任务属于那个处理线程
	int TaskServer_id = -1;
private:
	//定义一个数据类型 CellTask是一个无参数、无返回值的数据类型 
	typedef std::function<void()> CellTask;
private:
	//任务数据
	std::list<CellTask>_tasks;
	//任务数据缓存区
	std::list<CellTask>_tasksBuf;
	//改变任务数据缓存区时需要加锁
	std::mutex _mutex;
	//创建一个线程
	CELLThread _thread;
	
public:
	//添加任务,这里用引用（&）是为了减少拷贝操作
	void addTask(CellTask task)
	{
		//自解锁
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuf.push_back(task);

	}

	//启动服务线程
	void Start()
	{
		_thread.Start(nullptr, [this](CELLThread* pThread) {OnRun(pThread); });
	}
	void Close()
	{
		//输出该处理任务属于那个处理线程
		//CELLLog::Info("CellTaskServer%d.Close  begin\n", TaskServer_id);
		_thread.Close();
		//CELLLog::Info("CellTaskServer%d.Close  end\n", TaskServer_id);
		
	}
protected:
	//工作函数
	void OnRun(CELLThread* pThread)
	{
		while (pThread->isRun())
		{
			//从任务缓冲区取出数据，放到任务链表
			if (!_tasksBuf.empty())
			{
				//自解锁
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuf)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
			}
			//任务列表为空
			if (_tasks.empty())
			{
				//延迟1毫秒
				std::chrono::microseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			//处理任务
			for (auto pTask : _tasks)
			{
				//pTask()是一个匿名表达式，外部变量列表为空，参数列表为空，
				//函数体是SendData(pClient,header),返回值为空
				pTask();
			}
			_tasks.clear();
		}

		//处理缓冲队列中的任务
		for (auto pTask : _tasksBuf)
		{
			//pTask()是一个匿名表达式，外部变量列表为空，参数列表为空，
			//函数体是SendData(pClient,header),返回值为空
			pTask();
		}
		//CELLLog::Info("CellTaskServer%d.OnRun exit \n", TaskServer_id);
	}

};
#endif