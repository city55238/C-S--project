#ifndef _CELL_HPP_
#define _CELL_HPP_

#ifdef _WIN32
#define FD_SETSIZE   2506
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<signal.h>

#define SOCKET int
#define INVALID_SOCKET   (SOCKET)(~0)
#define SOCKET_ERROR			(-1)
#endif

#include<stdio.h>

#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"
#include"CELLTask.hpp"
#include"CELLLog.hpp"


//接收缓存区大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#define SEND_BUFF_SIZE 10240
#endif

#endif // !_CELL_HPP_

