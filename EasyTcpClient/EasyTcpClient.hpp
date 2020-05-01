#ifndef _EasyTcpClient_hpp_  //�ж�ͷ�ļ��Ƿ񱻶����
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
#include "MessageHeader.hpp"//�������ݽṹ��ͷ�ļ�

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

	//����������
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//��ʼ��socket
	void InitSocket()
	{
#ifdef _WIN32
		//����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>�رվ�����...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("���󣬽���Socketʧ��...\n");
		}
		else {
//			printf("����Socket�ɹ�...\n");
		}
	}

	//���ӷ�����
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		// 2 ���ӷ����� connect
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
//		printf("<socket=%d>�������ӷ�����<%s:%d>...\n", (int)_sock, ip, port);
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>�������ӷ�����<%s:%d>ʧ��...\n",(int) _sock, ip, port);
		}
		else {
			_isConnect = true;
//			printf("<socket=%d>���ӷ�����<%s:%d>�ɹ�...\n", (int)_sock, ip, port);
		}
		return ret;
	}

	//�ر��׽���closesocket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);
			//���Windows socket����
			WSACleanup();
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
		_isConnect = false;
	}

	//�ж�����˿�״̬���Ƿ��й���Ҫ����
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
				printf("<socket=%d>select�������1\n", (int)_sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);

				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select�������2\n", (int)_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET&& _isConnect;
	}

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif
	//���ջ�����
	//char _1Recv[RECV_BUFF_SIZE] = {};
	//�ڶ������� ��Ϣ������
	char _2Recv[RECV_BUFF_SIZE] = {};
	//��Ϣ������������β��λ��
	int _lastpos = 0;
	//�������� ����ճ�� ��ְ�
	int RecvData(SOCKET csock)// 5 ���տͻ�������
	{
		char* szRecv = _2Recv + _lastpos;
		int len = (int)recv(csock, szRecv, (RECV_BUFF_SIZE) - _lastpos, 0);
		//ȡ����һ���ջ�������ֱ�Ӱ����ݷŵ���Ϣ��������
		//int len = (int)recv(_csock, _1Recv, RECV_BUFF_SIZE, 0);
		//printf("len=%d\n", len);
		if (len <= 0)
		{
			printf("<socket=%d>��������Ͽ����ӣ����������\n", (int)_sock);
			//Close();
			return -1;
		}
		//����һ����������Ϣ�������ڶ�����������_1RECV�����ǰlen���ַ�ȥ�滻_2RECV��ǰlen���ַ���
		//_2RECV+_lastpos��Ϊ�˱����ϴ�δ����������ݣ���ν��Ŵ���
		//memcpy(_2Recv + _lastpos, _1Recv, len);

		//�ڶ�������������β��λ�ú���
		_lastpos += len;
		//�жϵڶ������������ݳ����Ƿ������Ϣͷ����DataHeader����֤���ݵ�������
		while (_lastpos >= sizeof(DataHeader))//while���ճ������
		{
			//�ӵڶ�������ȡ��һ����ͷ�ĳ���
			DataHeader* header = (DataHeader*)_2Recv;//������ǿ��ת���ɽṹ��
			//�ж�ȡ���������Ƿ�����(����ٰ�����)
			if (_lastpos >= header->DataLength)
			{
				//Ԥ�ȱ���ȡ��һ����ͷ���ݺ�ڶ�������ʣ�����ݵĳ���
				int _size = _lastpos - header->DataLength;
				//����ȡ����ͷ����Ϣ
				OnNetMsg(header);
				//���ڶ�������������ǰ��
				memcpy(_2Recv, _2Recv + header->DataLength, _size);
				//���ݽ�βλ��ǰ��
				_lastpos = _size;

			}
			else
			{
				//printf("������ʣ����Ϣ����һ��������Ϣ\n");
				break;
			}
		}
		return 0;
	}

	//��Ӧ������Ϣ(��������)
	void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
			{

				Login_result* login = (Login_result*)header;
				//printf("<socket=%d>�յ��������Ϣ��CMD_LOGIN_RESULT,���ݳ��ȣ�%d\n",(int)_sock, login->DataLength);
			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				Logout_result* logout = (Logout_result*)header;
				//printf("<socket=%d>�յ��������Ϣ��CMD_LOGOUT_RESULT,���ݳ��ȣ�%d\n", (int)_sock, logout->DataLength);
			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				NewUserJoin* userJoin = (NewUserJoin*)header;
				//printf("<socket=%d>�յ��������Ϣ��CMD_NEW_USER_JOIN,���ݳ��ȣ�%d\n", (int)_sock, userJoin->DataLength);
			}
			break;
			case CMD_ERROR:
			{
				printf("<socket=%d>�յ��������Ϣ��CMD_ERROR,���ݳ��ȣ�%d\n", (int)_sock, header->DataLength);
			}
			break;
			default:
			{
				printf("<socket=%d>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", (int)_sock, header->DataLength);
			}
		}
	}

	//��������
	int SendData(DataHeader* header, int nLen)
	{
		int ret = SOCKET_ERROR;
		if (isRun() && header)
		{
			ret = send(_sock, (const char*)header, nLen, 0);
			//����������ݳ��ִ���
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

