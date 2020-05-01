#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

//#include <windows.h>
#include<chrono>
using namespace std::chrono;

class CELLTime
{
public:
	//获取当前时间戳 (毫秒)
	static time_t getNowInMilliSec()
	{
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}
};

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
		//设置一个持续的时间段
		_begin = high_resolution_clock::now();
    }
   
    double getElapsedSecond()
    {
		//毫秒转化成秒
        return  getElapsedTimeInMicroSec() * 0.000001;
    }
  
    double getElapsedTimeInMilliSec()
    {
		//微秒转化成毫秒
        return this->getElapsedTimeInMicroSec() * 0.001;
    }
    /**
    *   获取微妙
    */
    long long getElapsedTimeInMicroSec()
    {
		//当前时间-进入时间，获取从开始计时到现在经历过的时间，以微妙为单位
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
    }
protected:
    

	//设置一个高精度时间点
	time_point<high_resolution_clock> _begin;
};

#endif // !_CELLTimestamp_hpp_