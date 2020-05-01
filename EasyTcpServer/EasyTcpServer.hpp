#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include<thread>
#include<mutex>
#include<atomic>

//CELL��һЩSocket��ص�ͷ�ļ����Զ����һЩ��
#include "CELL.hpp"
//CellClient�Ǵ���ͻ��˵��Զ����ļ������ͻ��˷�����Ϣ
#include "CELLClient.hpp"
//CELLClient���ӷ����������տͻ��˵����Ӻ;��տͻ��˵���Ϣ
#include "CELLServer.hpp"
//�����¼��Ĵ���ͳ�Ƽ��������ͻ��˼��롢�뿪�����տͻ�����Ϣ�����ͻ��˷�����Ϣ��
#include "INetEvent.hpp"
//�߳�
#include"CELLThread.hpp"
//
#include"CELLNetWork.hpp"

//��װ����˵���
class EasyTcpServer :public INetEvent//������
{
private:
	//����˽��տͻ��˵������߳�
	CELLThread _thread;
	//��Ϣ��������ڲ��������߳̽��տͻ�������
	std::vector<CELLServer*> _CellServers;
	//��ʱ����ʱ(��)��ͳ��ÿ��������ݰ��ĸ���
	CELLTimestamp _Ttime;
	//�������˿ں�
	SOCKET _sock;
protected:
	//ԭ�Ӳ�������, ͳ�Ʒ�����ڽ��ջ������Ľ��մ���
	std::atomic_int _recvCount;
	//ԭ�Ӳ�������, ͳ�Ʒ�����յ��ͻ�����Ϣ�ĸ���
	std::atomic_int _msgCount;
	//ԭ�Ӳ�������, ͳ�Ƽ���ͻ��˸���
	std::atomic_int _clientCount;

public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_msgCount = 0;
		_clientCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}

	//��ʼ��sock
	SOCKET InitSocket()
	{
		//��ʼ�����绷��
		CELLNetWork::Init();
		if (INVALID_SOCKET != _sock)
		{
			CELLLog::Info("<socket=%d>�رվ�����...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			CELLLog::Info("���󣬽���Socketʧ��...\n");
		}
		else {
			CELLLog::Info("����socket=<%d>�ɹ�...\n", (int)_sock);
		}
		return _sock;
	}

	//��ip�Ͷ˿ں�
	int Bind(const char* ip, unsigned short port)
	{
		//if (_sock == INVALID_SOCKET)
		//  InitSocket();
		//2��bind�����ڽ��ܿͻ������ӵ�����˿�
		sockaddr_in _sin = {};//�ṹ�壨��ͬ�壩
		_sin.sin_family = AF_INET;//��ַ����
		_sin.sin_port = htons(port);//�˿ں�
#ifdef _WIN32
		if (ip)//ip��ַ��Ϊ��
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip)//ip��ַ��Ϊ��
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}

#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (ret == SOCKET_ERROR)
		{
			CELLLog::Info("���󣬰�����˿�<%d>ʧ��....\n", port);
		}
		else
		{
			CELLLog::Info("������˿�<%d>�ɹ�....\n", port);
		}
		return ret;
	}

	//�����˿ں�
	int Listen(int n)//5�ǵȴ������������
	{
		int ret = listen(_sock, n);
		if (ret == SOCKET_ERROR)
			CELLLog::Info("���󣬼�������˿�ʧ��<socket=%d>....\n", (int)_sock);
		else
		{
			CELLLog::Info("��������˿ڳɹ�<socket=%d>....\n", (int)_sock);
		}
		return ret;
	}

	//���ܿͻ�������
	SOCKET Accept()
	{
		//4��accept�ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};//����ͻ�������
		int nAddrlen = sizeof(sockaddr_in);
		SOCKET csock = INVALID_SOCKET;//����һ����Ч�׽���
#ifdef _WIN32
		csock = accept(_sock, (sockaddr*)&clientAddr, &nAddrlen);

#else
		csock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrlen);

