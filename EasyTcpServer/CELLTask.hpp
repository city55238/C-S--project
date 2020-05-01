#ifndef _CELL_Task_H_
#include<thread>
#include<mutex>
#include<list>
#include<functional>

#include "CELLThread.hpp"
//#include "CELLLog.hpp"
//ִ������ķ�������
class CellTaskServer
{
public:
	//�ô������������Ǹ������߳�
	int TaskServer_id = -1;
private:
	//����һ���������� CellTask��һ���޲������޷���ֵ���������� 
	typedef std::function<void()> CellTask;
private:
	//��������
	std::list<CellTask>_tasks;
	//�������ݻ�����
	std::list<CellTask>_tasksBuf;
	//�ı��������ݻ�����ʱ��Ҫ����
	std::mutex _mutex;
	//����һ���߳�
	CELLThread _thread;
	
public:
	//�������,���������ã�&����Ϊ�˼��ٿ�������
	void addTask(CellTask task)
	{
		//�Խ���
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuf.push_back(task);

	}

	//���������߳�
	void Start()
	{
		_thread.Start(nullptr, [this](CELLThread* pThread) {OnRun(pThread); });
	}
	void Close()
	{
		//����ô������������Ǹ������߳�
		//CELLLog::Info("CellTaskServer%d.Close  begin\n", TaskServer_id);
		_thread.Close();
		//CELLLog::Info("CellTaskServer%d.Close  end\n", TaskServer_id);
		
	}
protected:
	//��������
	void OnRun(CELLThread* pThread)
	{
		while (pThread->isRun())
		{
			//�����񻺳���ȡ�����ݣ��ŵ���������
			if (!_tasksBuf.empty())
			{
				//�Խ���
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuf)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
			}
			//�����б�Ϊ��
			if (_tasks.empty())
			{
				//�ӳ�1����
				std::chrono::microseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			//��������
			for (auto pTask : _tasks)
			{
				//pTask()��һ���������ʽ���ⲿ�����б�Ϊ�գ������б�Ϊ�գ�
				//��������SendData(pClient,header),����ֵΪ��
				pTask();
			}
			_tasks.clear();
		}

		//����������е�����
		for (auto pTask : _tasksBuf)
		{
			//pTask()��һ���������ʽ���ⲿ�����б�Ϊ�գ������б�Ϊ�գ�
			//��������SendData(pClient,header),����ֵΪ��
			pTask();
		}
		//CELLLog::Info("CellTaskServer%d.OnRun exit \n", TaskServer_id);
	}

};
#endif