#ifndef _CEELSemaphore_HPP_
#define _CEELSemaphore_HPP_
#include<chrono>
#include<thread>
#include<mutex>
#include<condition_variable>
//�ź���
class CELLSemaphore
{
public:
	//������ǰ�߳�
	void wait()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0)
		{
			//�����ȴ�OnRun�˳�,lambda���ʽ�ж���ٻ��ѵ������������û�е�������������
			//����£��ȵ����˻��Ѻ��������º������������ʱ��û�˻��ѣ�һֱ������
			//ֻ�е�lambda���ʽ���ص�ֵΪfalseʱ�Ż������������
			_cv.wait(lock, [this]()->bool {return _wakeup > 0;});
			--_wakeup;
		}
	}
	//���ѵ�ǰ�߳�
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
	//������
	std::mutex _mutex;
	//�����ȴ�-��������
	std::condition_variable _cv;
	//wait�ȴ�����
	int _wait = 0;
	//wakeup���Ѽ���
	int _wakeup = 0;
};
#endif // !_CEELSemaphore_HPP_
