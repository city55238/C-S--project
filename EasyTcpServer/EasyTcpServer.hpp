#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include<thread>
#include<mutex>
#include<atomic>

//CELL是一些Socket相关的头文件和自定义的一些宏
#include "CELL.hpp"
//CellClient是处理客户端的自定义文件，给客户端发送消息
#include "CELLClient.hpp"
//CELLClient是子服务器，接收客户端的连接和就收客户端的消息
#include "CELLServer.hpp"
//网络事件的处理（统计计数）（客户端加入、离开、接收客户端消息、给客户端发送消息）
#include "INetEvent.hpp"
//线程
#include"CELLThread.hpp"
//
#include"CELLNetWork.hpp"

//封装服务端的类
class EasyTcpServer :public INetEvent//派生类
{
private:
	//服务端接收客户端的连接线程
	CELLThread _thread;
	//消息处理对象，内部创建多线程接收客户端连接
	std::vector<CELLServer*> _CellServers;
	//计时器计时(类)，统计每秒接收数据包的个数
	CELLTimestamp _Ttime;
	//服务器端口号
	SOCKET _sock;
protected:
	//原子操作计数, 统计服务端在接收缓存区的接收次数
	std::atomic_int _recvCount;
	//原子操作计数, 统计服务端收到客户端消息的个数
	std::atomic_int _msgCount;
	//原子操作计数, 统计加入客户端个数
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

	//初始化sock
	SOCKET InitSocket()
	{
		//初始化网络环境
		CELLNetWork::Init();
		if (INVALID_SOCKET != _sock)
		{
			CELLLog::Info("<socket=%d>关闭旧连接...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			CELLLog::Info("错误，建立Socket失败...\n");
		}
		else {
			CELLLog::Info("建立socket=<%d>成功...\n", (int)_sock);
		}
		return _sock;
	}

	//绑定ip和端口号
	int Bind(const char* ip, unsigned short port)
	{
		//if (_sock == INVALID_SOCKET)
		//  InitSocket();
		//2、bind绑定用于接受客户端连接的网络端口
		sockaddr_in _sin = {};//结构体（共同体）
		_sin.sin_family = AF_INET;//地址家族
		_sin.sin_port = htons(port);//端口号
#ifdef _WIN32
		if (ip)//ip地址不为空
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip)//ip地址不为空
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
			CELLLog::Info("错误，绑定网络端口<%d>失败....\n", port);
		}
		else
		{
			CELLLog::Info("绑定网络端口<%d>成功....\n", port);
		}
		return ret;
	}

	//监听端口号
	int Listen(int n)//5是等待的最大连接数
	{
		int ret = listen(_sock, n);
		if (ret == SOCKET_ERROR)
			CELLLog::Info("错误，监听网络端口失败<socket=%d>....\n", (int)_sock);
		else
		{
			CELLLog::Info("监听网络端口成功<socket=%d>....\n", (int)_sock);
		}
		return ret;
	}

	//接受客户端连接
	SOCKET Accept()
	{
		//4、accept等待接受客户端连接
		sockaddr_in clientAddr = {};//保存客户端数据
		int nAddrlen = sizeof(sockaddr_in);
		SOCKET csock = INVALID_SOCKET;//定义一个无效套接字
#ifdef _WIN32
		csock = accept(_sock, (sockaddr*)&clientAddr, &nAddrlen);

#else
		csock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrlen);

