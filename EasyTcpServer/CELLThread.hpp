#ifndef _CELL_Thread_HPP_
#define _CELL_Thread_HPP_

#include<thread>
#include<functional>
#include"CELLSemaphore.hpp"
class CELLThread
{
private:
	//����һ���¼����� EventCall,����һ���޲������޷���ֵ���������� 
	typedef std::function<void(CELLThread*)> EventCall;
public:
	//�����߳�
	void Start(EventCall onCreate = nullptr, EventCall onRun = nullptr, EventCall onDestory = nullptr)
	{
		//�Խ���
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

			//mem_fn�ѳ�Ա����תΪ��������ʹ�ö���ָ���������ý��а�
			std::thread t(std::mem_fn(&CELLThread::OnWork), this);
			t.detach();
		}
	}
	//�ر��߳�
	void Close()
	{
		//�Խ���
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
			_sem.wait();
		}
	}

	//OnRun()�����ڵ��ô����߳�֮ǰ�˳������˳�������Ҫ�������������������һֱ����
	void Exit()
	{
		//�Խ���
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
		}
	}
	//�߳��Ƿ�������
	bool isRun()
	{
		return _isRun;
	}
protected:
	//�߳�����ʱ�Ĺ�������
	void OnWork()
	{
		if (_onCreate)
			_onCreate(this);
		if (_onRun)
			_onRun(this);
		if (_onDestory)
			_onDestory(this);
		//��������������ʱ���ѹر��߳�
		_sem.wakeup();
	}

private:
	//�߳������¼�
	EventCall _onCreate;
	//�߳������¼�
	EventCall _onRun;
	//�̹߳ر��¼�
	EventCall _onDestory;
	//��ͬ�̸߳ı����ݵ�ʱ����Ҫ����
	std::mutex _mutex;
	//�ź��������̵߳��������˳�
	CELLSemaphore _sem;
	//�߳��Ƿ�����������
	bool _isRun = false;
};
#endif // !_CELL_Thread_HPP_