#endif
		if (csock == INVALID_SOCKET)
		{
			CELLLog::Info("���󣬽��յ���Ч�Ŀͻ���<socket=%d>....\n", (int)csock);
		}
		else
		{
			//�����ӵ��¿ͻ��˷��䵽��Ϣ�����߳�
			addClientToCellServer(new CELLClient(csock));
			//CELLLog::Info("<��%d��>�¿ͻ��˼���<socket=%d> ,IP=%s\n", (int)_clients.size(),(int)csock, inet_ntoa(clientAddr.sin_addr));//inet_ntoa()ת���ɿɼ�������IP
		}
		return csock;
	}
	//�����ӵ��¿ͻ��˷��䵽��Ϣ�����߳�
	void addClientToCellServer(CELLClient* pClient)
	{
		//���ҿͻ��������ٵ���Ϣ�����̣߳�CellServer��
		auto pMinServer = _CellServers[0];
		for (auto pCellServer : _CellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		//������������߳��м����¿ͻ���
		pMinServer->addClient(pClient);
	}

	//����������߳�
	void New_thread(int ThreadCount)
	{
		for (int i = 0; i < ThreadCount; i++)
		{
			//����һ��������̣߳�ָ�룩
			auto ser = new CELLServer(i+1);
			//�������̼߳��뵽�����߳�������ȥ
			_CellServers.push_back(ser);
			//ע�������¼����ն���
			ser->setEventObj(this);
			//������Ϣ�����߳�
			ser->Start();
		}
		_thread.Start(nullptr, [this](CELLThread* pThread) {OnRun(pThread); });
	}
	//�ر�socket
	void Close()
	{
		CELLLog::Info("EasyTcpServer.Close begin\n");
		_thread.Close();
		if (_sock != INVALID_SOCKET)
		{
			//�ͷŶ��̶߳���
			for (auto iter : _CellServers)
			{
				delete iter;
			}
			_CellServers.clear();
#ifdef _WIN32

			//8���ر��׽���closesocket()
			closesocket(_sock);
#else
			// 8 �ر��׽���closesocket
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
		CELLLog::Info("EasyTcpServer.Close end\n");
	}

	//�߳����ǰ�ȫ�ģ�ֻ�����̵߳���
	virtual void OnNetJoin(CELLClient* pClient)
	{
		//�ͻ���������һ
		_clientCount++;
	}
	//�߳��ϲ���ȫ�ģ���4��CellServer����
	virtual void OnNetLeave(CELLClient* pClient)
	{
		//�뿪�Ŀͻ���������һ
		_clientCount--;
		
	}
	//�߳��ϲ���ȫ�ģ���4��CellServer����
	virtual void OnNetMsg(CELLServer* pCellServer, CELLClient* pClient, DataHeader* header)
	{
		//ԭ�Ӳ�������,ͳ���յ��İ��ĸ���
		_msgCount++;
	}
	virtual void OnNetRecv(CELLClient* pClient)
	{
		_recvCount++;
	}
private:
	//����������Ϣ
	void OnRun(CELLThread* pThread)
	{
		while (pThread->isRun())
		{
			time4msg();
			//fd_set��һ���׽��ֵļ��ϣ����԰������ɺ���ʹ�� ��������һ���ṹ�壨��������������fd_arry[]��ʾ���ϣ�fd_count��ʾ���ϵĴ�С
			fd_set fdRead;      //�ɶ��Ķ˿ڼ���(�ͻ��˿���ʹ�õĶ˿�)

			//FD_ZERO()�Ƕ�fd_set()���������
			FD_ZERO(&fdRead);

			//FD_SET()��һ�������Ķ˿ڼ��뼯��֮��   ������˿�socketҲ�����ļ���������
			FD_SET(_sock, &fdRead);//������˶˿ںż���ɶ�����

			//��ʼ���ṹ��,���������΢����������,��������10�����ѯһ��
			timeval t = { 0,1 };
			//_sock+1��ʾ���������ж˿ںŵ����ֵ+1����ʾ�˿ڷ�Χ����Windows�������������Ϊ0
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);//���ӷ������˿ڵ�����״�������ɶ������ڵĶ˿�������Ҫ����ʱ��������ֵ
			//���ظ�ֵ�����쳣����
			//������ֵ���ɶ���д
			//���ظ�ֵ���ȴ���ʱ�����ɶ�����д
			//CELLLog::Info("select ret=%d  count=%d\n", ret, count++);

			if (ret < 0)
			{
				CELLLog::Info("EasyTcpServer.OnRun select error Exit.\n");
				pThread->Exit();
				break;
			}

			//_sock�Ƿ������Ķ˿ںţ������жϷ��������Ƿ������ݿɶ������Ƿ��� �¿ͻ� �˼��룩
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);//��һ�������Ķ˿ڣ��ļ����������Ӽ�����ɾ��
				Accept();
			}
		}

	}

	//���㲢���ÿ���յ������ݰ�����
	void time4msg()
	{
		//��ȡ��ǰ��
		auto t1 = _Ttime.getElapsedSecond();
		//ͳ��ÿ���ӽ��ն��ٸ���
		if (t1 >= 1.0)
		{
			CELLLog::Info("Thread<%d> ,time<%lf>,socket<%d>,_clients<%d>,recv<%d>,msg<%d>\n", (int)_CellServers.size(), t1, (int)_sock, (int)_clientCount, (int)(_recvCount / t1), (int)(_msgCount / t1));
			_recvCount = 0;
			_msgCount = 0;
			//�������һ��ʱ���
			_Ttime.update();
		}
	}

};

#endif

