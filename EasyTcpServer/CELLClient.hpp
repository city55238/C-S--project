#ifndef _CELLClient_HPP_
#define _CELLClient_HPP_

#include "CELL.hpp"
#include "CELLBuffer.hpp"
//客户端心跳死亡事件 
#define CLIENT_TIME 600000
//发送缓存区最大等待时间
#define SEND_BUFF_TIME 200

//封装客户端的类
class CELLClient
{
public:
	//客户端id
	int id = -1;
	//客户端所属线程id
	int ClientServer_id = -1;
public:
	//把每个客户端封装成对象,给缓存区_SendBuff和_RecvBuff赋初值
	CELLClient(SOCKET sockfd = INVALID_SOCKET):_SendBuff(SEND_BUFF_SIZE),_RecvBuff(RECV_BUFF_SIZE)
	{
		static int n = 1;
		id = n++;
		_sockfd = sockfd;
	
		resetDTHeart();
		resetDTSend();
	}
	~CELLClient()
	{
		CELLLog::Info("servr=%d CELLClient =%d.CELLClient\n", ClientServer_id, id);
		if (_sockfd != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}
	}
	SOCKET sockfd()
	{
		return _sockfd;
	}
	//客户端接收数据
	int RecvData()
	{
		return _RecvBuff.Read4Socket(_sockfd);
	}
	//判断接收缓存区是否有可以接收的消息
	bool HasMsg()
	{
		return _RecvBuff.HasMsg();
	}
	//获取可接收消息的头指针
	DataHeader* GetHeader()
	{
		return (DataHeader*)_RecvBuff.GetHeader();
	}
	//将该消息从缓存区中移除
	void pop_Header()
	{
		//缓存区中有消息时才能移除
		if (HasMsg())
			_RecvBuff.pop(GetHeader()->DataLength);
	}
	//到达时间立即发送数据
	int SendDataReal()
	{
		resetDTSend();
		return _SendBuff.Write2Socket(_sockfd);
	}
	//定量发送数据（每次只发送一个缓存区大小的数据）
	int SendData(DataHeader* header)
	{
		if (_SendBuff.push((const char*)header, header->DataLength))
		{
			return header->DataLength;
		}
		return SOCKET_ERROR;
	}

	//重置心跳死亡时间
	void resetDTHeart()
	{
		_dtHeart = 0;
	}
	//重置上次发送消息的时间
	void resetDTSend()
	{
		_dtSend = 0;
	}
	//心跳检测
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart >= CLIENT_TIME)
		{
			CELLLog::Info("checkHeart dead:s=%lld,time=%lld\n", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	//定时发送消息检测
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= SEND_BUFF_TIME)
		{
			//输出超时提示
			//CELLLog::Info("checkSend dead:s=%d,time=%d\n", _sockfd, _dtSend);
			//到达规定时间立即将缓存区的数据发送出去
			SendDataReal();
			//重置计时
			resetDTSend();
			return true;
		}
		return false;
	}
private:
	SOCKET _sockfd;
	//第二缓冲区 接收缓冲区
	CELLBuffer _RecvBuff;
	//消息缓冲区的数据尾部位置
	int _LastRecvPos;
	//发送缓存区
	CELLBuffer _SendBuff;
	//心跳死亡计时
	time_t _dtHeart;
	//上次发送消息的时间
	time_t _dtSend;
	//发送缓存区写满的次数
	int _sendBuffCount = 0;
};


#endif // !_CELLClient_HPP_