#endif
		if (csock == INVALID_SOCKET)
		{
			CELLLog::Info("错误，接收到无效的客户端<socket=%d>....\n", (int)csock);
		}
		else
		{
			//将连接的新客户端分配到消息处理线程
			addClientToCellServer(new CELLClient(csock));
			//CELLLog::Info("<第%d个>新客户端加入<socket=%d> ,IP=%s\n", (int)_clients.size(),(int)csock, inet_ntoa(clientAddr.sin_addr));//inet_ntoa()转换成可见的数字IP
		}
		return csock;
	}
	//将连接的新客户端分配到消息处理线程
	void addClientToCellServer(CELLClient* pClient)
	{
		//查找客户数量最少的消息处理线程（CellServer）
		auto pMinServer = _CellServers[0];
		for (auto pCellServer : _CellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		//在最少任务的线程中加入新客户端
		pMinServer->addClient(pClient);
	}

	//创建服务端线程
	void New_thread(int ThreadCount)
	{
		for (int i = 0; i < ThreadCount; i++)
		{
			//创建一个服务端线程（指针）
			auto ser = new CELLServer(i+1);
			//将服务线程加入到服务线程数组中去
			_CellServers.push_back(ser);
			//注册网络事件接收对象
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}
		_thread.Start(nullptr, [this](CELLThread* pThread) {OnRun(pThread); });
	}
	//关闭socket
	void Close()
	{
		CELLLog::Info("EasyTcpServer.Close begin\n");
		_thread.Close();
		if (_sock != INVALID_SOCKET)
		{
			//释放多线程对象
			for (auto iter : _CellServers)
			{
				delete iter;
			}
			_CellServers.clear();
#ifdef _WIN32

			//8、关闭套接字closesocket()
			closesocket(_sock);
#else
			// 8 关闭套节字closesocket
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
		CELLLog::Info("EasyTcpServer.Close end\n");
	}

	//线程上是安全的，只被主线程调用
	virtual void OnNetJoin(CELLClient* pClient)
	{
		//客户端数量加一
		_clientCount++;
	}
	//线程上不安全的，被4个CellServer调用
	virtual void OnNetLeave(CELLClient* pClient)
	{
		//离开的客户端数量减一
		_clientCount--;
		
	}
	//线程上不安全的，被4个CellServer调用
	virtual void OnNetMsg(CELLServer* pCellServer, CELLClient* pClient, DataHeader* header)
	{
		//原子操作计数,统计收到的包的个数
		_msgCount++;
	}
	virtual void OnNetRecv(CELLClient* pClient)
	{
		_recvCount++;
	}
private:
	//处理网络消息
	void OnRun(CELLThread* pThread)
	{
		while (pThread->isRun())
		{
			time4msg();
			//fd_set是一个套接字的集合，可以把他当成宏来使用 它本质是一个结构体（有两个参数），fd_arry[]表示集合，fd_count表示集合的大小
			fd_set fdRead;      //可读的端口集合(客户端可以使用的端口)

			//FD_ZERO()是对fd_set()的清空作用
			FD_ZERO(&fdRead);

			//FD_SET()将一个给定的端口加入集合之中   （这个端口socket也叫做文件描述符）
			FD_SET(_sock, &fdRead);//将服务端端口号加入可读集合

			//初始化结构体,里面有秒和微秒两个参数,这里设置10毫秒查询一次
			timeval t = { 0,1 };
			//_sock+1表示集合中所有端口号的最大值+1，表示端口范围，在Windows中这个参数可以为0
			int ret = select(_sock + 1, &fdRead, 0, 0, &t);//监视服务器端口的连接状况，当可读集合内的端口有任务要处理时，返回正值
			//返回负值：有异常错误
			//返回正值：可读可写
			//返回负值：等待超时，不可读不可写
			//CELLLog::Info("select ret=%d  count=%d\n", ret, count++);

			if (ret < 0)
			{
				CELLLog::Info("EasyTcpServer.OnRun select error Exit.\n");
				pThread->Exit();
				break;
			}

			//_sock是服务器的端口号，这里判断服务器中是否有数据可读（即是否有 新客户 端加入）
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);//将一个给定的端口（文件描述符）从集合中删除
				Accept();
			}
		}

	}

	//计算并输出每秒收到的数据包个数
	void time4msg()
	{
		//获取当前秒
		auto t1 = _Ttime.getElapsedSecond();
		//统计每秒钟接收多少个包
		if (t1 >= 1.0)
		{
			CELLLog::Info("Thread<%d> ,time<%lf>,socket<%d>,_clients<%d>,recv<%d>,msg<%d>\n", (int)_CellServers.size(), t1, (int)_sock, (int)_clientCount, (int)(_recvCount / t1), (int)(_msgCount / t1));
			_recvCount = 0;
			_msgCount = 0;
			//重新设计一个时间点
			_Ttime.update();
		}
	}

};

#endif

