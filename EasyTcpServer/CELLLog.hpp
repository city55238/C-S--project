#ifndef _CELLLog_HPP_
#define _CELLLog_HPP_

#include"CELL.hpp"
#include<ctime>
class CELLLog
{
private:
	CELLLog()
	{
		_taskServer.Start();
	}

	~CELLLog()
	{
		_taskServer.Close();
		if (_logFile)
		{
			Info("CELLLog fclose(_logFile)\n");
			fclose(_logFile);
			_logFile = nullptr;
		}
	}
public:
	//全局的单例模式
	static CELLLog& Instance()
	{
		static CELLLog sLog;
		return sLog;
	}
	//设置日志文件路径，mode是文件打开方式，logPath是文件路径
	void setLogPath(const char* logPath, const char* mode)
	{
		//中途需要更改日志文件的情况，初始化_logFile=nullptr
		if (_logFile)
		{
			Info("CELLLog::setLogPath _logFile != nullptr\n");
			fclose(_logFile);
			_logFile = nullptr;
		}


		_logFile = fopen(logPath, mode);
		if (_logFile)
		{
			Info("CELLLog::setLogPath success,<%s,%s>\n", logPath, mode);
		}
		else {
			Info("CELLLog::setLogPath failed,<%s,%s>\n", logPath, mode);
		}
	}

	static void Info(const char*pStr)
	{
		CELLLog* pLog = &Instance();
		//因为有很多日志需要记录，把记录日志文件放到任务系统中，=表示捕获所有外部参数
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile)
			{
				//获取系统的当前时间
				auto t = system_clock::now();
				//将获取的时间转换成time_t类型
				auto tNow = system_clock::to_time_t(t);

				//以系统的时间格式输出
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));

				 //ctime()函数将time_t类型的时间转化成字符串格式,这个字符串自带换行符
				std::tm* now = std::gmtime(&tNow);

				//写入日志的类型
				fprintf(pLog->_logFile, "%s", "Info ");

				//自定义时间格式，时间需要偏移（时区的不同）
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour+8, now->tm_min, now->tm_sec);
				
				//日志的内容
				fprintf(pLog->_logFile, "%s", pStr);
				
				//将日志信息实时写入日志文件，不必等在关闭文件的时候一起写入
				fflush(pLog->_logFile);
			}
			//记录日志的同时，也在控制台输出
			printf("%s", pStr);
		});
	}
	template<typename ...Args>
	static void Info(const char* pformat, Args...args)
	{
		CELLLog* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile)
			{
				//获取系统的当前时间
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);

				//以系统的时间格式输出
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));

				//时间格式转换
				std::tm* now = std::gmtime(&tNow);

				//写入日志的类型
				fprintf(pLog->_logFile, "%s", "Info ");

				//自定义时间格式，时间需要偏移（时区的不同）
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour + 8, now->tm_min, now->tm_sec);
				
				//
				fprintf(pLog->_logFile, pformat, args...);

				//将日志信息实时写入日志文件
				fflush(pLog->_logFile);
			}
			//记录日志的同时，也在控制台输出
			printf(pformat, args...);
		});
	}
private:
	//日志文件名称
	FILE* _logFile = nullptr;
	CellTaskServer _taskServer;
};
#endif // !_CELLLog_HPP_