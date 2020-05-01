#ifndef _EasyTcpClient_hpp_  //判断头文件是否被定义过
#define _EasyTcpClient_hpp_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
#include <stdio.h>
#include "MessageHeader.hpp"//定义数据结构的头文件

class EasyTcpClient
{
	SOCKET _sock;
	bool _isConnect;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
		_isConnect = false;
	}

	//虚析构函数
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//初始化socket
	void InitSocket()
	{
#ifdef _WIN32
		//启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>关闭旧连接...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("错误，建立Socket失败...\n");
		}
		else {
//			printf("建立Socket成功...\n");
		}
	}

	//连接服务器
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		// 2 连接服务器 connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
//		printf("<socket=%d>正在连接服务器<%s:%d>...\n", (int)_sock, ip, port);
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>错误，连接服务器<%s:%d>失败...\n",(int) _sock, ip, port);
		}
		else {
			_isConnect = true;
//			printf("<socket=%d>连接服务器<%s:%d>成功...\n", (int)_sock, ip, port);
		}
		return ret;
	}

	//关闭套节字closesocket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);
			//清除Windows socket环境
			WSACleanup();
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
		_isConnect = false;
	}

	//判断网络端口状态（是否有工作要处理）
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0,0 };
			int ret = select(_sock + 1, &fdReads, 0, 0, &t);

			if (ret < 0)
			{
				printf("<socket=%d>select任务结束1\n", (int)_sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);

				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select任务结束2\n", (int)_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET&& _isConnect;
	}

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif
	//接收缓冲区
	//char _1Recv[RECV_BUFF_SIZE] = {};
	//第二缓冲区 消息缓冲区
	char _2Recv[RECV_BUFF_SIZE] = {};
	//消息缓冲区的数据尾部位置
	int _lastpos = 0;
	//接收数据 处理粘包 拆分包
	int RecvData(SOCKET csock)// 5 接收客户端数据
	{
		char* szRecv = _2Recv + _lastpos;
		int len = (int)recv(csock, szRecv, (RECV_BUFF_SIZE) - _lastpos, 0);
		//取消第一接收缓存区，直接把数据放到消息缓存区中
		//int len = (int)recv(_csock, _1Recv, RECV_BUFF_SIZE, 0);
		//printf("len=%d\n", len);
		if (len <= 0)
		{
			printf("<socket=%d>与服务器断开连接，任务结束。\n", (int)_sock);
			//Close();
			return -1;
		}
		//将第一缓存区的消息拷贝到第二缓存区（用_1RECV数组的前len个字符去替换_2RECV的前len个字符）
		//_2RECV+_lastpos是为了保留上次未处理完的数据，这次接着处理
		//memcpy(_2Recv + _lastpos, _1Recv, len);

		//第二缓存区的数据尾部位置后移
		_lastpos += len;
		//判断第二缓存区的数据长度是否等于消息头长度DataHeader（保证数据的完整）
		while (_lastpos >= sizeof(DataHeader))//while解决粘包问题
		{
			//从第二缓存区取出一个报头的长度
			DataHeader* header = (DataHeader*)_2Recv;//把数组强制转换成结构体
			//判断取出的数据是否完整(解决少包问题)
			if (_lastpos >= header->DataLength)
			{
				//预先保留取出一个报头数据后第二缓存区剩余数据的长度
				int _size = _lastpos - header->DataLength;
				//处理取出报头的信息
				OnNetMsg(header);
				//将第二缓存区的数据前移
				memcpy(_2Recv, _2Recv + header->DataLength, _size);
				//数据结尾位置前移
				_lastpos = _size;

			}
			else
			{
				//printf("缓冲区剩余消息不够一条完整消息\n");
				break;
			}
		}
		return 0;
	}

	//响应网络消息(数据请求)
	void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
			{

				Login_result* login = (Login_result*)header;
				//printf("<socket=%d>收到服务端消息：CMD_LOGIN_RESULT,数据长度：%d\n",(int)_sock, login->DataLength);
			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				Logout_result* logout = (Logout_result*)header;
				//printf("<socket=%d>收到服务端消息：CMD_LOGOUT_RESULT,数据长度：%d\n", (int)_sock, logout->DataLength);
			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				NewUserJoin* userJoin = (NewUserJoin*)header;
				//printf("<socket=%d>收到服务端消息：CMD_NEW_USER_JOIN,数据长度：%d\n", (int)_sock, userJoin->DataLength);
			}
			break;
			case CMD_ERROR:
			{
				printf("<socket=%d>收到服务端消息：CMD_ERROR,数据长度：%d\n", (int)_sock, header->DataLength);
			}
			break;
			default:
			{
				printf("<socket=%d>收到未定义消息,数据长度：%d\n", (int)_sock, header->DataLength);
			}
		}
	}

	//发送数据
	int SendData(DataHeader* header, int nLen)
	{
		int ret = SOCKET_ERROR;
		if (isRun() && header)
		{
			ret = send(_sock, (const char*)header, nLen, 0);
			//如果发送数据出现错误
			if (ret == SOCKET_ERROR)
			{
				Close();
			}
		}
		return ret;
	}
private:

};

#endif

