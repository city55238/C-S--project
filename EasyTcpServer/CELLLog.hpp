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
	//ȫ�ֵĵ���ģʽ
	static CELLLog& Instance()
	{
		static CELLLog sLog;
		return sLog;
	}
	//������־�ļ�·����mode���ļ��򿪷�ʽ��logPath���ļ�·��
	void setLogPath(const char* logPath, const char* mode)
	{
		//��;��Ҫ������־�ļ����������ʼ��_logFile=nullptr
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
		//��Ϊ�кܶ���־��Ҫ��¼���Ѽ�¼��־�ļ��ŵ�����ϵͳ�У�=��ʾ���������ⲿ����
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile)
			{
				//��ȡϵͳ�ĵ�ǰʱ��
				auto t = system_clock::now();
				//����ȡ��ʱ��ת����time_t����
				auto tNow = system_clock::to_time_t(t);

				//��ϵͳ��ʱ���ʽ���
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));

				 //ctime()������time_t���͵�ʱ��ת�����ַ�����ʽ,����ַ����Դ����з�
				std::tm* now = std::gmtime(&tNow);

				//д����־������
				fprintf(pLog->_logFile, "%s", "Info ");

				//�Զ���ʱ���ʽ��ʱ����Ҫƫ�ƣ�ʱ���Ĳ�ͬ��
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour+8, now->tm_min, now->tm_sec);
				
				//��־������
				fprintf(pLog->_logFile, "%s", pStr);
				
				//����־��Ϣʵʱд����־�ļ������ص��ڹر��ļ���ʱ��һ��д��
				fflush(pLog->_logFile);
			}
			//��¼��־��ͬʱ��Ҳ�ڿ���̨���
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
				//��ȡϵͳ�ĵ�ǰʱ��
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);

				//��ϵͳ��ʱ���ʽ���
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));

				//ʱ���ʽת��
				std::tm* now = std::gmtime(&tNow);

				//д����־������
				fprintf(pLog->_logFile, "%s", "Info ");

				//�Զ���ʱ���ʽ��ʱ����Ҫƫ�ƣ�ʱ���Ĳ�ͬ��
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour + 8, now->tm_min, now->tm_sec);
				
				//
				fprintf(pLog->_logFile, pformat, args...);

				//����־��Ϣʵʱд����־�ļ�
				fflush(pLog->_logFile);
			}
			//��¼��־��ͬʱ��Ҳ�ڿ���̨���
			printf(pformat, args...);
		});
	}
private:
	//��־�ļ�����
	FILE* _logFile = nullptr;
	CellTaskServer _taskServer;
};
#endif // !_CELLLog_HPP_