#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

//#include <windows.h>
#include<chrono>
using namespace std::chrono;

class CELLTimestamp
{
public:
    CELLTimestamp()
    {
		update();
    }
    ~CELLTimestamp()
    {
	}

    void update()
    {
		//����һ��������ʱ���
		_begin = high_resolution_clock::now();
    }
   
    double getElapsedSecond()
    {
		//����ת������
        return  getElapsedTimeInMicroSec() * 0.000001;
    }
  
    double getElapsedTimeInMilliSec()
    {
		//΢��ת���ɺ���
        return this->getElapsedTimeInMicroSec() * 0.001;
    }
    /**
    *   ��ȡ΢��
    */
    long long getElapsedTimeInMicroSec()
    {
		//��ǰʱ��-����ʱ�䣬��ȡ�ӿ�ʼ��ʱ�����ھ�������ʱ�䣬��΢��Ϊ��λ
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
    }
protected:
    

	//����һ���߾���ʱ���
	time_point<high_resolution_clock> _begin;
};

#endif // !_CELLTimestamp_hpp_